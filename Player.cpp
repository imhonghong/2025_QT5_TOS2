#include "Player.h"
#include <QDebug>
#include "Enemy.h"
#include <QRandomGenerator>

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
                               int combo,
                               const QMap<QString, int>& ncarPerAttr)
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
            else if (def == "Light" && atk == "Dark") ac = 0.5;
            else if (def == "Dark" && atk == "Light") ac = 0.5;

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

    // 4. 每隻 Hero 出手一次
    for (Hero* h : heroes) {
        if (!h) continue;

        QString attr = h->attr;
        double damage = totalDamage.value(attr, 0);
        qDebug() << "[Hero] ID:" << h->id
                 << "| Attr:" << attr
                 << "| ATK:" << h->atk
                 << "| totalDamage =" << damage;

        if (damage <= 0) continue;

        // 找一隻還活著的敵人
        QVector<Enemy*> candidates;
        for (Enemy* e : aliveEnemies) {
            if (e->currentHp > 0)
                candidates.append(e);
        }

        if (candidates.isEmpty()) return;

        int idx = QRandomGenerator::global()->bounded(candidates.size());
        Enemy* target = candidates[idx];

        double ac = acTable[attr].value(target->attr, 1.0);
        int finalDmg = static_cast<int>(damage * ac + 0.5);

        qDebug() << " -> Attacking enemy ID:" << target->id
                 << "(Attr:" << target->attr
                 << "| HP before:" << target->currentHp
                 << ") AC =" << ac
                 << "| Final Damage =" << finalDmg;

        target->takeDamage(finalDmg);

        qDebug() << "    >> Enemy ID:" << target->id
                 << "| HP after:" << target->currentHp
                 << (target->currentHp <= 0 ? "(DEFEATED)" : "");
    }
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
