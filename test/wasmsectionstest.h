#ifndef WASMSECTIONSTEST_H
#define WASMSECTIONSTEST_H

#include <QObject>
#include <QtTest>
#include "testcollector.h"

#include "../src/wasmsections.h"

/**
 * Walking the WebAssembly container.
 *
 * The modules here are assembled byte by byte rather than loaded from a file,
 * for the same reason the XDR tests assemble their bytes by hand: a fixture
 * produced by the same code that reads it proves only that the code agrees with
 * itself. These bytes are laid out from the WebAssembly specification, and half
 * of them are deliberately wrong — a .wasm arrives from wherever the user got
 * it, and every length inside it is a number the file is free to lie about.
 */
class WasmSectionsTest: public QObject
{
    Q_OBJECT

    /** The eight bytes every module starts with: "\0asm" then version 1. */
    static QByteArray header()
    {
        return QByteArray::fromHex("0061736d01000000");
    }

    /** An unsigned LEB128 integer. */
    static QByteArray leb(quint32 value)
    {
        QByteArray out;
        do {
            quint8 byte = value & 0x7F;
            value >>= 7;
            if (value)
                byte |= 0x80;
            out.append(char(byte));
        } while (value);
        return out;
    }

    /** A well-formed custom section: id 0, size, name, payload. */
    static QByteArray customSection(const QByteArray& name, const QByteArray& payload)
    {
        const QByteArray body = leb(quint32(name.size())) + name + payload;
        return QByteArray(1, char(0)) + leb(quint32(body.size())) + body;
    }

    /** A section that is not custom, to prove the walk steps over it. */
    static QByteArray otherSection(quint8 id, const QByteArray& payload)
    {
        return QByteArray(1, char(id)) + leb(quint32(payload.size())) + payload;
    }

private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    // ─── The happy path ──────────────────────────────────────────────

    void testRecognisesAWasmHeader()
    {
        QVERIFY(WasmSections::looksLikeWasm(header()));
        QVERIFY(!WasmSections::looksLikeWasm(QByteArray()));
        QVERIFY(!WasmSections::looksLikeWasm(QByteArray("not wasm at all")));
        // Right magic, wrong version.
        QVERIFY(!WasmSections::looksLikeWasm(QByteArray::fromHex("0061736d02000000")));
        // Truncated inside the header.
        QVERIFY(!WasmSections::looksLikeWasm(QByteArray::fromHex("0061736d01")));
    }

    void testFindsACustomSectionByName()
    {
        const QByteArray wasm = header()
            + customSection("contractenvmetav0", QByteArray("META"))
            + customSection("contractspecv0", QByteArray("SPEC-PAYLOAD"));

        QCOMPARE(WasmSections::customSection(wasm, "contractspecv0"),
                 QByteArray("SPEC-PAYLOAD"));
        QCOMPARE(WasmSections::customSection(wasm, "contractenvmetav0"),
                 QByteArray("META"));
        QVERIFY(WasmSections::customSection(wasm, "nothing here").isEmpty());
    }

    void testStepsOverNonCustomSections()
    {
        // A real contract is mostly numbered sections; the custom ones are a
        // minority and are not first.
        const QByteArray wasm = header()
            + otherSection(1, QByteArray("type section"))
            + otherSection(3, QByteArray("function section"))
            + customSection("contractspecv0", QByteArray("FOUND"))
            + otherSection(10, QByteArray("code section"));

        QCOMPARE(WasmSections::customSection(wasm, "contractspecv0"), QByteArray("FOUND"));
        QCOMPARE(WasmSections::customSectionNames(wasm),
                 QStringList{QStringLiteral("contractspecv0")});
    }

    void testListsEveryCustomSectionInOrder()
    {
        const QByteArray wasm = header()
            + customSection("name", QByteArray("n"))
            + otherSection(1, QByteArray("x"))
            + customSection("contractspecv0", QByteArray("s"))
            + customSection("contractenvmetav0", QByteArray("e"));

        const QStringList names = WasmSections::customSectionNames(wasm);
        QCOMPARE(names.size(), 3);
        QCOMPARE(names.at(0), QStringLiteral("name"));
        QCOMPARE(names.at(1), QStringLiteral("contractspecv0"));
        QCOMPARE(names.at(2), QStringLiteral("contractenvmetav0"));
    }

