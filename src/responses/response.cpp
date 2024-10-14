#include "response.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaProperty>
#include <QNetworkRequest>
#include <QTimer>
#include <QTimerEvent>
#include <stdexcept>

#define RECONNECT_DELAY 1000

Response::Response(QNetworkReply *reply)    
    :m_lastErrorCode(QNetworkReply::NetworkError::NoError)
    ,m_status(0)
    ,m_timeoutTimerID(0)
    ,m_reconnectTimerID(0)
    ,m_retryTime(RECONNECT_DELAY)
    ,m_reply(nullptr)    
    ,m_rateLimitLimit(0)
    ,m_rateLimitRemaining(0)
    ,m_rateLimitReset(0)
{
    loadFromReply(reply);
}
void Response::clearReply(QObject* obj)
{
    if(this->m_reply==obj ){
        this->m_reply=nullptr;
    }
}

QNetworkReply::NetworkError Response::lastErrorCode() const
{
    return m_lastErrorCode;
}

int Response::getStatus() const
{
    return m_status;
}

void Response::restartTimeoutTimer()
{
    if(m_timeoutTimerID)
    {
        this->killTimer(m_timeoutTimerID);
    }
    m_timeoutTimerID= this->startTimer(STELLAR_QT_REPLY_TIMEOUT);
}
void Response::loadFromReply(QNetworkReply * reply)
{
    if(reply!=m_reply){
        if(m_reply){
            m_reply->deleteLater();
            m_reply=nullptr;
            this->reset();//we set properties to zero
        }

        if(reply){
            m_reply = reply;
            connect(m_reply,&QObject::destroyed,this,&Response::clearReply);
            if(this->isStreamingResponse()){
                connect(reply, &QNetworkReply::readyRead, this,&Response::processPartialResponse);
                connect(reply, &QNetworkReply::finished, this,&Response::reconnectStream);
                restartTimeoutTimer();
            }
            else{
                connect(reply, &QNetworkReply::finished, this,&Response::processResponse);
                restartTimeoutTimer();
            }
        }
    }
}

void Response::reset()
{
    m_lastErrorCode=QNetworkReply::NetworkError::NoError;
    const QMetaObject * mo = this->metaObject();
    for(int i=0;i<mo->propertyCount();i++)
    {
        if(mo->property(i).isWritable()){
            mo->property(i).write(this,QVariant());
        }
    }    
}

bool Response::isStreamingResponse() const
{
    return m_reply && (this->m_reply->request().rawHeader("Accept")=="text/event-stream");
}

Response::~Response(){
    if(m_reply){
        delete m_reply;
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

            int propertyUserTypeId = v.userType();

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            const QMetaType
#else
            const int
#endif
                variantMapType{qMetaTypeId<QVariantMap>()},
                propertyUserType{propertyUserTypeId};

            const QMetaObject* pmo = QMetaType::metaObjectForType(propertyUserTypeId);
            //if is a gadget/qobject and there is not a custom conversion function registered
            if(pmo && !QMetaType::hasRegisteredConverterFunction(variantMapType, propertyUserType)) {
                QJsonObject pobjJson = jsonObj.value(key).toObject();
                fillObject(pmo,v.data(),pobjJson);
            }
            else
            {
                v=jsonObj.value(key).toVariant();
            }
#ifdef STELLAR_QT_DEBUG_PARSER
            bool res;
#endif
            if(v.isNull()&& mp.type()==QVariant::String){
                v = QVariant::fromValue(QString());//null QString will fail if we dont do this
            }
            if(qobject){
#ifdef STELLAR_QT_DEBUG_PARSER
                res =
#endif
                mp.write(static_cast<QObject*>(obj),v);
            }
            else{
#ifdef STELLAR_QT_DEBUG_PARSER
                res =
#endif
                mp.writeOnGadget(obj,v);
            }
#ifdef STELLAR_QT_DEBUG_PARSER
            if(!res){
                qWarning() <<"error writing to "<< rawkey<< v;
            }
#endif
        }
#ifdef STELLAR_QT_DEBUG_PARSER
        else
        {
            qWarning() << "invalid : "<< v.typeName()<< rawkey;// we dont allow QVariant Q_PROPERTIES without initializing the type. You should verify that type match.
        }
#endif
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
    if(!m_reconnectTimerID)
    {
        m_reconnectTimerID= this->startTimer(m_retryTime);
    }
}
void Response::reconnectStreamDelayed()
{
    QNetworkAccessManager* manager = m_reply->manager();    
    QNetworkRequest request(m_reply->request());
    if(!m_lastID.isEmpty())
        request.setRawHeader("Last-Event-ID",m_lastID);
    request.setRawHeader("X-Client-Name", STELLAR_QT_SDK_CLIENT_NAME);
    request.setRawHeader("X-Client-Version", STELLAR_QT_SDK_CLIENT_VERSION);

    QNetworkReply * reply = manager->get(request);
    loadFromReply(reply);
}

