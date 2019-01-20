#include "slip10.h"

#include <QMessageAuthenticationCode>


SLIP10::SLIP10()
{

}

QByteArray SLIP10::deriveEd25519PrivateKey(const QByteArray seed, const QVector<int> indexes){
    QMessageAuthenticationCode mac(QCryptographicHash::Sha512,"ed25519 seed");
    mac.addData(seed);
    QByteArray I = mac.result();
    for (int i : indexes) {
        QMessageAuthenticationCode mac(QCryptographicHash::Sha512,I.mid(32,32));
        // Data = 0x00
        mac.addData(QByteArray(1,0));
        // Data += Il
        mac.addData(I.mid(0,32));
        // Data += ser32(i')
        mac.addData(QByteArray(1,static_cast<char>(i >> 24 | 0x80)));
        mac.addData(QByteArray(1,static_cast<char>(i >> 16)));
        mac.addData(QByteArray(1,static_cast<char>(i >> 8)));
        mac.addData(QByteArray(1,static_cast<char>(i)));
        // Write to I
        I = mac.result();
    }
    // copy head 32 bytes of I into Il
    return I.mid(0,32);
}
