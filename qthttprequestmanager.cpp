#include "qthttprequestmanager.h"
//#include <util/util.hpp>
#include "qsslerror.h"
#include "qlist.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariant>

// 函数用于将QMap<QString, QVariant>转换为JSON字符串
QString QMapToJSONString(const QMap<QString, QVariant> &data)
{
    QJsonObject jsonObj;

    // 遍历QMap并将每个键值对添加到QJsonObject中
    for (auto it = data.begin(); it != data.end(); ++it) {
        const QString &key = it.key();
        const QVariant &value = it.value();

        // 根据QVariant的类型添加到JSON对象中
        switch (value.type()) {
            case QVariant::Bool:
                jsonObj[key] = value.toBool();
                break;
            case QVariant::Double:
                jsonObj[key] = value.toDouble();
                break;
            case QVariant::Int:
            case QVariant::LongLong:
                jsonObj[key] = value.toLongLong();
                break;
            case QVariant::String:
                jsonObj[key] = value.toString();
                break;
            // 可以根据需要添加更多类型处理
            default:
                // 对于未处理的类型，可以选择跳过或以特定方式处理
                continue;
        }
    }

    // 将QJsonObject转换为QJsonDocument，再转换为JSON字符串
    QJsonDocument jsonDoc(jsonObj);
    return jsonDoc.toJson(QJsonDocument::Compact); // 或者QJsonDocument::Indented格式化输出
}

QtHttpRequestManager::QtHttpRequestManager(QObject *parent)
	: QObject(parent)
{
	// create network manager
	//blog(LOG_INFO, "QtHttpRequestManager newed.");
	m_networkManager = new QNetworkAccessManager(this);
	m_networkManager->setNetworkAccessible(QNetworkAccessManager::Accessible);
	connect(m_networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(handleFinished(QNetworkReply*)));

	// set HTTP headers
	m_headers["User-Agent"] = "QtHttpRequestManager 1.0";

	bool s = QSslSocket::supportsSsl();

	//blog(LOG_INFO, "check ssl is: %s", QSslSocket::supportsSsl() ? "supported" : "unsupported");
	//blog(LOG_INFO, "ssl version: %s", QSslSocket::sslLibraryVersionString().toStdString().c_str());
	//blog(LOG_INFO, "ssl build version: %s", QSslSocket::sslLibraryBuildVersionString().toStdString().c_str());
}

QtHttpRequestManager::~QtHttpRequestManager()
{
	delete m_networkManager;
}

void QtHttpRequestManager::setHeaders(QMap<QString, QString> headers){
    m_headers = headers;
}

///
/// \brief QtHttpRequestManager::HEAD
/// \param hostName
///
void QtHttpRequestManager::HEAD(const QString hostName)
{
	m_host = hostName;

	// step 1: create http request with custom headers
	QNetworkRequest request = this->constructNetworkRequest(hostName, m_headers);
	request.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/x-www-form-urlencoded");

	// step 2: HEAD to this resource
	QNetworkReply *reply = this->m_networkManager->head(request);

	// setup error handling
	connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onError(QNetworkReply::NetworkError)));

	connect(reply, SIGNAL(sslErrors(const QList<QSslError> &)), this, SLOT(onSslError(const QList<QSslError> &)));

	//reply->deleteLater();
	connect(reply, SIGNAL(readyRead()), this, SLOT(readyRead()));
}

///
/// \brief QtHttpRequestManager::PUT
/// \param hostName
///
void QtHttpRequestManager::PUT(const QString hostName, 
	QMap<QString, QVariant> data,
	std::function<void(QString)> onSuccess,
	std::function<void(QString, QString)> onFailure)
{
	m_success_func = onSuccess;
	m_failure_func = onFailure;

	m_host = hostName;

	// step 1: create http request with custom headers
	QNetworkRequest request = this->constructNetworkRequest(hostName, m_headers);
	request.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/x-www-form-urlencoded");

	// step 2: get PUT data
	QUrlQuery putData = this->constructPostData(data);

	// step 3: PUT to this resource
	QNetworkReply *reply = m_networkManager->put(request, putData.toString(QUrl::FullyEncoded).toUtf8());

	// setup error handling
	connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onError(QNetworkReply::NetworkError)));
	connect(reply, SIGNAL(sslErrors(const QList<QSslError> &)), this, SLOT(onSslError(const QList<QSslError> &)));

	//reply->deleteLater();
	connect(reply, SIGNAL(readyRead()), this, SLOT(readyRead()));
}

