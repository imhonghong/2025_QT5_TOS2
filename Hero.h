// Hero.h
#pragma once
#include <QString>

class Hero {
public:
    Hero(int id, const QString &attr, int hp, int atk, int cd, const QString &iconPath);

    int id;
    QString attr;
    int hp;
    int atk;
    int cd;
    QString iconPath;
};
