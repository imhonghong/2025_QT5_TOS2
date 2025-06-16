#pragma once

#include <QWidget>
#include <QGridLayout>
#include <QPoint>
#include "Gem.h"
#include "Player.h"


class GemAreaWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GemAreaWidget(QWidget* parent = nullptr);

    void initializeBoard();
    void resetBoard();
    QMap<QString, int> getNcarMap() const;
    int getComboCount() const;

    void setPlayer(Player* p);
    void randomSetWeathered(int count);


signals:
    void dragStarted(); // 發出訊號給 player
    void dragFinished();  // 玩家主動結束轉珠

public slots:
    void forceStopDragging();  // 給 Player 呼叫，強制停止拖曳

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    static const int ROWS = 5;
    static const int COLS = 6;
    Gem* gemGrid[ROWS][COLS];
    QGridLayout* gridLayout;
    QPoint pressedIndex;
    QVector<QPoint> passedCells;

    bool isDragging = false;

    QPoint getCellFromPosition(int x, int y) const;
    bool areAdjacent(QPoint a, QPoint b) const;
    void swapGems(QPoint a, QPoint b);

    Player* player = nullptr;
};
