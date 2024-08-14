#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "helpdialog.h"
#include "Windows.h"
#include "qmessagebox.h"
#include "qtranslator.h"
#include "qtimer.h"
#include "qevent.h"
#include "qrect.h"
#include "QDateTime"
#include "qapplication.h"
#include "qcursor.h"
#include "qscreen.h"
#include "util.h"
#include "common_defs.h"
#include "QDebug"
#include "QClipboard"
#include "QJsonArray"
#include "QJsonObject"
#include "QJsonDocument"
#include "qthttprequestmanager.h"
#include "QMetaObject"
#include "QThread"
#include "QCursor"
#include "QDir"
#include "QFileDialog"
#include "QUrl"
#include "QDesktopServices"
#include "advancesettingdialog.h"
#include <QScrollBar>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_settings(Settings::singleton())
    , m_chatRect(QColor(255, 0, 0), tr("MainWindow", "OcrBox"))
    , m_textRect(QColor(0, 255, 0), tr("MainWindow", "TextBox"))
    , m_sendBtnRect(QColor(0, 0, 255), tr("MainWindow", "SendBtnBox"))
    , m_chatWndRect(QColor(255, 165, 0), tr("MainWindow", "set ai automatic based on this hwnd state"))
    , m_ocrDrawWidget(QColor(255, 0, 0))
    , m_aiModeGroup(this)

{
    ui->setupUi(this);
    m_userId = generateRandomString(16);

    // init qa history file
    QString historyFilePath = m_settings->historyDir();
    QDir dir(historyFilePath);
    if(!dir.exists()){
        bool bSuccess = dir.mkpath(historyFilePath);
        if(!bSuccess){
            QMessageBox msgBox(QMessageBox::Icon::Warning, tr("MainWindow", "Error"), tr("MainWindow", "mk history path:%1 failure").arg(historyFilePath), QMessageBox::Yes |
                                            QMessageBox::Cancel, this);

            msgBox.setButtonText(QMessageBox::Yes, tr("MainWindow", "Yes"));
            msgBox.exec();

            exit(-998);
        }
    }

    // 获取当前日期时间
   QDateTime currentDateTime = QDateTime::currentDateTime();

   // 将当前日期时间格式化为"2024-06-21 11:39:21"这样的格式（这里实际会是当前的日期和时间）
   QString formattedDateTime = currentDateTime.toString("yyyy_MM_dd HH_mm_ss");

   QString historyFile = historyFilePath + "/" + formattedDateTime + ".txt";
    m_qaFile = new QFile(historyFile);
    bool bIsOpenSuccess = m_qaFile->open(QIODevice::Append | QIODevice::Text);
    if(!bIsOpenSuccess){
        QMessageBox msgBox(QMessageBox::Icon::Warning, tr("MainWindow", "Error"), tr("MainWindow", "cannot open hisotry file:%1").arg(historyFile), QMessageBox::Yes |
                                        QMessageBox::Cancel, this);

        msgBox.setButtonText(QMessageBox::Yes, tr("MainWindow", "Yes"));
        msgBox.exec();

        exit(-997);
    }

//    m_aiModeGroup.setExclusive(true);
//    m_aiModeGroup.addAction(ui->action_manualMode);
//    m_aiModeGroup.addAction(ui->action_autoMode);

//    ui->action_manualMode->blockSignals(true);
//    ui->action_autoMode->blockSignals(true);
//    ui->action_manualMode->setChecked(m_settings->isManualMode());
//    ui->action_autoMode->setChecked(!m_settings->isManualMode());
//    ui->action_manualMode->blockSignals(false);
//    ui->action_autoMode->blockSignals(false);

    bool bIsManualMode = m_settings->isManualMode();
    ui->action_aiMode->blockSignals(true);
    ui->menu_aiMode->setTitle(bIsManualMode ? tr("MainWindow", "AIManualMode") : tr("MainWindow", "AIAutoMode"));
    ui->action_aiMode->setText(bIsManualMode ? tr("MainWindow", "SwitchToAIAutoMode") : tr("MainWindow", "SwitchToAIManualMode"));
    ui->action_aiMode->blockSignals(false);

    bool bIsOnTop = m_settings->isOnTop();
    ui->action_top->setChecked(bIsOnTop);
    if(bIsOnTop){
        QTimer::singleShot(1000, this, [this, bIsOnTop](){
            on_action_top_toggled(bIsOnTop);
        });
    }

    m_chatRect.updateSelectedRect(m_settings->chatRect());
    m_textRect.updateSelectedRect(m_settings->textRect());
    m_sendBtnRect.updateSelectedRect(m_settings->sendBtnRect());

    initWndSelectedRect();

    ui->action_startOrStop->setData(false);

    m_aiTimer.setInterval(AI_PROCESS_INTERVAL_MSEC);
    m_chatWndRectUpdateTimer.setInterval(1000);
    m_capWndTimer.setInterval(2000);
    connect(&m_aiTimer, SIGNAL(timeout()), this, SLOT(aiProcess()));
    connect(&m_chatWndRectUpdateTimer, SIGNAL(timeout()), this, SLOT(on_update_chatWndRect_timeout()));
    connect(&m_capWndTimer,SIGNAL(timeout()),this,SLOT(on_update_capWnd_timeout()));

    m_chatWndRectUpdateTimer.start();
    m_capWndTimer.start();

    connect(this, SIGNAL(exceedMaxQAItemSignal()), this, SLOT(on_exceedMaxQAItemSlot()));
    connect(this, SIGNAL(appendQAHistorySignal(QString)), this, SLOT(on_appendQAHistorySlot(QString)));
    connect(this, SIGNAL(getSameQuestionReqSignal(QString)), this, SLOT(on_getSameQuestionReqSlot(QString)));
    connect(this, SIGNAL(ocrResArrived(QList<OcrResult>)), this, SLOT(on_ocrResult_arrived(QList<OcrResult>)));
}

