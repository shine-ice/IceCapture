# 0. 前言

参考：[Qt 之 实现截图小应用](https://blog.csdn.net/goforwardtostep/category_6671888.html)

使用CLion实现Qt5的截图软件



# 1. 简单截图

## 1.1 截图类

1. `CaptureImage.h`

```cpp
#ifndef ICECAPTURE_CAPTUREIMAGE_H
#define ICECAPTURE_CAPTUREIMAGE_H

#include <QWidget>
#include <QPainter>
#include <QPixmap>
#include <QMouseEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include <QScreen>

class CaptureImage : public QWidget{
    Q_OBJECT
public:
    explicit CaptureImage(QWidget *parent = nullptr);

signals:
    // 截图完成的信号
    void signalCompleteCapture(QPixmap image);

private:
    void initWindow();                  // 初始化截图窗口
    void loadBackgroundPixmap();        // 加载背景

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void paintEvent(QPaintEvent *event);

    // 根据beginCapturePoint, endCapturePoint获取当前选中的矩形
    QRect getRect(const QPoint &beginPoint, const QPoint &endPoint);

private:
    bool isMousePress;
    QPixmap screenPixmap;       // 屏幕原始图片
    QPixmap capturePixmap;      // 从屏幕原始图片中截取的图片
    QPixmap capturePixmapCopy;  // 截图的拷贝-用于标注
    int screenWidth;
    int screenHeight;
    QPoint beginCapturePoint;       // 开始截取图片时鼠标位置
    QPoint endCapturePoint;         // 结束截取图片时鼠标位置
    QPainter painter;               // 绘制截取的图片

};


#endif //ICECAPTURE_CAPTUREIMAGE_H

```



2. `CaptureImage.cpp`

```cpp
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

```



## 1.2 调用类

1. `IceCapture.h`

```cpp
#ifndef ICECAPTURE_ICECAPTURE_H
#define ICECAPTURE_ICECAPTURE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QPixmap>
#include <QPoint>
#include "CaptureImage.h"

class IceCapture : public QWidget {
    Q_OBJECT

public:
    IceCapture(QWidget *parent = NULL);
    ~IceCapture();

private slots:
    void onCompleteCapture(QPixmap pix);

private:
    QLabel *captureImageLabel;
    QPushButton *captureBtn;
    QVBoxLayout *vLayout;
    CaptureImage *captureImage;
};


#endif //ICECAPTURE_ICECAPTURE_H

```



2. `IceCapture.cpp`

```cpp
#include "IceCapture.h"

IceCapture::IceCapture(QWidget *parent) : QWidget(parent) {
    // 初始化参数
    captureImage = new CaptureImage();
    connect(captureImage, SIGNAL(signalCompleteCapture(QPixmap)),
            this, SLOT(onCompleteCapture(QPixmap)));

    captureImageLabel = new QLabel();
    captureBtn = new QPushButton(tr("截图"));
    connect(captureBtn, SIGNAL(clicked()), captureImage, SLOT(show()));

    vLayout = new QVBoxLayout();
    vLayout->addWidget(captureImageLabel);
    vLayout->addWidget(captureBtn);
    this->setLayout(vLayout);
    this->resize(300, 400);
}

IceCapture::~IceCapture()
{
}

void IceCapture::onCompleteCapture(QPixmap pix)
{
    this->captureImageLabel->resize(pix.size());
    this->captureImageLabel->setPixmap(pix);
}
```



## 1.3 main.cpp

```cpp
#include <QApplication>
#include "IceCapture.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    IceCapture w;
    w.show();
    return a.exec();
}
```



## 1.4 CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.20)
project(IceCapture)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_PREFIX_PATH "D:/MyPrograms/Qt/5.15.2/mingw81_32")

