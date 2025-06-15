// GameStageWidget.cpp
#include "GameStageWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include "Enemy.h"

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

    enemyAreaWidget = new QWidget(this);
    enemyAreaWidget->setFixedSize(540, 380);
    enemyAreaWidget->setStyleSheet("border: 1px solid gray;");
    enemyLayout = new QHBoxLayout(enemyAreaWidget);
    enemyLayout->setSpacing(10);
    enemyLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->addWidget(enemyAreaWidget);

    // setting button
    settingButton = new QPushButton("⚙", this);
    settingButton->setFixedSize(32, 32);
    settingButton->move(540-10-32, 10); // 右上角偏移
    settingButton->raise();
    connect(settingButton, &QPushButton::clicked, this, &GameStageWidget::pauseGame);

    // simulate button
    QWidget *simulateArea = new QWidget(this);
    simulateArea->setFixedSize(540, 30);
    simulateArea->setStyleSheet("border: 1px solid gray;");

    QHBoxLayout *simulateLayout = new QHBoxLayout(simulateArea);
    simulateLayout->setContentsMargins(10, 5, 10, 5);
    simulateLayout->setSpacing(20);

    QPushButton *winButton = new QPushButton("Win This Wave", this);
    connect(winButton, &QPushButton::clicked, this, &GameStageWidget::wavePass);
    QPushButton *failButton = new QPushButton("Fail", this);
    connect(failButton, &QPushButton::clicked, this, &GameStageWidget::gameFail);
    winButton->setFixedSize(150, 30);
    failButton->setFixedSize(100, 30);
    simulateLayout->addWidget(winButton);
    simulateLayout->addStretch();
    simulateLayout->addWidget(failButton);
    mainLayout->addWidget(simulateArea);

    // hero
    heroAreaWidget = new QWidget(this);
    heroAreaWidget->setFixedSize(540, 90);
    heroAreaWidget->setStyleSheet("border: 1px solid gray;");
    heroLayout = new QHBoxLayout(heroAreaWidget);
    heroLayout->setSpacing(10);
    mainLayout->addWidget(heroAreaWidget, 0, Qt::AlignHCenter);

    mainLayout->addWidget(createSection("HP/Timer Area", 40));

    mainLayout->addWidget(createSection("Gem Area", 450));

}

void GameStageWidget::setup(const QVector<Hero*>& heroes, int mission)
{
    if (mission != 1) return;

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

    // enemy
    initWaves(mission);
    currentWave = 0;
    showWave(currentWave);

}

void GameStageWidget::initWaves(int mission)
{
    waves.clear();

    if (mission == 1) {
        using SM = Enemy::SpecialMechanism;

        // Wave 1
        waves.append({
            new Enemy(1, "Water", 100, 200, 3, ":/enemy/data/enemy/slime_w.png", SM::None),
            new Enemy(2, "Fire", 100, 200, 3, ":/enemy/data/enemy/slime_f.png", SM::None),
            new Enemy(3, "Earth", 100, 200, 3, ":/enemy/data/enemy/slime_e.png", SM::None)
        });

        // Wave 2
        waves.append({
            new Enemy(4, "Light", 100, 200, 3, ":/enemy/data/enemy/slime_l.png", SM::None),
            new Enemy(5, "Earth", 300, 200, 3, ":/enemy/data/enemy/267n.png", SM::WeatheredRunestone),
            new Enemy(6, "Dark", 100, 200, 3, ":/enemy/data/enemy/slime_d.png", SM::None)
        });

        // Wave 3
        waves.append({
            new Enemy(7, "Fire", 700, 400, 5, ":/enemy/data/enemy/180n.png", SM::BurningBoard)
        });
    }
}

void GameStageWidget::showWave(int wave_idx)
{
    if (wave_idx < 0 || wave_idx >= waves.size()) return;

    // 清除畫面上舊敵人
    QLayoutItem *child;
    while ((child = enemyLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    // 將此 wave 的敵人逐一顯示
    for (Enemy* enemy : waves[wave_idx]) {
        QWidget* enemyWidget = enemy->createEnemyWidget(this);
        enemyLayout->addWidget(enemyWidget);
    }
}

void GameStageWidget::nextWave()
{
    currentWave++;
    if (currentWave >= waves.size()) {
        emit gamePass();  // optional: 所有 wave 成功
    } else {
        showWave(currentWave);
    }
}

