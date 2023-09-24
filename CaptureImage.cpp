//
// Created by Wsy on 2023/9/24.
//

#include "CaptureImage.h"

CaptureImage::CaptureImage(QWidget *parent)
        : QWidget{parent},
          currentCaptureState(InitCapture)
{
    initWindow();
    loadBackgroundPixmap();
}


void CaptureImage::initData() {
    currentCaptureState = InitCapture;
    beginCapturePoint.setX(0);
    beginCapturePoint.setY(0);
    endCapturePoint.setX(0);
    endCapturePoint.setY(0);

    currentSelectRect.setRect(0, 0, 0, 0);
    beginMovePoint.setX(0);
    beginMovePoint.setY(0);
    endMovePoint.setX(0);
    endMovePoint.setY(0);
}

void CaptureImage::initWindow()
{
    this->setMouseTracking(true);
    // 覆盖指定图片
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
        if (currentCaptureState == InitCapture)
        {
            currentCaptureState = BeginCaptureImage;
            beginCapturePoint = event->pos();
        }
        else if (isPressPointInSelectRect(event->pos()))
        {
            currentCaptureState = BeginMoveCaptureArea;
            setCursor(Qt::SizeAllCursor);
            beginMovePoint = event->pos();
        }
    }

    return QWidget::mousePressEvent(event);
}

void CaptureImage::mouseMoveEvent(QMouseEvent *event)
{
    // TODO 获取窗口句柄
    if (currentCaptureState == BeginCaptureImage)
    {
        endCapturePoint = event->pos();
        update();
    }
    else if (currentCaptureState == BeginMoveCaptureArea)
    {
        endMovePoint = event->pos();
        update();
    }

    // 根据鼠标是否在选中区域内设置鼠标样式;
    if (isPressPointInSelectRect(event->pos()))
    {
        setCursor(Qt::SizeAllCursor);
    }
    else if (!isPressPointInSelectRect(event->pos()) && currentCaptureState != BeginMoveCaptureArea)
    {
        setCursor(Qt::ArrowCursor);
    }

    return QWidget::mouseMoveEvent(event);
}

void CaptureImage::mouseReleaseEvent(QMouseEvent *event)
{
    if (currentCaptureState == BeginCaptureImage)
    {
        currentCaptureState = FinishCaptureImage;
        endCapturePoint = event->pos();
        update();
    }
    else if (currentCaptureState == BeginMoveCaptureArea)
    {
        currentCaptureState = FinishMoveCaptureArea;
        endMovePoint = event->pos();
        update();
    }

    return QWidget::mouseReleaseEvent(event);
}

void CaptureImage::paintEvent(QPaintEvent *event)
{
    painter.begin(this);

    QColor shadowColor = QColor(0, 0, 0, 100);                          // 设置遮罩颜色
    painter.setPen(QPen(Qt::blue, 1, Qt::SolidLine, Qt::FlatCap));      // 设置画笔
    painter.drawPixmap(0, 0, screenPixmap);                             // 绘制原始图片
    painter.fillRect(screenPixmap.rect(), shadowColor);                 // 绘制遮罩层

    switch (currentCaptureState) {
        case InitCapture:
        {
            break;
        }
        case BeginCaptureImage:
        case FinishCaptureImage:
        case BeginMoveCaptureArea:
        case FinishMoveCaptureArea:
        {
            currentSelectRect = getSelectRect();
            drawCaptureImage();
        }
    }

    painter.end();
}

void CaptureImage::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
    {
        initData();
        close();
    }
    else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
    {
        emit signalCompleteCapture(capturePixmap);
        initData();
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

bool CaptureImage::isPressPointInSelectRect(QPoint mousePressPoint) {
    QRect selectRect = getRect(beginCapturePoint, endCapturePoint);
    if (selectRect.contains(mousePressPoint))
    {
        return true;
    }

    return false;
}

// 根据当前截取状态获取当前选中的截图区域;
QRect CaptureImage::getSelectRect()
{
    if (currentCaptureState == BeginCaptureImage || currentCaptureState == FinishCaptureImage)
    {
        return getRect(beginCapturePoint, endCapturePoint);
    }
    else if (currentCaptureState == BeginMoveCaptureArea || currentCaptureState == FinishMoveCaptureArea)
    {
        return getMoveRect();
    }

    return QRect(0, 0, 0, 0);
}

// 绘制当前选中的截图区域;
void CaptureImage::drawCaptureImage()
{
    capturePixmap = screenPixmap.copy(currentSelectRect);                    // 从原始图片中截取的图片
    painter.drawPixmap(currentSelectRect.topLeft(), capturePixmap);          // 绘制截取的图片
    painter.drawRect(currentSelectRect);                                     // 绘制边框
}

// 获取移动后,当前选中的矩形;
QRect CaptureImage::getMoveRect()
{
    // 通过getMovePoint方法，先检查当前是否移动超出屏幕
    QPoint movePoint = getMovePoint();
    QPoint beginPoint = beginCapturePoint + movePoint;
    QPoint endPoint = endCapturePoint + movePoint;

    // 结束移动选区时，更新当前beginCapturePoint, endCapturePoint
    // 防止下一次操作时截取的图片有问题
    if (currentCaptureState == FinishMoveCaptureArea)
    {
        beginCapturePoint = beginPoint;
        endCapturePoint = endPoint;
        beginMovePoint = QPoint(0, 0);
        endMovePoint = QPoint(0, 0);
    }

    return getRect(beginPoint, endPoint);
}

QPoint CaptureImage::getMovePoint()
{
    QPoint movePoint = endMovePoint - beginMovePoint;
    QRect currentRect = getRect(beginCapturePoint, endCapturePoint);

    // 检查当前是否移动超出屏幕

    // 移动选区是否超出屏幕左边界
    if (currentRect.topLeft().x() + movePoint.x() < 0)
    {
        movePoint.setX(0 - currentRect.topLeft().x());
    }
    // 移动选区是否超出屏幕上边界
    if (currentRect.topLeft().y() + movePoint.y() < 0)
    {
        movePoint.setY(0 - currentRect.topLeft().y());
    }
    // 移动选区是否超出屏幕右边界
    if (currentRect.bottomRight().x() + movePoint.x() > screenWidth)
    {
        movePoint.setX(screenWidth - currentRect.bottomRight().x());
    }
    // 移动选区是否超出屏幕下边界
    if (currentRect.bottomRight().y() + movePoint.y() > screenHeight)
    {
        movePoint.setY(screenHeight - currentRect.bottomRight().y());
    }

    return movePoint;
}

