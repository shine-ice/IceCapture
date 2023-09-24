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
