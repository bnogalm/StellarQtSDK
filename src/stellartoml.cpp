#include "stellartoml.h"

#include <QDate>
#include <QRegularExpression>
#include <QStringList>
#include <QVariantList>
#include <QtGlobal>

#include <cmath>
#include <map>
#include <memory>
#include <stdexcept>
#include <vector>

QSTELLAR_BEGIN_NS

/*
 * A TOML 1.0 reader. The structure follows the reference reader in Python's
 * standard library (tomllib) closely, so the two accept and refuse the same
 * documents; see the header for the deliberate differences.
 *
 * The document is read once, left to right, by a cursor (`pos`). Values are
 * parsed recursively, but the recursion is bounded by MaxDepth, so a hostile
 * file cannot exhaust the stack.
 *
 * Tables are first built as a tree of Node objects, which remember HOW a table
 * came to exist — TOML forbids declaring a table twice, adding to an inline
 * table, re-opening with a [header] a table made by dotted keys, and so on —
 * and the tree is turned into QVariantMap / QVariantList at the end.
 */

namespace {

constexpr int kMaxDepth = StellarToml::MaxDepth;

struct Node
{
    enum Kind { Table, TableArray, Value };

    explicit Node(Kind k) : kind(k) {}

    Kind kind;
    QVariant value;                                      // kind == Value
    std::map<QString, std::unique_ptr<Node>> children;   // kind == Table
    std::vector<std::unique_ptr<Node>> elements;         // kind == TableArray

    // Closed to a later [header] and to dotted keys from another section:
    // set by its own [header], or (at the next header) when it was made by
    // dotted keys. A table that merely exists because a deeper header named
    // it (`[a.b]` makes `a`) stays open, as TOML allows.
    bool closed = false;

    QVariant toVariant() const
    {
        switch (kind) {
        case Value:
            return value;
        case TableArray: {
            QVariantList list;
            for (const auto& e : elements) list.append(e->toVariant());
            return list;
        }
        case Table:
            break;
        }
        QVariantMap map;
        for (const auto& c : children) map.insert(c.first, c.second->toVariant());
        return map;
    }
};

bool isBareKeyChar(int c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')
           || c == '_' || c == '-';
}

// Control characters no TOML string or comment may contain (tab is allowed
// everywhere; newline only where the caller says so).
bool isControl(int c)
{
    return (c >= 0 && c < 0x20 && c != '\t') || c == 0x7f;
}

bool isHex(int c)
{
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

QString quoteKey(const QStringList& key)
{
    QStringList parts;
    for (const QString& k : key) {
        bool bare = !k.isEmpty();
        for (const QChar c : k) bare = bare && isBareKeyChar(c.unicode());
        parts.append(bare ? k : QLatin1Char('"') + k + QLatin1Char('"'));
    }
    return parts.join(QLatin1Char('.'));
}

// `validUtf16`: `s` is known to be valid UTF-16, so PCRE2 is not asked to
// check it again. Asked, it checks it from `pos` to the END on every match:
// a document of n values cost O(n^2), an estimated few seconds for a 100 KB
// array of ints.
QRegularExpressionMatch matchAt(const QRegularExpression& re, const QString& s, int pos, bool validUtf16)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QRegularExpression::MatchOptions options = QRegularExpression::AnchorAtOffsetMatchOption;
#else
    QRegularExpression::MatchOptions options = QRegularExpression::AnchoredMatchOption;
#endif
    if (validUtf16)
        options |= QRegularExpression::DontCheckSubjectStringMatchOption;
    return re.match(s, pos, QRegularExpression::NormalMatch, options);
}

// The same grammar tomllib uses (TOML 1.0: seconds are mandatory).
const QString kTime = QStringLiteral(
    "([01][0-9]|2[0-3]):([0-5][0-9]):([0-5][0-9])(?:\\.([0-9]+))?");

const QRegularExpression& dateTimeRe()
{
    static const QRegularExpression re(
        QStringLiteral("([0-9]{4})-(0[1-9]|1[0-2])-(0[1-9]|[12][0-9]|3[01])"
                       "(?:[Tt ]") + kTime
        + QStringLiteral("(?:[Zz]|[+-](?:[01][0-9]|2[0-3]):[0-5][0-9])?)?"));
    return re;
}

const QRegularExpression& localTimeRe()
{
    static const QRegularExpression re(kTime);
    return re;
}

const QRegularExpression& numberRe()
{
    static const QRegularExpression re(QStringLiteral(
        "0(?:x[0-9A-Fa-f](?:_?[0-9A-Fa-f])*|b[01](?:_?[01])*|o[0-7](?:_?[0-7])*)"
        "|[+-]?(?:0|[1-9](?:_?[0-9])*)"
        "(?<float>(?:\\.[0-9](?:_?[0-9])*)?(?:[eE][+-]?[0-9](?:_?[0-9])*)?)"));
    return re;
}

class Parser
{
public:
    explicit Parser(const QString& text) : src(text)
    {
        // TOML treats CRLF as a newline; a bare CR anywhere is an error, and
        // is caught as a control character / unexpected character below.
        src.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
        // Checked once, for every match (see matchAt). A QString from
        // fromUtf8 always is; one handed in with a lone surrogate is still
        // read, with PCRE2 checking it at each match as before.
        validUtf16 = src.isValidUtf16();
    }

