#pragma once

#include <QWidget>
#include <QVector>
#include "Hero.h"

class QLabel;
class QComboBox;
class QSpinBox;
class QPushButton;
class QGridLayout;

class PrepareStageWidget : public QWidget {
    Q_OBJECT

public:
    explicit PrepareStageWidget(QWidget *parent = nullptr);
    QVector<Hero*> getSelectedHeroes() const;
    int getSelectedMission() const;

signals:
    void startGame();

private:
    QVector<Hero*> heroPool;              // 所有可選 Hero
    QVector<Hero*> selectedHeroes;        // 使用者選擇的 6 格 Hero*
    QVector<QComboBox*> heroBoxes;        // 6 個 ComboBox

    QSpinBox *missionSpinBox;
};

Q_DECLARE_METATYPE(Hero*)  // 允許 QVariant 傳遞 Hero*
