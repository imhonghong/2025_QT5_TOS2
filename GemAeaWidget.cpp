#include "GemAreaWidget.h"
#include "Gem.h"
#include <QMouseEvent>
#include <QDebug>
#include <QRandomGenerator>
#include <QPropertyAnimation>

GemAreaWidget::GemAreaWidget(QWidget* parent)
    : QWidget(parent)
{
    setFixedSize(COLS * GEM_SIZE, ROWS * GEM_SIZE);
    initializeBoard();
}

void GemAreaWidget::initializeBoard()
{
    QVector<QString> allAttrs = {"Water", "Fire", "Earth", "Light", "Dark", "Heart"};

    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < COLS; ++col) {

            QVector<QString> candidates = allAttrs;

            if (col >= 2) {
                Gem* left1 = gemGrid[row][col - 1];
                Gem* left2 = gemGrid[row][col - 2];
                if (left1 && left2 && left1->getAttr() == left2->getAttr()) {
                    candidates.removeAll(left1->getAttr());
                }
            }
            if (row >= 2) {
                Gem* up1 = gemGrid[row - 1][col];
                Gem* up2 = gemGrid[row - 2][col];
                if (up1 && up2 && up1->getAttr() == up2->getAttr()) {
                    candidates.removeAll(up1->getAttr());
                }
            }

            // 從合法屬性中選擇一個
            QString attr = candidates[QRandomGenerator::global()->bounded(candidates.size())];

            // 建立 Gem
            Gem* gem = new Gem(attr, "Normal", this);
            gem->setFixedSize(GEM_SIZE, GEM_SIZE);
            gem->move(col * GEM_SIZE, row * GEM_SIZE);  // ✅ 設定正確位置
            gem->show();                                 // ✅ 顯示出來
            gemGrid[row][col] = gem;
        }
    }
}

void GemAreaWidget::resetBoard()
{
    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < COLS; ++col) {
            if (gemGrid[row][col]) {
                delete gemGrid[row][col];
                gemGrid[row][col] = nullptr;
            }
        }
    }
    initializeBoard();
}

QPoint GemAreaWidget::getCellFromPosition(int x, int y) const
{
    return QPoint(y / GEM_SIZE, x / GEM_SIZE);  // (row, col)
}

bool GemAreaWidget::areAdjacent(QPoint a, QPoint b) const
{
    int dr = abs(a.x() - b.x());
    int dc = abs(a.y() - b.y());
    return (dr <= 1 && dc <= 1) && !(dr == 0 && dc == 0);
    // return (dr + dc == 1);
}

void GemAreaWidget::mousePressEvent(QMouseEvent* event)
{
    pressedIndex = getCellFromPosition(event->x(), event->y());
    passedCells.clear();
    isDragging = true;
    grabMouse();
    passedCells.append(pressedIndex);
    emit dragStarted();  // 發出拖曳開始訊號給 player
}

void GemAreaWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (!isDragging) return;
    QPoint currentIndex = getCellFromPosition(event->x(), event->y());
    if (currentIndex.x() < 0 || currentIndex.x() >= ROWS ||
        currentIndex.y() < 0 || currentIndex.y() >= COLS)
        return;

    Gem* currentGem = gemGrid[currentIndex.x()][currentIndex.y()];
    if (!currentGem) return;

    // ✅ 檢查是否為風化珠
    if (currentGem->getState() == "Weathered") {
        currentGem->setState("Normal");
        if (player) {
            player->takeDamage(100);
        }

        forceStopDragging();
        emit dragFinished();
        return;
    }

    QPoint last = passedCells.last();
    if (areAdjacent(currentIndex, last)) {
        swapGems(last, currentIndex);
        // qDebug() << "[GemArea] Swapping" << last << "<->" << currentIndex;
        passedCells.append(currentIndex);
    }

}

void GemAreaWidget::mouseReleaseEvent(QMouseEvent* /*event*/)
{
    if (!isDragging) return;
        isDragging = false;
    releaseMouse();
    emit dragFinished();
    if (!isComboResolving)
            resolveComboCycle();

    qDebug() << "[GemArea] Drag ended. Passed through:" << passedCells;
}

