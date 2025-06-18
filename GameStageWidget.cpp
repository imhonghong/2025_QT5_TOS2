// GameStageWidget.cpp
#include "GameStageWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QPushButton>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
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
        qDebug() << "[GameStage] drag finished — manual stop";
        if (player) {
            player->stopMoveTimer();
        }
        qDebug() << "[GameStage] drag finished — calling combo check";
        gemArea->resolveComboCycle();
    });

    connect(player, &Player::attackFinished, this, [=]() {
        qDebug() << "[GameStage] All heroes attacked — check enemies";
        checkAllEnemiesDefeated(true);  // ✅ 明確觸發下一關判斷
        if (!checkAllEnemiesDefeated(false)) {
                player->processEnemyTurn(enemies);
            }
    });

    connect(gemArea, &GemAreaWidget::showBurnDamage, this, [=]() {
        burnDamageLabel->setText("-30");
        burnDamageLabel->show();
        QTimer::singleShot(400, this, [=]() {
            burnDamageLabel->hide();
        });
    });

    connect(player, &Player::playerDead, this, [=]() {
        qDebug() << "[GameStage] Detected player death — emit gameFail()";
        emit gameFail();  // ✅ 傳給 MainWindow 觸發畫面切換
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
        icon->setFixedSize(90, 90);

        if (h) {
            QPixmap pix(h->iconPath);
            icon->setPixmap(pix.scaled(90, 90));
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

    // combo and recovery
    comboOverlay = new QWidget(this);
    comboOverlay->setStyleSheet("background-color: rgba(0, 0, 0, 100);");
    comboOverlay->setGeometry(0, 510, 540, 450);  // ✅ 絕對座標
    comboOverlay->setAttribute(Qt::WA_TransparentForMouseEvents);
    comboOverlay->hide();

    comboLabel = new QLabel(comboOverlay);  // ✅ 讓 label 蓋在 overlay 上
    comboLabel->setStyleSheet("QLabel { color: orange; font-size: 24px; font-weight: bold; }");
    comboLabel->setGeometry(100, 50, 960-100-20, 450-50-20);        // ✅ 相對於 comboOverlay 的位置
    comboLabel->setAlignment(Qt::AlignCenter);
    comboLabel->hide();

    recoveryLabel = new QLabel(this);
    recoveryLabel->setStyleSheet("QLabel { color: green; font-size: 24px; font-weight: bold; }");
    recoveryLabel->setFixedSize(100, 50);
    recoveryLabel->move((560-100)/2, 960-450-50);  // 可視 UI 調整
    recoveryLabel->setAlignment(Qt::AlignCenter);
    recoveryLabel->hide();

    burnDamageLabel = new QLabel(this);
    burnDamageLabel->setStyleSheet("QLabel { color: red; font-size: 24px; font-weight: bold; }");
    burnDamageLabel->setFixedSize(100, 50);
    burnDamageLabel->move((560-100)/2, 960-450-50);  // 距離回血顯示右側一點
    burnDamageLabel->setAlignment(Qt::AlignCenter);
    burnDamageLabel->hide();
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

    gemArea->setEnemies(enemies);

    // 將此 wave 的敵人逐一顯示
    for (Enemy* enemy : waves[wave_idx]) {
        QWidget* enemyWidget = enemy->createEnemyWidget(this);
            if (wave_idx==2) enemyLayout->addStretch(); //wave2 自動加空格
            enemyLayout->addWidget(enemyWidget, 0, Qt::AlignVCenter);
            if (wave_idx==2) enemyLayout->addStretch();
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
    isWaveTransitioning = false;
}

bool GameStageWidget::checkAllEnemiesDefeated(bool emitIfPassed)
{
    for (Enemy* e : enemies) {
        if (e && e->currentHp > 0) {
            return false;
        }
    }
    if (emitIfPassed) {
        emit wavePass();
    }
    return true;
}

void GameStageWidget::handleComboResolved(int combo, QMap<QString, int> ncarMap)
{
    if (combo > 0) {
        comboOverlay->show();
        comboLabel->setText(QString("COMBO %1").arg(combo));
        comboLabel->adjustSize();
        comboLabel->move((comboOverlay->width() - comboLabel->width()) / 2,
                         comboOverlay->height() - 100);
        comboLabel->setGraphicsEffect(nullptr);
        comboLabel->show();

        // 飄升動畫
        QPropertyAnimation* moveAnim = new QPropertyAnimation(comboLabel, "pos");
        moveAnim->setDuration(500);
        moveAnim->setStartValue(comboLabel->pos());
        moveAnim->setEndValue(QPoint(300, 420 - 60));  // 上飄
        moveAnim->setEasingCurve(QEasingCurve::OutCubic);

        QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(comboLabel);
        comboLabel->setGraphicsEffect(effect);

        QPropertyAnimation* fadeAnim = new QPropertyAnimation(effect, "opacity");
        fadeAnim->setDuration(1000);
        fadeAnim->setStartValue(1.0);
        fadeAnim->setEndValue(0.0);

        moveAnim->start(QAbstractAnimation::DeleteWhenStopped);
        fadeAnim->start(QAbstractAnimation::DeleteWhenStopped);

        connect(fadeAnim, &QPropertyAnimation::finished, this, [=]() {
            comboLabel->hide();
            comboOverlay->hide();

            QTimer::singleShot(300, this, [=]() {
                    player->attackAllEnemies(enemies, combo, ncarMap);
                });
            player->recoverHp(combo, ncarMap.value("Heart", 0));
            int recovery = combo * ncarMap.value("Heart", 0) * 5;
            recoveryLabel->setText(QString("+%1").arg(recovery));
            if (recovery !=0 ){
                recoveryLabel->show();
                QTimer::singleShot(1000, this, [=]() {
                    recoveryLabel->hide();
                });
            }
        });
    }
}