/// Create a HTTP POST request and setup signals/slots
/// \brief QtHttpRequestManager::POST
/// \param hostName
/// \param data
///
void QtHttpRequestManager::POST(const QString hostName, 
    QString data,
	std::function<void(QString)> onSuccess,
	std::function<void(QString, QString)> onFailure)
{
	m_success_func = onSuccess;
	m_failure_func = onFailure;

	m_host = hostName;

	// step 1: create http request with custom headers
	QNetworkRequest request = this->constructNetworkRequest(hostName, m_headers);
    request.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/json");

	// step 2: get POST data
    //QUrlQuery postData = this->constructPostData(data);

	// step 3: POST to this resource

    QNetworkReply *reply = m_networkManager->post(request, data.toUtf8());
	// setup error handling
	connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onError(QNetworkReply::NetworkError)));
	connect(reply, SIGNAL(sslErrors(const QList<QSslError> &)), this, SLOT(onSslError(const QList<QSslError> &)));

	//reply->deleteLater();
	connect(reply, SIGNAL(readyRead()), this, SLOT(readyRead()));
}

/// Create a HTTP GET request and setup signals/slots
/// \brief QtHttpRequestManager::GET
/// \param hostName
///
void QtHttpRequestManager::GET(const QString hostName,
	std::function<void(QString)> onSuccess,
	std::function<void(QString, QString)> onFailure)
{
	m_success_func = onSuccess;
	m_failure_func = onFailure;

	m_host = hostName;

	// step 1: create http request with custom User-Agent headers
	QNetworkRequest request = this->constructNetworkRequest(hostName, m_headers);
	request.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/x-www-form-urlencoded");

	// step 2: send http request
	QNetworkReply *reply = m_networkManager->get(request);

	// setup error handling
	connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onError(QNetworkReply::NetworkError)));
	connect(reply, SIGNAL(sslErrors(const QList<QSslError> &)), this, SLOT(onSslError(const QList<QSslError> &)));

	//reply->deleteLater();
	connect(reply, SIGNAL(readyRead()), this, SLOT(readyRead()));
}

void QtHttpRequestManager::PATCH(const QString hostName,
	QString data,
	std::function<void(QString)> onSuccess,
	std::function<void(QString, QString)> onFailure)
{
	m_success_func = onSuccess;
	m_failure_func = onFailure;

	m_host = hostName;

	// step 1: create http request with custom headers
	QNetworkRequest request = this->constructNetworkRequest(hostName, m_headers);
	request.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/json");

	// step 2: get POST data
	// QUrlQuery postData = this->constructPostData(data);

	// step 3: POST to this resource
	QNetworkReply *reply = m_networkManager->sendCustomRequest(request, "PATCH", data.toUtf8());
	// setup error handling
	connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onError(QNetworkReply::NetworkError)));
	connect(reply, SIGNAL(sslErrors(const QList<QSslError> &)), this, SLOT(onSslError(const QList<QSslError> &)));

	//reply->deleteLater();
	connect(reply, SIGNAL(readyRead()), this, SLOT(readyRead()));
}

void QtHttpRequestManager::readyRead()
{
	//blog(LOG_INFO, "host:%s ready to read.", m_host.toStdString().c_str());
}

/*
 *
 * SIGNALS/SLOTS
 *
 */
 /// HTTP network request has finished
 /// \brief QtHttpRequestManager::handleFinished
 /// \param networkReply
 ///