void GemAreaWidget::swapGems(QPoint a, QPoint b)
{
    int rowA = a.x(), colA = a.y();
    int rowB = b.x(), colB = b.y();

    std::swap(gemGrid[rowA][colA], gemGrid[rowB][colB]);

    Gem* gemA = gemGrid[rowA][colA];
    Gem* gemB = gemGrid[rowB][colB];

    QPoint posA(colA * GEM_SIZE, rowA * GEM_SIZE);
    QPoint posB(colB * GEM_SIZE, rowB * GEM_SIZE);

    QPropertyAnimation* animA = new QPropertyAnimation(gemA, "pos");
    animA->setDuration(80);
    animA->setEndValue(posA);
    animA->start(QAbstractAnimation::DeleteWhenStopped);

    QPropertyAnimation* animB = new QPropertyAnimation(gemB, "pos");
    animB->setDuration(80);
    animB->setEndValue(posB);
    animB->start(QAbstractAnimation::DeleteWhenStopped);
}

QMap<QString, int> GemAreaWidget::getNcarMap() const
{
    QMap<QString, int> map;
    // Placeholder for future elimination logic
    return map;
}

int GemAreaWidget::getComboCount() const
{
    return 0; // TODO: implement combo logic
}

void GemAreaWidget::forceStopDragging()
{
    if (!isDragging) return;     // ✅ 避免重複
        isDragging = false;

    releaseMouse();
    resolveComboCycle();

    qDebug() << "[GemArea] Force stopped dragging due to timeout.";
}

void GemAreaWidget::setPlayer(Player* p)
{
    player = p;
}


void GemAreaWidget::randomSetWeathered(int count)
{
    QVector<Gem*> normalGems;

    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            Gem* g = gemGrid[r][c];
            if (g && g->getState() == "Normal") {
                normalGems.append(g);
            }
        }
    }
    if (normalGems.size() < count) count = normalGems.size();

    for (int i = 0; i < count; ++i) {
        int idx = QRandomGenerator::global()->bounded(normalGems.size());
        Gem* g = normalGems[idx];
        g->setState("Weathered");
        normalGems.removeAt(idx);
    }

    qDebug() << "[GemArea] Set" << count << "weathered runestones";
}

bool GemAreaWidget::checkAndMarkCombo() {
    const int ROWS = 5;
    const int COLS = 6;
    QSet<QPair<int, int>> candidateSet;

    comboCount = 0;
    ncarMap.clear();

    // Step 1: 找出橫向三連
    for (int row = 0; row < ROWS; ++row) {
        int col = 0;
        while (col < COLS - 2) {
            QString attr = gemGrid[row][col]->getAttr();
            if (gemGrid[row][col + 1]->getAttr() == attr &&
                gemGrid[row][col + 2]->getAttr() == attr) {
                // int start = col;
                while (col < COLS && gemGrid[row][col]->getAttr() == attr) {
                    candidateSet.insert(qMakePair(row, col));
                    ++col;
                }
            } else {
                ++col;
            }
        }
    }

    // Step 2: 找出縱向三連
    for (int col = 0; col < COLS; ++col) {
        int row = 0;
        while (row < ROWS - 2) {
            QString attr = gemGrid[row][col]->getAttr();
            if (gemGrid[row + 1][col]->getAttr() == attr &&
                gemGrid[row + 2][col]->getAttr() == attr) {
                // int start = row;
                while (row < ROWS && gemGrid[row][col]->getAttr() == attr) {
                    candidateSet.insert(qMakePair(row, col));
                    ++row;
                }
            } else {
                ++row;
            }
        }
    }

    // Step 3: 找出連通區塊
    QSet<QPair<int, int>> visited;
    for (const auto& pos : candidateSet) {
        if (visited.contains(pos)) continue;

        QVector<QPair<int, int>> group;
        QString attr = gemGrid[pos.first][pos.second]->getAttr();
        dfsCombo(pos.first, pos.second, attr, candidateSet, visited, group);

        if (!group.isEmpty()) {
            comboCount++;
            ncarMap[attr] += group.size();
            for (const auto& p : group) {
                gemGrid[p.first][p.second]->setState("Clearing");
            }
        }
    }
    if (comboCount > 0) {
            qDebug() << "[GemArea] Combo detected:" << comboCount;
            return true;
        } else {
            return false;
        }

    qDebug() << "Combo Count:" << comboCount;
    qDebug() << "ncarMap:" << ncarMap;
    return comboCount > 0;
}

void GemAreaWidget::dfsCombo(int r, int c, const QString& attr,
                             const QSet<QPair<int, int>>& candidates,
                             QSet<QPair<int, int>>& visited,
                             QVector<QPair<int, int>>& group) {
    if (r < 0 || r >= 5 || c < 0 || c >= 6) return;
    QPair<int, int> pos(r, c);
    if (visited.contains(pos) || !candidates.contains(pos)) return;
    if (gemGrid[r][c]->getAttr() != attr) return;

    visited.insert(pos);
    group.append(pos);

    dfsCombo(r + 1, c, attr, candidates, visited, group);
    dfsCombo(r - 1, c, attr, candidates, visited, group);
    dfsCombo(r, c + 1, attr, candidates, visited, group);
    dfsCombo(r, c - 1, attr, candidates, visited, group);
}

