#include "MainWindow.h"
#include "PrepareStageWidget.h"
#include "GameStageWidget.h"
#include "PauseStageWidget.h"
#include "FinishStageWidget.h"
#include <QApplication>

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

    connect(finishStage, &FinishStageWidget::restartGame, this, [=]() {
        switchToPrepareStage();
    });

    connect(finishStage, &FinishStageWidget::exitGame, this, [=]() {
        QApplication::quit();
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
