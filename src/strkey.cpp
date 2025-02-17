#include "strkey.h"
#include "external/cyoencode/src/CyoEncode.h"
#include "external/cyoencode/src/CyoDecode.h"
#include <QDebug>
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
        return decodedVersionByte;
    default:
        throw std::runtime_error("Version byte is invalid");
    }
}

StrKey::VersionByte StrKey::decodeVersionByte(QString encoded)
{
    return StrKey::decodeVersionByte(encoded.toLocal8Bit());
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
        if((quint8)encoded.at(i)>127){            
            throw std::runtime_error("Illegal characters in encoded char array.");
        }
    }
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
    QByteArray payload(decoded.data(),decoded.length()-2);
    QByteArray data(decoded.data()+1,decoded.length()-3);
    QByteArray checksum(decoded.data()+ decoded.length()-2,2);
    if(decodedVersionByte != versionByte){
        throw std::runtime_error("Version byte is invalid");
    }

    QByteArray expectedChecksum= StrKey::calculateChecksum(payload);
    if(expectedChecksum!=checksum){
        throw std::runtime_error("Checksum invalid");
    }
    if (VersionByte::SEED == decodedVersionByte) {
        decoded.fill('\0',decoded.length());
        payload.fill('\0',payload.length());
    }

    return data;
}

QByteArray StrKey::calculateChecksum(QByteArray bytes) {
    // This code calculates CRC16-XModem checksum
    // Ported from https://github.com/alexgorbatchev/node-crc
    qint16 crc = 0x0000;

    int count = bytes.length();
    int i = 0;
    int code =0;

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
    crc=qToLittleEndian(crc);
    return QByteArray((const char*)&crc,sizeof(qint16));
}


