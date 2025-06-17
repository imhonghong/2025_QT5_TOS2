#include "Player.h"
#include <QDebug>
#include "Enemy.h"
#include <QRandomGenerator>
#include <QPropertyAnimation>


Player::Player(QObject *parent)
    : QObject(parent),
      maxHp(2000),
      currentHp(2000),
      hpBar(nullptr),
      moveTimer(new QTimer(this)),
      moveTime(10)
{
    connect(moveTimer, &QTimer::timeout, this, &Player::onMoveTimerTimeout);
}

void Player::reset()
{
    currentHp = maxHp;
    updateHpBar();
    stopMoveTimer();
    showAsHp();
}

void Player::bindHpBar(QProgressBar *bar)
{
    hpBar = bar;
    if (!hpBar) return;
    hpBar->setRange(0, maxHp);
    hpBar->setValue(currentHp);
    hpBar->setFormat("HP: %v / %m");
    hpBar->setTextVisible(true);
    hpBar->setStyleSheet(
        "QProgressBar { background-color: saddlebrown; border: 1px solid black; }"
        "QProgressBar::chunk { background-color: pink; }"
    );
}

void Player::takeDamage(int dmg)
{
    currentHp -= dmg;
    if (currentHp < 0) currentHp = 0;
    updateHpBar();
    emit hpChanged(currentHp, maxHp);
}

bool Player::isDead() const
{
    return currentHp <= 0;
}

void Player::updateHpBar()
{
    if (hpBar) {
        hpBar->setRange(0, maxHp);
        hpBar->setValue(currentHp);
        hpBar->setFormat("HP: %v / %m");
    }
}

void Player::showAsHp()
{
    if (hpBar) {
        hpBar->setRange(0, maxHp);
        hpBar->setValue(currentHp);
        hpBar->setFormat("HP: %v / %m");
    }
}


void Player::startMoveTimer()
{
    if (!hpBar || moveTimer->isActive()) return;

    showAsTimer();
    currentTimeValue = moveTime;  // reset
    hpBar->setRange(0, moveTime * 10);   // 100 steps for smoothness
    hpBar->setValue(currentTimeValue * 10);

    moveTimer->start(100);  // 100 ms
}

void Player::showAsTimer()
{
    if (!hpBar) return;
    hpBar->setRange(0, moveTime);
    hpBar->setValue(moveTime);
    hpBar->setFormat("");

    // 仍用粉紅色 + 棕色背景
    hpBar->setStyleSheet(
        "QProgressBar { background-color: saddlebrown; border: 1px solid black; }"
        "QProgressBar::chunk { background-color: pink; }"
    );
}

void Player::stopMoveTimer()
{
    moveTimer->stop();
    emit moveTimeUp();  // 🔔 倒數結束，發出通知
    showAsHp();
}

void Player::onMoveTimerTimeout()
{
    if (!hpBar) return;

    currentTimeValue -= 0.1;
    if (currentTimeValue <= 0.0) {
        moveTimer->stop();
        emit moveTimeUp();
        qDebug() << "[Player] moveTime reached 0. Emitting moveTimeUp()";
        showAsHp();
        updateHpBar();
    } else {
        hpBar->setValue(static_cast<int>(currentTimeValue * 10));
    }
}

void Player::setHeroTeam(const QVector<Hero*>& team)
{
    heroes = team;
}

const QVector<Hero*>& Player::getHeroTeam() const
{
    return heroes;
}

void Player::processEnemyTurn(const QVector<Enemy*>& enemies)
{

    for (Enemy* e : enemies) {
        if (!e || e->currentHp <=0 ) continue;  // ✅ 死亡敵人不做任何事
        if (e) {
                e->applySkill_ID5(gemArea);  // 每隻敵人每回合都可施放技能
            }
        e->cd--;
        if (e->cd <= 0) {
            takeDamage(e->atk);
            e->cd = e->originalCd;
        }
        e->updateCdLabel();
        qDebug() << "[Player] enemy" << e->id << "CD now:" << e->cd;
    }
}

