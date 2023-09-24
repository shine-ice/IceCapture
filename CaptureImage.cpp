//
// Created by Wsy on 2023/9/24.
//

#include "CaptureImage.h"

CaptureImage::CaptureImage(QWidget *parent)
        : QWidget{parent},
          isMousePress(false)
{
    initWindow();
    loadBackgroundPixmap();
}

void CaptureImage::initWindow()
{
    this->setMouseTracking(true);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    this->setWindowState(Qt::WindowActive | Qt::WindowFullScreen);
    //this->setAttribute(Qt::WA_TranslucentBackground);
}

void CaptureImage::loadBackgroundPixmap()
{
    // 多显示器截屏
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect scg = screen->virtualGeometry();
    screenPixmap = screen->grabWindow(QApplication::desktop()->winId(), scg.x(), scg.y(), scg.width(), scg.height());

    // 单个显示器
    //screenPixmap = QPixmap::grabWindow(QApplication::desktop()->winId());       // 抓取当前屏幕的图片

    screenWidth = screenPixmap.width();
    screenHeight = screenPixmap.height();

    // 多个显示器需要重新设置窗体大小，并移动到左上角
    this->resize(screenWidth, screenHeight);
    this->move(0, 0);
}

void CaptureImage::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        isMousePress = true;
        beginCapturePoint = event->pos();
    }

    return QWidget::mousePressEvent(event);
}

void CaptureImage::mouseMoveEvent(QMouseEvent *event)
{
    if (isMousePress)
    {
        endCapturePoint = event->pos();
        update();
    }
    else
    {
        // 获取窗口句柄
    }

    return QWidget::mouseMoveEvent(event);
}

void CaptureImage::mouseReleaseEvent(QMouseEvent *event)
{
    endCapturePoint = event->pos();
    isMousePress = false;
    return QWidget::mouseReleaseEvent(event);
}

void CaptureImage::paintEvent(QPaintEvent *event)
{
    painter.begin(this);

    QColor shadowColor = QColor(0, 0, 0, 100);                          // 设置遮罩颜色
    painter.setPen(QPen(Qt::blue, 1, Qt::SolidLine, Qt::FlatCap));      // 设置画笔
    painter.drawPixmap(0, 0, screenPixmap);                             // 绘制原始图片
    painter.fillRect(screenPixmap.rect(), shadowColor);                 // 绘制遮罩层

    if (isMousePress)
    {
        QRect selectedRect = getRect(beginCapturePoint, endCapturePoint);   // 截取的范围
        capturePixmap = screenPixmap.copy(selectedRect);                    // 从原始图片中截取的图片
        painter.drawPixmap(selectedRect.topLeft(), capturePixmap);          // 绘制截取的图片
        painter.drawRect(selectedRect);                                     // 绘制边框
    }

    painter.end();
}

void CaptureImage::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
    {
        close();
    }
    else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
    {
        emit signalCompleteCapture(capturePixmap);
        close();
    }
}

QRect CaptureImage::getRect(const QPoint &beginPoint, const QPoint &endPoint)
{
    int x, y, width, height;
    width = qAbs(beginPoint.x() - endPoint.x());
    height = qAbs(beginPoint.y() - endPoint.y());
    x = beginPoint.x() < endPoint.x() ? beginPoint.x() : endPoint.x();
    y = beginPoint.y() < endPoint.y() ? beginPoint.y() : endPoint.y();

    QRect selectedRect = QRect(x, y, width, height);
    // 避免宽或高为零时拷贝截图有误
    // 参考QQ截图，当选取截图宽或高为零时，默认为2
    if (selectedRect.width() == 0)
    {
        selectedRect.setWidth(1);
    }
    if (selectedRect.height() == 0)
    {
        selectedRect.setHeight(1);
    }

    return selectedRect;
}
