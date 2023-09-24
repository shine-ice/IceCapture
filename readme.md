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



