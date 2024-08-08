#include "ocrresult.h"
#include "qdebug.h"
#include "QJsonParseError"
#include "QPoint"
#include "util.h"
#include "QDebug"


OcrResult::OcrResult()
{

}

OcrResult::OcrResult(const OcrResult &other){
    this->box_id = other.box_id;
    this->rect = other.rect;
    this->text = other.text;
    this->confidence = other.confidence;
    this->isAnswer = other.isAnswer;
    this->isTime = other.isTime;
}

QList<OcrResult> OcrResult::parseFromJsonStr(QString jsonStr){

    QList<OcrResult> resultList;

    QString resultText;
    QJsonParseError jsonError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonStr.toUtf8(), &jsonError);

    if (jsonError.error != QJsonParseError::NoError) {
        qDebug() << "JSON解析错误: " << jsonError.errorString();
        return resultList;
    }

    QString status = jsonDoc.object()["status"].toString();
    if(status != "000"){
        qDebug() << "OcrResult status error:" << status;
        return resultList;
    }

    QJsonArray results = jsonDoc.object()["results"].toArray();
    if(results.size() == 0){
        qDebug() << "empty results array";
        return resultList;
    }

    QJsonArray result = results[0].toArray();

    // 转换QJsonArray为QJsonObject的list
    QList<QJsonObject> boxes;
    for (const auto &value : result) {
        boxes.append(value.toObject());
    }

    // 使用std::sort进行排序
    std::sort(boxes.begin(), boxes.end(), compareBoxes);

    qDebug() << "++++++++++++start parse ocr result++++++++++++";

    for(const auto &box : boxes){
        OcrResult ocrResult = OcrResult::parseFromJson(box);
        if(ocrResult.text == ""){
            qDebug() << "parseFromJsonStr fatal error!!! emtpy text";
            continue;
        }

        qDebug() << ocrResult.text;

        resultList.push_back(ocrResult);
    }

    qDebug() << "++++++++++++end parse ocr result++++++++++++";


    return resultList;
}

static int iOcrCount = 0;

OcrResult OcrResult::parseFromJson(QJsonObject jsonObj){
    OcrResult res;

    int box_id = iOcrCount++;//jsonObj["box_id"].toInt();

    if(!jsonObj.contains("confidence") ||
            !jsonObj.contains("text") ||
            !jsonObj.contains("text_region")){
        return res;
    }

    float confidence = jsonObj["confidence"].toDouble();

    QJsonArray boxArray = jsonObj["text_region"].toArray();

    int x1 = boxArray[0].toArray()[0].toInt();
    int y1 = boxArray[0].toArray()[1].toInt();
    int x2 = boxArray[2].toArray()[0].toInt();
    int y2 = boxArray[2].toArray()[1].toInt();

    QRect rt(QPoint(x1, y1), QPoint(x2, y2));

    QString text = jsonObj["text"].toString();

    res.box_id = box_id;
    res.rect = rt;
    res.text = text;
    res.confidence = confidence;
    res.isTime = isValidTime(text);

    return res;

}


