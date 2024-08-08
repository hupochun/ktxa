#ifndef ADVANCESETTINGDIALOG_H
#define ADVANCESETTINGDIALOG_H

#include <QDialog>
#include "settings.h"
#include "selectedrect.h"

namespace Ui {
class AdvanceSettingDialog;
}

class AdvanceSettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AdvanceSettingDialog(QWidget *parent = nullptr);
    ~AdvanceSettingDialog();

    QRect getSelectedRect();

signals:
    void showOcrRectSettingChanged();
    void showOcrTextSettingChanged();

protected slots:


private slots:
    void on_listWidget_currentRowChanged(int currentRow);

    void on_okBtn_clicked();

    void on_showBoxCbx_clicked(bool checked);

    void on_stopAutomateCbx_clicked(bool checked);

    void on_wndCmb_highlighted(int index);

    // void on_wndCmb_currentIndexChanged(int index);

    void on_showOcrRectCheckBox_clicked(bool checked);

    void on_showOcrTextCheckbox_clicked(bool checked);

    void on_wndCmb_activated(int index);

private:
    QMap<HWND, QString>     m_wndMap;
    SelectedRect            m_selectedHWNDRect;
    Settings                *m_settings;
    Ui::AdvanceSettingDialog *ui;
};

#endif // ADVANCESETTINGDIALOG_H
