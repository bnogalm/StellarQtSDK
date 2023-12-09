#include "federationserver.h"
#ifdef USE_CPPTOML
#include "../../external/cpptoml/include/cpptoml.h"
#endif
#include <QNetworkReply>
#include <QUrlQuery>

#include <QCoreApplication>
#include <stdexcept>

QNetworkAccessManager* FederationServer::s_httpClient = nullptr;
QHash<QString,FederationServer*>* FederationServer::s_cachedFederationServers = nullptr;
FederationServer::FederationServer()
{

}

QNetworkReply *FederationServer::buildReply(const QString& address)
{
    QStringList tokens = address.split("*");
    if (tokens.size() != 2) {
        throw std::runtime_error("MalformedAddressException");
    }

    QUrlQuery uriBuilder;
    uriBuilder.addQueryItem("type","name");
    uriBuilder.addQueryItem("q",address);

    QString uri = QString().append(m_serverUri.toString()).append("?").append(uriBuilder.toString());
    return FederationServer::httpClient().get(QNetworkRequest(uri));
}

QNetworkAccessManager& FederationServer::httpClient(){
    if(!FederationServer::s_httpClient)
        FederationServer::s_httpClient= new QNetworkAccessManager(qApp);
    return *FederationServer::s_httpClient;
}

bool FederationServer::isCacheActive(){
    return FederationServer::s_cachedFederationServers;
}

void FederationServer::activeCache(bool active)
{
    if(active && !s_cachedFederationServers){
        s_cachedFederationServers = new QHash<QString,FederationServer*>();
    }
    else if(!active && s_cachedFederationServers){
        clearCached();
        delete s_cachedFederationServers;
    }
}

void FederationServer::clearCached()
{
    if(s_cachedFederationServers){
        for(FederationServer * s : s_cachedFederationServers->values())
            s->deleteLater();
        s_cachedFederationServers->clear();
    }
}

FederationServer::FederationServer(QUrl serverUri, QString domain) {
    m_serverUri = serverUri;
    if (m_serverUri.scheme() != "https") {
        throw std::runtime_error("FederationServerInvalidException");
    }
    m_domain = domain;
}

FederationServer::FederationServer(QString serverUri, QString domain) {

    m_serverUri = QUrl(serverUri);
    if(!m_serverUri.isValid())
        throw std::runtime_error("invalid federation uri");

    m_domain = domain;
}



FederationServer *FederationServer::createForDomain(QString domain) {

    if(s_cachedFederationServers){
        FederationServer * cached = s_cachedFederationServers->value(domain,nullptr);
        if(cached)
            return cached;
    }
    QUrl stellarTomlUri(QString("https://%1/.well-known/stellar.toml").arg(domain));
    if(!stellarTomlUri.isValid()){
        throw std::runtime_error("url invalid");
    }



    QNetworkReply * reply = FederationServer::httpClient().get(QNetworkRequest(stellarTomlUri));

    FederationServer *federationServer = new FederationServer();
    federationServer->m_domain=domain;
    QObject::connect(reply, &QNetworkReply::finished, federationServer , &FederationServer::initialize);



    return federationServer;
}

bool FederationServer::valid() const
{
    return this->m_serverUri.isValid();
}

FederationResponse *FederationServer::resolveAddress(const QString &address) {
    if(!this->valid()){
        //delay request
        auto response = new FederationResponse(nullptr);

        connect(this,&FederationServer::ready,[this,address,response](){
            auto reply = buildReply(address);
            response->loadFromReply(reply);
        });
        connect(this,&FederationServer::error,response,&FederationResponse::error);
        return response;
    }
    auto reply = buildReply(address);
    return new FederationResponse(reply);
}

#ifdef USE_CPPTOML
//some helpers
class membuf : public std::basic_streambuf<char> {
public:
    membuf(const char *p, size_t l) {
        setg((char*)p, (char*)p, (char*)p + l);
    }
};
class memstream : public std::istream {
public:
    memstream(const char *p, size_t l) :
        std::istream(&_buffer),
        _buffer(p, l) {
        rdbuf(&_buffer);
    }

private:
    membuf _buffer;
};
#endif

//initalize the federation server from reply, reply is the sender()
void FederationServer::initialize()
{
    QNetworkReply * reply = static_cast<QNetworkReply*>(sender());
    reply->deleteLater();
    if(reply->error()>=300){
        emit error("Stellar Toml not found or invalid");
        return;
    }
    QByteArray data = reply->readAll();
#ifdef USE_CPPTOML
    memstream stream(data.data(),data.size());
    try{
        auto reader = cpptoml::parser(stream);
        auto config = reader.parse();

        auto val = config->get_as<std::string>("FEDERATION_SERVER");
        if (val)
        {
            std::string server = *val;

            this->m_serverUri = QUrl(QString::fromStdString(server));
            if(this->m_serverUri.isValid()){
                if(s_cachedFederationServers){
                    s_cachedFederationServers->insert(this->m_domain,this);
                    connect(this,&QObject::destroyed,[](QObject *obj){
                        s_cachedFederationServers->remove(static_cast<FederationServer*>(obj)->m_domain);
                    });
                }
                emit ready();
            }
            else
                emit error("Invalid federation server");

        }
        else
        {
            emit error("No federation server");

        }
    }
    catch(...)
    {
        emit error("Error parsing toml");
    }
#else
    //workaround until i find a working alternative in android.. or android decide to fix their sdk problems once
    QList<QByteArray> lines = data.split('\n');
    for(const QByteArray& line : lines){

        if(line.contains("FEDERATION_SERVER=")){
            QString textLine = QString::fromLatin1(line);
            textLine=textLine.trimmed();
            textLine.replace("FEDERATION_SERVER=","");
            int from = textLine.indexOf('"')+1;
            int to = textLine.lastIndexOf('"');
            if(to>from){
                QString address= textLine.mid(from,to-from);
                this->m_serverUri = QUrl(address);
                if(this->m_serverUri.isValid()){
                    emit ready();
                }
                else{
                    emit error("Invalid federation server");
                }
                return;
            }
            else{
                emit error("Error parsing toml");
                return;
            }
        }
    }
    emit error("No federation server");
#endif
}