    void testAnEmptyPayloadIsStillASection()
    {
        // Distinguishable from "absent" only through customSectionNames, which
        // is why that method exists.
        const QByteArray wasm = header() + customSection("contractspecv0", QByteArray());
        QVERIFY(WasmSections::customSection(wasm, "contractspecv0").isEmpty());
        QCOMPARE(WasmSections::customSectionNames(wasm),
                 QStringList{QStringLiteral("contractspecv0")});
    }

    void testTheFirstOfADuplicatedNameWins()
    {
        // The specification permits repeats. Concatenating them, or silently
        // taking the last, would be inventing a rule nobody agreed on.
        const QByteArray wasm = header()
            + customSection("contractspecv0", QByteArray("FIRST"))
            + customSection("contractspecv0", QByteArray("SECOND"));
        QCOMPARE(WasmSections::customSection(wasm, "contractspecv0"), QByteArray("FIRST"));
        QCOMPARE(WasmSections::customSectionNames(wasm).size(), 2);
    }

    // ─── Files that lie ──────────────────────────────────────────────

    void testASectionLongerThanTheFileEndsTheWalk()
    {
        // The classic overrun: a size prefix that points past the buffer.
        QByteArray wasm = header();
        wasm.append(char(0));                    // custom section
        wasm.append(leb(9999));                  // ...claiming 9999 bytes
        wasm.append("short");
        QVERIFY(WasmSections::customSection(wasm, "anything").isEmpty());
        QVERIFY(WasmSections::customSectionNames(wasm).isEmpty());
    }

    void testANameLongerThanItsSectionIsRejected()
    {
        // Name length inside the section header claims more than the section
        // holds, which would run the payload slice off the end.
        const QByteArray body = leb(200) + QByteArray("short name");
        QByteArray wasm = header();
        wasm.append(char(0));
        wasm.append(leb(quint32(body.size())));
        wasm.append(body);
        QVERIFY(WasmSections::customSectionNames(wasm).isEmpty());
    }

    void testAnUnterminatedLengthIsRejected()
    {
        // Continuation bits set forever: a 32-bit value cannot occupy more than
        // five bytes, so a sixth is not a big number, it is a malformed file.
        QByteArray wasm = header();
        wasm.append(char(0));
        wasm.append(QByteArray::fromHex("ffffffffffff"));
        QVERIFY(WasmSections::customSectionNames(wasm).isEmpty());
    }

    void testAFifthByteWithTooManyBitsIsRejected()
    {
        // 4x7 + 4 = 32, so the fifth byte of a u32 has only FOUR usable bits.
        //
        // The obvious hostile value, "ff ff ff ff 7f", is NOT the one to test
        // with: it truncates to 0xFFFFFFFF, which the bounds check downstream
        // rejects anyway, so the test would pass with no fifth-byte check at
        // all. The dangerous shape is one whose overflow lands on a PLAUSIBLE
        // number. "80 80 80 80 10" carries 0x10 in the fifth byte: shifted 28
        // places it falls entirely outside 32 bits, so a reader that shifts
        // anyway reads it as a perfectly reasonable size of ZERO, walks on,
        // and reports the section after it as though the file were sound.
        QByteArray wasm = header();
        wasm.append(char(0));
        wasm.append(QByteArray::fromHex("8080808010"));
        wasm.append(customSection("contractspecv0", QByteArray("REACHED")));

        // Rejecting the number ends the walk, so nothing past it is reported.
        QVERIFY2(WasmSections::customSectionNames(wasm).isEmpty(),
                 "a fifth byte with more than four bits must end the parse");
        QVERIFY(WasmSections::customSection(wasm, "contractspecv0").isEmpty());

        // And the largest legal u32 must still READ: 0xFFFFFFFF is
        // "ff ff ff ff 0f". It fails afterwards on bounds, which is the next
        // line of defence, but it must not fail in the number reader.
        QByteArray big = header();
        big.append(char(0));
        big.append(QByteArray::fromHex("ffffffff0f"));
        QVERIFY(WasmSections::customSectionNames(big).isEmpty());
    }

