#ifndef REVISE_H
#define REVISE_H

#include <QDialog>

namespace Ui {
class Revise;
}

class Revise : public QDialog
{
    Q_OBJECT

public:
    explicit Revise(QWidget *parent = nullptr);
    ~Revise();
    QString GetReviseAns();
    void SetText(QString OriginAns);
signals:
    void ReviseOK();

private slots:
    void on_pushButton_released();

private:
    Ui::Revise *ui;
};

#endif // REVISE_H
