#include "selectrect.h"
#include "ui_selectrect.h"
#include <qpainter.h>
#include <qpen.h>
#include <qevent.h>
#include <QTranslator>
#include <QBrush>
#include <QTimer>
#include <QScreen>
#include <QMessageBox>
#include <qdebug.h>

SelectRect::SelectRect(QColor color) :
    QDialog(nullptr),
    ui(new Ui::SelectRect),
    m_selectedRect(0, 0, 0, 0),
    m_bIsMousePressed(false),
    m_color(color)
{
    ui->setupUi(this);

    setWindowFlag(Qt::WindowStaysOnTopHint, true);
    setWindowFlag(Qt::FramelessWindowHint, true);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setMouseTracking(true);

    connect(this, SIGNAL(closeRequest()), this, SLOT(onCloseRequest()));

}

SelectRect::~SelectRect()
{
    delete ui;
}

QRect SelectRect::getSelectedRect(){
    return m_selectedRect;
}

void SelectRect::mousePressEvent(QMouseEvent *event){

    m_bIsMousePressed = true;

    m_startPoint = event->globalPos();

    QDialog::mousePressEvent(event);
}

void SelectRect::mouseReleaseEvent(QMouseEvent *event){
    QTimer::singleShot(100, [=](){
        QMessageBox msgBox(QMessageBox::Icon::Warning, tr("SelectRect", "ExitTitle"), tr("SelectRect", "ExitText"), QMessageBox::Yes |
                                        QMessageBox::Cancel, this);

        msgBox.setButtonText(QMessageBox::Yes, tr("SelectRect", "Yes"));
        msgBox.setButtonText(QMessageBox::Cancel, tr("SelectRect", "Cancel"));
        msgBox.exec();

        if(msgBox.result() == QMessageBox::Cancel){
            m_selectedRect = QRect(0,0,0,0);
            m_bIsMousePressed = false;
            return;
        }


        emit closeRequest();
    });

    QDialog::mouseReleaseEvent(event);
}

void SelectRect::mouseMoveEvent(QMouseEvent *event){  

    if(m_bIsMousePressed ){
        QPoint currentPosition = event->globalPos();

        m_selectedRect = QRect(m_startPoint, currentPosition);

        update();
    }

    QDialog::mouseMoveEvent(event);
}


void SelectRect::onCloseRequest()
{
    done(QDialog::Accepted); // 或者使用 QDialog::Rejected
}

void SelectRect::paintEvent(QPaintEvent *event){
    QPainter painter(this); // 创建一个画家对象，指定绘图设备为当前widget

    painter.fillRect(rect(), QColor(0, 0, 0, 50));

    // 设置画家的画笔属性，比如颜色、宽度等
    QPen pen(QBrush(m_color), 3); // 红色，宽度为3的画笔
    painter.setPen(pen);

    // 绘制一个矩形作为示例
    painter.drawRect(mapGlobalRectToLocal(m_selectedRect));

    // paint help text
    // 假设 painter 是已经初始化的 QPainter 实例，text 是要绘制的字符串
    QFont ft = font(); // 获取当前字体
    QFontMetrics fm(ft); // 创建一个 QFontMetrics 对象来测量字体

    // 使用 boundingRect 计算实际绘制该文本需要的高度
    QPen pen2(QBrush(QColor(255, 0, 0, 200)), 3); // 红色，宽度为3的画笔
    painter.setPen(pen2);
    QString text = tr("SelectRect", "please set rect, click ESC to quit");
    QRect textRect = fm.boundingRect(QRect(), Qt::AlignTop | Qt::AlignLeft, text);
    int w = textRect.width();
    int h = textRect.height();
    painter.drawText(QPoint((width() - w)/2, h + 20), text);

}

QRect SelectRect::mapGlobalRectToLocal(const QRect &globalRect)
{
    // 将全局坐标系下的矩形的左上角和右下角映射到widget的局部坐标系
    QPoint topLeftLocal = mapFromGlobal(globalRect.topLeft());
    QPoint bottomRightLocal = mapFromGlobal(globalRect.bottomRight());

    // 根据映射后的顶点构造新的局部坐标系下的矩形
    return QRect(topLeftLocal, bottomRightLocal);
}
