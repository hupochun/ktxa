#include "ocrdrawwidget.h"
#include "ui_ocrdrawwidget.h"
#include "QPainter"

OcrDrawWidget::OcrDrawWidget(QColor color) :
    QDialog(NULL),
    ui(new Ui::OcrDrawWidget),
    m_color(color),
    m_settings(Settings::singleton()),
    m_drawBorderSize(3)
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

    show();
}

OcrDrawWidget::~OcrDrawWidget()
{
    delete ui;
}

void OcrDrawWidget::updateOcrResult(QList<OcrResult> ocrResult){
    m_ocrResult = ocrResult;
}

void OcrDrawWidget::updateSelectedRect(QRect rect){
    setGeometry(rect);

    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    setMouseTracking(false);

    update();
}

void OcrDrawWidget::paintEvent(QPaintEvent *event){
    QPainter painter(this); // 创建一个画家对象，指定绘图设备为当前widget

    for(const auto &ocrResult : m_ocrResult){
        const QRect &rt = ocrResult.rect;

        if(m_settings->showOcrRect()){
            QColor color = QColor(63, 72, 204); // 蓝色;

            if(ocrResult.isTime){
                color = QColor(200, 191, 231); // 淡紫色
            }
            else if(!ocrResult.isAnswer){
                color = m_color;
            }
            // 设置画家的画笔属性，比如颜色、宽度等
            QPen pen(QBrush(color), m_drawBorderSize);
            painter.setPen(pen);

            painter.drawRect(rt);
        }

        if(m_settings->showOcrText()){
            // 设置画家的画笔属性，比如颜色、宽度等
            QPen pen(QBrush(m_color), m_drawBorderSize); // 红色，宽度为3的画笔
            painter.setPen(pen);

            painter.drawText(rt.left(), rt.bottom(), ocrResult.text);
        }
    }
}
