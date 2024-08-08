#include "qaitem.h"
#include "ui_qaitem.h"
#include "QDateTime"

QAItem::QAItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QAItem)
{
    ui->setupUi(this);

    // 获取当前日期时间
   QDateTime currentDateTime = QDateTime::currentDateTime();

   // 将当前日期时间格式化为"2024-06-21 11:39:21"这样的格式（这里实际会是当前的日期和时间）
   QString formattedDateTime = currentDateTime.toString("yyyy-MM-dd HH:mm:ss");

   ui->timeLbl->setText(formattedDateTime);

}

QAItem::~QAItem()
{
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

