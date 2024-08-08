#include "helpdialog.h"
#include "ui_helpdialog.h"

HelpDialog::HelpDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HelpDialog)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() &~Qt::WindowContextHelpButtonHint);
}

HelpDialog::~HelpDialog()
{
    delete ui;
}

void HelpDialog::on_okBtn_clicked()
{
    close();
}
