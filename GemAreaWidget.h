#pragma once

#include <QWidget>
#include <QGridLayout>
#include <QPoint>
#include <QMap>
#include "Gem.h"
#include "Player.h"


class GemAreaWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GemAreaWidget(QWidget* parent = nullptr);

    void initializeBoard();
    void resetBoard();

    void setPlayer(Player* p);
    void randomSetWeathered(int count);

    QMap<QString, int> getNcarMap() const;
    int getComboCount() const;
    bool checkAndMarkCombo();
    void dfsCombo(int r, int c, const QString& attr,
                  const QSet<QPair<int, int>>& candidates,
                  QSet<QPair<int, int>>& visited,
                  QVector<QPair<int, int>>& group); // helper of checkAndMarkCombo()
    void clearMatchedGems();
    void dropGems();
    void refillGems();
    void resolveComboCycle();

signals:
    void dragStarted(); // 發出訊號給 player
    void dragFinished();  // 玩家主動結束轉珠
    void comboResolved(int comboCount, QMap<QString, int> ncarMap); //把combo傳給mainwindow

public slots:
    void forceStopDragging();  // 給 Player 呼叫，強制停止拖曳

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    static const int ROWS = 5;
    static const int COLS = 6;
    static const int GEM_SIZE = 90;
    Gem* gemGrid[ROWS][COLS];
    QGridLayout* gridLayout;
    QPoint pressedIndex;
    QVector<QPoint> passedCells;

    bool isDragging = false;
    int totalComboCount = 0;
    QMap<QString, int> totalNcarMap;
    bool isComboResolving = false;

    QPoint getCellFromPosition(int x, int y) const;
    bool areAdjacent(QPoint a, QPoint b) const;
    void swapGems(QPoint a, QPoint b);

    Player* player = nullptr;

    int comboCount;
    QMap<QString, int> ncarMap;
    Gem* gems[5][6];
};