MainWindow::~MainWindow()
{
    m_qaFile->close();
    if(m_qaFile->size() == 0){
        m_qaFile->remove();
    }
    delete m_qaFile;
    m_chatRect.close();
    m_textRect.close();
    m_sendBtnRect.close();
    m_aiTimer.stop();
    m_chatWndRectUpdateTimer.stop();
    delete m_settings;
    delete ui;
}

void MainWindow::start(){
    m_aiTimer.start();

    ui->statusbar->showMessage(tr("MainWindow", "Already start!"), 10000);

    ui->menu_start->setTitle(tr("MainWindow", "Click to Stop"));
}

void MainWindow::stop(){
    m_aiTimer.stop();

    ui->statusbar->showMessage(tr("MainWindow", "Already stopped!"), 10000);

    ui->menu_start->setTitle(tr("MainWindow", "Click to Start"));
}

void MainWindow::on_action_help_triggered()
{
    HelpDialog dialog(this);
    dialog.exec();
}

void MainWindow::on_action_top_toggled(bool checked)
{
    m_settings->setOnTop(checked);

    WId win_id = winId();

    SetWindowPos((HWND)win_id, checked ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

void MainWindow::on_action_exit_triggered()
{
    QMessageBox msgBox(QMessageBox::Icon::Warning, tr("MainWindow", "ExitTitle"), tr("MainWindow", "ExitTitle"), QMessageBox::Yes |
                                    QMessageBox::Cancel, this);

    msgBox.setButtonText(QMessageBox::Yes, tr("MainWindow", "Yes"));
    msgBox.setButtonText(QMessageBox::Cancel, tr("MainWindow", "Cancel"));
    msgBox.exec();

    if(msgBox.result() == QMessageBox::Cancel){
        return;
    }

    close();
}

void MainWindow::on_action_set_chat_box_triggered()
{
    hide();
    QPoint globalPos = QCursor::pos();

    QScreen *screen = QApplication::screenAt(globalPos);
    if(!screen){
        QMessageBox::critical(this, tr("MainWindow", "GetScreenError.Title"), tr("MainWindow", "GetScreenError.Content"));

        show();
        return;
    }

    hideAllDrawRect();

    QApplication::processEvents();

    SelectRect dialog(QColor(255, 0, 0));
    dialog.setGeometry(screen->geometry());
    dialog.exec();

    QRect selectedRect = dialog.getSelectedRect();

    m_chatRect.updateSelectedRect(selectedRect);

    m_settings->setChatRect(selectedRect);

    showAllDrawRect();

    show();

}

void MainWindow::on_action_set_text_box_triggered()
{
    hide();
    QPoint globalPos = QCursor::pos();

    QScreen *screen = QApplication::screenAt(globalPos);
    if(!screen){
        QMessageBox::critical(this, tr("MainWindow", "GetScreenError.Title"), tr("MainWindow", "GetScreenError.Content"));

        show();
        return;
    }

    hideAllDrawRect();

    QApplication::processEvents();

    SelectRect dialog(QColor(0, 255, 0));
    dialog.setGeometry(screen->geometry());
    dialog.exec();

    QRect selectedRect = dialog.getSelectedRect();

    m_textRect.updateSelectedRect(selectedRect);

    m_settings->setTextRect(selectedRect);

    showAllDrawRect();

    show();
}

void MainWindow::on_action_set_sendBtn_box_triggered()
{
    hide();

    QPoint globalPos = QCursor::pos();

    QScreen *screen = QApplication::screenAt(globalPos);
    if(!screen){
        QMessageBox::critical(this, tr("MainWindow", "GetScreenError.Title"), tr("MainWindow", "GetScreenError.Content"));

        show();
        return;
    }

    hideAllDrawRect();

    QApplication::processEvents();

    SelectRect dialog(QColor(0, 0, 255));
    dialog.setGeometry(screen->geometry());
    dialog.exec();

    QRect selectedRect = dialog.getSelectedRect();

    m_sendBtnRect.updateSelectedRect(selectedRect);

    m_settings->setSendBtnRect(selectedRect);

    showAllDrawRect();

    show();
}

void MainWindow::aiProcess(){
    ui->statusbar->showMessage(tr("MainWindow", "startOcr"), 3000);

    if(m_settings->isStopAutomaticWhenHWNDNotActivate()){
        HWND hwnd = m_settings->hwndActivated();
        if(!isWndGetFocus(hwnd)){
            ui->statusbar->showMessage(tr("MainWindow", "lost hwnd focus and stop ai"), 3000);
            return;
        }
    }

    ocrReq();
}

void MainWindow::on_update_chatWndRect_timeout(){
    HWND hwnd = m_settings->hwndActivated();
    QRect chatWndRect = m_chatWndRect.getSelectedRect();
    if(hwnd == NULL){
        if(chatWndRect != QRect(0,0,0,0)){
            m_chatWndRect.updateSelectedRect(QRect(0,0,0,0));
        }

        hideAllDrawRect();

        qDebug() << "on_update_chatWndRect_timeout !hwnd hideAllDrawRect";

        return;
    }

    if(!isWndGetFocus(hwnd)){
        hideAllDrawRect();

        qDebug() << "on_update_chatWndRect_timeout !isWndGetFocus hideAllDrawRect";

        return;
    }

    QRect rt = getWindowRect(hwnd);
    if(chatWndRect != rt){
        m_chatWndRect.updateSelectedRect(rt);
    }

    showAllDrawRect();

    qDebug() << "on_update_chatWndRect_timeout showAllDrawRect";

}

bool isThisMainThread() {
    return QThread::currentThread() == QCoreApplication::instance()->thread();
}

void MainWindow::ocrReq(){
    // 1. ocr
    QRect selectedChatRect = m_chatRect.getSelectedRect();

    beforeCapture();
    QString imgBase64jpgData = getSelectedRectJPGImageBase64Data(selectedChatRect);
    afterCatpure();

    if(imgBase64jpgData.isEmpty()){
        return;
    }

    QJsonArray imagesArrayJson;
    imagesArrayJson.append(QJsonValue(imgBase64jpgData));

    QJsonObject reqData;
    reqData["images"] = imagesArrayJson;

    QJsonDocument jsonDoc(reqData);
    QString reqDataStr = jsonDoc.toJson(QJsonDocument::Compact); // 或者QJsonDocument::Indented格式化输出

    QtHttpRequestManager *httpReqManager = new QtHttpRequestManager(this);
    httpReqManager->POST(OCR_URL, reqDataStr, [this, httpReqManager](QString respData){

        httpReqManager->deleteLater();

        QList<OcrResult> ocrResultList = OcrResult::parseFromJsonStr(respData);

        ocrResultList = processOcrRes(ocrResultList);

        emit ocrResArrived(ocrResultList);

        QString ocrResult = getQuestionFromOcrRes(ocrResultList);
        //qDebug() << "ocrResp:" << QString(respData);

        qDebug() << "ocrResult question:" << ocrResult;

        if(ocrResult.isEmpty()){
            ui->statusbar->showMessage(tr("MainWindow", "ocr result is empty"), 3000);

            return;
        }
        else if(ocrResult.startsWith(tr("MainWindow", "inputing"))){
            ui->statusbar->showMessage(tr("MainWindow", "user is inputing"), 3000);

            return;
        }

        if(m_qaMap.contains(ocrResult)){
            emit getSameQuestionReqSignal(ocrResult);

            return;
        }

        m_qaMap[ocrResult] = "";
        // start ai answer
        llmReq(ocrResult);

    }, [this, httpReqManager](QString error1, QString error2){

        qDebug() << "ocr http error:" << error1 << error2;

        httpReqManager->deleteLater();

        ui->statusbar->showMessage(tr("MainWindow", "ocr error:%1").arg(error2), 3000);

    });
}

// http://192.168.100.20/app/7f617cb2-d96c-4ca2-80b3-af0867254204/develop
void MainWindow::llmReq(QString question){
     ui->statusbar->showMessage(tr("MainWindow", "llm is answering"), 3000);

    QJsonObject fileObj;
    fileObj["type"] = "image";
    fileObj["transfer_method"] = "remote_url";
    fileObj["url"] = "https://cloud.dify.ai/logo/logo-site.png";

    QJsonArray filesArray;
    filesArray.push_back(fileObj);

    QJsonObject reqData;
    reqData["inputs"] = QJsonObject();
    reqData["query"] = question;//QString(tr("MainWindow", "What are the llm answer of the %1?")).arg(question);
    reqData["response_mode"] = "blocking"; // streaming
    reqData["conversation_id"] = "";
    reqData["user"] = m_userId;
    reqData["files"] = filesArray;

    QMap<QString, QString> headers;
    headers["Authorization"] = QString("Bearer %1").arg(AI_API_KEY);
    headers["Accept"] = "application/json";
    headers["Content-Type"] = "application/json";

    QtHttpRequestManager *httpReqManager = new QtHttpRequestManager(this);
    httpReqManager->setHeaders(headers);

    QJsonDocument jsonDoc(reqData);
    QString reqDataStr = jsonDoc.toJson(QJsonDocument::Compact); // 或者QJsonDocument::Indented格式化输出

    httpReqManager->POST(AI_URL, reqDataStr, [this, httpReqManager, question](QString respData){

        QString answer = getllmAnswer(respData);
        m_qaMap[question] = answer;

        qDebug() << "llm answer:" << answer;

        QAItem *item = new QAItem(ui->listWidget);

        item->connect(item, SIGNAL(copyBtnClicked()), this, SLOT(onCopyAnswerBtnClicked()));
        item->connect(item, SIGNAL(sendBtnClicked()), this, SLOT(onSendAnswerBtnClicked()));
        item->adjustSize();
        // 使用QListWidgetItem包裹自定义Widget
        item->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        QListWidgetItem *listWidgetItem = new QListWidgetItem(ui->listWidget);
        connect(item, &QAItem::sizeChanged, [listWidgetItem, item]() {
            listWidgetItem->setSizeHint(item->sizeHint());
        });
        // 设置item不直接管理Widget的大小，以便自定义Widget能自由调整大小
        //listWidgetItem->setSizeHint(item->sizeHint());
        // 添加到列表中
        if (item != nullptr) {
            listWidgetItem->setSizeHint(item->size());
        }
        ui->listWidget->setItemWidget(listWidgetItem, item);
        item->setQuestion(question);
        item->setAnswer(answer);

        emit appendQAHistorySignal(question + "? " + answer);

        if(ui->listWidget->count() > MAX_QA_ITEM_COUNT){
            emit exceedMaxQAItemSignal();
        }

        if(m_settings->isStopAutomaticWhenHWNDNotActivate()){
            HWND hwnd = m_settings->hwndActivated();
            if(!isWndGetFocus(hwnd)){
                ui->statusbar->showMessage(tr("MainWindow", "lost hwnd focus and stop ai"), 3000);
                return;
            }
        }

        autoCopyAndSend(answer, !(m_settings->isManualMode()));

        httpReqManager->deleteLater();
    }, [this, httpReqManager](QString error1, QString error2){

        qDebug() << "llm http error:" << error1 << error2;

        httpReqManager->deleteLater();

        ui->statusbar->showMessage(tr("MainWindow", "llm error:%1").arg(error2), 3000);

    });
}

void MainWindow::on_startBtn_clicked()
{
    // check chatRect
    QRect selectedChatRect = m_chatRect.getSelectedRect();
    if(!isRectValid(selectedChatRect)){
        QMessageBox msgBox(QMessageBox::Icon::Warning, tr("MainWindow", "Error"), tr("MainWindow", "ChatRect.Error"), QMessageBox::Yes, this);

        msgBox.setButtonText(QMessageBox::Yes, tr("MainWindow", "Yes"));
        msgBox.exec();

        return;
    }

    // check textRect
    QRect selectedTextRect = m_textRect.getSelectedRect();
    if(!isRectValid(selectedTextRect)){
        QMessageBox msgBox(QMessageBox::Icon::Warning, tr("MainWindow", "Error"), tr("MainWindow", "TextRect.Error"), QMessageBox::Yes, this);

        msgBox.setButtonText(QMessageBox::Yes, tr("MainWindow", "Yes"));
        msgBox.exec();

        return;
    }

    // check sendBtnRect
    QRect selectedSendBtnRect = m_sendBtnRect.getSelectedRect();
    if(!isRectValid(selectedSendBtnRect)){
        QMessageBox msgBox(QMessageBox::Icon::Warning, tr("MainWindow", "Error"), tr("MainWindow", "SendBtnRect.Error"), QMessageBox::Yes, this);

        msgBox.setButtonText(QMessageBox::Yes, tr("MainWindow", "Yes"));
        msgBox.exec();

        return;
    }

    start();
}

void MainWindow::onCopyAnswerBtnClicked(){
    QAItem *item =  (QAItem*)sender();
    if(nullptr == item){
        return;
    }

    QString answer = item->getAnswer();

    autoCopyAndSend(answer, false);
}

void MainWindow::onSendAnswerBtnClicked(){
    QAItem *item =  (QAItem*)sender();
    if(nullptr == item){
        return;
    }

    QString answer = item->getAnswer();

    autoCopyAndSend(answer, true);
}

void MainWindow::autoCopyAndSend(QString answer, bool bShouldSend){
    if(answer.isEmpty()){
        ui->statusbar->showMessage(tr("MainWindow", "send answer is empty"), 3000);
        return;
    }

    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(answer);

    QPoint oldCursorPoint = QCursor::pos();

    QRect selectedTextRect = m_textRect.getSelectedRect();
    QPoint selectedTextRectCenterPoint = selectedTextRect.center();
    // if(bShouldSend){
        // copy
        SimulateMouseClick(selectedTextRectCenterPoint.x(), selectedTextRectCenterPoint.y());

        SimulatePaste();

        ui->statusbar->showMessage(tr("MainWindow", "copy to text rect success"), 3000);

        // send
        if(bShouldSend){
        QRect selectedSendBtnRect = m_sendBtnRect.getSelectedRect();
        QPoint sendBtnCenterPoint = selectedSendBtnRect.center();

        SimulateMouseClick(sendBtnCenterPoint.x(), sendBtnCenterPoint.y());

        ui->statusbar->showMessage(tr("MainWindow", "auto send success"), 3000);
    }

    SetCursorPos(oldCursorPoint.x(), oldCursorPoint.y());
}

void MainWindow::hideAllDrawRect(){
    m_chatRect.hide();
    m_textRect.hide();
    m_sendBtnRect.hide();
    m_chatWndRect.hide();
    m_ocrDrawWidget.hide();
}

void MainWindow::showAllDrawRect(){
    m_chatRect.show();
    m_textRect.show();
    m_sendBtnRect.show();
    m_chatWndRect.show();
    m_ocrDrawWidget.show();
}

void MainWindow::beforeCapture(){
    m_ocrDrawWidget.hide();

    QApplication::processEvents();
}

void MainWindow::afterCatpure(){
    m_ocrDrawWidget.show();
}

QList<OcrResult> MainWindow::processOcrRes(QList<OcrResult> &results){
    QList<OcrResult> finalResults;

    int horizonalTolerance = 100;
    QRect chatRect = m_chatRect.getSelectedRect();

    int chatRectX1 = 0;//chatRect.left();
    int chatRectX2 = chatRect.width();//chatRect.right();

    if(results.isEmpty()){
        return finalResults;
    }

    if(results.size() == 1){
        OcrResult result = results.at(0);
        QRect resultRect = result.rect;
        int resultRectX1 = resultRect.left();
        int resultRectX2 = resultRect.right();

        if(!result.isTime && (resultRectX1 > chatRectX1 + horizonalTolerance ||
                resultRectX2 < chatRectX2 - horizonalTolerance)){
            result.isAnswer = true;

            qDebug() << result.text << " is tagged as answer0.";

        }

        finalResults.push_back(result);

        return finalResults;
    }

    qDebug() << "start print original ocr result";
    for(int i = 0; i < results.size(); i++){
        OcrResult result = results.at(i);
        qDebug() << result.text;
    }

    qDebug() << "end print original ocr result";


    int leftLabelX1 = INT_MAX;
    int rightLabelX2 = INT_MIN;

    int lineAndlineHeighMinInterval = INT_MAX;

    // get min max border and lineAndlineHeight
    for(int i = 0; i < results.size(); i++){
        OcrResult curResult = results.at(i);
        QString curResultText = curResult.text;
        QRect curResultRect = curResult.rect;

        // 获取最左侧和最右侧的x值
        //if(!curResult.isTime){
            if(curResultRect.left() < leftLabelX1){
                leftLabelX1 = curResultRect.left();
            }
            else if(curResultRect.right() > rightLabelX2){
                rightLabelX2 = curResultRect.right();
            }
        //}

        // 获取到最小的行与行之间的高度差
        if(i < results.size() - 1){
            OcrResult nextResult = results.at(i + 1);
            QString nextResultText = nextResult.text;
            QRect nextResultRect = nextResult.rect;

            int lineAndlineHeightInterval = std::abs(nextResultRect.top() - curResultRect.bottom());
            if(lineAndlineHeightInterval < lineAndlineHeighMinInterval){
                lineAndlineHeighMinInterval = lineAndlineHeightInterval;
            }
        }
    }

    lineAndlineHeighMinInterval = 10;

    // filter
    QList<OcrResult> tmpResults;
    for(int i = 0; i < results.size(); i++){
        OcrResult result = results.at(i);
        QRect resultRect = result.rect;
        int resultRectX1 = resultRect.left();

        if(result.isTime){
            tmpResults.push_back(result);
            continue;
        }

        bool isAnswer = false;
        if(resultRectX1 > leftLabelX1 + horizonalTolerance){
            isAnswer = true;

            qDebug() << result.text << " is tagged as answer1.";

        }

        result.isAnswer = isAnswer;
        tmpResults.push_back(result);
    }

    // merge line And line smaller than lineAndlineHeighMinInterval
    QList<OcrResult> tmpResults2;
    int tolerance = 2;
    for(int i = 0; i < tmpResults.size(); i++){
        OcrResult curResult = tmpResults.at(i);
        QRect curResultRect = curResult.rect;
        int curResultRectBottomY2 = curResultRect.bottom();

        OcrResult newResult(curResult);

        if(curResult.isTime || i == tmpResults.size() - 1){
            tmpResults2.push_back(newResult);
            continue;
        }

        // 获取到最小的行与行之间的高度差
        for(int j = i + 1; j < tmpResults.size(); j++){
            OcrResult nextResult = tmpResults.at(j);
            QRect nextResultRect = nextResult.rect;
            int nextResultRectTopY1 = nextResultRect.top();

            i = j;

            if(nextResult.isTime){

                tmpResults2.push_back(newResult);

                tmpResults2.push_back(nextResult);

                break;
            }

            if(std::abs(nextResultRectTopY1 - curResultRectBottomY2) < lineAndlineHeighMinInterval + tolerance){
                // 行差小于阈值，合并段落
                newResult.text += nextResult.text;
                newResult.rect = newResult.rect.united(nextResultRect);

                if(j == tmpResults.size() - 1){
                    tmpResults2.push_back(newResult);
                }
            }
            else{

                tmpResults2.push_back(newResult);

                tmpResults2.push_back(nextResult);

                break;
            }
        }
    }

    // 再遍历一遍，如果是时间，且时间在右侧，那么下一个段落就是回答，而不是问题
    QList<OcrResult> tmpResults3;
    for(int i = 0; i < tmpResults2.size(); i++){
        OcrResult curResult = tmpResults2.at(i);
        QRect curResultRect = curResult.rect;
        int resultRectX1 = curResultRect.left();

        tmpResults3.push_back(curResult);

        if(i == tmpResults2.size() -1){
            break;
        }

        if(curResult.isTime){
            bool isAnswer = false;
            if(resultRectX1 > leftLabelX1 + horizonalTolerance){
                isAnswer = true;
            }

            OcrResult nextResult = tmpResults2.at(i + 1);
            if(isAnswer){
                nextResult.isAnswer = true;
                qDebug() << nextResult.text << " is tagged as answer.";

                i += 1;

                tmpResults3.push_back(nextResult);
            }

        }
    }

    // 反向遍历，只能有一个question
    qDebug() << "start print final ocr result";
    bool bIsFindQuestion = false;
    bool bHasQuestion = false;
    if(tmpResults3.size() > 0 &&
            !tmpResults3.at(tmpResults3.size() - 1).isAnswer &&
            !tmpResults3.at(tmpResults3.size() - 1).isTime){
        bHasQuestion = true;
    }

    for(int i = tmpResults3.size() - 1; i >= 0; i--){
        OcrResult result = tmpResults3.at(i);

        if(bHasQuestion){
            if(i != tmpResults3.size() - 1){
                result.isAnswer = true;
                qDebug() << result.text << " is tagged as answer4.";
            }
        }
        else{
            if(result.isTime){

            }
            else{
                if(bIsFindQuestion){
                    result.isAnswer = true;
                    qDebug() << result.text << " is tagged as answer3.";

                }
                else{
                    if(!result.isAnswer){
                        bIsFindQuestion = true;
                    }
                }
            }
        }

        finalResults.push_front(result);
    }

    qDebug() << "end print final ocr result";

    return finalResults;
}


QString MainWindow::getQuestionFromOcrRes(QList<OcrResult> &results){
    if(results.size() > 0 &&
            !results.at(results.size() - 1).isAnswer &&
            !results.at(results.size() - 1).isTime){
        return results.at(results.size() - 1).text;
    }

    return "";
}

void MainWindow::initWndSelectedRect(){
    int index = -1;
    int lastActivatedHwndIndex = -1;

    HWND lastActivatedHwnd = m_settings->hwndActivated();
    QString hwndExeAndTitleName = m_settings->hwndExeAndTitleName();

    QMap<HWND, QString> map = getWindowList(window_search_mode::INCLUDE_MINIMIZED);
    for(QMap<HWND, QString>::iterator ite = map.begin();
        ite != map.end();
        ite ++){
        HWND hwnd = ite.key();
        QVariant variantHwnd = QVariant::fromValue(reinterpret_cast<quintptr>(hwnd));

        index++;

        if(hwnd == lastActivatedHwnd){
            lastActivatedHwndIndex = index;
        }
    }

    if(lastActivatedHwndIndex != -1){
        QRect rt = getWindowRect(lastActivatedHwnd);
        m_chatWndRect.updateSelectedRect(rt);
        m_chatWndRect.setDrawText(tr("MainWindow", "set ai automatic based on this hwnd state") + " " + hwndExeAndTitleName);
        m_chatWndRect.show();

    }
    else{
        QStringList parts = hwndExeAndTitleName.split(":", Qt::SkipEmptyParts);
        if (parts.size() == 2) {
            QString exeName = parts[0].replace("[", "").replace("]", "");
            QString titleName = parts[1];

            HWND wnd = find_window(window_search_mode::INCLUDE_MINIMIZED, window_priority::WINDOW_PRIORITY_EXE, "", titleName, exeName);
            if(wnd != NULL && map.contains(wnd)){
                QRect rt = getWindowRect(wnd);

                m_chatWndRect.updateSelectedRect(rt);
                m_chatWndRect.setDrawText(tr("MainWindow", "set ai automatic based on this hwnd state") + " " + map[wnd]);
                m_chatWndRect.show();
            }
            else{

            }
        }
    }
}

void MainWindow::on_action_manualMode_triggered(bool checked)
{
    m_settings->setManualMode(true);
}


void MainWindow::on_action_autoMode_triggered(bool checked)
{
    m_settings->setManualMode(false);

}


void MainWindow::on_action_startOrStop_triggered()
{
    bool bIsStart = ui->action_startOrStop->data().toBool();

    !bIsStart ? start() : stop();

    ui->action_startOrStop->setText(bIsStart ? tr("MainWindow", "Start") : tr("MainWindow", "Stop"));

    ui->action_startOrStop->setData(!bIsStart);
}


void MainWindow::on_action_aiMode_triggered()
{
    bool bIsManualMode = m_settings->isManualMode();

    bIsManualMode = !bIsManualMode;

    m_settings->setManualMode(bIsManualMode);

    ui->menu_aiMode->setTitle(bIsManualMode ? tr("MainWindow", "AIManualMode") : tr("MainWindow", "AIAutoMode"));
    ui->action_aiMode->setText(bIsManualMode ? tr("MainWindow", "SwitchToAIAutoMode") : tr("MainWindow", "SwitchToAIManualMode"));
}


void MainWindow::on_action_open_history_dir_triggered()
{
    QString historyDir = m_settings->historyDir();
    QDesktopServices::openUrl(QUrl::fromLocalFile(historyDir));
}

void removeExcessItemsFromStart(QListWidget* listWidget, int itemCountLimit) {
    int currentItemCount = listWidget->count();

    if (currentItemCount > itemCountLimit) {
        // 需要移除的项目数
        int itemsToRemove = currentItemCount - itemCountLimit;

        // 从开始处移除项目
        for (int i = 0; i < itemsToRemove; ++i) {
            // 从列表开头移除一个项目
            QListWidgetItem* firstItem = listWidget->item(0);
            if (firstItem) {
                // 删除关联的自定义小部件以避免内存泄漏
                delete listWidget->itemWidget(firstItem);
                listWidget->removeItemWidget(firstItem); // 移除小部件与项的关联
                delete firstItem;
            }
        }

        qDebug() << "remove exceed qa item count:" << itemsToRemove;
    }
}

void trimMap(QMap<QString, QString>& map, int itemCountLimit) {
    int currentItemCount = map.size();

    if (currentItemCount > itemCountLimit) {
        // 计算需要移除的条目数量
        int itemsToRemove = currentItemCount - itemCountLimit;

        // 获取键的迭代器，从头开始
        QMap<QString, QString>::iterator it = map.begin();

        // 移除最早的itemsToRemove个条目
        for (int i = 0; i < itemsToRemove; ++i) {
            it = map.erase(it);
            // 如果it变为end()，我们需要将其重置为begin()以继续删除下一个（因为erase会返回下一个元素的迭代器）
            if (it == map.end()) {
                it = map.begin();
            }
        }
    }
}

void MainWindow::on_exceedMaxQAItemSlot(){
    removeExcessItemsFromStart(ui->listWidget, MAX_QA_ITEM_COUNT);
    trimMap(m_qaMap, MAX_QA_ITEM_COUNT);
}

void MainWindow::on_appendQAHistorySlot(QString text){
    if(!m_qaFile || !m_qaFile->isOpen()){
        qDebug() << "empty qa file or is not open";
        return;
    }

    try{
        // 创建一个QTextStream对象来写入文本
        QTextStream out(m_qaFile);
        // 写入一行内容，endl表示换行
        out << text << endl;
    }
    catch(std::exception e){
        qDebug() << "on_appendQAHistorySlot crash:" << e.what();
    }
}

void MainWindow::on_getSameQuestionReqSlot(QString question){
    int iCount = ui->listWidget->count();

    if(iCount <= 1){
        return;
    }

    if(!m_qaMap.contains(question)){
        return;
    }

    QString answer = m_qaMap[question];

    for(int i = 0; i < iCount; i++){
        QListWidgetItem* item = ui->listWidget->item(iCount - 1 - i);

        QWidget *widget = ui->listWidget->itemWidget(item);
        QAItem *qaItem = (QAItem*)widget;
        if(qaItem && qaItem->getQuestion() == question){
            if(i != 0){
               QListWidgetItem* item = ui->listWidget->item(iCount - 1 - i);
               if (item) {
                   // 删除关联的自定义小部件以避免内存泄漏
                   delete ui->listWidget->itemWidget(item);
                   ui->listWidget->removeItemWidget(item); // 移除小部件与项的关联
                   delete item;

                   // add new one
                   QAItem *item = new QAItem(ui->listWidget);

                   item->connect(item, SIGNAL(copyBtnClicked()), this, SLOT(onCopyAnswerBtnClicked()));
                   item->connect(item, SIGNAL(sendBtnClicked()), this, SLOT(onSendAnswerBtnClicked()));

                   // 使用QListWidgetItem包裹自定义Widget
                   QListWidgetItem *listWidgetItem = new QListWidgetItem(ui->listWidget);
                   // 设置item不直接管理Widget的大小，以便自定义Widget能自由调整大小
                   listWidgetItem->setSizeHint(item->sizeHint());
                   // 添加到列表中
                   ui->listWidget->setItemWidget(listWidgetItem, item);
                   item->setQuestion(question);
                   item->setAnswer(answer);
               }
            }

            return;
        }
    }
}

void MainWindow::on_action_advance_setting_triggered()
{
    m_chatWndRect.hide();
    AdvanceSettingDialog dlg(this);
    dlg.connect(&dlg, SIGNAL(showOcrRectSettingChanged()), this, SLOT(on_ocrSetting_changed()));
    dlg.connect(&dlg, SIGNAL(showOcrTextSettingChanged()), this, SLOT(on_ocrSetting_changed()));
    //dlg=new AdvanceSettingDialog(this);
    // dlg->connect(dlg, SIGNAL(showOcrRectSettingChanged()), this, SLOT(on_ocrSetting_changed()));
    // dlg->connect(dlg, SIGNAL(showOcrTextSettingChanged()), this, SLOT(on_ocrSetting_changed()));
    dlg.exec();

    QRect rt = dlg.getSelectedRect();
    if(rt != QRect(0,0,0,0)){
        m_chatWndRect.updateSelectedRect(rt);
    }
    m_chatWndRect.setDrawText(tr("MainWindow", "set ai automatic based on this hwnd state") + " " + m_settings->hwndExeAndTitleName());
    m_chatWndRect.show();
}

void MainWindow::on_ocrResult_arrived(QList<OcrResult> ocrResultList){
    try {
        m_ocrDrawWidget.updateSelectedRect(m_chatRect.getSelectedRect());
        m_ocrDrawWidget.updateOcrResult(ocrResultList);
    } catch (std::exception &e) {
        qDebug() << "on_ocrResult_arrived crash:" << e.what();
    }
}

void MainWindow::on_ocrSetting_changed(){
    m_ocrDrawWidget.update();
}


void MainWindow::on_update_capWnd_timeout()
{
    QMap<HWND, QString> m_wndMap;
    m_wndMap = getWindowList(window_search_mode::EXCLUDE_MINIMIZED);
    for(QMap<HWND, QString>::iterator ite = m_wndMap.begin();
         ite != m_wndMap.end();
         ite ++){
        QString str=ite.value();
        QStringList parts = str.split(":", Qt::SkipEmptyParts);
        if (parts.size() == 2) {
            QString exeName = parts[0].replace("[", "").replace("]", "");
            QString titleName = parts[1];

            if(exeName=="CCKF.exe"&&titleName=="CC客服")
            {
                HWND hwnd = ite.key();
                m_settings->setHwndActivated(hwnd);
                m_settings->setHwndExeAndTitleName(m_wndMap[hwnd]);
            }
        }
    }
}

