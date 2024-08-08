#include "selectedrect.h"
#include "ui_selectedrect.h"
#include <QPainter>
#include <QEvent>
#include <QPen>
#include <QScreen>
#include <qmessagebox.h>
#include <QTranslator>
#include <QFont>
#include <QFontMetrics>
#include "QPixmap"
#include "QImage"

SelectedRect::SelectedRect(QColor color, QString drawText) :
    QWidget(nullptr),
    ui(new Ui::SelectedRect),
    m_selectedRect(0,0,0,0),
    m_color(color),
    m_drawBorderSize(3),
    m_drawText(drawText)
{
    ui->setupUi(this);

    setWindowFlag(Qt::WindowStaysOnTopHint, true);
    setWindowFlag(Qt::FramelessWindowHint, true);
    setWindowFlag(Qt::WindowTransparentForInput, true);
    setWindowFlag(Qt::Tool, true);
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setMouseTracking(true);

    setGeometry(0, 0, 0, 0);

    // 假设 painter 是已经初始化的 QPainter 实例，text 是要绘制的字符串
    QFont ft = font(); // 获取当前字体
    QFontMetrics fm(ft); // 创建一个 QFontMetrics 对象来测量字体

    // 使用 boundingRect 计算实际绘制该文本需要的高度
    m_drawTextHeight = fm.boundingRect(QRect(), Qt::AlignTop | Qt::AlignLeft, m_drawText).height();

    show();
}

SelectedRect::~SelectedRect()
{
    delete ui;
}

void SelectedRect::setDrawText(QString text){
    m_drawText = text;
}

void SelectedRect::updateSelectedRect(QRect rect){

    m_selectedRect = rect;

    rect.setY(rect.y() - m_drawTextHeight - m_drawBorderSize);

    setGeometry(rect);

    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    setMouseTracking(false);

    update();
}

QRect SelectedRect::getSelectedRect(){
    return m_selectedRect;
}

void SelectedRect::paintEvent(QPaintEvent *event){
    QPainter painter(this); // 创建一个画家对象，指定绘图设备为当前widget

    if(m_selectedRect.width() == 0 ||
            m_selectedRect.height() == 0){
        return;
    }

    //painter.fillRect(rect(), QColor(0, 0, 0, 1));

    // 设置画家的画笔属性，比如颜色、宽度等
    QPen pen(QBrush(m_color), m_drawBorderSize); // 红色，宽度为3的画笔
    painter.setPen(pen);

    // 绘制一个矩形作为示例
    QRect rt = rect();
    rt.setY(rt.y() + m_drawTextHeight + m_drawBorderSize);

    painter.drawRect(rt);

    // 如果需要，可以在这里添加更多绘图指令，比如画线、圆、文字等
    painter.drawText(QPoint(0, m_drawTextHeight), m_drawText);
}

