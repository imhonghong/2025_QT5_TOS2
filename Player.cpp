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
    if (hpTextLabel) {
            hpTextLabel->setText(QString("HP: %1 / %2").arg(currentHp).arg(maxHp));
        }
    if (hpBar) {
        hpBar->setValue(currentHp);
    }
}

void Player::bindHpBar(QProgressBar* bar, QLabel* label)
{
    hpBar = bar;
    hpTextLabel = label;

    if (!hpBar) return;

    hpBar->setRange(0, maxHp);
    hpBar->setValue(currentHp);
    hpBar->setTextVisible(false);  // 由文字控件顯示

    // 若 label 存在，顯示數字
    if (hpTextLabel) {
        hpTextLabel->setText(QString("HP: %1 / %2").arg(currentHp).arg(maxHp));
        hpTextLabel->show();
    }
}

void Player::takeDamage(int dmg)
{
    currentHp -= dmg;
    if (currentHp < 0) currentHp = 0;
    updateHpBar();
    emit hpChanged(currentHp, maxHp);
    if (hpTextLabel) {
        hpTextLabel->setText(QString("HP: %1 / %2").arg(currentHp).arg(maxHp));
    }
    if (currentHp <= 0) {
        qDebug() << "[Player] HP dropped to 0 — emit playerDead";
        emit playerDead();
    }
}

bool Player::isDead() const
{
    return currentHp <= 0;
}

void Player::updateHpBar()
{
    if (!hpBar) return;

    if (!isMoving) {
        // ✅ 只有非轉珠時才更新條的長度
        hpBar->setRange(0, maxHp);
        hpBar->setValue(currentHp);

        // 顏色條件可自選（保持舊邏輯也行）
        int percent = (currentHp * 100) / maxHp;
        QString color;
        if (percent >= 0) color = "pink";
        // else if (percent >= 30) color = "orange";
        //else color = "red";

        hpBar->setStyleSheet(
            "QProgressBar { background-color: saddlebrown; border: 1px solid black; }"
            "QProgressBar::chunk { background-color: " + color + "; }"
        );
        if (hpTextLabel) {
            hpTextLabel->setText(QString("HP: %1 / %2").arg(currentHp).arg(maxHp));
        }
    }

    // ✅ 無論是否在轉珠，文字永遠更新
    hpBar->setFormat(QString("HP: %1 / %2").arg(currentHp).arg(maxHp));
    hpBar->setAlignment(Qt::AlignRight);
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
    isMoving = true;  // ✅ 進入轉珠狀態
    showAsTimer();
    currentTimeValue = moveTime;  // reset
    // ✅ 初始化條件：進度條 0~100
    hpBar->setRange(0, 100);
    int percent = static_cast<int>((currentTimeValue / moveTime) * 100);
    hpBar->setValue(percent);
    hpBar->setFormat(QString("HP: %1 / %2").arg(currentHp).arg(maxHp));  // 血量數字

    moveTimer->start(100);  // 100 ms
}

void Player::showAsTimer()
{
    if (!hpBar) return;
    hpBar->setRange(0, moveTime);

    // 仍用粉紅色 + 棕色背景
    hpBar->setStyleSheet(
        "QProgressBar { background-color: saddlebrown; border: 1px solid black; }"
        "QProgressBar::chunk { background-color: pink; }"
    );
}

void Player::stopMoveTimer()
{
    moveTimer->stop();
    isMoving = false;  // ✅ 回復一般狀態
    emit moveTimeUp();  // 🔔 倒數結束，發出通知
    showAsHp();
    updateHpBar();
    if (timerAnim && timerAnim->state() == QAbstractAnimation::Running)
        timerAnim->stop();
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
        int newValue = static_cast<int>((currentTimeValue / moveTime) * 100);

        // ✅ 使用動畫平滑設定進度條值
        if (!timerAnim) {
            timerAnim = new QPropertyAnimation(hpBar, "value", this);
            timerAnim->setDuration(80);  // 平滑時長
            timerAnim->setEasingCurve(QEasingCurve::Linear);
        }
        timerAnim->stop();
        timerAnim->setStartValue(hpBar->value());
        timerAnim->setEndValue(newValue);
        timerAnim->start();

        hpBar->setFormat(QString("HP: %1 / %2").arg(currentHp).arg(maxHp));  // 保留血量文字
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
    processEnemyTurnSequentially(enemies, 0);  // ✅ 開始第一隻
}

