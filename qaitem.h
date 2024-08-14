#ifndef QAITEM_H
#define QAITEM_H

#include <QWidget>
#include "revise.h"
#include <QFile>
namespace Ui {
class QAItem;
}

class QAItem : public QWidget
{
    Q_OBJECT

public:
    explicit QAItem(QWidget *parent = nullptr);
    virtual ~QAItem();

    void setQuestion(QString q);
    void setAnswer(QString a);

    QString getQuestion();
    QString getAnswer();

signals:
    void copyBtnClicked();
    void sendBtnClicked();
    void sizeChanged();
    void SendText(QString text);


private slots:
    void on_copyBtn_clicked();

    void on_sendBtn_clicked();

    void on_ReviseBtn_released();

    void ReviseOK();

private:
    Ui::QAItem *ui;
    Revise* Rev;
    QFile *revise_qaFile;
};

#endif // QAITEM_H