    QVariantMap run()
    {
        Node root(Node::Table);
        Node* current = &root;
        int headerDepth = 0;

        for (;;) {
            skipWs();
            if (atEnd()) break;
            const int c = ch();
            if (c == '\n') { ++pos; continue; }

            if (isBareKeyChar(c) || c == '"' || c == '\'') {
                keyValue(current, headerDepth);
            } else if (c == '[') {
                // Tables made by dotted keys in the section that just ended
                // are now closed to headers and to further dotted keys.
                for (Node* n : pendingClose) n->closed = true;
                pendingClose.clear();
                if (ch(1) == '[') current = tableArrayHeader(root, headerDepth);
                else              current = tableHeader(root, headerDepth);
            } else if (c != '#') {
                error(QStringLiteral("invalid statement (expected a key, a [table] or a comment)"));
            }
            skipWs();
            skipComment();
            if (atEnd()) break;
            if (ch() != '\n')
                error(QStringLiteral("unexpected '%1' after a statement (expected the end of the line)")
                          .arg(describe(ch())));
            ++pos;
        }
        return root.toVariant().toMap();
    }

private:
    QString src;
    int pos = 0;
    std::vector<Node*> pendingClose;
    bool validUtf16 = false;

    // ── cursor helpers ──────────────────────────────────────────────

    bool atEnd() const { return pos >= src.size(); }

    int ch(int offset = 0) const
    {
        const int p = pos + offset;
        return p < src.size() ? src.at(p).unicode() : -1;
    }

    bool lookingAt(const char* s) const
    {
        for (int i = 0; s[i]; ++i)
            if (ch(i) != static_cast<unsigned char>(s[i])) return false;
        return true;
    }

    static QString describe(int c)
    {
        if (c < 0) return QStringLiteral("end of file");
        if (c == '\n') return QStringLiteral("\\n");
        if (c < 0x20 || c == 0x7f) return QStringLiteral("\\x%1").arg(c, 2, 16, QLatin1Char('0'));
        return QString(QChar(c));
    }

    [[noreturn]] void error(const QString& what) const
    {
        const int line = int(src.left(pos).count(QLatin1Char('\n'))) + 1;
        throw std::runtime_error(
            QStringLiteral("stellar.toml: line %1: %2").arg(line).arg(what).toStdString());
    }

    void skipWs()
    {
        while (ch() == ' ' || ch() == '\t') ++pos;
    }