find_package(Qt5 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

add_executable(IceCapture main.cpp IceCapture.cpp IceCapture.h CaptureImage.cpp CaptureImage.h)
target_link_libraries(IceCapture
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        )

#if (WIN32)
#    set(DEBUG_SUFFIX)
#    if (CMAKE_BUILD_TYPE MATCHES "Debug")
#        set(DEBUG_SUFFIX "")
#    endif ()
#    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
#    if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
#        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
#        if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
#            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
#        endif ()
#    endif ()
#    if (EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
#        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#                COMMAND ${CMAKE_COMMAND} -E make_directory
#                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
#        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#                COMMAND ${CMAKE_COMMAND} -E copy
#                "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
#                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
#    endif ()
#    foreach (QT_LIB Core Gui Widgets)
#        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#                COMMAND ${CMAKE_COMMAND} -E copy
#                "${QT_INSTALL_PATH}/bin/Qt5${QT_LIB}${DEBUG_SUFFIX}.dll"
#                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
#    endforeach (QT_LIB)
#endif ()

```



## 1.5 测试

![image-20230924135745554](E:\CodeSpace\QtSpace\WorkSpace007\IceCapture\assets\image-20230924135745554.png)



# 2. 移动选中区域

## 2.1 截图类

1. `CaptureImage.h`

```cpp
#ifndef ICECAPTURE_CAPTUREIMAGE_H
#define ICECAPTURE_CAPTUREIMAGE_H

#include <QWidget>
#include <QPainter>
#include <QPixmap>
#include <QMouseEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include <QScreen>

// 截图状态
enum CaptureState {
    InitCapture,
    BeginCaptureImage,
    FinishCaptureImage,
    BeginMoveCaptureArea,
    FinishMoveCaptureArea,
    BeginMoveStretchRect,
    FinishMoveStretchRect,
    BeginDraw,
    FinishDraw,
    FinishCapture
};

class CaptureImage : public QWidget{
    Q_OBJECT
public:
    explicit CaptureImage(QWidget *parent = nullptr);

signals:
    // 截图完成的信号
    void signalCompleteCapture(QPixmap image);

private:
    void initData();                    // 初始化数据
    void initWindow();                  // 初始化截图窗口
    void loadBackgroundPixmap();        // 加载背景

    // 根据beginCapturePoint, endCapturePoint获取当前选中的矩形
    QRect getRect(const QPoint &beginPoint, const QPoint &endPoint);
    // 当前鼠标坐标是否在选取的矩形区域内
    bool isPressPointInSelectRect(QPoint mousePressPoint);
    // 根据当前截取状态获取当前选中的截图区域
    QRect getSelectRect();
    // 绘制当前选中的截图区域
    void drawCaptureImage();
    // 获取移动后，当前选中的矩形
    QRect getMoveRect();
    // 检查当前是否移动超出屏幕，获取移动距离
    QPoint getMovePoint();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void paintEvent(QPaintEvent *event);

private:
    QPixmap screenPixmap;       // 屏幕原始图片
    QPixmap capturePixmap;      // 从屏幕原始图片中截取的图片
    QPixmap capturePixmapCopy;  // 截图的拷贝-用于标注
    int screenWidth;
    int screenHeight;
    QPoint beginCapturePoint;       // 开始截取图片时鼠标位置
    QPoint endCapturePoint;         // 结束截取图片时鼠标位置
    QPainter painter;               // 绘制截取的图片

    CaptureState currentCaptureState;       // 当前截图状态
    QRect currentSelectRect;                // 当前选择区域
    QPoint beginMovePoint;          // 开始移动选框时鼠标位置
    QPoint endMovePoint;            // 结束移动选框时鼠标位置
};


#endif //ICECAPTURE_CAPTUREIMAGE_H

```



2. `CaptureImage.cpp`

```cpp
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


```



## 2.2 测试

![image-20230924143920135](E:\CodeSpace\QtSpace\WorkSpace007\IceCapture\assets\image-20230924143920135.png)



# 3. 选区拖拽

## 3.1 截图类

1. `CaptureImage.h`

```cpp
#ifndef ICECAPTURE_CAPTUREIMAGE_H
#define ICECAPTURE_CAPTUREIMAGE_H

#include <QWidget>
#include <QPainter>
#include <QPixmap>
#include <QMouseEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include <QScreen>

// 矩形选中区边框宽度;
#define SELECT_RECT_BORDER_WIDTH                2

// 选中矩形8个拖拽点小矩形的宽高;
#define STRETCH_RECT_WIDTH                      6
#define STRETCH_RECT_HEIGHT                     6

// 截图状态
enum CaptureState {
    InitCapture,
    BeginCaptureImage,
    FinishCaptureImage,
    BeginMoveCaptureArea,
    FinishMoveCaptureArea,
    BeginMoveStretchRect,
    FinishMoveStretchRect,
    BeginDraw,
    FinishDraw,
    FinishCapture
};

// 拖拽的边框顶点
enum StretchRectState {
    NotSelect,
    TopLeftRect,
    TopCenterRect,
    TopRightRect,
    RightCenterRect,
    BottomRightRect,
    BottomCenterRect,
    BottomLeftRect,
    LeftCenterRect
};

class CaptureImage : public QWidget{
    Q_OBJECT
public:
    explicit CaptureImage(QWidget *parent = nullptr);

signals:
    // 截图完成的信号
    void signalCompleteCapture(QPixmap image);

private:
    void initData();                    // 初始化数据
    void initWindow();                  // 初始化截图窗口
    void loadBackgroundPixmap();        // 加载背景

    // 根据beginCapturePoint, endCapturePoint获取当前选中的矩形
    QRect getRect(const QPoint &beginPoint, const QPoint &endPoint);

    // 当前鼠标坐标是否在选取的矩形区域内
    bool isPressPointInSelectRect(QPoint mousePressPoint);
    // 根据当前截取状态获取当前选中的截图区域
    QRect getSelectRect();
    // 绘制当前选中的截图区域
    void drawCaptureImage();
    // 获取移动后，当前选中的矩形
    QRect getMoveRect();
    // 检查当前是否移动超出屏幕，获取移动距离
    QPoint getMovePoint();

    void initStretchRect();     // 初始化边框拖拽点
    QRect getStretchRect();     // 获取拖拽框区域
    void drawStretchRect();     // 绘制选中矩形各拖拽点小矩形
    // 获取鼠标位于哪个拖拽点
    StretchRectState getStretchRectState(QPoint point);
    // 设置拖拽点上鼠标样式
    void setStretchCursorStyle(StretchRectState stretchRectState);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void paintEvent(QPaintEvent *event);

private:
    QPixmap screenPixmap;       // 屏幕原始图片
    QPixmap capturePixmap;      // 从屏幕原始图片中截取的图片
    QPixmap capturePixmapCopy;  // 截图的拷贝-用于标注
    int screenWidth;
    int screenHeight;
    QPoint beginCapturePoint;       // 开始截取图片时鼠标位置
    QPoint endCapturePoint;         // 结束截取图片时鼠标位置
    QPainter painter;               // 绘制截取的图片

    CaptureState currentCaptureState;       // 当前截图状态
    QRect currentSelectRect;                // 当前选择区域
    QPoint beginMovePoint;          // 开始移动选框时鼠标位置
    QPoint endMovePoint;            // 结束移动选框时鼠标位置

    StretchRectState currentStretchRectState;       // 当前拖拽点的选择状态
    QRect topLeftRect;                              // 左上拖拽点
    QRect topRightRect;                             // 右上拖拽点
    QRect bottomLeftRect;                           // 左下拖拽点
    QRect bottomRightRect;                          // 右下拖拽点
    QRect topCenterRect;                            // 中上拖拽点
    QRect leftCenterRect;                           // 中左拖拽点
    QRect rightCenterRect;                          // 中右拖拽点
    QRect bottomCenterRect;                         // 中下拖拽点
};


#endif //ICECAPTURE_CAPTUREIMAGE_H

```



2. `CaptureImage.cpp`

```cpp
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
            break;
        }
        case BeginCaptureImage:
        case FinishCaptureImage:
        case BeginMoveCaptureArea:
        case FinishMoveCaptureArea:
        case BeginMoveStretchRect:
        case FinishMoveStretchRect:
        {
            currentSelectRect = getSelectRect();
            drawCaptureImage();
        }
        case FinishCapture:
            break;
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


```



## 3.2 测试

![image-20230924153429051](E:\CodeSpace\QtSpace\WorkSpace007\IceCapture\assets\image-20230924153429051.png)



