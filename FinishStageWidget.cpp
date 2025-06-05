// FinishStageWidget.cpp
#include "FinishStageWidget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QApplication>

FinishStageWidget::FinishStageWidget(QWidget *parent)
    : QWidget(parent)
    ,resultLabel(new QLabel(this))
    ,restartButton(new QPushButton("Restart", this))
    ,exitButton(new QPushButton("Exit", this))

{
    auto layout = new QVBoxLayout(this);

    // 顯示結果 (You Win! / You Lose!)
    resultLabel->setAlignment(Qt::AlignCenter);
    QFont f;
    f.setPointSize(24);
    resultLabel->setFont(f);
    layout->addStretch();
    layout->addWidget(resultLabel);

    layout->addSpacing(30);
    // 底下兩個按鈕：Restart、Go Back
    auto hLayout = new QHBoxLayout;
    restartButton->setFixedSize(180, 60);
    exitButton->setFixedSize(180, 60);

    hLayout->addWidget(restartButton);
    hLayout->addSpacing(40);
    hLayout->addWidget(exitButton);

    layout->addLayout(hLayout);
    layout->addStretch();

    connect(restartButton, &QPushButton::clicked, this, &FinishStageWidget::restartGame);
    connect(exitButton, &QPushButton::clicked, this, &FinishStageWidget::exitGame);
}

void FinishStageWidget::setResult(bool win) {
    resultLabel->setText(win ? "You Win!" : "You Lose!");
}
