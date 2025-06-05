#pragma once

#include <QMainWindow>
#include <QStackedWidget>


#include "Hero.h"
class PrepareStageWidget;
class GameStageWidget;
class PauseStageWidget;
class FinishStageWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    void switchToPrepareStage();
    void switchToGameStage();
    void switchToPauseStage();
    void switchToFinishStage(bool win);
    void setup(const QVector<Hero*>& heroes, int mission);

private:
    QStackedWidget *stack;
    PrepareStageWidget *prepareStage;
    GameStageWidget *gameStage;
    PauseStageWidget *pauseStage;
    FinishStageWidget *finishStage;
};