    void skipComment()
    {
        if (ch() != '#') return;
        ++pos;
        while (!atEnd() && ch() != '\n') {
            if (isControl(ch()))
                error(QStringLiteral("control character %1 in a comment").arg(describe(ch())));
            ++pos;
        }
    }

    // Inside an array, newlines and comments count as whitespace.
    void skipArraySpace()
    {
        for (;;) {
            while (ch() == ' ' || ch() == '\t' || ch() == '\n') ++pos;
            if (ch() != '#') return;
            skipComment();
        }
    }

    void expect(const char* token, const QString& what)
    {
        if (!lookingAt(token)) error(what);
        pos += int(qstrlen(token));
    }

    // ── keys ────────────────────────────────────────────────────────

    QString keyPart()
    {
        const int c = ch();
        if (c == '"') { ++pos; return basicString(false); }
        if (c == '\'') { ++pos; return literalString(false); }
        const int start = pos;
        while (isBareKeyChar(ch())) ++pos;
        if (pos == start)
            error(QStringLiteral("invalid character '%1' at the start of a key").arg(describe(c)));
        return src.mid(start, pos - start);
    }

    QStringList key(int depthSoFar)
    {
        QStringList parts;
        parts.append(keyPart());
        skipWs();
        while (ch() == '.') {
            ++pos;
            skipWs();
            parts.append(keyPart());
            skipWs();
        }
        if (depthSoFar + parts.size() > kMaxDepth)
            error(QStringLiteral("nesting deeper than %1 levels").arg(kMaxDepth));
        return parts;
    }

    // ── tables ──────────────────────────────────────────────────────

    // Walks (creating plain tables as needed) from `from` along key[0, end),
    // entering the LAST table of an array of tables, as headers do.
    Node* walk(Node* from, const QStringList& k, int end)
    {
        Node* cur = from;
        for (int i = 0; i < end; ++i) {
            std::unique_ptr<Node>& slot = cur->children[k.at(i)];
            if (!slot) slot.reset(new Node(Node::Table));
            cur = slot.get();
            if (cur->kind == Node::TableArray) cur = cur->elements.back().get();
            else if (cur->kind == Node::Value)
                error(QStringLiteral("'%1' is already a value, not a table")
                          .arg(quoteKey(k.mid(0, i + 1))));
        }
        return cur;
    }

    Node* tableHeader(Node& root, int& headerDepth)
    {
        ++pos;  // '['
        skipWs();
        const QStringList k = key(0);
        expect("]", QStringLiteral("expected ']' to close the table header"));
        Node* parent = walk(&root, k, k.size() - 1);
        std::unique_ptr<Node>& slot = parent->children[k.last()];
        if (!slot) {
            slot.reset(new Node(Node::Table));
        } else if (slot->kind == Node::Value) {
            error(QStringLiteral("[%1] redefines a value as a table").arg(quoteKey(k)));
        } else if (slot->kind == Node::TableArray) {
            error(QStringLiteral("[%1] is already an array of tables").arg(quoteKey(k)));
        } else if (slot->closed) {
            error(QStringLiteral("table [%1] is defined more than once").arg(quoteKey(k)));
        }
        slot->closed = true;
        headerDepth = k.size();
        return slot.get();
    }

    Node* tableArrayHeader(Node& root, int& headerDepth)
    {
        pos += 2;  // '[['
        skipWs();
        const QStringList k = key(0);
        expect("]]", QStringLiteral("expected ']]' to close the array-of-tables header"));
        Node* parent = walk(&root, k, k.size() - 1);
        std::unique_ptr<Node>& slot = parent->children[k.last()];
        if (!slot) slot.reset(new Node(Node::TableArray));
        else if (slot->kind != Node::TableArray)
            error(QStringLiteral("[[%1]] names something that is not an array of tables")
                      .arg(quoteKey(k)));
        slot->elements.emplace_back(new Node(Node::Table));
        headerDepth = k.size();
        return slot->elements.back().get();
    }

