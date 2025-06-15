#include "Enemy.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPixmap>

Enemy::Enemy(int id, const QString &attr, int hp, int atk, int cd, const QString &iconPath,
             SpecialMechanism sp)
    : Hero(id, attr, hp, atk, cd, iconPath), special(sp), maxHp(hp), currentHp(hp)
{}

QWidget* Enemy::createEnemyWidget(QWidget *parent) {
    QWidget *enemyWidget = new QWidget(parent);
    QVBoxLayout *vLayout = new QVBoxLayout(enemyWidget);
    vLayout->setSpacing(5);
    vLayout->setContentsMargins(0, 0, 0, 0);

    // CD
    QLabel *cdLabel = new QLabel(QString("CD: %1").arg(cd), enemyWidget);
    cdLabel->setAlignment(Qt::AlignCenter);
    vLayout->addWidget(cdLabel);

    // icon
    QLabel *icon = new QLabel(enemyWidget);
    icon->setFixedSize(180, 280);
    icon->setPixmap(QPixmap(iconPath).scaled(180, 180));
    icon->setAlignment(Qt::AlignLeft);
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
