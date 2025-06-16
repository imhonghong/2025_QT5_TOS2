#pragma once
#include "Hero.h"
#include <QWidget>
#include <QLabel>

class QProgressBar;
class GemAreaWidget;  // forward declaration

class Enemy : public Hero {
public:
    enum class SpecialMechanism {
        None,
        WeatheredRunestone,
        BurningBoard
    };

    Enemy(int id, const QString &attr, int hp, int atk, int cd, const QString &iconPath,
          SpecialMechanism sp = SpecialMechanism::None);

    SpecialMechanism special;
    int currentHp;
    int maxHp;
    int originalCd; // ⬅️ 新增：初始冷卻時間
    int cd;         // ⬅️ 新增：當前冷卻倒數時間

    QProgressBar* hpBar = nullptr;  // 可選：讓 Enemy 控制自己的 UI 血條
    QLabel* cdLabel = nullptr;

    void applySkill_ID5(GemAreaWidget* gemArea);  // ID=5 weather stone

    static QVector<Enemy*> createWave(int waveID);
    QWidget* createEnemyWidget(QWidget *parent = nullptr);
    void takeDamage(int dmg);
    void updateCdLabel();
};
