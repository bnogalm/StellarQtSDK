#include "stellartoml.h"

#include <QStringList>
#include <QVariantList>
#include <stdexcept>

QSTELLAR_BEGIN_NS

namespace {

// Reads a single-line basic string starting at s[0]=='"'. Returns the
// unescaped content (ignores anything after the closing quote).
QString unquoteBasic(const QString& s)
{
    QString out;
    int i = 1;  // skip opening quote
    while (i < s.size()) {
        const QChar c = s.at(i);
        if (c == '\\' && i + 1 < s.size()) {
            const QChar n = s.at(i + 1);
            switch (n.unicode()) {
            case '"':  out.append('"');  break;
            case '\\': out.append('\\'); break;
            case 'n':  out.append('\n'); break;
            case 't':  out.append('\t'); break;
            case 'r':  out.append('\r'); break;
            case '/':  out.append('/');  break;
            default:   out.append(n);    break;
            }
            i += 2;
        } else if (c == '"') {
            return out;
        } else {
            out.append(c);
            i++;
        }
    }
    throw std::runtime_error("stellar.toml: unterminated string");
}

QVariant parseScalar(QString s)
{
    s = s.trimmed();
    if (s.isEmpty()) throw std::runtime_error("stellar.toml: empty value");
    if (s.startsWith('"')) return unquoteBasic(s);

    // Bare value: strip a trailing comment.
    const int hash = s.indexOf('#');
    if (hash >= 0) s = s.left(hash).trimmed();

    if (s == QStringLiteral("true"))  return true;
    if (s == QStringLiteral("false")) return false;

    bool ok = false;
    const qlonglong asInt = s.toLongLong(&ok);
    if (ok) return asInt;
    const double asDouble = s.toDouble(&ok);
    if (ok) return asDouble;
    return s;  // lenient: treat anything else as a string
}

// Split an array body (the text between [ and ]) on top-level commas,
// respecting quotes.
QStringList splitArrayElements(const QString& inner)
{
    QStringList parts;
    QString cur;
    bool inStr = false;
    int depth = 0;
    for (int i = 0; i < inner.size(); ++i) {
        const QChar c = inner.at(i);
        if (inStr) {
            cur.append(c);
            if (c == '"' && (i == 0 || inner.at(i - 1) != '\\')) inStr = false;
        } else if (c == '"') {
            inStr = true; cur.append(c);
        } else if (c == '[') { depth++; cur.append(c); }
        else if (c == ']')   { depth--; cur.append(c); }
        else if (c == ',' && depth == 0) {
            parts.append(cur); cur.clear();
        } else {
            cur.append(c);
        }
    }
    if (!cur.trimmed().isEmpty()) parts.append(cur);
    return parts;
}

QVariant parseArray(const QString& acc)
{
    const int lb = acc.indexOf('[');
    const int rb = acc.lastIndexOf(']');
    if (lb < 0 || rb < lb) throw std::runtime_error("stellar.toml: malformed array");
    const QString inner = acc.mid(lb + 1, rb - lb - 1);
    QVariantList out;
    for (const QString& e : splitArrayElements(inner)) {
        const QString t = e.trimmed();
        if (!t.isEmpty()) out.append(parseScalar(t));
    }
    return out;
}

int countTripleQuote(const QString& s)
{
    int n = 0, from = 0;
    while ((from = s.indexOf(QStringLiteral("\"\"\""), from)) >= 0) { n++; from += 3; }
    return n;
}

bool arrayBalanced(const QString& s)
{
    int depth = 0; bool inStr = false;
    for (int i = 0; i < s.size(); ++i) {
        const QChar c = s.at(i);
        if (inStr) { if (c == '"' && s.at(i - 1) != '\\') inStr = false; }
        else if (c == '"') inStr = true;
        else if (c == '[') depth++;
        else if (c == ']') depth--;
    }
    return depth == 0;
}

QString parseMultiline(const QString& acc)
{
    int start = acc.indexOf(QStringLiteral("\"\"\"")) + 3;
    int end = acc.lastIndexOf(QStringLiteral("\"\"\""));
    // An unterminated multi-line string leaves `end` pointing at the OPENING
    // delimiter, so `end - start` is negative — and QString::mid clamps a
    // negative length to "everything from start", which would silently return
    // the whole remainder of the document as the value and swallow every key
    // after it. Fail loudly instead, as the header documents.
    if (end < start)
        throw std::runtime_error("stellar.toml: unterminated multi-line string");
    QString body = acc.mid(start, end - start);
    if (body.startsWith('\n')) body.remove(0, 1);          // TOML trims the first newline
    else if (body.startsWith(QStringLiteral("\r\n"))) body.remove(0, 2);
    return body;
}

QString unquoteKey(QString k)
{
    k = k.trimmed();
    if (k.size() >= 2 && k.startsWith('"') && k.endsWith('"'))
        return k.mid(1, k.size() - 2);
    return k;
}

} // namespace

QVariantMap StellarToml::parse(const QString& toml)
{
    QVariantMap root;
    QString section;                 // "" = top level
    bool sectionIsArray = false;
    QVariantMap table;               // accumulates the current section's keys

    auto flush = [&]() {
        if (section.isEmpty()) return;
        if (sectionIsArray) {
            QVariantList list = root.value(section).toList();
            list.append(table);
            root.insert(section, list);
        } else {
            root.insert(section, table);
        }
        table.clear();
    };

    const QStringList lines = toml.split('\n');
    int i = 0;
    while (i < lines.size()) {
        QString line = lines.at(i).trimmed();
        i++;
        if (line.isEmpty() || line.startsWith('#')) continue;

        if (line.startsWith(QStringLiteral("[["))) {
            flush();
            const int close = line.indexOf(QStringLiteral("]]"));
            section = line.mid(2, close - 2).trimmed();
            sectionIsArray = true;
            continue;
        }
        if (line.startsWith('[')) {
            flush();
            const int close = line.indexOf(']');
            section = line.mid(1, close - 1).trimmed();
            sectionIsArray = false;
            continue;
        }

        const int eq = line.indexOf('=');
        if (eq < 0) throw std::runtime_error(("stellar.toml: bad line: " + line).toStdString());
        const QString key = unquoteKey(line.left(eq));
        QString valStr = line.mid(eq + 1).trimmed();

        QVariant value;
        if (valStr.startsWith(QStringLiteral("\"\"\""))) {
            QString acc = valStr;
            while (countTripleQuote(acc) < 2 && i < lines.size()) { acc += "\n" + lines.at(i); i++; }
            value = parseMultiline(acc);
        } else if (valStr.startsWith('[')) {
            QString acc = valStr;
            while (!arrayBalanced(acc) && i < lines.size()) { acc += "\n" + lines.at(i); i++; }
            value = parseArray(acc);
        } else {
            value = parseScalar(valStr);
        }

        if (section.isEmpty()) root.insert(key, value);
        else                   table.insert(key, value);
    }
    flush();
    return root;
}

QSTELLAR_END_NS
