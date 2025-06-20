#pragma once
#include <QObject>
#include <QVector>
#include <QTimer>
#include <QProgressBar>
#include <QPropertyAnimation>
#include <QLabel>
#include "Hero.h"

class Enemy;
class GemAreaWidget;

class Player : public QObject {
    Q_OBJECT

public:
    explicit Player(QObject *parent = nullptr);

    void reset();

    void bindHpBar(QProgressBar* bar, QLabel* label = nullptr);
    void updateHpBar();
    void takeDamage(int dmg);
    bool isDead() const;
    void showAsHp();
    void showAsTimer();
    void startMoveTimer();
    void stopMoveTimer();

    void setHeroTeam(const QVector<Hero*>& team);
    const QVector<Hero*>& getHeroTeam() const;

    void setBulletParent(QWidget* parent);

    void processEnemyTurn(const QVector<Enemy*>& enemies);

    void processEnemyTurnSequentially(const QVector<Enemy*>& enemies,
                                      int index);  // helper function if enemy attack

    void attackAllEnemies(QVector<Enemy*>& enemies, int combo,
                          const QMap<QString, int>& ncarPerAttr);

    void attackSequentially(QVector<Enemy*> enemies,
                            const QMap<QString, double>& totalDamage,
                            const QMap<QString, QMap<QString, double>>& acTable,
                            QVector<Hero*> heroesToAttack,
                            int index); //hepler funciton of attackAllEnemies
    void showBulletAnimation(QWidget* from, QWidget* to, QWidget* commonParent);

    void recoverHp(int combo, int nHeart);

    void setGemArea(GemAreaWidget* g);  // ✅ 傳入外部盤面


signals:
    void moveTimeUp();                      // 倒數結束
    void hpChanged(int current, int max);  // 血量改變
    void attackFinished(); // hero attack enemy
    void enemyAttackFinished(); //enemy attack hero
    void playerDead();

private slots:
    void onMoveTimerTimeout();

private:
    QVector<Hero*> heroes;
    int maxHp;
    int currentHp;

    QProgressBar* hpBar;
    QTimer* moveTimer;
    int moveTime;
    bool hasStartedMove = false; //只轉珠一次的防呆
    bool isMoving = false;  // 是否處於轉珠狀態
    double currentTimeValue = 10.0;

    GemAreaWidget* gemArea = nullptr;
    QPropertyAnimation* timerAnim = nullptr;
    QLabel* hpTextLabel = nullptr;
    QWidget* bulletParent = nullptr;

    bool hasEmittedAttackFinished = false;
};
