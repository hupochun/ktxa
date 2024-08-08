#ifndef SELECTEDRECT_H
#define SELECTEDRECT_H

#include <QWidget>
#include <QRect>
#include <QBuffer>
#include <QColor>
#include <QEvent>
#include<QMouseEvent>

namespace Ui {
class SelectedRect;
}

class SelectedRect : public QWidget
{
    Q_OBJECT

public:
    explicit SelectedRect(QColor color, QString drawText);
    ~SelectedRect();

    void updateSelectedRect(QRect rect);
    QRect getSelectedRect();

    void setDrawText(QString text);

protected:
    virtual void paintEvent(QPaintEvent *event);


private:
    QColor              m_color;
    QString             m_drawText;
    int                 m_drawTextHeight;
    int                 m_drawBorderSize;
    QRect               m_selectedRect;
    Ui::SelectedRect    *ui;
};

#endif // SELECTEDRECT_H
