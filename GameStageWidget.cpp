// GameStageWidget.cpp
#include "GameStageWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QPushButton>
#include <QMouseEvent>
#include "Enemy.h"
#include "Player.h"

GameStageWidget::GameStageWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);

    enemyAreaWidget = new QWidget(this);
    enemyAreaWidget->setFixedSize(540, 350);
    // enemyAreaWidget->setStyleSheet("border: 1px solid gray;");
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

    QProgressBar* hpBar = new QProgressBar(this);
    hpBar->setFixedSize(530, 40);
    hpBar->setTextVisible(true);
    mainLayout->addWidget(hpBar, 0, Qt::AlignHCenter);

    // 綁定 player 與初始化
    player = new Player(this);
    player->bindHpBar(hpBar);
    player->reset();

    // 符石區
    gemArea = new GemAreaWidget(this);
    gemArea->setFixedSize(540, 450);
    mainLayout->addWidget(gemArea);
    gemArea->setPlayer(player);

    // 轉珠計時連接
    connect(gemArea, &GemAreaWidget::dragStarted, this, [=]() {
        if (player) player->startMoveTimer();
    });

    connect(player, &Player::moveTimeUp, this, [=]() {
        gemArea->forceStopDragging();
        // gemArea->enableDrag(false);  // 禁用拖曳（時間到）
    });

    connect(gemArea, &GemAreaWidget::dragFinished, this, [=]() {
        if (player) player->stopMoveTimer();
        qDebug() << "[GameStage] drag finished — calling combo check";
        gemArea->checkAndMarkCombo();

    });

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
    player->setHeroTeam(heroes);
    player->setGemArea(gemArea);
}

void GameStageWidget::initWaves(int mission)
{
    waves.clear();
    if (mission == 1) {
        for (int i = 1; i <= 3; ++i) {
            waves.append(Enemy::createWave(i));
        }
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
    enemies = waves[wave_idx];
    // 將此 wave 的敵人逐一顯示
    for (Enemy* enemy : waves[wave_idx]) {

        if (enemy && enemy->id == 5) {
            enemy->applySkill_ID5(gemArea);
        }

        QWidget* enemyWidget = enemy->createEnemyWidget(this);
        enemyLayout->addWidget(enemyWidget, 0, Qt::AlignVCenter);
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

bool GameStageWidget::checkAllEnemiesDefeated(bool emitIfPassed)
{
    for (Enemy* e : enemies) {
            if (e && e->currentHp > 0) return false;  // 加上 e != nullptr 檢查
    }
    qDebug() << "[GameStage] All enemies defeated. Emitting wavePass";
    if (emitIfPassed)
        emit wavePass();

    return true;
}
