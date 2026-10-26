#ifndef ACCOUNTDATARESPONSE_H
#define ACCOUNTDATARESPONSE_H
#include "response.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/**
 * One data entry of an account, as Horizon's GET /accounts/{id}/data/{key}
 * answers it: {"value": "<base64>"}, and "sponsor" when the entry's reserve
 * is sponsored (see AccountsRequestBuilder::accountData). js asks for it
 * through an account record's data({key}) link and py with
 * Server.data(account_id, data_name); both return the JSON as is.
 *
 * The value is read exactly: text that is not the canonical base64 of at most
 * 64 bytes (what a data entry holds), or no value at all, makes the response
 * emit error() with an UnknownResponseException, as any unreadable answer.
 * py hands the text back without decoding it.
 */
class AccountDataResponse : public Response
{
    Q_OBJECT
    Q_PROPERTY(QString value MEMBER m_value)
    Q_PROPERTY(QString sponsor MEMBER m_sponsor)

    QString m_value;
    QString m_sponsor;
    QByteArray m_decoded;
protected:
    void reset() override;
public:
    Q_INVOKABLE explicit AccountDataResponse(QNetworkReply* reply = nullptr);
    virtual ~AccountDataResponse();

    /** Reads the entry, and decodes its value; throws std::runtime_error naming what could not be read. */
    void loadFromJson(QByteArray data) override;

    /** The entry's value, decoded: 0 to 64 bytes. Empty (not null) for an empty value; null before an answer. */
    QByteArray value() const;

    /** The value as Horizon sent it, base64. */
    QString valueBase64() const;

    /** The account sponsoring the entry's reserve, when Horizon names one; empty otherwise. */
    QString sponsor() const;
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(AccountDataResponse)

#endif // ACCOUNTDATARESPONSE_H
