#include "Player.h"
#include <QDebug>
#include "Enemy.h"

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
        e->cd--;
        if (e->cd <= 0) {
            takeDamage(e->atk);
            e->cd = e->originalCd;
        }
        e->updateCdLabel();
        qDebug() << "[Player] enemy" << e->id << "CD now:" << e->cd;
    }
}
