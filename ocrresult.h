#ifndef OCRRESULT_H
#define OCRRESULT_H

#include <QObject>
#include <qstring.h>
#include <QRect>
#include <QJsonObject>
#include <qjsonarray.h>

class OcrResult
{    
public:
    OcrResult();

    OcrResult(const OcrResult &other);

    static QList<OcrResult> parseFromJsonStr(QString jsonStr);
    static OcrResult parseFromJson(QJsonObject jsonObj);

    int box_id;
    QRect rect;
    QString text = 0;
    float confidence = 0.0f;

    bool isTime = false;
    bool isAnswer = false;
};

#endif // OCRRESULT_H
