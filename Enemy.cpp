#include "Enemy.h"
#include "GemAreaWidget.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPixmap>
#include <QDebug>

Enemy::Enemy(int id, const QString &attr, int hp, int atk, int cd, const QString &iconPath,
             SpecialMechanism sp)
    : Hero(id, attr, hp, atk, cd, iconPath), special(sp), currentHp(hp), maxHp(hp),
      originalCd(cd), cd(cd)
{}

QWidget* Enemy::createEnemyWidget(QWidget *parent) {
    QWidget *enemyWidget = new QWidget(parent);
    QVBoxLayout *vLayout = new QVBoxLayout(enemyWidget);
    vLayout->setSpacing(5);
    vLayout->setContentsMargins(0, 0, 0, 0);

    // 圖片區域包裝（包含 CD 與 HP）
    QWidget* iconWrapper = new QWidget(enemyWidget);
    iconWrapper->setFixedSize(180, 210);  // 180 + 30
    iconWrapper->setStyleSheet("background-color: transparent;");

    // icon 本體
    QLabel* icon = new QLabel(iconWrapper);
    icon->setFixedSize(180, 180);
    icon->setPixmap(QPixmap(iconPath).scaled(180, 180));
    icon->setAlignment(Qt::AlignHCenter);
    icon->move(0, 0);
    this->iconLabel = icon;

    // CD 標籤
    cdLabel = new QLabel(QString("CD: %1").arg(cd), iconWrapper);
    cdLabel->setFixedSize(50, 20);
    cdLabel->move(5, 10);
    cdLabel->setStyleSheet("color: red;");

    // HP bar 直接疊在圖下方（框內）
    hpBar = new QProgressBar(iconWrapper);
    hpBar->setRange(0, maxHp);
    hpBar->setValue(currentHp);
    int hpBar_height = 15;
    hpBar->setFixedSize(140, hpBar_height);
    hpBar->setAlignment(Qt::AlignRight);
    hpBar->move(0, 180 + 30 -hpBar_height);  // 在圖片下方 2px，icon 是 180 高

    // 設定顏色
    QString color;
    if (attr == "Water") color = "blue";
    else if (attr == "Fire") color = "red";
    else if (attr == "Earth") color = "green";
    else if (attr == "Light") color = "gold";
    else if (attr == "Dark") color = "purple";
    hpBar->setStyleSheet(QString("QProgressBar::chunk { background-color: %1; }").arg(color));

    // 加入整體 layout
    vLayout->addWidget(iconWrapper);



    this->enemyWidget = enemyWidget;
    return enemyWidget;
}


void Enemy::takeDamage(int dmg) {
    currentHp -= dmg;
    if (currentHp < 0) currentHp = 0;
    if (hpBar) hpBar->setValue(currentHp);

    if (currentHp <= 0 && enemyWidget) {
            enemyWidget->setEnabled(false);     // 不再互動
            enemyWidget->setStyleSheet("opacity: 0.3;");  // 可選：淡出風格
            QLayout* layout = enemyWidget->layout();
            if (layout) {
                for (int i = 0; i < layout->count(); ++i) {
                    QWidget* child = layout->itemAt(i)->widget();
                    if (child) child->hide();  // ✅ 只隱藏內部圖像
                }
            }
        }
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

void Enemy::onEnterWave(GemAreaWidget* gemArea)
{
    if (id == 5) {
        applySkill_ID5(gemArea);  // 入場觸發
    }
}

void Enemy::applySkill_ID7(GemAreaWidget* gemArea)
{
    if (id == 7 && currentHp > 0 && gemArea) {
        for (const QPoint& pt : gemArea->getPassedCells()) {
            int row = pt.x();
            int col = pt.y();
            Gem* g = gemArea->getGem(row, col);
            if (g && g->getState() == "Normal") {
                g->setState("Burning");
            }
        }
        qDebug() << "[Enemy] ID 7 skill: Set burning gems along path.";
    }
}
