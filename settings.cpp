#include "settings.h"
#include <QStandardPaths>

Settings::Settings(QObject *parent)
    :QObject(parent)
    ,m_settings("DYH", "ktxaAI")

{

}

Settings *Settings::singleton()
{
    static Settings* instance = 0;
    if (!instance)
    {
        instance = new Settings();
    }
    return instance;
}

bool Settings::isManualMode(){
    return m_settings.value("isManualMode", true).toBool();
}

void Settings::setManualMode(bool bIsManual){
    m_settings.setValue("isManualMode", bIsManual);
}

bool Settings::isOnTop(){
    return m_settings.value("top", true).toBool();
}

void Settings::setOnTop(bool top){
    m_settings.setValue("top", top);
}

QRect Settings::chatRect(){
    return m_settings.value("chatRect", QRect(0,0,0,0)).toRect();
}

void Settings::setChatRect(QRect &rect){
    m_settings.setValue("chatRect", rect);
}

QRect Settings::textRect(){
    return m_settings.value("textRect", QRect(0,0,0,0)).toRect();
}

void Settings::setTextRect(QRect &rect){
    m_settings.setValue("textRect", rect);
}

QRect Settings::sendBtnRect(){
    return m_settings.value("sendBtnRect", QRect(0,0,0,0)).toRect();
}

void Settings::setSendBtnRect(QRect &rect){
    m_settings.setValue("sendBtnRect", rect);
}

QString Settings::historyDir(){
    if(!m_settings.contains("historyDir")){
        QStringList historyDirs = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
        QString defaultPath = historyDirs.size() > 0 ? historyDirs.at(0) : "";

        m_settings.setValue("historyDir", defaultPath + "/history");
    }

    return m_settings.value("historyDir", "").toString();
}

bool Settings::isDisplayBoxText(){
    return m_settings.value("isDisplayBoxText", true).toBool();
}

void Settings::setDisplayBoxText(bool display){
    m_settings.setValue("isDisplayBoxText", display);
}

bool Settings::isStopAutomaticWhenHWNDNotActivate(){
    return m_settings.value("isStopAutomaticWhenHWNDNotActivate", true).toBool();
}

void Settings::setStopAutomaticWhenHWNDNotActivate(bool activate){
    m_settings.setValue("isStopAutomaticWhenHWNDNotActivate", activate);
}

HWND Settings::hwndActivated(){
    QVariant v = m_settings.value("hwndActivated", QVariant::fromValue(0));
    HWND hwnd = reinterpret_cast<HWND>(v.value<quintptr>());

    return hwnd;
}

QString Settings::hwndExeAndTitleName(){
    return m_settings.value("hwndExeAndTitleName", "").toString();
}

void Settings::setHwndActivated(HWND hwnd){
    QVariant variantHwnd = QVariant::fromValue(reinterpret_cast<quintptr>(hwnd));

    m_settings.setValue("hwndActivated", variantHwnd);
}

void Settings::setHwndExeAndTitleName(QString name){
    m_settings.setValue("hwndExeAndTitleName", name);
}

void Settings::setShowOcrText(bool show){
    m_settings.setValue("showOcrText", show);
}

void Settings::setShowOcrRect(bool show){
    m_settings.setValue("showOcrRect", show);
}

bool Settings::showOcrText(){
    return m_settings.value("showOcrText", false).toBool();
}

bool Settings::showOcrRect(){
    return m_settings.value("showOcrRect", false).toBool();
}
