#pragma once
#include <QObject>
#include <QVector>
#include <QTimer>
#include <QProgressBar>
#include "Hero.h"

class Enemy;
class GemAreaWidget;

class Player : public QObject {
    Q_OBJECT

public:
    explicit Player(QObject *parent = nullptr);

    void reset();
    void bindHpBar(QProgressBar *bar);
    void updateHpBar();
    void takeDamage(int dmg);
    bool isDead() const;
    void showAsHp();
    void showAsTimer();
    void startMoveTimer();
    void stopMoveTimer();

    void setHeroTeam(const QVector<Hero*>& team);
    const QVector<Hero*>& getHeroTeam() const;

    void processEnemyTurn(const QVector<Enemy*>& enemies);

    void attackAllEnemies(QVector<Enemy*>& enemies, int combo,
                          const QMap<QString, int>& ncarPerAttr);

    void attackSequentially(QVector<Enemy*> enemies,
                            const QMap<QString, double>& totalDamage,
                            const QMap<QString, QMap<QString, double>>& acTable,
                            QVector<Hero*> heroesToAttack,
                            int index); //hepler funciton of attackAllEnemies

    void recoverHp(int combo, int nHeart);

    void setGemArea(GemAreaWidget* g);  // ✅ 傳入外部盤面


signals:
    void moveTimeUp();                      // 倒數結束
    void hpChanged(int current, int max);  // 血量改變
    void attackFinished(); // hero attack enemy

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
    double currentTimeValue = 10.0;

    GemAreaWidget* gemArea = nullptr;
};