void Player::attackAllEnemies(QVector<Enemy*>& enemies,
                              int combo, const QMap<QString, int>& ncarPerAttr)
{
    qDebug() << "[Player] attackAllEnemies called. Combo =" << combo;
    qDebug() << "[Player] Hero count =" << heroes.size();

    // 1. AC 表
    QMap<QString, QMap<QString, double>> acTable;
    QStringList attrs = {"Water", "Fire", "Earth", "Light", "Dark"};

    for (const QString& atk : attrs) {
        for (const QString& def : attrs) {
            double ac = 1.0;
            if (atk == "Water" && def == "Fire") ac = 2.0;
            else if (atk == "Fire" && def == "Earth") ac = 2.0;
            else if (atk == "Earth" && def == "Water") ac = 2.0;
            else if (atk == "Light" && def == "Dark") ac = 2.0;
            else if (atk == "Dark" && def == "Light") ac = 2.0;

            if (def == "Water" && atk == "Earth") ac = 0.5;
            else if (def == "Fire" && atk == "Water") ac = 0.5;
            else if (def == "Earth" && atk == "Fire") ac = 0.5;
            // else if (def == "Light" && atk == "Dark") ac = 0.5;
            // else if (def == "Dark" && atk == "Light") ac = 0.5;

            acTable[atk][def] = ac;
        }
    }

    // 2. 屬性總傷害
    QMap<QString, double> totalDamage;
    for (const QString& attr : attrs) {
        int ncar = ncarPerAttr.value(attr, 0);
        totalDamage[attr] = combo * ncar;
        qDebug() << "[DmgCalc] attr:" << attr
                 << "| NCAR =" << ncar
                 << "| base dmg =" << totalDamage[attr];
    }

    // 3. 活著的敵人
    QVector<Enemy*> aliveEnemies;
    for (Enemy* e : enemies) {
        if (e && e->currentHp > 0) {
            aliveEnemies.append(e);
        }
    }
    // 4. 循序攻擊敵人
    attackSequentially(enemies, totalDamage, acTable, heroes, 0);
}

void Player::recoverHp(int combo, int nHeart)
{
    if (combo <= 0 || nHeart <= 0) return;

    int recovery = combo * nHeart * 5;
    int before = currentHp;
    currentHp += recovery;

    if (currentHp > maxHp) currentHp = maxHp;

    updateHpBar();
    emit hpChanged(currentHp, maxHp);
    qDebug() << "[Recover] Combo:" << combo
                 << "| Hearts:" << nHeart
                 << "| Recovery:" << recovery
                 << "| HP:" << before << "->" << currentHp;
}

void Player::setGemArea(GemAreaWidget* g)
{
    gemArea = g;
}

void Player::attackSequentially(QVector<Enemy*> enemies,
                                const QMap<QString, double>& totalDamage,
                                const QMap<QString, QMap<QString, double>>& acTable,
                                QVector<Hero*> heroesToAttack,
                                int index)
{
    if (index >= heroesToAttack.size()) {  // 所有英雄已出手完畢
        emit attackFinished();
        return;
    }

    Hero* h = heroesToAttack[index];
    if (!h) {
        attackSequentially(enemies, totalDamage, acTable, heroesToAttack, index + 1);
        return;
    }

    QString attr = h->attr;
    double damage = totalDamage.value(attr, 0);
    if (damage <= 0) {
        attackSequentially(enemies, totalDamage, acTable, heroesToAttack, index + 1);
        return;
    }

    QVector<Enemy*> aliveEnemies;
    for (Enemy* e : enemies) {
        if (e && e->currentHp > 0) aliveEnemies.append(e);
    }
    if (aliveEnemies.isEmpty()) {
        QTimer::singleShot(300, this, [=]() {
            emit attackFinished();  // ✅ 一定要發出
        });
        return;
    }

    Enemy* target = aliveEnemies[QRandomGenerator::global()->bounded(aliveEnemies.size())];
    double ac = acTable[attr].value(target->attr, 1.0);
    int finalDmg = static_cast<int>(damage * ac + 0.5);

    target->takeDamage(finalDmg);

    // 🔥 顯示動畫（依屬性上色）
    if (target->enemyWidget) {
        QLabel* dmgLabel = new QLabel("-" + QString::number(finalDmg), target->enemyWidget->parentWidget());
        QString color;
        if (attr == "Fire") color = "red";
        else if (attr == "Water") color = "blue";
        else if (attr == "Earth") color = "green";
        else if (attr == "Light") color = "gold";
        else if (attr == "Dark") color = "purple";
        else color = "black";

        dmgLabel->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 20px;").arg(color));
        dmgLabel->adjustSize();
        QPoint startPos = target->enemyWidget->pos() + QPoint(target->enemyWidget->width()/2 - dmgLabel->width()/2, 0);
        dmgLabel->move(startPos);
        dmgLabel->show();

        QPropertyAnimation* anim = new QPropertyAnimation(dmgLabel, "pos");
        anim->setDuration(800);
        anim->setStartValue(startPos);
        anim->setEndValue(startPos - QPoint(0, 40));
        anim->setEasingCurve(QEasingCurve::OutQuad);
        anim->start(QAbstractAnimation::DeleteWhenStopped);

        QTimer::singleShot(700, dmgLabel, [=]() {
            dmgLabel->hide();
            dmgLabel->deleteLater();
        });
    }

    // 🔁 延遞下一位 Hero 攻擊
    QTimer::singleShot(400, this, [=]() {
        attackSequentially(enemies, totalDamage, acTable, heroesToAttack, index + 1);
    });
}
