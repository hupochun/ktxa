#include "qaitem.h"
#include "ui_qaitem.h"
#include <QDateTime>
#include "settings.h"
#include <QFile>
#include <QMessageBox>
#include <QDebug>

QAItem::QAItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QAItem)
{
    ui->setupUi(this);
    revise_qaFile=NULL;
    // 获取当前日期时间
    QDateTime currentDateTime = QDateTime::currentDateTime();

    // 将当前日期时间格式化为"2024-06-21 11:39:21"这样的格式（这里实际会是当前的日期和时间）
    QString formattedDateTime = currentDateTime.toString("yyyy-MM-dd HH:mm:ss");

    ui->timeLbl->setText(formattedDateTime);

}

QAItem::~QAItem()
{
    if(revise_qaFile!=NULL)
    {
        revise_qaFile->close();
        if(revise_qaFile->size() == 0){
            revise_qaFile->remove();
        }
        delete revise_qaFile;
    }
    delete ui;

}

void QAItem::setQuestion(QString q){
    ui->questionLbl->setText(q);
}

void QAItem::setAnswer(QString a){


    ui->answerLbl->setText(a);
    ui->answerLbl->adjustSize();
    this->adjustSize();
    emit sizeChanged();
    //this->layout()->setSizeConstraint(QLayout::SetFixedSize);
}

QString QAItem::getQuestion(){
    return ui->questionLbl->text();
}

QString QAItem::getAnswer(){
    return ui->answerLbl->text();
}

void QAItem::on_copyBtn_clicked()
{
    emit copyBtnClicked();
}


void QAItem::on_sendBtn_clicked()
{
    emit sendBtnClicked();
}





void QAItem::on_ReviseBtn_released()
{

    Rev=new Revise(this);

    connect(Rev,&Revise::ReviseOK,this,&QAItem::ReviseOK);
    Rev->SetText(this->getAnswer());
    Rev->show();
}

void QAItem::ReviseOK()
{
    ui->answerLbl->setText(Rev->GetReviseAns());
    QString text=ui->questionLbl->text() + "?" + ui->answerLbl->text();
    Settings* m_settings=Settings::singleton();
    QString historyFilePath = m_settings->historyDir();
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString formattedDateTime = currentDateTime.toString("yyyy_MM_dd HH_mm_ss");
    QString historyFile = historyFilePath + "/" + formattedDateTime + "_Revised" + ".txt";

    revise_qaFile = new QFile(historyFile);
    bool bIsOpenSuccess = revise_qaFile->open(QIODevice::Append | QIODevice::Text);
    if(!bIsOpenSuccess){
        QMessageBox msgBox(QMessageBox::Icon::Warning, tr("MainWindow", "Error"), tr("MainWindow", "cannot open hisotry file:%1").arg(historyFile), QMessageBox::Yes |
                                                                                                                                                        QMessageBox::Cancel, this);

        msgBox.setButtonText(QMessageBox::Yes, tr("MainWindow", "Yes"));
        msgBox.exec();

        exit(-997);
    }
    if(!revise_qaFile || !revise_qaFile->isOpen()){
        qDebug() << "empty qa file or is not open";
        return;
    }

    try{
        // 创建一个QTextStream对象来写入文本
        QTextStream out(revise_qaFile);
        // 写入一行内容，endl表示换行
        out << text << endl;
    }
    catch(std::exception e){
        qDebug() << "on_appendQAHistorySlotNew crash:" << e.what();
    }
}

