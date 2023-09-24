//
// Created by Wsy on 2023/9/24.
//

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
