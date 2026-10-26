#include "strkey.h"
#include "external/cyoencode/src/CyoEncode.h"
#include "external/cyoencode/src/CyoDecode.h"
#include <QDebug>
#include <QScopeGuard>

QSTELLAR_BEGIN_NS

StrKey::StrKey()
{

}

QString StrKey::encodeStellarMuxedAccount(const stellar::MuxedAccount &muxedAccount) {
    switch (muxedAccount.type) {
    case stellar::CryptoKeyType::KEY_TYPE_MUXED_ED25519:
    {
        QByteArray data((const char*)muxedAccount.med25519.ed25519,sizeof(stellar::uint256));
        QDataStream stream(&data,QIODeviceEnums::Append);
        stream << muxedAccount.med25519.id;
        return encodeCheck(VersionByte::MUXED_ACCOUNT, data);
    }
    case stellar::CryptoKeyType::KEY_TYPE_ED25519:
    {
        return encodeCheck(VersionByte::ACCOUNT_ID, QByteArray::fromRawData((const char*)(muxedAccount.ed25519),sizeof(muxedAccount.ed25519)));
    }
    default:
        throw std::runtime_error("invalid discriminant");
    }
}

StrKey::VersionByte StrKey::decodeVersionByte(QByteArray encoded) {
    // CyoEncode's Base32::GetLength expects padded (multiple of 8) inputs;
    // unpadded strkeys like M / P (69 chars) would otherwise yield length 0.
    int cyoWorkaroundPaddingLength = encoded.size() % 8;
    if (cyoWorkaroundPaddingLength > 0) {
        cyoWorkaroundPaddingLength = 8 - cyoWorkaroundPaddingLength;
        encoded.append(cyoWorkaroundPaddingLength, '=');
    }

    QByteArray decoded(CyoDecode::Base32::GetLength(encoded.length()),'\0');

    if(decoded.isEmpty())
        throw std::runtime_error("empty data");

    CyoDecode::Base32::Decode((CyoEncode::byte_t*)decoded.data(),encoded.data(),encoded.length());

    StrKey::VersionByte decodedVersionByte = static_cast<StrKey::VersionByte>(decoded.at(0));

    switch(decodedVersionByte)
    {
    case StrKey::VersionByte::ACCOUNT_ID:
    case StrKey::VersionByte::MUXED_ACCOUNT:
    case StrKey::VersionByte::SEED:
    case StrKey::VersionByte::PRE_AUTH_TX:
    case StrKey::VersionByte::SHA256_HASH:
    case StrKey::VersionByte::SIGNED_PAYLOAD:
    case StrKey::VersionByte::CONTRACT:
    case StrKey::VersionByte::CLAIMABLE_BALANCE:
    case StrKey::VersionByte::LIQUIDITY_POOL:
        return decodedVersionByte;
    default:
        throw std::runtime_error("Version byte is invalid");
    }
}

StrKey::VersionByte StrKey::decodeVersionByte(QString encoded)
{
    // Latin-1, as every decoder here: the local 8-bit codec made the same
    // text read differently from one machine to another. A character
    // outside base32 is refused either way.
    return StrKey::decodeVersionByte(encoded.toLatin1());
}

StrKey::Decoded StrKey::decodeAny(const QString& encoded)
{
    const QByteArray text = encoded.toLatin1();
    // Non-Latin-1 characters became '?', which no base32 alphabet has.
    const VersionByte version = decodeVersionByte(text);
    return Decoded{version, decodeCheck(version, text)};
}

