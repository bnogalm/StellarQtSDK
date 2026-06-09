#ifndef PROBLEM_H
#define PROBLEM_H

#include <QString>
#include <QStringList>
#include <QByteArray>
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/**
 * @brief RFC 7807 problem-details object returned by Horizon on errors.
 *
 * Horizon serves error bodies as `application/problem+json`. Example:
 *
 *   {
 *     "type": "https://stellar.org/horizon-errors/transaction_failed",
 *     "title": "Transaction Failed",
 *     "status": 400,
 *     "detail": "The transaction failed when submitted to the stellar network.",
 *     "extras": {
 *       "hash": "...",
 *       "envelope_xdr": "...",
 *       "result_xdr": "...",
 *       "result_codes": { "transaction": "...", "operations": [...] }
 *     }
 *   }
 *
 * Plain C++ class (no Q_GADGET) — parsed via QJsonDocument; getters are const.
 *
 * @see https://datatracker.ietf.org/doc/html/rfc7807
 */
class Problem
{
public:
    class Extras
    {
    public:
        class ResultCodes
        {
        public:
            ResultCodes() = default;

            QString     getTransaction() const      { return m_transaction; }
            QString     getInnerTransaction() const { return m_innerTransaction; }
            QStringList getOperations() const       { return m_operations; }

            void setTransaction(const QString& v)      { m_transaction = v; }
            void setInnerTransaction(const QString& v) { m_innerTransaction = v; }
            void setOperations(const QStringList& v)   { m_operations = v; }

        private:
            QString     m_transaction;
            QString     m_innerTransaction;
            QStringList m_operations;
        };

        Extras() = default;

        QString     getHash() const        { return m_hash; }
        QString     getEnvelopeXdr() const { return m_envelopeXdr; }
        QString     getResultXdr() const   { return m_resultXdr; }
        ResultCodes getResultCodes() const { return m_resultCodes; }

        void setHash(const QString& v)        { m_hash = v; }
        void setEnvelopeXdr(const QString& v) { m_envelopeXdr = v; }
        void setResultXdr(const QString& v)   { m_resultXdr = v; }
        void setResultCodes(const ResultCodes& v) { m_resultCodes = v; }

    private:
        QString     m_hash;
        QString     m_envelopeXdr;
        QString     m_resultXdr;
        ResultCodes m_resultCodes;
    };

    Problem() : m_status(-1) {}

    QString getType()   const { return m_type; }
    QString getTitle()  const { return m_title; }
    int     getStatus() const { return m_status; } // -1 if absent
    QString getDetail() const { return m_detail; }
    Extras  getExtras() const { return m_extras; }

    /**
     * Parse JSON body into this Problem. Best-effort: missing fields are left
     * with default values. Throws qstellar::exception::SdkException on
     * unparseable JSON.
     */
    void parseFromJson(const QByteArray& json);

    /**
     * Convenience static: returns a newly allocated Problem* or nullptr if
     * parsing fails. Caller owns the returned pointer.
     */
    static Problem* tryParse(const QByteArray& json);

private:
    QString m_type;
    QString m_title;
    int     m_status;
    QString m_detail;
    Extras  m_extras;
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(Problem)




#endif // PROBLEM_H
