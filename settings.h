#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>
#include <QRect>
#include <Windows.h>

class Settings : public QObject
{
    Q_OBJECT
public:
    Settings(QObject *parent = nullptr);

    static Settings* singleton();

    // 自动模式：自动ocr，自动llm，自动复制发送
    // 手动模式：自动ocr，自动llm，手动点击复制发送按钮
    bool isManualMode();
    void setManualMode(bool bIsManual);

    bool isOnTop();
    void setOnTop(bool top);

    QRect chatRect();
    void setChatRect(QRect &rect);

    QRect textRect();
    void setTextRect(QRect &rect);

    QRect sendBtnRect();
    void setSendBtnRect(QRect &rect);

    QString historyDir();

    bool isDisplayBoxText();
    void setDisplayBoxText(bool display);

    bool isStopAutomaticWhenHWNDNotActivate();
    void setStopAutomaticWhenHWNDNotActivate(bool activate);

    HWND hwndActivated();
    QString hwndExeAndTitleName();

    void setHwndActivated(HWND hwnd);
    void setHwndExeAndTitleName(QString name);

    void setShowOcrText(bool show);
    void setShowOcrRect(bool show);
    bool showOcrText();
    bool showOcrRect();

private:
    QSettings m_settings;
};

#endif // SETTINGS_H