    // `a.b.c = value` into `table`. `inSection` is false inside an inline
    // table, where there are no later sections to close tables against.
    void assign(Node* table, const QStringList& k, const QVariant& value, bool inSection)
    {
        Node* cur = table;
        for (int i = 0; i + 1 < k.size(); ++i) {
            std::unique_ptr<Node>& slot = cur->children[k.at(i)];
            if (!slot) {
                slot.reset(new Node(Node::Table));
            } else if (slot->kind != Node::Table) {
                error(QStringLiteral("dotted key '%1': '%2' is not an open table")
                          .arg(quoteKey(k), quoteKey(k.mid(0, i + 1))));
            } else if (slot->closed) {
                error(QStringLiteral("dotted key '%1' adds to table '%2', which is defined elsewhere")
                          .arg(quoteKey(k), quoteKey(k.mid(0, i + 1))));
            }
            if (inSection) pendingClose.push_back(slot.get());
            cur = slot.get();
        }
        std::unique_ptr<Node>& slot = cur->children[k.last()];
        if (slot) error(QStringLiteral("duplicate key '%1'").arg(quoteKey(k)));
        slot.reset(new Node(Node::Value));
        slot->value = value;
    }

    void keyValue(Node* table, int headerDepth)
    {
        const QStringList k = key(headerDepth);
        expect("=", QStringLiteral("expected '=' after the key '%1'").arg(quoteKey(k)));
        skipWs();
        const QVariant v = value(headerDepth + k.size());
        assign(table, k, v, true);
    }

    // ── values ──────────────────────────────────────────────────────

    // `depth` is how deeply the value being read is nested.
    QVariant value(int depth)
    {
        if (depth > kMaxDepth)
            error(QStringLiteral("nesting deeper than %1 levels").arg(kMaxDepth));

        const int c = ch();
        if (c == '"') {
            if (lookingAt("\"\"\"")) return multilineString(false);
            ++pos;
            return basicString(false);
        }
        if (c == '\'') {
            if (lookingAt("'''")) return multilineString(true);
            ++pos;
            return literalString(false);
        }
        if (lookingAt("true"))  { pos += 4; return true; }
        if (lookingAt("false")) { pos += 5; return false; }
        if (c == '[') return array(depth);
        if (c == '{') return inlineTable(depth);

        QRegularExpressionMatch m = matchAt(dateTimeRe(), src, pos, validUtf16);
        if (m.hasMatch()) {
            if (!QDate::isValid(m.captured(1).toInt(), m.captured(2).toInt(), m.captured(3).toInt()))
                error(QStringLiteral("invalid date '%1'").arg(m.captured(0)));
            pos = m.capturedEnd(0);
            return m.captured(0);  // the original text, see the header
        }
        m = matchAt(localTimeRe(), src, pos, validUtf16);
        if (m.hasMatch()) {
            pos = m.capturedEnd(0);
            return m.captured(0);
        }
        m = matchAt(numberRe(), src, pos, validUtf16);
        if (m.hasMatch()) {
            pos = m.capturedEnd(0);
            return number(m.captured(0), !m.captured(QStringLiteral("float")).isEmpty());
        }
        static const struct { const char* text; double value; } specials[] = {
            { "inf", HUGE_VAL }, { "+inf", HUGE_VAL }, { "-inf", -HUGE_VAL },
            { "nan", std::nan("") }, { "+nan", std::nan("") }, { "-nan", std::nan("") },
        };
        for (const auto& special : specials) {
            if (lookingAt(special.text)) {
                pos += int(qstrlen(special.text));
                return special.value;
            }
        }
        error(QStringLiteral("invalid value starting with '%1'").arg(describe(c)));
    }