void GemAreaWidget::clearMatchedGems()
{
    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < COLS; ++col) {
            Gem* gem = gemGrid[row][col];
            if (gem && gem->getState() == "Clearing") {
                gem->hide();  // ✅ 隱藏視覺
                gem->setState("Normal");  // ✅ 重設狀態（以便 drop/reuse）
                gemGrid[row][col] = nullptr;  // ✅ 標示該格為空，方便 dropGems 使用
            }
        }
    }

    qDebug() << "[GemArea] clearMatchedGems with hide()";
}

void GemAreaWidget::dropGems()
{
    const int dropDuration = 150;

    for (int col = 0; col < COLS; ++col) {
        for (int row = ROWS - 1; row >= 0; --row) {
            if (!gemGrid[row][col] || gemGrid[row][col]->getState() == "Clearing") {
                int searchRow = row - 1;
                while (searchRow >= 0) {
                    if (gemGrid[searchRow][col] && gemGrid[searchRow][col]->getState() != "Clearing") {
                        // 1. 從上層撿一顆珠子
                        Gem* fallingGem = gemGrid[searchRow][col];
                        gemGrid[row][col] = fallingGem;
                        gemGrid[searchRow][col] = nullptr;

                        // 2. 落下動畫
                        QPoint startPos = fallingGem->pos();
                        QPoint endPos = QPoint(col * GEM_SIZE, row * GEM_SIZE);

                        QPropertyAnimation* anim = new QPropertyAnimation(fallingGem, "pos");
                        anim->setDuration(dropDuration);
                        anim->setStartValue(startPos);
                        anim->setEndValue(endPos);
                        anim->setEasingCurve(QEasingCurve::OutBounce);
                        anim->start(QAbstractAnimation::DeleteWhenStopped);

                        break;
                    }
                    --searchRow;
                }
            }
        }
    }

    qDebug() << "[GemArea] dropGems animation triggered.";
}

void GemAreaWidget::refillGems()
{
    const int GEM_SIZE = 90;  // 根據你設計的大小
    QStringList attrs = {"Water", "Fire", "Earth", "Light", "Dark", "Heart"};

    for (int col = 0; col < COLS; ++col) {
        for (int row = 0; row < ROWS; ++row) {
            if (!gemGrid[row][col]) {
                QString attr = attrs[QRandomGenerator::global()->bounded(attrs.size())];

                Gem* newGem = new Gem(attr, "Normal", this);  // ✅ 設定 parent 為 gemArea
                newGem->setFixedSize(GEM_SIZE, GEM_SIZE);
                newGem->move(col * GEM_SIZE, -GEM_SIZE);  // ✅ 從上面進入，Y 正確
                newGem->show();

                gemGrid[row][col] = newGem;

                // 動畫落下
                QPropertyAnimation* anim = new QPropertyAnimation(newGem, "pos");
                anim->setDuration(200);
                anim->setStartValue(QPoint(col * GEM_SIZE, -GEM_SIZE));
                anim->setEndValue(QPoint(col * GEM_SIZE, row * GEM_SIZE));
                anim->setEasingCurve(QEasingCurve::OutBounce);
                anim->start(QAbstractAnimation::DeleteWhenStopped);
            }
        }
    }

    qDebug() << "[GemArea] refillGems: new gems generated";
}

void GemAreaWidget::resolveComboCycle()
{
    // ✅ 清空這一輪 combo 統計（防止重複累加）
    comboCount = 0;
    ncarMap.clear();

    if (!isComboResolving) {
        totalComboCount = 0;
        totalNcarMap.clear();
        isComboResolving = true;
    }

    if (checkAndMarkCombo()) {
        // ✨ 累積統計值
        totalComboCount += comboCount;
        for (const QString& key : ncarMap.keys()) {
            totalNcarMap[key] += ncarMap[key];
        }

        QTimer::singleShot(300, this, [=]() {
            clearMatchedGems();
            dropGems();

            QTimer::singleShot(300, this, [=]() {
                refillGems();

                QTimer::singleShot(400, this, [=]() {
                    resolveComboCycle();  // 🔁 再次檢查 combo
                });
            });
        });
    } else {
        isComboResolving = false;  // ✅ 遞迴終止
        comboCycleFinished = true;
        emit comboResolved(totalComboCount, totalNcarMap);  // 現有的
        emit comboFullyResolved();  // ✅ 僅這裡 emit
    }

}
