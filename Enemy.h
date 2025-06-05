// Enemy.h
#pragma once
#include "Hero.h"

class Enemy : public Hero {
public:
    Enemy(int id, const QString &attr, int hp, int atk, int cd, const QString &iconPath);
    // 可以加上特殊能力、攻擊範圍等
};