    QVariant number(QString text, bool isFloat)
    {
        text.remove(QLatin1Char('_'));
        bool ok = false;
        if (isFloat) {
            const double d = text.toDouble(&ok);
            // Qt reports an overflow (to inf) or underflow (to 0) as !ok, but
            // TOML, like IEEE 754, rounds such a literal to inf or 0.
            if (!ok && (std::isinf(d) || d == 0.0)) ok = true;
            if (!ok) error(QStringLiteral("invalid float '%1'").arg(text));
            return d;
        }
        qlonglong n = 0;
        if (text.startsWith(QLatin1String("0x")))      n = text.mid(2).toLongLong(&ok, 16);
        else if (text.startsWith(QLatin1String("0o"))) n = text.mid(2).toLongLong(&ok, 8);
        else if (text.startsWith(QLatin1String("0b"))) n = text.mid(2).toLongLong(&ok, 2);
        else n = (text.startsWith(QLatin1Char('+')) ? text.mid(1) : text).toLongLong(&ok, 10);
        if (!ok)
            error(QStringLiteral("integer %1 is outside the signed 64-bit range").arg(text));
        return n;
    }

    QVariant array(int depth)
    {
        ++pos;  // '['
        QVariantList out;
        skipArraySpace();
        if (ch() == ']') { ++pos; return out; }
        for (;;) {
            out.append(value(depth + 1));
            skipArraySpace();
            if (ch() == ']') { ++pos; return out; }
            if (ch() != ',')
                error(QStringLiteral("unclosed array (expected ',' or ']', found '%1')")
                          .arg(describe(ch())));
            ++pos;
            skipArraySpace();
            if (ch() == ']') { ++pos; return out; }
        }
    }

    QVariant inlineTable(int depth)
    {
        ++pos;  // '{'
        Node table(Node::Table);
        skipWs();
        if (ch() == '}') { ++pos; return QVariantMap(); }
        for (;;) {
            const QStringList k = key(depth);
            expect("=", QStringLiteral("expected '=' after the key '%1'").arg(quoteKey(k)));
            skipWs();
            const QVariant v = value(depth + k.size());
            assign(&table, k, v, false);
            skipWs();
            if (ch() == '}') { ++pos; return table.toVariant(); }
            if (ch() != ',')
                error(QStringLiteral("unclosed inline table (expected ',' or '}', found '%1')")
                          .arg(describe(ch())));
            ++pos;
            skipWs();
        }
    }

    // ── strings ─────────────────────────────────────────────────────

    // After the opening quote. Reads up to and including the closing quote
    // (for a multi-line string: up to and including the first `"""`).
    QString basicString(bool multiline)
    {
        QString out;
        for (;;) {
            if (atEnd()) error(QStringLiteral("unterminated string"));
            const int c = ch();
            if (c == '"') {
                if (!multiline) { ++pos; return out; }
                if (lookingAt("\"\"\"")) { pos += 3; return out; }
                out.append(QLatin1Char('"'));
                ++pos;
            } else if (c == '\\') {
                escape(out, multiline);
            } else if (c == '\n' && !multiline) {
                error(QStringLiteral("unterminated string (a basic string cannot span lines)"));
            } else if (isControl(c) && c != '\n') {
                error(QStringLiteral("control character %1 in a string").arg(describe(c)));
            } else {
                out.append(QChar(c));
                ++pos;
            }
        }
    }

    void escape(QString& out, bool multiline)
    {
        const int e = ch(1);
        pos += 2;
        if (multiline && (e == ' ' || e == '\t' || e == '\n')) {
            // A line-ending backslash: drop the newline and all whitespace
            // (including further newlines) up to the next visible character.
            if (e != '\n') {
                skipWs();
                if (atEnd()) return;
                if (ch() != '\n')
                    error(QStringLiteral("a '\\' followed by spaces must end the line"));
                ++pos;
            }
            while (ch() == ' ' || ch() == '\t' || ch() == '\n') ++pos;
            return;
        }
        switch (e) {
        case 'b':  out.append(QChar(0x08)); return;
        case 't':  out.append(QChar(0x09)); return;
        case 'n':  out.append(QChar(0x0a)); return;
        case 'f':  out.append(QChar(0x0c)); return;
        case 'r':  out.append(QChar(0x0d)); return;
        case '"':  out.append(QLatin1Char('"'));  return;
        case '\\': out.append(QLatin1Char('\\')); return;
        case 'u':  unicodeEscape(out, 4); return;
        case 'U':  unicodeEscape(out, 8); return;
        default:
            pos -= 2;
            error(QStringLiteral("unknown escape sequence '\\%1' in a string").arg(describe(e)));
        }
    }

