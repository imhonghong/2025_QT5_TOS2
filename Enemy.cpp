#include "Enemy.h"
#include "GemAreaWidget.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPixmap>
#include <QDebug>

Enemy::Enemy(int id, const QString &attr, int hp, int atk, int cd, const QString &iconPath,
             SpecialMechanism sp)
    : Hero(id, attr, hp, atk, cd, iconPath), special(sp), maxHp(hp), currentHp(hp),
      originalCd(cd), cd(cd)
{}

QWidget* Enemy::createEnemyWidget(QWidget *parent) {
    QWidget *enemyWidget = new QWidget(parent);
    QVBoxLayout *vLayout = new QVBoxLayout(enemyWidget);
    vLayout->setSpacing(5);
    vLayout->setContentsMargins(0, 0, 0, 0);

    // CD
    cdLabel = new QLabel(QString("CD: %1").arg(cd), enemyWidget);
    cdLabel->setAlignment(Qt::AlignCenter);
    vLayout->addWidget(cdLabel);

    // icon
    QLabel *icon = new QLabel(enemyWidget);
    icon->setFixedSize(180, 280);
    icon->setPixmap(QPixmap(iconPath).scaled(180, 180));
    icon->setAlignment(Qt::AlignCenter);
    vLayout->addWidget(icon);

    // HP bar
    hpBar = new QProgressBar(enemyWidget);
    hpBar->setRange(0, maxHp);
    hpBar->setValue(currentHp);
    hpBar->setFixedWidth(180);

    QString color;
    if (attr == "Water") color = "blue";
    else if (attr == "Fire") color = "red";
    else if (attr == "Earth") color = "green";
    else if (attr == "Light") color = "yellow";
    else if (attr == "Dark") color = "purple";
    hpBar->setStyleSheet(QString("QProgressBar::chunk { background-color: %1; }").arg(color));

    vLayout->addWidget(hpBar);
/*
    if (special == SpecialMechanism::WeatheredRunestone) {
        QLabel *label = new QLabel("Weathered", enemyWidget);
        label->setStyleSheet("color: gray;");
        label->setAlignment(Qt::AlignCenter);
        vLayout->addWidget(label);
    } else if (special == SpecialMechanism::BurningBoard) {
        QLabel *label = new QLabel("Burning", enemyWidget);
        label->setStyleSheet("color: red;");
        label->setAlignment(Qt::AlignCenter);
        vLayout->addWidget(label);
    }
*/
    return enemyWidget;
}

void Enemy::takeDamage(int dmg) {
    currentHp -= dmg;
    if (currentHp < 0) currentHp = 0;
    if (hpBar) hpBar->setValue(currentHp);
}

QVector<Enemy*> Enemy::createWave(int waveID) {
    using SM = Enemy::SpecialMechanism;

    switch (waveID) {
        case 1:
            return {
                new Enemy(1, "Water", 100, 200, 3, ":/enemy/data/enemy/slime_w.png", SM::None),
                new Enemy(2, "Fire", 100, 200, 3, ":/enemy/data/enemy/slime_f.png", SM::None),
                new Enemy(3, "Earth", 100, 200, 3, ":/enemy/data/enemy/slime_e.png", SM::None)
            };
        case 2:
            return {
                new Enemy(4, "Light", 100, 200, 3, ":/enemy/data/enemy/slime_l.png", SM::None),
                new Enemy(5, "Earth", 300, 200, 3, ":/enemy/data/enemy/267n.png", SM::WeatheredRunestone),
                new Enemy(6, "Dark", 100, 200, 3, ":/enemy/data/enemy/slime_d.png", SM::None)
            };
        case 3:
            return {
                new Enemy(7, "Fire", 700, 400, 5, ":/enemy/data/enemy/180n.png", SM::BurningBoard)
            };
        default:
            return {};
        }
}

void Enemy::updateCdLabel()
{
    if (cdLabel) {
        cdLabel->setText(QString("CD: %1").arg(cd));
    }
}

void Enemy::applySkill_ID5(GemAreaWidget* gemArea)
{
    if (id == 5 && currentHp > 0 && gemArea) {
        gemArea->randomSetWeathered(2);
        qDebug() << "[Enemy] ID 5 skill: Set 2 gems to Weathered";
    }
}
