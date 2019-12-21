#include "strkey.h"
#include "external/cyoencode/src/CyoEncode.h"
#include "external/cyoencode/src/CyoDecode.h"
#include <QDebug>
StrKey::StrKey()
{

}

QByteArray StrKey::encodeCheck(StrKey::VersionByte versionByte, QByteArray data) {

    QByteArray payload;
    QDataStream outputStream(&payload,QIODevice::WriteOnly);
    outputStream<< (quint8)versionByte;
    //outputStream<< data;
    outputStream.writeRawData(data.data(),data.length());
    QByteArray checksum = StrKey::calculateChecksum(payload);
    //outputStream<< checksum;
    outputStream.writeRawData(checksum.data(),checksum.length());


    QByteArray bytesEncoded(CyoEncode::Base32::GetLength(payload.length()),'\0');

    CyoEncode::Base32::Encode(bytesEncoded.data(),(CyoEncode::byte_t*)payload.data(),payload.length());

    if (VersionByte::SEED == versionByte) {
        payload.fill('\0',payload.length());
    }

    return bytesEncoded;
}

QByteArray StrKey::decodeCheck(StrKey::VersionByte versionByte, QByteArray encoded) {

    if(versionByte!= VersionByte::SEED && encoded.startsWith('S'))
    {
        throw std::runtime_error("decoding secret to wrong place");//this can be dangerous as it can expose a secret key on public application places
    }
    for(int i=0;i< encoded.length();i++){
        if((quint8)encoded.at(i)>127){
            //qDebug("Illegal characters in encoded char array.");
            throw FormatException();
        }
    }
    QByteArray decoded(CyoDecode::Base32::GetLength(encoded.length()),'\0');

    if(decoded.isEmpty())
        throw FormatException();

    CyoDecode::Base32::Decode((CyoEncode::byte_t*)decoded.data(),encoded.data(),encoded.length());

    StrKey::VersionByte decodedVersionByte = (StrKey::VersionByte)decoded.at(0);
    QByteArray payload(decoded.data(),decoded.length()-2);
    QByteArray data(decoded.data()+1,decoded.length()-3);
    QByteArray checksum(decoded.data()+ decoded.length()-2,2);
    if(decodedVersionByte != versionByte){
        //qDebug("Version byte is invalid");
        throw FormatException();
    }

    QByteArray expectedChecksum= StrKey::calculateChecksum(payload);
    if(expectedChecksum!=checksum){
        //qDebug() << expectedChecksum.toHex()<< " != "<< checksum.toHex();
        //qDebug() << "data: "<< data.toHex();
        //qDebug("Checksum invalid");
        throw FormatException();
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