QByteArray StrKey::encodeCheck(StrKey::VersionByte versionByte, QByteArray data) {

    QByteArray payload;
    QDataStream outputStream(&payload,QIODevice::WriteOnly);
    outputStream<< (quint8)versionByte;

    outputStream.writeRawData(data.data(),data.length());
    QByteArray checksum = StrKey::calculateChecksum(payload);

    outputStream.writeRawData(checksum.data(),checksum.length());


    QByteArray bytesEncoded(CyoEncode::Base32::GetLength(payload.length()),'\0');

    CyoEncode::Base32::Encode(bytesEncoded.data(),(CyoEncode::byte_t*)payload.data(),payload.length());

    //TODO remove CyoEncode encoding, we dont require padding and it just cause problems for just base32 encoding decoding
    int requiresTrim= bytesEncoded.indexOf('=');
    if(requiresTrim>=0)
    {
        bytesEncoded.resize(requiresTrim);
    }
    if (bytesEncoded[bytesEncoded.size() - 1] == '\0') {
        bytesEncoded.chop(1);
    }
    if (VersionByte::SEED == versionByte) {
        payload.fill('\0',payload.length());
    }

    return bytesEncoded;
}
//is would be better to use a constexp to fill it
uchar b32Table[] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};

namespace {

/**
 * Why a signed payload's body is not framed as SEP-23 frames it (the
 * ed25519 key, a big-endian length of 1 to 64, the payload, zeros to the
 * next multiple of 4), null when it is: js's assertSignedPayloadFraming.
 * py 14.1.0 takes non-zero padding, which core's XDR refuses.
 */
const char* signedPayloadFramingProblem(const QByteArray& body)
{
    if (body.size() < 32 + 4)
        return "signed payload: too short to carry a payload length";
    const quint32 declared = (quint32(quint8(body.at(32))) << 24) | (quint32(quint8(body.at(33))) << 16)
                           | (quint32(quint8(body.at(34))) << 8) | quint32(quint8(body.at(35)));
    if (declared < 1 || declared > 64)
        return "signed payload: declared length outside 1-64";
    if (body.size() != int(32 + 4 + (declared + 3) / 4 * 4))
        return "signed payload: declared length does not match the data";
    for (int i = int(32 + 4 + declared); i < body.size(); ++i) {
        if (body.at(i) != 0)
            return "signed payload: non-zero padding";
    }
    return nullptr;
}

} // namespace

