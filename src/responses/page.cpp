#include "page.h"
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include <stdexcept>

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
        void * obj = mo->newInstance();
        fillObject(mo, obj, records.at(i).toObject(),true);
        this->m_records.append((Response*)obj);
    }
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


    QJsonObject object = doc.object();

    if(!this->isStreamingResponse()){
        const QJsonObject links = object.value("_links").toObject();

        Response::fillObject(&PageAttach::Links::staticMetaObject,&m_links,links);
        QJsonObject embedded = object.value("_embedded").toObject();


        const QJsonArray records = embedded.value("records").toArray();
        this->processRecords(records);
    }
    else{
        //in case it is a streaming we just fill first record with actual data, ready signal will be emitted so you can process the record
        this->reset();
        QJsonArray records;
        records.append(object);
        this->processRecords(records);
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

    if (this->getLinks().getNext().getHref().isNull()) {
        return nullptr;
    }
    QNetworkAccessManager* manager = this->m_reply->manager();
    QNetworkRequest request(this->getLinks().getNext().getHref());
    request.setRawHeader("X-Client-Name", STELLAR_QT_SDK_CLIENT_NAME);
    request.setRawHeader("X-Client-Version", STELLAR_QT_SDK_CLIENT_VERSION);
    QNetworkReply * reply = manager->get(request);
    this->loadFromReply(reply);
    return this;//we reuse same object
}
