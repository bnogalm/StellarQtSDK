#ifndef SLIP10_H
#define SLIP10_H
#include <QVector>
class SLIP10
{
    SLIP10();
public:
    /**
     * Derives only the private key for ED25519 in the manor defined in
     * <a href="https://github.com/satoshilabs/slips/blob/master/slip-0010.md">SLIP-0010</a>.
     *
     * @param seed    Seed, the BIP0039 output.
     * @param indexes an array of indexes that define the path. E.g. for m/1'/2'/3', pass 1, 2, 3.
     *                As with Ed25519 non-hardened child indexes are not supported, this function treats all indexes
     *                as hardened.
     * @return Private key.
     */
    static QByteArray deriveEd25519PrivateKey(const QByteArray seed, const QVector<int> indexes = QVector<int>());
};

#endif // SLIP10_H