QByteArray StrKey::decodeCheck(StrKey::VersionByte versionByte, QByteArray encoded) {

    // The minimal binary decoded length is 3 bytes (version byte and 2-byte CRC) which,
    // in unpadded base32 (since each character provides 5 bits) corresponds to ceiling(8*3/5) = 5
    if (encoded.length() < 5) {
        throw std::runtime_error("Encoded char array must have a length of at least 5.");
    }

    int leftoverBits = (encoded.length() * 5) % 8;
    // 1. Make sure there is no full unused leftover byte at the end
    //   (i.e. there shouldn't be 5 or more leftover bits)
    if (leftoverBits >= 5) {
        throw std::runtime_error("Encoded char array has leftover character.");
    }
    if (leftoverBits > 0) {
        uchar lastChar = encoded[encoded.length()-1];
        uchar decodedLastChar = b32Table[lastChar];
        uchar leftoverBitsMask = (uchar)(0x0f >> (4 - leftoverBits));
        if ((decodedLastChar & leftoverBitsMask) != 0) {
            throw std::runtime_error("Unused bits should be set to 0.");
        }
    }
    if(versionByte!= VersionByte::SEED && encoded.startsWith('S'))
    {
        throw std::runtime_error("decoding secret to wrong place");//this can be dangerous as it can expose a secret key on public application places
    }
    for(int i=0;i< encoded.length();i++){
        const quint8 c = (quint8)encoded.at(i);
        // The base32 alphabet only, upper case (SEP-23): lower case was taken
        // ("gcfiry65..." signed where js and py refuse it), and so was '=',
        // which the padding-aware decoder read as zero bits — the same key
        // under another spelling.
        if(c>127 || b32Table[c] == 0xff){
            throw std::runtime_error("Illegal characters in encoded char array.");
        }
    }
    const QByteArray original = encoded;
    //TODO delete Cyo dependency, we don't want padding and we only use base32 encoding decoding.
    int cyoWorkaroundPaddingLength = encoded.size()%8;
    if(cyoWorkaroundPaddingLength>0)
    {
        cyoWorkaroundPaddingLength = 8 - cyoWorkaroundPaddingLength;
        encoded.append(cyoWorkaroundPaddingLength,'=');
    }
    QByteArray decoded(CyoDecode::Base32::GetLength(encoded.length()),'\0');

    if(decoded.isEmpty())
        throw FormatException();

    // What an S... decodes to is the seed, give or take a typo: the copies
    // made of it below are wiped on every way out, each throw included (a
    // seed with one character mistyped fails the checksum with 32 bytes of
    // it in hand). `data` is what a good one returns: kept on success.
    QByteArray payload;
    QByteArray data;
    QByteArray reencoded;
    bool returningData = false;
    auto wipeSeedCopies = qScopeGuard([&] {
        if (versionByte != VersionByte::SEED
                && (decoded.isEmpty() || StrKey::VersionByte(quint8(decoded.at(0))) != VersionByte::SEED))
            return;
        decoded.fill('\0', decoded.length());
        payload.fill('\0', payload.length());
        // The re-encoding is the S... strkey again: wiped, not just freed.
        reencoded.fill('\0', reencoded.length());
        if (!returningData)
            data.fill('\0', data.length());
    });

    CyoDecode::Base32::Decode((CyoEncode::byte_t*)decoded.data(),encoded.data(),encoded.length());
    //TODO delete Cyo dependency, we don't want padding and we only use base32 encoding decoding.
    if(cyoWorkaroundPaddingLength>0)
    {
        int decodedPadLength= (cyoWorkaroundPaddingLength*5)/8 + ((cyoWorkaroundPaddingLength*5) % 8 !=0);
        if(!decoded.endsWith(QByteArray(decodedPadLength,'\0')))
        {
            throw std::runtime_error("Unused bits should be set to 0.");
        }
        decoded.resize(decoded.size()-decodedPadLength);
    }

    StrKey::VersionByte decodedVersionByte = (StrKey::VersionByte)decoded.at(0);
    payload = QByteArray(decoded.data(),decoded.length()-2);
    data = QByteArray(decoded.data()+1,decoded.length()-3);
    QByteArray checksum(decoded.data()+ decoded.length()-2,2);
    if(decodedVersionByte != versionByte){
        throw std::runtime_error("Version byte is invalid");
    }

    QByteArray expectedChecksum= StrKey::calculateChecksum(payload);
    if(expectedChecksum!=checksum){
        throw std::runtime_error("Checksum invalid");
    }

    // The body each kind has (SEP-23), as js's decodeCheck requires: a G of
    // five bytes or an M of 32 carried a good checksum and was accepted.
    bool lengthOk = false;
    switch (decodedVersionByte) {
    case VersionByte::ACCOUNT_ID:
    case VersionByte::SEED:
    case VersionByte::PRE_AUTH_TX:
    case VersionByte::SHA256_HASH:
    case VersionByte::CONTRACT:
    case VersionByte::LIQUIDITY_POOL:
        lengthOk = data.size() == 32;
        break;
    case VersionByte::MUXED_ACCOUNT:
        lengthOk = data.size() == 40;
        break;
    case VersionByte::CLAIMABLE_BALANCE:
        // One byte of ClaimableBalanceIDType, and V0 is the only one.
        lengthOk = data.size() == 33 && data.at(0) == 0;
        break;
    case VersionByte::SIGNED_PAYLOAD:
        // ed25519 key, a length, then 1..64 payload bytes padded to 4.
        lengthOk = data.size() >= 40 && data.size() <= 100 && (data.size() - 36) % 4 == 0;
        break;
    default:
        break;
    }
    // And exactly one spelling: what decodes must encode back to the input.
    if (lengthOk)
        reencoded = encodeCheck(decodedVersionByte, data);
    const bool canonical = lengthOk && reencoded == original;
    if (!lengthOk)
        throw std::runtime_error("invalid encoded string length");
    if (!canonical)
        throw std::runtime_error("invalid encoded string: not canonical");
    // A P... body framed as its declared length says, here and not only in
    // decodeSignedPayload: js's decodeCheck refuses one that is not, py's
    // one whose length does not match.
    if (VersionByte::SIGNED_PAYLOAD == decodedVersionByte) {
        if (const char* problem = signedPayloadFramingProblem(data))
            throw std::runtime_error(problem);
    }

    returningData = true;
    return data;
}

