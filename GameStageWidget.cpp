// GameStageWidget.cpp
#include "GameStageWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

GameStageWidget::GameStageWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);

    auto createSection = [](const QString& text, int height) -> QWidget* {
           QLabel* section = new QLabel(text);
           section->setFixedSize(540, height);
           section->setAlignment(Qt::AlignCenter);
           section->setStyleSheet("border: 1px solid gray; font-size: 18px;");
           return section;
       };

    QHBoxLayout *enemyArea = new QHBoxLayout;
    mainLayout->addWidget(createSection("Enemy Area", 330));
    settingButton = new QPushButton("⚙", this);
    settingButton->setFixedSize(32, 32);
    settingButton->move(540-10-32, 10); // 右上角偏移
    // 永遠浮在最上層
    settingButton->raise();

    // 點擊事件保持不變
    connect(settingButton, &QPushButton::clicked, this, &GameStageWidget::pauseGame);
    enemyArea->addWidget(settingButton, Qt::AlignLeft);

    mainLayout->addWidget(createSection("Simulate Button Area", 30));
    heroAreaWidget = new QWidget(this);
    heroAreaWidget->setFixedSize(540, 90);
    heroAreaWidget->setStyleSheet("border: 1px solid gray;");
    heroLayout = new QHBoxLayout(heroAreaWidget);
    heroLayout->setSpacing(10);
    mainLayout->addWidget(heroAreaWidget, 0, Qt::AlignHCenter);
    mainLayout->addWidget(createSection("HP/Timer Area", 40));
    mainLayout->addWidget(createSection("Gem Area", 450));

    connect(settingButton, &QPushButton::clicked, this, &GameStageWidget::pauseGame);
}

void GameStageWidget::setup(const QVector<Hero*>& heroes, int mission)
{
    // 清除舊圖示
    QLayoutItem *child;
    while ((child = heroLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    // 顯示角色或空格框
    for (Hero* h : heroes) {
        QLabel* icon = new QLabel(this);
        icon->setFixedSize(85, 85);

        if (h) {
            QPixmap pix(h->iconPath);
            icon->setPixmap(pix.scaled(85, 85));
        //} else {
        // 留白但加上框線
        //icon->setStyleSheet("border: 1px solid gray;");
        }
        heroLayout->addWidget(icon);
    }
}
