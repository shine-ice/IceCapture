//
// Created by Wsy on 2023/9/24.
//

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