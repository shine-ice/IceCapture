//
// Created by Wsy on 2023/9/24.
//

#include "CaptureImage.h"

CaptureImage::CaptureImage(QWidget *parent)
        : QWidget{parent},
          currentCaptureState(InitCapture)
{
    initWindow();
    initStretchRect();
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

// 初始化拖拽点
void CaptureImage::initStretchRect()
{
    currentStretchRectState = NotSelect;

    topLeftRect = QRect(0, 0, 0, 0);
    topRightRect = QRect(0, 0, 0, 0);
    bottomLeftRect = QRect(0, 0, 0, 0);
    bottomRightRect = QRect(0, 0, 0, 0);

    topCenterRect = QRect(0, 0, 0, 0);
    leftCenterRect = QRect(0, 0, 0, 0);
    rightCenterRect = QRect(0, 0, 0, 0);
    bottomCenterRect = QRect(0, 0, 0, 0);
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
    currentStretchRectState = getStretchRectState(event->pos());
    if (event->button() == Qt::LeftButton)
    {
        if (currentCaptureState == InitCapture)
        {
            currentCaptureState = BeginCaptureImage;
            beginCapturePoint = event->pos();
        }
        // 是否在拉伸的矩形中
        else if (currentStretchRectState != NotSelect)
        {
            currentCaptureState = BeginMoveStretchRect;
            // 当前鼠标在拖动选中区顶点时，设置鼠标当前状态
            setStretchCursorStyle(currentStretchRectState);
            beginMovePoint = event->pos();
        }
        // 鼠标在截图选中区域时，按下鼠标左键进行选区移动
        else if (isPressPointInSelectRect(event->pos()))
        {
            currentCaptureState = BeginMoveCaptureArea;
            beginMovePoint = event->pos();
        }
    }

    return QWidget::mousePressEvent(event);
}

void CaptureImage::mouseMoveEvent(QMouseEvent *event)
{
    // TODO 获取窗口句柄
    // 准备截图
    if (currentCaptureState == InitCapture)
    {
        update();
    }
    // 截图
    else if (currentCaptureState == BeginCaptureImage)
    {
        endCapturePoint = event->pos();
        update();
    }
    // 移动选区
    else if (currentCaptureState == BeginMoveCaptureArea)
    {
        endMovePoint = event->pos();
        update();
    }
    // 拖拽
    else if (currentCaptureState == BeginMoveStretchRect)
    {
        endMovePoint = event->pos();
        update();
        // 当前鼠标在拖动选中区顶点时，在鼠标未停止移动前，一直保持鼠标当前状态
        return QWidget::mouseMoveEvent(event);
    }

    // 根据鼠标是否在选中区域内设置鼠标样式
    StretchRectState stretchRectState = getStretchRectState(event->pos());
    if (stretchRectState != NotSelect)
    {
        setStretchCursorStyle(stretchRectState);
    }
    else if (isPressPointInSelectRect(event->pos()))
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
    // 截图
    if (currentCaptureState == BeginCaptureImage)
    {
        currentCaptureState = FinishCaptureImage;
        endCapturePoint = event->pos();
        update();
    }
        // 移动
    else if (currentCaptureState == BeginMoveCaptureArea)
    {
        currentCaptureState = FinishMoveCaptureArea;
        endMovePoint = event->pos();
        update();
    }
        // 拖拽
    else if (currentCaptureState == BeginMoveStretchRect)
    {
        currentCaptureState = FinishMoveStretchRect;
        endMovePoint = event->pos();
        update();
    }

    return QWidget::mouseReleaseEvent(event);
}

void CaptureImage::paintEvent(QPaintEvent *event)
{
    painter.begin(this);

    QColor shadowColor = QColor(0, 0, 0, 100);                          // 设置遮罩颜色
    painter.drawPixmap(0, 0, screenPixmap);                             // 绘制原始图片
    painter.fillRect(screenPixmap.rect(), shadowColor);                 // 绘制遮罩层

    switch (currentCaptureState) {
        case InitCapture:
        {
            // 绘制鼠标拖拽时选区矩形的右下顶点的放大图
            drawEndPointImage();
            break;
        }
        case BeginCaptureImage:
        {
            currentSelectRect = getSelectRect();
            drawCaptureImage();
            // 绘制鼠标拖拽时选区矩形的右下顶点的放大图
            drawEndPointImage();
            break;
        }
        case FinishCaptureImage:
        {
            currentSelectRect = getSelectRect();
            drawCaptureImage();
            break;
        }
        case BeginMoveCaptureArea:
        {
            currentSelectRect = getSelectRect();
            drawCaptureImage();
            // 绘制鼠标拖拽时选区矩形的右下顶点的放大图
            drawEndPointImage();
            break;
        }
        case FinishMoveCaptureArea:
        {
            currentSelectRect = getSelectRect();
            drawCaptureImage();
            break;
        }
        case BeginMoveStretchRect:
        {
            currentSelectRect = getSelectRect();
            drawCaptureImage();
            // 绘制鼠标拖拽时选区矩形的右下顶点的放大图
            drawEndPointImage();
            break;
        }
        case FinishMoveStretchRect:
        {
            currentSelectRect = getSelectRect();
            drawCaptureImage();
            break;
        }
        case FinishCapture:
        {
            currentSelectRect = getSelectRect();
            drawCaptureImage();
            break;
        }
        default:
            break;
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
    QRect selectRect{0, 0, 0, 0};
    if (currentCaptureState == BeginCaptureImage || currentCaptureState == FinishCaptureImage)
    {
        selectRect = getRect(beginCapturePoint, endCapturePoint);
    }
    else if (currentCaptureState == BeginMoveCaptureArea || currentCaptureState == FinishMoveCaptureArea)
    {
        selectRect = getMoveRect();
    }
    else if (currentCaptureState == BeginMoveStretchRect || currentCaptureState == FinishMoveStretchRect)
    {
        selectRect = getStretchRect();
    }

    return selectRect;
}

// 绘制当前选中的截图区域;
void CaptureImage::drawCaptureImage()
{
    capturePixmap = screenPixmap.copy(currentSelectRect);                   // 对选框区域进行截图
    painter.drawPixmap(currentSelectRect.topLeft(), capturePixmap);         // 在遮罩上绘制截取的图片
    painter.setPen(QPen(QColor(0, 180, 255), SELECT_RECT_BORDER_WIDTH));    // 设置画笔
    painter.drawRect(currentSelectRect);                                    // 绘制边框
    drawStretchRect();                                                      // 绘制拖拽点
    drawSelectRectInfo();                                                   // 绘制选中矩形框信息
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

// 获取当前鼠标位于哪一个拖拽顶点
StretchRectState CaptureImage::getStretchRectState(QPoint point)
{
    StretchRectState stretchRectState = NotSelect;

    if (topLeftRect.contains(point))
    {
        stretchRectState = TopLeftRect;
    }
    else if (topCenterRect.contains(point))
    {
        stretchRectState = TopCenterRect;
    }
    else if (topRightRect.contains(point))
    {
        stretchRectState = TopRightRect;
    }
    else if (rightCenterRect.contains(point))
    {
        stretchRectState = RightCenterRect;
    }
    else if (bottomRightRect.contains(point))
    {
        stretchRectState = BottomRightRect;
    }
    else if (bottomCenterRect.contains(point))
    {
        stretchRectState = BottomCenterRect;
    }
    else if (bottomLeftRect.contains(point))
    {
        stretchRectState = BottomLeftRect;
    }
    else if (leftCenterRect.contains(point))
    {
        stretchRectState = LeftCenterRect;
    }

    return stretchRectState;
}

// 设置鼠标停在拖拽点处的样式
void CaptureImage::setStretchCursorStyle(StretchRectState stretchRectState)
{
    switch (stretchRectState)
    {
        case NotSelect:
            setCursor(Qt::ArrowCursor);
            break;
        case TopLeftRect:
        case BottomRightRect:
            setCursor(Qt::SizeFDiagCursor);
            break;
        case TopRightRect:
        case BottomLeftRect:
            setCursor(Qt::SizeBDiagCursor);
            break;
        case LeftCenterRect:
        case RightCenterRect:
            setCursor(Qt::SizeHorCursor);
            break;
        case TopCenterRect:
        case BottomCenterRect:
            setCursor(Qt::SizeVerCursor);
            break;
        default:
            break;
    }
}

// 获取拖拽后的矩形选中区域
QRect CaptureImage::getStretchRect()
{
    QRect stretchRect;
    QRect currentRect = getRect(beginCapturePoint, endCapturePoint);
    switch (currentStretchRectState)
    {
        case NotSelect:
            stretchRect = getRect(beginCapturePoint, endCapturePoint);
            break;
        case TopLeftRect:
            stretchRect = getRect(currentRect.bottomRight(), endMovePoint);
            break;
        case TopRightRect:
            stretchRect = getRect(currentRect.bottomLeft(), endMovePoint);
            break;
        case BottomLeftRect:
            stretchRect = getRect(currentRect.topRight(), endMovePoint);
            break;
        case BottomRightRect:
            stretchRect = getRect(currentRect.topLeft(), endMovePoint);
            break;
        case LeftCenterRect:
        {
            QPoint beginPoint = QPoint(endMovePoint.x(), currentRect.topLeft().y());
            stretchRect = getRect(currentRect.bottomRight(), beginPoint);
        }
            break;
        case TopCenterRect:
        {
            QPoint beginPoint = QPoint(currentRect.topLeft().x(), endMovePoint.y());
            stretchRect = getRect(currentRect.bottomRight(), beginPoint);
        }
            break;
        case RightCenterRect:
        {
            QPoint endPoint = QPoint(endMovePoint.x(), currentRect.bottomRight().y());
            stretchRect = getRect(currentRect.topLeft(), endPoint);
        }
            break;
        case BottomCenterRect:
        {
            QPoint endPoint = QPoint(currentRect.bottomRight().y(), endMovePoint.y());
            stretchRect = getRect(currentRect.topLeft(), endPoint);
        }
            break;
        default:
            stretchRect = getRect(beginCapturePoint, endCapturePoint);
            break;
    }

    // 拖动结束更新beginCapturePoint, endCapturePoint
    if (currentCaptureState == FinishMoveStretchRect)
    {
        beginCapturePoint = stretchRect.topLeft();
        endCapturePoint = stretchRect.bottomRight();
    }

    return stretchRect;
}

// 绘制选中矩形各拖拽点小矩形;
void CaptureImage::drawStretchRect()
{
    QColor color = QColor(0, 174, 255);
    // 四个角坐标;
    QPoint topLeft = currentSelectRect.topLeft();
    QPoint topRight = currentSelectRect.topRight();
    QPoint bottomLeft = currentSelectRect.bottomLeft();
    QPoint bottomRight = currentSelectRect.bottomRight();
    // 四条边中间点坐标;
    QPoint leftCenter = QPoint(topLeft.x(), (topLeft.y() + bottomLeft.y()) / 2);
    QPoint topCenter = QPoint((topLeft.x() + topRight.x()) / 2, topLeft.y());
    QPoint rightCenter = QPoint(topRight.x(), leftCenter.y());
    QPoint bottomCenter = QPoint(topCenter.x(), bottomLeft.y());

    topLeftRect = QRect(topLeft.x() - STRETCH_RECT_WIDTH / 2, topLeft.y() - STRETCH_RECT_HEIGHT / 2, STRETCH_RECT_WIDTH, STRETCH_RECT_HEIGHT);
    topRightRect = QRect(topRight.x() - STRETCH_RECT_WIDTH / 2, topRight.y() - STRETCH_RECT_HEIGHT / 2, STRETCH_RECT_WIDTH, STRETCH_RECT_HEIGHT);
    bottomLeftRect = QRect(bottomLeft.x() - STRETCH_RECT_WIDTH / 2, bottomLeft.y() - STRETCH_RECT_HEIGHT / 2, STRETCH_RECT_WIDTH, STRETCH_RECT_HEIGHT);
    bottomRightRect = QRect(bottomRight.x() - STRETCH_RECT_WIDTH / 2, bottomRight.y() - STRETCH_RECT_HEIGHT / 2, STRETCH_RECT_WIDTH, STRETCH_RECT_HEIGHT);

    leftCenterRect = QRect(leftCenter.x() - STRETCH_RECT_WIDTH / 2, leftCenter.y() - STRETCH_RECT_HEIGHT / 2, STRETCH_RECT_WIDTH, STRETCH_RECT_HEIGHT);
    topCenterRect = QRect(topCenter.x() - STRETCH_RECT_WIDTH / 2, topCenter.y() - STRETCH_RECT_HEIGHT / 2, STRETCH_RECT_WIDTH, STRETCH_RECT_HEIGHT);
    rightCenterRect = QRect(rightCenter.x() - STRETCH_RECT_WIDTH / 2, rightCenter.y() - STRETCH_RECT_HEIGHT / 2, STRETCH_RECT_WIDTH, STRETCH_RECT_HEIGHT);
    bottomCenterRect = QRect(bottomCenter.x() - STRETCH_RECT_WIDTH / 2, bottomCenter.y() - STRETCH_RECT_HEIGHT / 2, STRETCH_RECT_WIDTH, STRETCH_RECT_HEIGHT);

    painter.fillRect(topLeftRect, color);
    painter.fillRect(topRightRect, color);
    painter.fillRect(bottomLeftRect, color);
    painter.fillRect(bottomRightRect, color);
    painter.fillRect(leftCenterRect, color);
    painter.fillRect(topCenterRect, color);
    painter.fillRect(rightCenterRect, color);
    painter.fillRect(bottomCenterRect, color);
}

// 绘制选中矩形信息宽高
void CaptureImage::drawSelectRectInfo()
{
    int posX, posY;
    QPoint topLeftPoint = currentSelectRect.topLeft();

    posX = topLeftPoint.x() + SELECT_RECT_BORDER_WIDTH;
    if (topLeftPoint.y() > SELECT_RECT_INFO_HEIGHT)
    {
        posY = topLeftPoint.y() - SELECT_RECT_INFO_HEIGHT - SELECT_RECT_BORDER_WIDTH;
    }
    else
    {
        posY = topLeftPoint.y() + SELECT_RECT_BORDER_WIDTH;
    }
    topLeftPoint = QPoint(posX, posY);

    QColor backColor = QColor(0, 0, 0, 160);
    painter.fillRect(QRect(topLeftPoint, QSize(SELECT_RECT_INFO_WIDTH, SELECT_RECT_INFO_HEIGHT)), backColor);

    // 当前选中矩形的宽高信息;
    QString selectRectSizeInfo = QString("%1 * %2").arg(currentSelectRect.width()).arg(currentSelectRect.height());
    int fontWidth = this->fontMetrics().horizontalAdvance(selectRectSizeInfo);
    painter.setPen(QPen(Qt::white));
    painter.drawText(QPoint(topLeftPoint.x() + (SELECT_RECT_INFO_WIDTH - fontWidth) / 2, topLeftPoint.y() + 14), selectRectSizeInfo);
}

// 绘制鼠标拖拽时选区矩形的右下顶点的放大图
void CaptureImage::drawEndPointImage()
{
    int posX, posY;
    QPoint topLeftPoint = QCursor::pos();

    // 5、25 分别为鼠标所在点距离放大图的X轴、Y轴距离;
    //当放大图片区域超出右边屏幕时;
    if (topLeftPoint.x() + END_POINT_RECT_WIDTH + 5 > screenWidth)
    {
        // 这里暂时未考虑到双屏幕（多屏幕）;
        if (topLeftPoint.x() > screenWidth)
        {
            posX = screenWidth - END_POINT_RECT_WIDTH - 5;
        }
        else
        {
            posX = topLeftPoint.x() - END_POINT_RECT_WIDTH - 5;
        }
    }
    else
    {
        posX = topLeftPoint.x() + 5;
    }

    // 当放大图片区域超出屏幕下方时;
    if (topLeftPoint.y() + END_POINT_RECT_HEIGHT + 25 > screenHeight)
    {
        posY = topLeftPoint.y() - END_POINT_RECT_HEIGHT - 25;
    }
        // 当鼠标未屏幕下方，正常显示时;
    else
    {
        posY = topLeftPoint.y() + 25;
    }

    topLeftPoint = QPoint(posX, posY);

    // 绘制放大图;
    QPixmap endPointImage = screenPixmap.copy(QRect(QCursor::pos().x() - 15, QCursor::pos().y() - 11, 30, 22)).scaled(END_POINT_RECT_WIDTH, END_POINT_IMAGE_HEIGHT);
    painter.drawPixmap(topLeftPoint, endPointImage);

    // 绘制十字坐标;
    painter.setPen(QPen(QColor(0, 180, 255 , 90), 4));
    // 竖线;
    painter.drawLine(QPoint(topLeftPoint.x() + END_POINT_RECT_WIDTH / 2, topLeftPoint.y() + 2), QPoint(topLeftPoint.x() + END_POINT_RECT_WIDTH / 2, topLeftPoint.y() + END_POINT_IMAGE_HEIGHT - 2));
    // 横线;
    painter.drawLine(QPoint(topLeftPoint.x() + 2 , topLeftPoint.y() + END_POINT_IMAGE_HEIGHT / 2), QPoint(topLeftPoint.x() + END_POINT_RECT_WIDTH - 2 , topLeftPoint.y() + END_POINT_IMAGE_HEIGHT / 2));

    painter.setPen(QPen(Qt::white, 3));
    painter.drawRect(QRect(QPoint(topLeftPoint.x() + 1 , topLeftPoint.y() + 1), QSize(END_POINT_RECT_WIDTH - 2, END_POINT_IMAGE_HEIGHT - 2)));
    painter.setPen(QPen(Qt::black, 1));
    painter.drawRect(QRect(topLeftPoint, QSize(END_POINT_RECT_WIDTH, END_POINT_IMAGE_HEIGHT)));

    // 绘制放大图信息;
    topLeftPoint = QPoint(topLeftPoint.x(), topLeftPoint.y() + END_POINT_IMAGE_HEIGHT);
    QColor backColor = QColor(0, 0, 0, 160);
    painter.fillRect(QRect(topLeftPoint, QSize(END_POINT_RECT_WIDTH, END_POINT_RECT_HEIGHT - END_POINT_IMAGE_HEIGHT)), backColor);
    // 当前选中矩形的宽高信息;
    //QString selectRectSizeInfo = QString("%1 * %2").arg(currentSelectRect.width()).arg(currentSelectRect.height());
    // 鼠标位置
    QString selectRectSizeInfo = QString("POS:(%1 * %2)").arg(QCursor::pos().x()).arg(QCursor::pos().y());
    QImage image = screenPixmap.toImage();
    QColor endPointColor = image.pixel(QCursor::pos());
    QString selectPointRGBInfo = QString("RGB:(%1,%2,%3)").arg(endPointColor.red()).arg(endPointColor.green()).arg(endPointColor.blue());
    painter.setPen(Qt::white);
    painter.drawText(QPoint(topLeftPoint.x() + 6, topLeftPoint.y() + 14), selectRectSizeInfo);
    painter.drawText(QPoint(topLeftPoint.x() + 6, topLeftPoint.y() + 27), selectPointRGBInfo);
}