QString StrKey::encodeSignedPayload(QByteArray ed25519, QByteArray payload)
{
    if (ed25519.size() != 32) {
        throw std::runtime_error("ed25519 must be 32 bytes");
    }
    if (payload.isEmpty() || payload.size() > 64) {
        throw std::runtime_error("signed payload length must be in [1, 64]");
    }
    QByteArray body;
    QDataStream s(&body, QIODevice::WriteOnly);
    s.writeRawData(ed25519.constData(), 32);
    s << static_cast<quint32>(payload.size());
    s.writeRawData(payload.constData(), payload.size());
    const int pad = (4 - (payload.size() % 4)) % 4;
    if (pad) {
        char zero[4] = {0,0,0,0};
        s.writeRawData(zero, pad);
    }
    return QString::fromLatin1(encodeCheck(VersionByte::SIGNED_PAYLOAD, body));
}

QPair<QByteArray, QByteArray> StrKey::decodeSignedPayload(QString data)
{
    QByteArray body = decodeCheck(VersionByte::SIGNED_PAYLOAD, data.toLatin1());
    // body layout: 32 ed25519 + 4 len + payload + 0-3 padding
    if (body.size() < 32 + 4) {
        throw std::runtime_error("signed payload strkey too short");
    }
    QByteArray ed25519 = body.left(32);
    QDataStream s(body);
    s.skipRawData(32);
    quint32 payloadLen = 0;
    s >> payloadLen;
    if (payloadLen == 0 || payloadLen > 64) {
        throw std::runtime_error("signed payload length out of range");
    }
    if (body.size() < static_cast<int>(32 + 4 + payloadLen)) {
        throw std::runtime_error("signed payload truncated");
    }
    QByteArray payload(payloadLen, '\0');
    s.readRawData(payload.data(), static_cast<int>(payloadLen));
    const quint32 pad = (4 - (payloadLen % 4)) % 4;
    if (static_cast<int>(32 + 4 + payloadLen + pad) != body.size()) {
        throw std::runtime_error("signed payload padding invalid");
    }
    if (pad) {
        char zero[4] = {0,0,0,0};
        s.readRawData(zero, static_cast<int>(pad));
        for (quint32 i = 0; i < pad; ++i)
            if (zero[i] != 0) throw std::runtime_error("non-zero padding");
    }
    return qMakePair(ed25519, payload);
}

QByteArray StrKey::calculateChecksum(QByteArray bytes) {
    // This code calculates CRC16-XModem checksum
    // Ported from https://github.com/alexgorbatchev/node-crc
    // FIX §7.4: use quint16. Right-shift of signed values with the top bit
    // set is implementation-defined in C++; unsigned is well-defined.
    quint16 crc = 0x0000;

    int count = bytes.length();
    int i = 0;
    int code = 0;

    while (count > 0) {
        code = (crc >> 8) & 0xFF;
        code ^= bytes[i++] & 0xFF;
        code ^= (code >> 4);
        crc = (crc << 8) & 0xFFFF;
        crc ^= code;
        code = (code << 5) & 0xFFFF;
        crc ^= code;
        code = (code << 7) & 0xFFFF;
        crc ^= code;
        count--;
    }

    // little-endian
    crc = qToLittleEndian(crc);
    return QByteArray((const char*)&crc, sizeof(quint16));
}

QSTELLAR_END_NS