int Response::getRateLimitLimit() const{
    return m_rateLimitLimit;
}

int Response::getRateLimitRemaining() const{
    return m_rateLimitRemaining;
}

int Response::getRateLimitReset() const{
    return m_rateLimitReset;
}

void Response::timerEvent(QTimerEvent *event)
{
    if(event->timerId()==m_reconnectTimerID)
    {
        this->reconnectStreamDelayed();
        this->killTimer(m_reconnectTimerID);
        m_reconnectTimerID=0;
    }
    else if(event->timerId()==m_timeoutTimerID)
    {
        this->killTimer(m_timeoutTimerID);
        if(this->m_reply->isRunning())
        {
            this->m_reply->close();
        }
        m_timeoutTimerID= 0;
    }

}
bool Response::preprocessResponse(QNetworkReply *response)
{
    QNetworkReply::NetworkError errorCode= response->error();
    m_lastErrorCode=errorCode;
    m_rateLimitLimit  =response->rawHeader("X-Ratelimit-Limit").toInt();
    m_rateLimitRemaining =response->rawHeader("X-Ratelimit-Remaining").toInt();
    m_rateLimitReset =response->rawHeader("X-Ratelimit-Reset").toInt();
    emit rateLimitChanged();
    // Too Many Requests
    if (errorCode == 429) {        
        int retryAfter = response->rawHeader("Retry-After").toInt();
        m_retryTime=qMax(retryAfter,m_retryTime);
        reconnectStream();
        //QString err = QString("too many request exception %1").arg(retryAfter);
        //throw std::runtime_error(err.toStdString());
        //qDebug() << err;

        emit error();

        return false;
    }
    // Other errors
    if (errorCode >= 300) {        
        //throw std::runtime_error(response->errorString().toStdString());
        //qDebug() << response->errorString();
        emit error();
        return false;
    }
    return true;
}

void Response::processPartialResponse()
{
    this->restartTimeoutTimer();
    QNetworkReply* response = static_cast<QNetworkReply*>(sender());
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
            //qDebug() << "DATA: "<< QString::fromLatin1(data);
            {
                //read retry time
                int retryIndex = data.indexOf("retry: ");
                if(retryIndex>=0)
                {
                    retryIndex+=static_cast<int>(strlen("retry: "));
                    int endLineRetry = data.indexOf('\n',retryIndex);
                    if(endLineRetry>0)
                    {
                        QByteArray retryTime = QByteArray::fromRawData(pendingData.data()+retryIndex,endLineRetry - retryIndex);
                        m_retryTime = qMax(retryTime.toInt(),RECONNECT_DELAY);
                        //qDebug() << "RETRY TIME: "<< retryTime << retryTime.toInt();
                    }
                }
                int idIndex = data.indexOf("id: ");
                if(idIndex>=0)
                {
                    idIndex+=static_cast<int>(strlen("id: "));
                    int endLineId = data.indexOf('\n',idIndex);
                    if(endLineId>0)
                    {
                        m_lastID = QByteArray(pendingData.data()+idIndex,endLineId - idIndex);
                        //qDebug() << "ID: "<< m_lastID << m_lastID.toLongLong();
                    }
                }
            }
            try{
                int dataIndex = data.indexOf("data: ");
                if(dataIndex>=0)
                {
                    dataIndex+=static_cast<int>(strlen("data: "));
                    QByteArray entity = QByteArray::fromRawData(pendingData.data()+dataIndex,searchIndex-dataIndex);
                    if(entity=="\"hello\"")
                    {
                        alreadyProcessed+=searchIndex+2;
                        searchIndex=0;
                    }
                    else if(entity=="\"byebye\""){
                        alreadyProcessed+=searchIndex+2;
                        searchIndex=0;
                    }
                    else{
                        this->loadFromJson(entity);
                        emit ready();
                        alreadyProcessed+=searchIndex+2;
                        searchIndex=0;
                    }
                }
            }
            catch(std::runtime_error& err)
            {
                Q_UNUSED(err)
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
    //emit ready();
}
void Response::processResponse()
{
    QNetworkReply* response = static_cast<QNetworkReply*>(sender());


    QByteArray entity;
    if(response->isOpen())
        entity= response->readAll();
#ifdef STELLAR_QT_DEBUG_NETWORK_REQUESTS
    qDebug() << "QUERY : " <<  response->request().url();
    qDebug() << "RESPONSE : "<< QString::fromLatin1(entity);
    qDebug() << "ERROR CODE : "<< response->error() << response->errorString();
#endif
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
    catch(std::runtime_error& err)
    {
        Q_UNUSED(err)
        return;
    }



    emit ready();
}
