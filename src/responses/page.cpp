#include "page.h"
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include <stdexcept>

QSTELLAR_BEGIN_NS


PageBase::PageBase(QNetworkReply *reply):Response(reply), m_responseType(0)
{
}

PageBase::~PageBase()
{
    for(Response * r: m_records)
    {
        delete r;
    }
    m_records.clear();
}

void PageBase::processRecords(const QJsonArray &records)
{
    const QMetaObject *mo = QMetaType::metaObjectForType(this->m_responseType);
    for(int i=0;i<records.size();i++){
        Response* record = static_cast<Response*>(mo->newInstance());
        try {
            fillObject(mo, record, recordObject(records, i), true, recordField(i));
        } catch (...) {
            delete record;
            throw;
        }
        this->m_records.append(record);
    }
}

QJsonObject PageBase::recordObject(const QJsonArray &records, int index) const
{
    const QJsonValue record = records.at(index);
    // Anything else read as a record with every member missing.
    if (!record.isObject())
        throw std::runtime_error(QStringLiteral("%1: expected an object").arg(recordField(index)).toStdString());
    return record.toObject();
}

QString PageBase::recordField(int index) const
{
    if (this->isStreamingResponse())
        return QString();
    return QStringLiteral("_embedded.records[%1]").arg(index);
}

void PageBase::loadFromJson(QByteArray data)
{
    if(!m_responseType)
        throw std::runtime_error("missing response type, use setType templated method");
    QJsonParseError check;
    QJsonDocument doc = QJsonDocument::fromJson(data,&check);

    if(check.error){
        throw std::runtime_error(check.errorString().toStdString());
    }
    if (!doc.isObject())
        throw std::runtime_error("expected a JSON object");


    QJsonObject object = doc.object();

    // A page that could not be read whole keeps none of its records: the
    // ones read before the failure used to stay, as if they were the page.
    const int before = m_records.size();
    try {
        if(!this->isStreamingResponse()){
            // The links getNextPage() follows, each with its href. Paths are
            // not paginated: they come without links, or only with self.
            const QJsonValue links = object.value("_links");
            if (!links.isUndefined()) {
                if (!links.isObject())
                    throw std::runtime_error("_links: expected an object");
                PageAttach::Links read;
                Response::fillObject(&PageAttach::Links::staticMetaObject, &read, links.toObject(), false,
                                     QStringLiteral("_links"));
                m_links = read;
            }
            // An answer without them used to read as an empty page.
            const QJsonValue embedded = object.value("_embedded");
            if (!embedded.isObject())
                throw std::runtime_error("_embedded: missing, but Horizon always sends it");
            const QJsonValue records = embedded.toObject().value("records");
            if (!records.isArray())
                throw std::runtime_error("_embedded.records: missing, but Horizon always sends it");
            this->processRecords(records.toArray());
        }
        else{
            //in case it is a streaming we just fill first record with actual data, ready signal will be emitted so you can process the record
            this->reset();
            QJsonArray records;
            records.append(object);
            this->processRecords(records);
        }
    } catch (...) {
        while (m_records.size() > before)
            delete m_records.takeLast();
        throw;
    }
}

void PageBase::reset()
{
    for(Response * r: m_records)
    {
        delete r;
    }
    m_records.clear();
    Response::reset();
}

void PageBase::setRecords(QList<Response *> records)
{
    m_records = records;
}

void PageBase::setLinks(PageAttach::Links links)
{
    m_links = links;
}

PageBase *PageBase::getNextPage(){
    return followLink(this->getLinks().getNext().getHref());
}

PageBase *PageBase::getPrevPage(){
    return followLink(this->getLinks().getPrev().getHref());
}

PageBase *PageBase::followLink(const QString& href){

    if (href.isNull()) {
        return nullptr;
    }
    // m_reply is null on a page read with loadFromJson(), and after
    // clearReply() once the reply (or its manager, which owns it) is gone.
    QNetworkAccessManager* manager = this->m_reply ? this->m_reply->manager() : nullptr;
    if (!manager) {
        return nullptr;
    }
    QNetworkRequest request{QUrl(href)};
    request.setRawHeader("X-Client-Name", STELLAR_QT_SDK_CLIENT_NAME);
    request.setRawHeader("X-Client-Version", STELLAR_QT_SDK_CLIENT_VERSION);
    QNetworkReply * reply = manager->get(request);
    this->loadFromReply(reply);
    return this;//we reuse same object
}
QSTELLAR_END_NS
