#ifndef MUXEDACCOUNT_H
#define MUXEDACCOUNT_H

#include <QString>
#include <QtGlobal>

#include "xdr/stellartransaction.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/**
 * First-class wrapper around stellar::MuxedAccount. SEP-23 — represents both
 * a plain ed25519 account (G-strkey) and a muxed account (M-strkey) carrying
 * a 64-bit muxed id.
 *
 * Construct via `fromAddress(str)` (parses G or M strkey) or
 * `fromAccountIdAndMuxedId(g, id)`.
 */
class MuxedAccount
{
    stellar::MuxedAccount m_xdr;
public:
    MuxedAccount();
    explicit MuxedAccount(const stellar::MuxedAccount& xdr) : m_xdr(xdr) {}

    /** Parses any G-strkey or M-strkey. */
    static MuxedAccount fromAddress(const QString& strkey);
    /** Builds a muxed account from a G-strkey + 64-bit muxed id. */
    static MuxedAccount fromAccountIdAndMuxedId(const QString& accountId, quint64 muxedId);
    /** Builds a plain (non-muxed) account from a G-strkey. */
    static MuxedAccount fromAccountId(const QString& accountId);

    /** True when the underlying type is KEY_TYPE_MUXED_ED25519. */
    bool isMuxed() const;

    /** Returns the G-strkey of the base account (always defined). */
    QString getAccountId() const;
    /** Returns the muxed id when muxed; 0 otherwise. */
    quint64 getMuxedId() const;
    /** Returns the canonical address: G when not muxed, M when muxed. */
    QString toAddress() const;

    stellar::MuxedAccount toXdr() const { return m_xdr; }

    bool operator==(const MuxedAccount& other) const { return m_xdr == other.m_xdr; }
    bool operator!=(const MuxedAccount& other) const { return !(*this == other); }
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(MuxedAccount)

#endif // MUXEDACCOUNT_H