void Player::processEnemyTurnSequentially(const QVector<Enemy*>& enemies, int index)
{
    if (index >= enemies.size()) {
        emit enemyAttackFinished();  // ✅ 最後才 emit 回合結束
        return;
    }

    Enemy* e = enemies[index];
    if (!e || e->currentHp <= 0) {
        processEnemyTurnSequentially(enemies, index + 1);  // skip
        return;
    }

    e->cd--;

    if (e->cd <= 0) {
        showBulletAnimation(e->iconLabel, hpBar, bulletParent);
        takeDamage(e->atk);
        e->cd = e->originalCd;
    }

    e->updateCdLabel();

    // 技能觸發條件仍保留
    if (e->id == 5 && e->currentHp > 0) {
        e->applySkill_ID5(gemArea);
    }

    // 等待動畫延遲後進入下一隻敵人
    QTimer::singleShot(300, [=]() {
        processEnemyTurnSequentially(enemies, index + 1);
    });
}

void Player::attackAllEnemies(QVector<Enemy*>& enemies,
                              int combo, const QMap<QString, int>& ncarPerAttr)
{
    qDebug() << "[Player] attackAllEnemies called. Combo =" << combo;
    // qDebug() << "[Player] Hero count =" << heroes.size();

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
    hasEmittedAttackFinished = false;
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

void Player::setBulletParent(QWidget* parent) {
    bulletParent = parent;
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
    if (index >= heroesToAttack.size()) {  // 所有英雄出手完畢
        if (!hasEmittedAttackFinished) {
            hasEmittedAttackFinished = true;
            emit attackFinished();
        }
        return;
    }

    Hero* h = heroesToAttack[index];
    if (!h) {
        QTimer::singleShot(150, this, [=]() {
            attackSequentially(enemies, totalDamage, acTable, heroesToAttack, index + 1);
        });
        return;
    }

    QString attr = h->attr;
    double damage = totalDamage.value(attr, 0);
    if (damage <= 0) {
        QTimer::singleShot(150, this, [=]() {
            attackSequentially(enemies, totalDamage, acTable, heroesToAttack, index + 1);
        });
        return;
    }

    QVector<Enemy*> aliveEnemies;
    for (Enemy* e : enemies) {
        if (e && e->currentHp > 0) aliveEnemies.append(e);
    }

    if (aliveEnemies.isEmpty()) {
        QTimer::singleShot(300, this, [=]() {
            if (!hasEmittedAttackFinished) {
                hasEmittedAttackFinished = true;
                emit attackFinished();
            }
        });
        return;
    }

    Enemy* target = aliveEnemies[QRandomGenerator::global()->bounded(aliveEnemies.size())];
    double ac = acTable[attr].value(target->attr, 1.0);
    int finalDmg = round(damage * ac);

    qDebug() << "[Hero] icon parent:" << h->iconLabel->parentWidget();
    qDebug() << "[Enemy] icon parent:" << target->iconLabel->parentWidget();
    showBulletAnimation(h->iconLabel, target->iconLabel, bulletParent);
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

void Player::showBulletAnimation(QWidget* from, QWidget* to, QWidget* commonParent)
{
    if (!from || !to || !commonParent) return;

    QLabel* bullet = new QLabel(commonParent);
    bullet->setPixmap(QPixmap(":/bullet/data/bullet.png").scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    bullet->resize(40, 40);
    bullet->setAttribute(Qt::WA_TransparentForMouseEvents);

    QPoint start = from->mapTo(commonParent, from->rect().center()) - QPoint(20, 20);
    QPoint end = to->mapTo(commonParent, to->rect().center()) - QPoint(20, 20);

    bullet->move(start);
    bullet->show();

    QPropertyAnimation* anim = new QPropertyAnimation(bullet, "pos");
    anim->setDuration(400);
    anim->setStartValue(start);
    anim->setEndValue(end);
    anim->setEasingCurve(QEasingCurve::InOutQuad);
    anim->start(QAbstractAnimation::DeleteWhenStopped);

    connect(anim, &QPropertyAnimation::finished, bullet, &QLabel::deleteLater);
}