void QtHttpRequestManager::handleFinished(QNetworkReply *networkReply)
{
	// free later
	networkReply->deleteLater();

	// get HTTP status code
	qint32 httpStatusCode = networkReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

	// no error in request
	if (networkReply->error() == QNetworkReply::NoError)
	{
		// 200
		if (httpStatusCode >= 200 && httpStatusCode < 300) // OK
		{
			if(m_success_func)
				m_success_func(networkReply->readAll());
		}
		//else if (httpStatusCode >= 300 && httpStatusCode < 400) // 300 Redirect
		//{
		//	// Get new url, can be relative
		//	QUrl relativeUrl = networkReply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();

		//	// url can be relative, we use the previous url to resolve it
		//	QUrl redirectUrl = networkReply->url().resolved(relativeUrl);

		//	// redirect to new url
		//	networkReply->manager()->get(QNetworkRequest(redirectUrl));

		//	// maintain manager
		//	return;
		//}
		else
		{
			if (m_failure_func)
				m_failure_func("http request error", "error code is1:" + QString::number(httpStatusCode));	
		}
	}
	else
	{
		if (m_failure_func)
			m_failure_func("http request error", networkReply->errorString() + "error code is2:" + QString::number(httpStatusCode));
	}

	networkReply->manager()->deleteLater();
}

/// Error in HTTP request
/// \brief QtHttpRequestManager::onError
/// \param code
///
void QtHttpRequestManager::onError(QNetworkReply::NetworkError code)
{
	QString errorCode = QMetaEnum::fromType<QNetworkReply::NetworkError>().valueToKey(code);
	if (m_failure_func)
		m_failure_func("http request failure, code is:", 
			errorCode);

	//blog(LOG_ERROR, "http request error, host:%s, code:%s", m_host.toStdString().c_str(),
		//errorCode.toStdString().c_str());
}

void QtHttpRequestManager::onSslError(const QList<QSslError> &errors)
{
	for (int i = 0; i < errors.size(); i++)
	{
		QSslError sslError = errors.at(i);

		//blog(LOG_ERROR, "http ssl error:%s", sslError.errorString().toStdString().c_str());
	}
}

/*
 *
 *  HELPERS
 *
 */

 /// Create correct POST data
 /// \brief QtHttpRequestManager::constructPostData
 /// \param data
 /// \return
 ///
QUrlQuery QtHttpRequestManager::constructPostData(QMap<QString, QVariant> data)
{
	// Create POST/PUT data
	QUrlQuery postData;
	QMapIterator<QString, QVariant> iterator(data);

	// add all keys from map
	while (iterator.hasNext()) {
		iterator.next();
		postData.addQueryItem(iterator.key(), iterator.value().toString());
	}
	return postData;
}

/// Create network request
/// \brief QtHttpRequestManager::constructNetworkRequest
/// \param hostName
/// \param headers
/// \return
///
QNetworkRequest QtHttpRequestManager::constructNetworkRequest(const QString hostName, QMap<QString, QString> headers)
{
    // set HTTP headers
    headers["User-Agent"] = "QtHttpRequestManager 1.0";
	//qDebug() << QSslSocket::supportsSsl() << QSslSocket::sslLibraryBuildVersionString() << QSslSocket::sslLibraryVersionString();

	// create HTTP request and set hostname
	QNetworkRequest request;

	// 锟斤拷锟斤拷https锟斤拷锟斤拷前准锟斤拷锟斤拷锟斤拷;
	QSslConfiguration config;
	QSslConfiguration conf = request.sslConfiguration();
	conf.setPeerVerifyMode(QSslSocket::VerifyNone);
	conf.setProtocol(QSsl::TlsV1SslV3);
	request.setSslConfiguration(conf);

	request.setUrl(QUrl::fromUserInput(hostName));

	// add headers
	if (!headers.isEmpty()) {
		QMapIterator<QString, QString> iterator(headers);
		while (iterator.hasNext()) {
			iterator.next();
			request.setRawHeader(QByteArray::fromStdString(iterator.key().toStdString()), QByteArray::fromStdString(iterator.value().toStdString()));
		}
	}

	return request;
}
