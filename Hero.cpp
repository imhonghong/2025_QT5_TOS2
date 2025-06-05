// Hero.cpp
#include "Hero.h"

Hero::Hero(int id, const QString &attr, int hp, int atk, int cd, const QString &iconPath)
    : id(id), attr(attr), hp(hp), atk(atk), cd(cd), iconPath(iconPath) {}
