#include "MainWindow.h"
#include "PrepareStageWidget.h"
#include "GameStageWidget.h"
#include "PauseStageWidget.h"
#include "FinishStageWidget.h"
#include "GemAreaWidget.h"
#include "Player.h"
#include <QApplication>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      stack(new QStackedWidget(this)),
      prepareStage(new PrepareStageWidget(this)),
      gameStage(new GameStageWidget(this)),
      pauseStage(new PauseStageWidget(this)),
      finishStage(new FinishStageWidget(this))
{
    setCentralWidget(stack);
    setFixedSize(540, 960);
    setWindowTitle("Tower of Saviors");

    stack->addWidget(prepareStage);
    stack->addWidget(gameStage);
    stack->addWidget(pauseStage);
    stack->addWidget(finishStage);

    switchToPrepareStage();

    // ✅ Prepare → Game
    connect(prepareStage, &PrepareStageWidget::startGame, this, [=]() {
        QVector<Hero*> heroes = prepareStage->getSelectedHeroes();
        int mission = prepareStage->getSelectedMission();
        gameStage->setup(heroes, mission);
        switchToGameStage();
    });

    // ✅ Game → Pause
    connect(gameStage, &GameStageWidget::pauseGame, this, [=]() {
        switchToPauseStage();
    });

    // ✅ Pause → Resume
    connect(pauseStage, &PauseStageWidget::resumeGame, this, [=]() {
        switchToGameStage();
    });

    // ✅ Pause → Surrender
    connect(pauseStage, &PauseStageWidget::surrender, this, [=]() {
        switchToFinishStage(false);  // false = lose
    });

    // ✅ finish → restart
    connect(finishStage, &FinishStageWidget::restartGame, this, [=]() {
        switchToPrepareStage();
    });

    // ✅ finish → exit
    connect(finishStage, &FinishStageWidget::exitGame, this, [=]() {
        QApplication::quit();
    });

    // ✅ Game → finish (win)
    connect(gameStage, &GameStageWidget::gamePass, this, [=]() {
        switchToFinishStage(true);  // true = Win!
    });

    // ✅ Game → finish (lose)
    connect(gameStage, &GameStageWidget::gameFail, this, [=]() {
        switchToFinishStage(false);
    });

    // ✅ Game → Game (next wave)
    connect(gameStage, &GameStageWidget::wavePass, this, [=]() {
        gameStage->nextWave();
    });

    connect(gameStage->getGemArea(), &GemAreaWidget::comboResolved,
            this, [=](int combo, QMap<QString, int> ncar) {
        Player* p = gameStage->getPlayer();
        QVector<Enemy*> enemies = gameStage->getCurrentEnemies();
        p->attackAllEnemies(enemies, combo, ncar);
        p->recoverHp(combo, ncar.value("Heart", 0));

        // 🟢 檢查是否全數擊敗
        if (gameStage->checkAllEnemiesDefeated(false)) {
            qDebug() << "All enemies dead — now trigger wavePass";
            gameStage->checkAllEnemiesDefeated(true);  // 明確 emit
            return;
        }

        // 🟢 敵人回合
        p->processEnemyTurn(enemies);

        // 🟢 檢查自己是否死亡
        if (p->isDead()) {
            switchToFinishStage(false);
        }
    });
}

void MainWindow::switchToPrepareStage() {
    stack->setCurrentWidget(prepareStage);
}

void MainWindow::switchToGameStage() {
    stack->setCurrentWidget(gameStage);
}

void MainWindow::switchToPauseStage() {
    stack->setCurrentWidget(pauseStage);
}

void MainWindow::switchToFinishStage(bool win) {
    finishStage->setResult(win);
    stack->setCurrentWidget(finishStage);
}
