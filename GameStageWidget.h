#pragma once
#include <QWidget>
#include <QVector>
#include "Hero.h"
#include "Enemy.h"
#include "Player.h"
class QPushButton;
class QLabel;
class QHBoxLayout;


class GameStageWidget : public QWidget {
    Q_OBJECT

public:
    explicit GameStageWidget(QWidget *parent = nullptr);
    void setup(const QVector<Hero*>& heroes, int mission);
    void initWaves(int mission);
    void showWave(int wave_idx);
    void nextWave();
    void mousePressEvent(QMouseEvent *event);
    Player* getPlayer() const { return player; }
    QVector<Enemy*> getCurrentEnemies() const { return enemies; }
    //void checkAllEnemiesDefeated();
    bool checkAllEnemiesDefeated(bool emitIfPassed = true);


signals:
    void pauseGame();
    void wavePass();
    void gameFail();
    void gamePass();

private:
    QPushButton *settingButton;

    // enemy
    QWidget* enemyAreaWidget;
    QHBoxLayout* enemyLayout;

    // hero
    QHBoxLayout* heroLayout;
    QWidget* heroAreaWidget;

    // player
    Player* player;

    // waves
    QVector<QVector<Enemy*>> waves;
    int currentWave;
    QVector<Enemy*> enemies;

};
