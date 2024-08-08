#ifndef QTHTTPREQUESTMANAGER_H
#define QTHTTPREQUESTMANAGER_H

#include <QObject>
#include <qnetworkaccessmanager.h>
#include <qnetworkreply.h>
#include <qnetworkrequest.h>
#include <qurlquery.h>
#include <qmap.h>
#include <QMetaEnum>
#include <functional>

class QtHttpRequestManager : public QObject
{
	Q_OBJECT
public:
    QtHttpRequestManager(QObject* parent = nullptr);
	~QtHttpRequestManager();

    void setHeaders(QMap<QString, QString> headers);

	void HEAD(const QString hostName);
	void PUT(const QString hostName, QMap<QString, QVariant> data,
		std::function<void(QString)> onSuccess,
		std::function<void(QString, QString)> onFailure);
	void POST(const QString hostName, 
        QString data,
		std::function<void(QString)> onSuccess,
		std::function<void(QString, QString)> onFailure);
	void GET(const QString hostName,
		std::function<void(QString)> onSuccess,
		std::function<void(QString, QString)> onFailure);
	void PATCH(const QString hostName,
		QString data,
		std::function<void(QString)> onSuccess,
		std::function<void(QString, QString)> onFailure);

protected:
	QUrlQuery constructPostData(QMap<QString, QVariant> data);
	QNetworkRequest constructNetworkRequest(const QString hostName, QMap<QString, QString> headers);

protected Q_SLOTS:
	void readyRead();
	void handleFinished(QNetworkReply *networkReply);
	void onError(QNetworkReply::NetworkError code);
	void onSslError(const QList<QSslError> &errors);

protected:
	QNetworkAccessManager* m_networkManager = nullptr;

	QMap<QString, QString> m_headers;

	QString m_host = "";

	std::function<void(QString)> m_success_func = nullptr;
	std::function<void(QString, QString)> m_failure_func = nullptr;
};

#endif // QTHTTPREQUESTMANAGER_H