    void unicodeEscape(QString& out, int digits)
    {
        for (int i = 0; i < digits; ++i) {
            if (!isHex(ch(i)))
                error(QStringLiteral("a \\%1 escape needs %2 hex digits")
                          .arg(digits == 4 ? QLatin1Char('u') : QLatin1Char('U')).arg(digits));
        }
        const uint cp = src.mid(pos, digits).toUInt(nullptr, 16);
        if (cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF))
            error(QStringLiteral("escape \\%1 is not a Unicode scalar value")
                      .arg(src.mid(pos - 1, digits + 1)));
        pos += digits;
        if (cp > 0xFFFF) {
            out.append(QChar(QChar::highSurrogate(cp)));
            out.append(QChar(QChar::lowSurrogate(cp)));
        } else {
            out.append(QChar(ushort(cp)));
        }
    }

    // After the opening apostrophe. Reads up to and including the closing one.
    QString literalString(bool multiline)
    {
        const int start = pos;
        for (;;) {
            if (atEnd()) error(QStringLiteral("unterminated literal string"));
            const int c = ch();
            if (multiline ? lookingAt("'''") : c == '\'') {
                const QString out = src.mid(start, pos - start);
                pos += multiline ? 3 : 1;
                return out;
            }
            if (c == '\n' && !multiline)
                error(QStringLiteral("unterminated literal string (it cannot span lines)"));
            if (isControl(c) && c != '\n')
                error(QStringLiteral("control character %1 in a string").arg(describe(c)));
            ++pos;
        }
    }

    // At `"""` or `'''`.
    QString multilineString(bool literal)
    {
        pos += 3;
        if (ch() == '\n') ++pos;  // a newline right after the opening is trimmed
        QString out = literal ? literalString(true) : basicString(true);
        // `""""` / `"""""` at the end: one or two quotes belong to the value.
        const int delim = literal ? '\'' : '"';
        for (int extra = 0; extra < 2 && ch() == delim; ++extra) {
            out.append(QChar(delim));
            ++pos;
        }
        return out;
    }
};

} // namespace

QVariantMap StellarToml::parse(const QString& toml)
{
    return Parser(toml).run();
}

QVariantMap StellarToml::currency(const QVariantMap& toml, const QString& code, const QString& issuer)
{
    const auto matchesTemplate = [](const QString& pattern, const QString& value) {
        if (pattern.size() != value.size())
            return false;
        for (int i = 0; i < pattern.size(); ++i) {
            if (pattern.at(i) != QLatin1Char('?') && pattern.at(i) != value.at(i))
                return false;
        }
        return true;
    };
    for (const QVariant& entry : toml.value(QStringLiteral("CURRENCIES")).toList()) {
        const QVariantMap currency = entry.toMap();
        if (currency.value(QStringLiteral("issuer")).toString() != issuer || issuer.isEmpty())
            continue;
        const QVariant listedCode = currency.value(QStringLiteral("code"));
        const QVariant listedTemplate = currency.value(QStringLiteral("code_template"));
        if ((listedCode.userType() == QMetaType::QString && listedCode.toString() == code)
            || (listedTemplate.userType() == QMetaType::QString && matchesTemplate(listedTemplate.toString(), code))) {
            return currency;
        }
    }
    return QVariantMap();
}

QSTELLAR_END_NS
