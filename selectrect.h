#ifndef SELECTRECT_H
#define SELECTRECT_H

#include <QDialog>
#include <QRect>
#include <QColor>
#include <QPushButton>

namespace Ui {
class SelectRect;
}

class SelectRect : public QDialog
{
    Q_OBJECT

public:
    explicit SelectRect(QColor color);
    ~SelectRect();

    QRect getSelectedRect();

signals:
    void closeRequest();

protected slots:
    void onCloseRequest();

protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);

    QRect mapGlobalRectToLocal(const QRect &globalRect);

private:
    QColor                  m_color;
    bool                    m_bIsMousePressed;
    QPoint                  m_startPoint;
    QRect                   m_selectedRect;

    Ui::SelectRect *ui;
};

#endif // SELECTRECT_H
