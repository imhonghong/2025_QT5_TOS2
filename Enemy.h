#pragma once
#include "Hero.h"
#include <QWidget>

class QProgressBar;

class Enemy : public Hero {
public:
    enum class SpecialMechanism {
        None,
        WeatheredRunestone,
        BurningBoard
    };

    SpecialMechanism special;
    int currentHp;
    int maxHp;

    QProgressBar* hpBar = nullptr;  // 可選：讓 Enemy 控制自己的 UI 血條

    Enemy(int id, const QString &attr, int hp, int atk, int cd, const QString &iconPath,
          SpecialMechanism sp = SpecialMechanism::None);

    QWidget* createEnemyWidget(QWidget *parent = nullptr);
    void takeDamage(int dmg);
};
