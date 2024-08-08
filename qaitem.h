#ifndef QAITEM_H
#define QAITEM_H

#include <QWidget>

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

private slots:
    void on_copyBtn_clicked();

    void on_sendBtn_clicked();

private:
    Ui::QAItem *ui;
};

#endif // QAITEM_H
