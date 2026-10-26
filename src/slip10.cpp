#include "slip10.h"

#include <QMessageAuthenticationCode>

QSTELLAR_BEGIN_NS

SLIP10::SLIP10()
{

}

QByteArray SLIP10::deriveEd25519PrivateKey(const QByteArray seed, const QVector<int> indexes){
    QMessageAuthenticationCode mac(QCryptographicHash::Sha512,"ed25519 seed");
    mac.addData(seed);
    QByteArray I = mac.result();
    for (int i : indexes) {
        // Each I is a private key and its chain code: the copies made here
        // are wiped once used (what QMessageAuthenticationCode keeps inside
        // is out of reach).
        QByteArray chainCode = I.mid(32,32);
        QByteArray key = I.mid(0,32);
        QMessageAuthenticationCode mac(QCryptographicHash::Sha512,chainCode);
        // Data = 0x00
        mac.addData(QByteArray(1,0));
        // Data += Il
        mac.addData(key);
        // Data += ser32(i')
        mac.addData(QByteArray(1,static_cast<char>(i >> 24 | 0x80)));
        mac.addData(QByteArray(1,static_cast<char>(i >> 16)));
        mac.addData(QByteArray(1,static_cast<char>(i >> 8)));
        mac.addData(QByteArray(1,static_cast<char>(i)));
        // Write to I
        I.fill('\0');
        key.fill('\0');
        chainCode.fill('\0');
        I = mac.result();
    }
    // copy head 32 bytes of I into Il
    const QByteArray il = I.mid(0,32);
    I.fill('\0');
    return il;
}

QSTELLAR_END_NS
