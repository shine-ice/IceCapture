//
// Created by Wsy on 2023/9/24.
//

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
