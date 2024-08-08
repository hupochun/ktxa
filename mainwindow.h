#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QMap>
#include "qaitem.h"
#include "settings.h"
#include "selectrect.h"
#include "selectedrect.h"
#include "ocrdrawwidget.h"
#include <QListWidgetItem>
#include <QActionGroup>
#include <QFile>
#include <QList>
#include <ocrresult.h>
#include "advancesettingdialog.h"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void start();
    void stop();

protected:
    void ocrReq();
    void llmReq(QString question);
    void autoCopyAndSend(QString answer, bool bShouldSend);
    void initWndSelectedRect();

    void hideAllDrawRect();
    void showAllDrawRect();
    void beforeCapture();
    void afterCatpure();

    QList<OcrResult> processOcrRes(QList<OcrResult> &results);
    QString getQuestionFromOcrRes(QList<OcrResult> &results);

signals:
    void exceedMaxQAItemSignal();
    void appendQAHistorySignal(QString);
    void getSameQuestionReqSignal(QString);
    void ocrResArrived(QList<OcrResult>);

private slots:
    void on_action_help_triggered();

    void on_action_top_toggled(bool checked);

    void on_action_exit_triggered();

    void on_action_set_chat_box_triggered();

    void on_action_set_sendBtn_box_triggered();

    void aiProcess();

    void on_update_chatWndRect_timeout();

    void on_startBtn_clicked();

    void on_action_set_text_box_triggered();

    void onCopyAnswerBtnClicked();
    void onSendAnswerBtnClicked();

    void on_action_manualMode_triggered(bool checked);

    void on_action_autoMode_triggered(bool checked);

    void on_action_startOrStop_triggered();

    void on_action_aiMode_triggered();

    void on_action_open_history_dir_triggered();

    void on_exceedMaxQAItemSlot();

    void on_appendQAHistorySlot(QString text);

    void on_getSameQuestionReqSlot(QString question);

    void on_action_advance_setting_triggered();

    void on_ocrResult_arrived(QList<OcrResult>);

    void on_ocrSetting_changed();

    void on_update_capWnd_timeout();

private:
    QTimer                  m_aiTimer;
    QTimer                  m_chatWndRectUpdateTimer;
    QTimer                  m_capWndTimer;
    SelectedRect            m_chatRect;
    SelectedRect            m_textRect;
    SelectedRect            m_sendBtnRect;
    SelectedRect            m_chatWndRect;
    OcrDrawWidget           m_ocrDrawWidget;
    QString                 m_userId;

    QFile                   *m_qaFile;

    QActionGroup            m_aiModeGroup;

    QMap<QString, QString>  m_qaMap;

    Settings                *m_settings;
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
