#include "GemAreaWidget.h"
#include "Gem.h"
#include <QMouseEvent>
#include <QDebug>
#include <QRandomGenerator>

GemAreaWidget::GemAreaWidget(QWidget* parent)
    : QWidget(parent), gridLayout(new QGridLayout(this))
{
    setFixedSize(6 * 50, 5 * 50);
    setLayout(gridLayout);
    gridLayout->setSpacing(0);
    gridLayout->setMargin(0);
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
            gemGrid[row][col] = gem;
            gridLayout->addWidget(gem, row, col);
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
    return QPoint(y / 90, x / 90);  // (row, col)
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
    hasComboChecked = false;
    grabMouse();
    passedCells.append(pressedIndex);
    emit dragStarted();  // 發出拖曳開始訊號給 player
}

void GemAreaWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (!isDragging) return;
    QPoint currentIndex = getCellFromPosition(event->x(), event->y());
    if (!gridLayout->itemAtPosition(currentIndex.x(), currentIndex.y())) return;
    // if (passedCells.contains(currentIndex)) return;

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
    checkAndMarkCombo();
    emit dragFinished();
    qDebug() << "[GemArea] Drag ended. Passed through:" << passedCells;
}

void GemAreaWidget::swapGems(QPoint a, QPoint b)
{
    if (!gridLayout->itemAtPosition(a.x(), a.y()) || !gridLayout->itemAtPosition(b.x(), b.y()))
        return;

    QWidget* widgetA = gridLayout->itemAtPosition(a.x(), a.y())->widget();
    QWidget* widgetB = gridLayout->itemAtPosition(b.x(), b.y())->widget();

    if (!widgetA || !widgetB) return;

    gridLayout->removeWidget(widgetA);
    gridLayout->removeWidget(widgetB);

    gridLayout->addWidget(widgetA, b.x(), b.y());
    gridLayout->addWidget(widgetB, a.x(), a.y());

    std::swap(gemGrid[a.x()][a.y()], gemGrid[b.x()][b.y()]);
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
    if (!hasComboChecked) {
        emit dragFinished(); // ⚠️ 僅 emit 一次
    }
    releaseMouse();
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

void GemAreaWidget::checkAndMarkCombo() {
    const int ROWS = 5;
    const int COLS = 6;
    QSet<QPair<int, int>> candidateSet;

    comboCount = 0;
    ncarMap.clear();
    if (hasComboChecked) return;
    hasComboChecked = true;

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
    emit comboResolved(comboCount, ncarMap);
    qDebug() << "Combo Count:" << comboCount;
    qDebug() << "ncarMap:" << ncarMap;
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

void GemAreaWidget::clearMatchedGems() {
    const int ROWS = 5;
    const int COLS = 6;

    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < COLS; ++col) {
            Gem* gem = gems[row][col];
            if (gem && gem->getState() == "Clearing") {
                gridLayout->removeWidget(gem);
                gem->setParent(nullptr);
                delete gem;
                gems[row][col] = nullptr;
            }
        }
    }
}
