#include "revise.h"
#include "ui_revise.h"

Revise::Revise(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Revise)
{
    ui->setupUi(this);
}

Revise::~Revise()
{
    delete ui;
}

QString Revise::GetReviseAns()
{
    return ui->textEdit->toPlainText();
}

void Revise::SetText(QString OriginAns)
{
    ui->textEdit->setText(OriginAns);
}

void Revise::on_pushButton_released()
{
    emit ReviseOK();
    this->close();
}

