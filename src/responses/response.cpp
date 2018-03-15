#include "response.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaProperty>
#include <QNetworkRequest>
#include <QTimer>
#define RECONNECT_DELAY 250

Response::Response(QNetworkReply *reply)    
    :m_status(0)
    ,m_reply(0)
    ,m_rateLimitLimit(0)
    ,m_rateLimitRemaining(0)
    ,m_rateLimitReset(0)
{
    loadFromReply(reply);
}
void Response::clearReply(QObject* obj)
{
    if(this->m_reply==obj ){
        this->m_reply=0;
    }
}

void Response::loadFromReply(QNetworkReply * reply)
{
    if(reply!=m_reply){
        if(m_reply){
            m_reply->deleteLater();
            m_reply=0;
            this->reset();//we set properties to zero
        }

        if(reply){
            m_reply = reply;
            connect(m_reply,&QObject::destroyed,this,&Response::clearReply);
            if(this->isStreamingResponse()){
                connect(reply, &QNetworkReply::readyRead, this,&Response::processPartialResponse);
                connect(reply, &QNetworkReply::finished, this,&Response::reconnectStream);
            }
            else{
                connect(reply, &QNetworkReply::finished, this,&Response::processResponse);
            }
        }
    }
}

void Response::reset()
{
    const QMetaObject * mo = this->metaObject();
    for(int i=0;i<mo->propertyCount();i++)
    {
        if(mo->property(i).isWritable()){
            mo->property(i).write(this,QVariant());
        }
    }
}

bool Response::isStreamingResponse()
{
    return m_reply && (this->m_reply->request().rawHeader("Accept")=="text/event-stream");
}

Response::~Response(){
    if(m_reply){
        delete m_reply;//delete the reply makes the server to clean up the hash
    }
}

void Response::fillObject(const QMetaObject* mo, void* obj,const QJsonObject& jsonObj, bool qobject)
{
    for(QString key : jsonObj.keys())
    {
        QByteArray rawkey = key.toUtf8();
        int propertyIndex = mo->indexOfProperty(rawkey.data());
        if(propertyIndex<0)
            continue;
        QMetaProperty mp = mo->property(propertyIndex);
        QVariant v;
        if(qobject){
            v = mp.read(static_cast<QObject*>(obj));
        }
        else{
            v = mp.readOnGadget(obj);
        }
        //we check we have that property
        if(v.isValid())
        {
            if(const QMetaObject* pmo = QMetaType::metaObjectForType(v.userType()))
            {
                QJsonObject pobjJson = jsonObj.value(key).toObject();
                fillObject(pmo,v.data(),pobjJson);
            }
            else
            {
                v=jsonObj.value(key).toVariant();
            }
            bool res;
            if(v.isNull()&& mp.type()==QVariant::String){
                v = QVariant::fromValue(QString());//null QString will fail if we dont do this
            }
            if(qobject){
                res = mp.write(static_cast<QObject*>(obj),v);
            }
            else{
                res = mp.writeOnGadget(obj,v);
            }
            if(!res){
                qWarning() <<"error writing to "<< rawkey<< v;
            }
        }
        else
        {
            qWarning() << "invalid : "<< v.typeName()<< rawkey;
        }
    }
}

void Response::loadFromJson(QByteArray data)
{

    QJsonParseError check;
    QJsonDocument doc = QJsonDocument::fromJson(data,&check);

    if(check.error){
        throw std::runtime_error(check.errorString().toStdString());
    }


    QJsonObject object = doc.object();
    fillObject(this->metaObject(), this, object,true);


}

void Response::reconnectStream()
{
    QTimer::singleShot(RECONNECT_DELAY,this,&Response::reconnectStreamDelayed);
}
void Response::reconnectStreamDelayed()
{
    QNetworkAccessManager* manager = m_reply->manager();
    //qDebug() << "RECONNECT TO "<<m_reply->request().url()<< m_reply->request().rawHeaderList()<< " POINTER "<< m_reply;
    QNetworkReply * reply = manager->get(QNetworkRequest(m_reply->request()));
    loadFromReply(reply);
}

int Response::getRateLimitLimit() {
    return m_rateLimitLimit;
}

int Response::getRateLimitRemaining() {
    return m_rateLimitRemaining;
}

int Response::getRateLimitReset() {
    return m_rateLimitReset;
}
bool Response::preprocessResponse(QNetworkReply *response)
{
    QNetworkReply::NetworkError errorCode= response->error();
    m_rateLimitLimit  =response->rawHeader("X-Ratelimit-Limit").toInt();
    m_rateLimitRemaining =response->rawHeader("X-Ratelimit-Remaining").toInt();
    m_rateLimitReset =response->rawHeader("X-Ratelimit-Reset").toInt();

    // Too Many Requests
    if (errorCode == 429) {
        int retryAfter = response->rawHeader("Retry-After").toInt();

        QString err = QString("too many request exception %1").arg(retryAfter);
        //throw std::runtime_error(err.toStdString());
        qDebug() << err;
        emit error();
        return false;
    }
    // Other errors
    if (errorCode >= 300) {
        //throw std::runtime_error(response->errorString().toStdString());
        qDebug() << response->errorString();
        emit error();
        return false;
    }
    return true;
}

void Response::processPartialResponse()
{
    QNetworkReply* response = (QNetworkReply*)sender();
    if(!preprocessResponse(response)){
        return;
    }

    m_pendingData.append(response->readAll());

    int searchIndex=0;
    int alreadyProcessed = 0;
    do{
        QByteArray pendingData = QByteArray::fromRawData(m_pendingData.data()+alreadyProcessed,m_pendingData.size()-alreadyProcessed);
        searchIndex = pendingData.indexOf("\n\n",searchIndex);

        if(searchIndex>=0){
            QByteArray data = QByteArray::fromRawData(pendingData.data(),searchIndex);
            try{
                int dataIndex = data.indexOf("data: ")+strlen("data: ");
                QByteArray entity = QByteArray::fromRawData(pendingData.data()+dataIndex,searchIndex-dataIndex);
                if(entity=="\"hello\"")
                {
                    alreadyProcessed+=searchIndex+2;
                    searchIndex=0;
                }
                else{
                    this->loadFromJson(entity);
                    alreadyProcessed+=searchIndex+2;
                    searchIndex=0;
                }
            }
            catch(std::runtime_error err)
            {
                //alreadyProcessed+=pendingData.indexOf("\n")+1;
                //searchIndex= 0;
                searchIndex++;//what happends if there is a \n\n inside a memo? we should keep searching without discarting data.
            }
        }
        else
            break;
        if(pendingData.isEmpty())
            break;
    }while(searchIndex>=0);
    m_pendingData.remove(0,alreadyProcessed);
    emit ready();
}
void Response::processResponse()
{
    QNetworkReply* response = (QNetworkReply*)sender();


    QByteArray entity = response->readAll();
    //qDebug() << "QUERY : " <<  response->request().url();
    //qDebug() << "RESPONSE : "<< QString::fromLatin1(entity);

    if(!preprocessResponse(response)){
        return;
    }

    // No content
    if (entity.isNull()) {
        //throw std::runtime_error("Response contains no content");
        //qDebug() << "Response contains no content";
        emit error();
        return;
    }
    try{
        this->loadFromJson(entity);
    }
    catch(std::runtime_error err)
    {
        return;
    }



    emit ready();
}
