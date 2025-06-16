#include "GemAreaWidget.h"
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
    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < COLS; ++col) {
            Gem* gem = new Gem(this);
            gem->randomize();
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
        passedCells.append(currentIndex);
    }
}

void GemAreaWidget::mouseReleaseEvent(QMouseEvent* /*event*/)
{
    if (!isDragging) return;
        isDragging = false;
    releaseMouse();
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
