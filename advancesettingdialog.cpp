#include "advancesettingdialog.h"
#include "ui_advancesettingdialog.h"
#include "util.h"
#include "QList"

AdvanceSettingDialog::AdvanceSettingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AdvanceSettingDialog),
    m_settings(Settings::singleton()),
    m_selectedHWNDRect(QColor(255, 165, 0), tr("AdvanceSettingDialog", "set ai automatic based on this hwnd state"))
{
    ui->setupUi(this);    

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ui->stopAutomateCbx->setChecked(m_settings->isStopAutomaticWhenHWNDNotActivate());

    ui->wndCmb->blockSignals(true);

    HWND lastActivatedHwnd = m_settings->hwndActivated();
    QString hwndExeAndTitleName = m_settings->hwndExeAndTitleName();

    int index = -1;
    int lastActivatedHwndIndex = -1;
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
            QVariant variantHwnd = QVariant::fromValue(reinterpret_cast<quintptr>(hwnd));

            ui->wndCmb->addItem(ite.value(), variantHwnd);

            index++;

            lastActivatedHwndIndex = index;
            m_settings->setHwndActivated(hwnd);
            m_settings->setHwndExeAndTitleName(m_wndMap[hwnd]);
        }
        }
    }

    if(lastActivatedHwndIndex != -1){
        ui->wndCmb->setCurrentIndex(lastActivatedHwndIndex);
    }
    else{
        QStringList parts = hwndExeAndTitleName.split(":", Qt::SkipEmptyParts);
        if (parts.size() == 2) {
            QString exeName = parts[0].replace("[", "").replace("]", "");
            QString titleName = parts[1];

            HWND wnd = find_window(window_search_mode::EXCLUDE_MINIMIZED, window_priority::WINDOW_PRIORITY_EXE, "", titleName, exeName);
            if(wnd != NULL && m_wndMap.contains(wnd)){
                QList<HWND> hwndList = m_wndMap.keys();
                int index = hwndList.indexOf(wnd);

                ui->wndCmb->setCurrentIndex(index);
            }
            // for(QMap<HWND, QString>::iterator ite = m_wndMap.begin();
            //      ite != m_wndMap.end();
            //      ite ++)
            // {
            //     if(ite.value()=="CCKF")
            //     {

            //         HWND wnd = ite.key();
            //         QList<HWND> hwndList = m_wndMap.keys();
            //         int index = hwndList.indexOf(wnd);

            //         ui->wndCmb->setCurrentIndex(index);
            //     }
            // }
        }
    }

    ui->wndCmb->blockSignals(false);

    ui->showOcrRectCheckBox->blockSignals(true);
    ui->showOcrTextCheckbox->blockSignals(true);

    ui->showOcrTextCheckbox->setChecked(m_settings->showOcrText());
    ui->showOcrRectCheckBox->setChecked(m_settings->showOcrRect());

    ui->showOcrRectCheckBox->blockSignals(false);
    ui->showOcrTextCheckbox->blockSignals(false);

}

AdvanceSettingDialog::~AdvanceSettingDialog()
{
    delete ui;
}

QRect AdvanceSettingDialog::getSelectedRect(){
    return m_selectedHWNDRect.getSelectedRect();
}

void AdvanceSettingDialog::on_listWidget_currentRowChanged(int currentRow)
{
    ui->stackedWidget->setCurrentIndex(currentRow);
}


void AdvanceSettingDialog::on_okBtn_clicked()
{
    close();
}


void AdvanceSettingDialog::on_showBoxCbx_clicked(bool checked)
{
    m_settings->setDisplayBoxText(checked);
}

void AdvanceSettingDialog::on_stopAutomateCbx_clicked(bool checked)
{
    m_settings->setStopAutomaticWhenHWNDNotActivate(checked);
}


void AdvanceSettingDialog::on_wndCmb_highlighted(int index)
{
    QVariant itemData = ui->wndCmb->itemData(index);
    HWND hwnd = reinterpret_cast<HWND>(itemData.value<quintptr>());

    QRect rt = getWindowRect(hwnd);

    m_selectedHWNDRect.updateSelectedRect(rt);
    m_selectedHWNDRect.setDrawText(tr("AdvanceSettingDialog", "set ai automatic based on this hwnd state") + " " + m_wndMap[hwnd]);
    m_selectedHWNDRect.show();
}


// void AdvanceSettingDialog::on_wndCmb_currentIndexChanged(int index)
// {
//     QVariant itemData = ui->wndCmb->itemData(index);
//     HWND hwnd = reinterpret_cast<HWND>(itemData.value<quintptr>());

//     m_settings->setHwndActivated(hwnd);
//     m_settings->setHwndExeAndTitleName(m_wndMap[hwnd]);
// }



void AdvanceSettingDialog::on_showOcrRectCheckBox_clicked(bool checked)
{
    m_settings->setShowOcrRect(checked);

    emit showOcrRectSettingChanged();
}


void AdvanceSettingDialog::on_showOcrTextCheckbox_clicked(bool checked)
{
    m_settings->setShowOcrText(checked);

    emit showOcrTextSettingChanged();
}


void AdvanceSettingDialog::on_wndCmb_activated(int index)
{
    QVariant itemData = ui->wndCmb->itemData(index);
    HWND hwnd = reinterpret_cast<HWND>(itemData.value<quintptr>());

    m_settings->setHwndActivated(hwnd);
    m_settings->setHwndExeAndTitleName(m_wndMap[hwnd]);
}

