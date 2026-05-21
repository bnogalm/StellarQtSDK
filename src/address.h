#ifndef ADDRESS_H
#define ADDRESS_H

#include <QByteArray>
#include <QString>

/**
 * Polymorphic address that can hold any of the 5 SEP-23 strkey forms:
 * G (account), M (muxed), C (contract), B (claimable balance), L (liquidity pool).
 *
 * The class is a thin wrapper over the decoded bytes — convert to XDR with
 * the type-specific helper that lives alongside the matching domain object
 * (e.g. SCAddress for Soroban contracts, ClaimableBalanceID, PoolID).
 *
 * SCAddress conversion (.toScAddress/.fromScAddress) lands in 0.9.0
 * alongside the Soroban XDR.
 */
class Address
{
public:
    enum class Type {
        ACCOUNT,           // G
        MUXED_ACCOUNT,     // M
        CONTRACT,          // C
        CLAIMABLE_BALANCE, // B (33-byte body)
        LIQUIDITY_POOL     // L
    };

private:
    Type m_type;
    QByteArray m_bytes;   // raw decoded body. For B = 33 bytes (type+hash); for M = 40 bytes (32 ed25519 + 8 muxed id big-endian); others = 32 bytes.

public:
    Address() : m_type(Type::ACCOUNT), m_bytes() {}
    Address(Type type, QByteArray bytes) : m_type(type), m_bytes(std::move(bytes)) {}

    /** Parses any of G / M / C / B / L. Throws on invalid prefix or checksum. */
    static Address fromString(const QString& strkey);

    Type getType() const { return m_type; }
    const QByteArray& getBytes() const { return m_bytes; }

    /** Returns the canonical strkey for this address. */
    QString toString() const;

    bool operator==(const Address& other) const
    {
        return m_type == other.m_type && m_bytes == other.m_bytes;
    }
    bool operator!=(const Address& other) const { return !(*this == other); }
};

#endif // ADDRESS_H