    void testANonMinimalLengthIsAccepted()
    {
        // "80 80 80 80 00" is a well-formed zero. Tools that pad a size to a
        // fixed width so it can be patched later emit exactly this, so a
        // validator demanding minimal encodings would reject real modules.
        const QByteArray name = "contractspecv0";
        const QByteArray body = leb(quint32(name.size())) + name + QByteArray("P");
        QByteArray wasm = header();
        wasm.append(char(0));
        wasm.append(leb(quint32(body.size())));
        wasm.append(body);
        // ...followed by a section whose size is a padded zero.
        wasm.append(char(0));
        wasm.append(QByteArray::fromHex("8080808000"));

        QCOMPARE(WasmSections::customSection(wasm, "contractspecv0"), QByteArray("P"));
    }

    void testAComponentIsNotACoreModule()
    {
        // The Component Model shares the magic and differs only in the version
        // word. A reader that checks four bytes instead of eight starts its
        // section loop on a component and produces confident nonsense.
        QByteArray component = QByteArray::fromHex("0061736d0d000100");
        component.append(customSection("contractspecv0", QByteArray("NOPE")));
        QVERIFY(!WasmSections::looksLikeWasm(component));
        QVERIFY(WasmSections::customSection(component, "contractspecv0").isEmpty());
    }

    void testTheNameMustMatchExactly()
    {
        // Not a prefix, not a substring, not case-insensitively. A reader that
        // searched the file for the bytes would find them in a debug section
        // or in the data of a contract that happens to mention the name.
        const QByteArray wasm = header()
            + customSection("contractspecv0x", QByteArray("DECOY"))
            + customSection("CONTRACTSPECV0", QByteArray("SHOUTING"))
            + customSection("contractspecv0", QByteArray("REAL"));

        QCOMPARE(WasmSections::customSection(wasm, "contractspecv0"), QByteArray("REAL"));
        QVERIFY(WasmSections::customSection(wasm, "contractspec").isEmpty());
    }

    void testTruncationMidSectionIsSurvivable()
    {
        const QByteArray whole = header()
            + customSection("contractspecv0", QByteArray("PAYLOAD-THAT-GETS-CUT"));
        for (int cut = 8; cut < whole.size(); ++cut) {
            const QByteArray truncated = whole.left(cut);
            // The assertion is that none of these reads out of bounds or
            // throws; what they return is beside the point.
            WasmSections::customSection(truncated, "contractspecv0");
            WasmSections::customSectionNames(truncated);
        }
        QVERIFY(true);
    }

    void testGarbageIsNotAModule()
    {
        const QByteArray garbage = QByteArray::fromHex("deadbeefcafebabe0102030405060708");
        QVERIFY(WasmSections::customSection(garbage, "contractspecv0").isEmpty());
        QVERIFY(WasmSections::customSectionNames(garbage).isEmpty());
    }

    void testABadSectionDoesNotHideTheOnesAfterIt()
    {
        // A custom section whose name cannot be read is skipped, not fatal: the
        // specification says a host ignores custom sections it does not
        // understand, so one bad one must not blind us to the rest.
        const QByteArray badBody = leb(200) + QByteArray("too short");
        QByteArray wasm = header();
        wasm.append(char(0));
        wasm.append(leb(quint32(badBody.size())));
        wasm.append(badBody);
        wasm.append(customSection("contractspecv0", QByteArray("STILL FOUND")));

        QCOMPARE(WasmSections::customSection(wasm, "contractspecv0"),
                 QByteArray("STILL FOUND"));
    }
};

ADD_TEST(WasmSectionsTest)
#endif // WASMSECTIONSTEST_H
