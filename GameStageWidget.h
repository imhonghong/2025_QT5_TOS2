#pragma once
#include <QWidget>
#include <QVector>
#include "Hero.h"
class QPushButton;
class QLabel;
class QHBoxLayout;


class GameStageWidget : public QWidget {
    Q_OBJECT

public:
    explicit GameStageWidget(QWidget *parent = nullptr);
    void setup(const QVector<Hero*>& heroes, int mission);

signals:
    void pauseGame();

private:
    QPushButton *settingButton;
    QHBoxLayout* heroLayout;
    QWidget* heroAreaWidget;
};
