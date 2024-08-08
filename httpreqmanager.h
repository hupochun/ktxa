#ifndef HTTPREQMANAGER_H
#define HTTPREQMANAGER_H

#include <QObject>


class HttpReqManager : public QObject
{
    Q_OBJECT
public:
    HttpReqManager(QObject *parent = nullptr);


};

#endif // HTTPREQMANAGER_H
