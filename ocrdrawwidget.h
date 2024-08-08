#ifndef OCRDRAWWIDGET_H
#define OCRDRAWWIDGET_H

#include <QDialog>
#include <QColor>
#include <QRect>
#include <ocrresult.h>
#include "settings.h"

namespace Ui {
class OcrDrawWidget;
}

class OcrDrawWidget : public QDialog
{
    Q_OBJECT

public:
    explicit OcrDrawWidget(QColor color);
    ~OcrDrawWidget();

    void updateOcrResult(QList<OcrResult> ocrResult);

    void updateSelectedRect(QRect rect);

protected:
    virtual void paintEvent(QPaintEvent *event);

private:
    QList<OcrResult>    m_ocrResult;
    QColor              m_color;
    QString             m_drawText;
    int                 m_drawTextHeight;
    int                 m_drawBorderSize;
    QRect               m_selectedRect;

    Settings                *m_settings;

    Ui::OcrDrawWidget *ui;
};

#endif // OCRDRAWWIDGET_H
