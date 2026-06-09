#ifndef STELLARADDRESS_H
#define STELLARADDRESS_H

#include <QByteArray>
#include <QString>

#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/**
 * Polymorphic Stellar address that can hold any of the 5 SEP-23 strkey forms:
 * G (account), M (muxed), C (contract), B (claimable balance), L (liquidity pool).
 *
 * The class is a thin wrapper over the decoded bytes — convert to XDR with
 * the type-specific helper that lives alongside the matching domain object
 * (e.g. SCAddress for Soroban contracts, ClaimableBalanceID, PoolID).
 *
 * SCAddress conversion (.toScAddress/.fromScAddress) lands in 0.9.0
 * alongside the Soroban XDR.
 *
 * Renamed from `Address` in 0.9.0 to avoid collision with Boost.Asio, Qt's
 * networking address types, and downstream `Address` classes in user code.
 */
class StellarAddress
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
    StellarAddress() : m_type(Type::ACCOUNT), m_bytes() {}
    StellarAddress(Type type, QByteArray bytes) : m_type(type), m_bytes(std::move(bytes)) {}

    /** Parses any of G / M / C / B / L. Throws on invalid prefix or checksum. */
    static StellarAddress fromString(const QString& strkey);

    Type getType() const { return m_type; }
    const QByteArray& getBytes() const { return m_bytes; }

    /** Returns the canonical strkey for this address. */
    QString toString() const;

    bool operator==(const StellarAddress& other) const
    {
        return m_type == other.m_type && m_bytes == other.m_bytes;
    }
    bool operator!=(const StellarAddress& other) const { return !(*this == other); }
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(StellarAddress)

#endif // STELLARADDRESS_H
