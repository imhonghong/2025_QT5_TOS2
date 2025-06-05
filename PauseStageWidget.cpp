// PauseStageWidget.cpp
#include "PauseStageWidget.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

PauseStageWidget::PauseStageWidget(QWidget *parent)
    : QWidget(parent)
    , pauseTitle (new QLabel("暫停中", this))
    , resumeButton(new QPushButton("Resume", this))
    ,surrenderButton(new QPushButton("Surrender", this))

{
    auto layout = new QVBoxLayout(this);

    // 「暫停中」大標題
    pauseTitle->setAlignment(Qt::AlignCenter);
    QFont f;
    f.setPointSize(24);
    pauseTitle->setFont(f);
    layout->addStretch();
    layout->addWidget(pauseTitle);

    layout->addSpacing(30);

    // Resume 按鈕
    resumeButton->setFixedSize(200, 60);
    connect(resumeButton, &QPushButton::clicked, this, &PauseStageWidget::resumeGame);
    layout->addWidget(resumeButton, 0, Qt::AlignCenter);

    layout->addSpacing(20);

    // Surrender 按鈕
    surrenderButton->setFixedSize(200, 60);
    connect(surrenderButton, &QPushButton::clicked, this, &PauseStageWidget::surrender);
    layout->addWidget(surrenderButton, 0, Qt::AlignCenter);

    layout->addStretch();

}
