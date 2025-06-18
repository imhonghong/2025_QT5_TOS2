#pragma once

#include <QWidget>
#include <QVector>
#include "Gem.h"
#include "Player.h"
#include "Enemy.h"
#include <QMap>


class GemAreaWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GemAreaWidget(QWidget* parent = nullptr);

    void initializeBoard();
    void resetBoard();

    void setPlayer(Player* p);
    void randomSetWeathered(int count);

    Gem* getGem(int row, int col) const { return gemGrid[row][col]; };
    QVector<QPoint> getPassedCells() const { return passedCells; };
    QVector<Enemy*> getEnemies() const { return enemies; }
    void setEnemies(const QVector<Enemy*>& es) { enemies = es; };

    int getComboCount() const;

    int getTotalComboCount() const { return totalComboCount; }
    QMap<QString, int> getTotalNcarMap() const { return totalNcarMap; }

    bool checkAndMarkCombo();
    void dfsCombo(int r, int c, const QString& attr,
                  const QSet<QPair<int, int>>& candidates,
                  QSet<QPair<int, int>>& visited,
                  QVector<QPair<int, int>>& group); // helper of checkAndMarkCombo()
    void clearMatchedGems();
    void dropGems();
    void refillGems();
    void resolveComboCycle();

    void clearAllBurningGems();

    void finishDragging();
    void deleteMarkedGems();



signals:
    void dragStarted(); // 發出訊號給 player
    void dragFinished();  // 玩家主動結束轉珠
    void comboResolved(int comboCount, QMap<QString, int> ncarMap); //把combo傳給mainwindow
    void comboFullyResolved();  // combo 結束、攻擊結束後通知主控
    void comboStepResolved(int comboStepCount); //階段性combo
    void showBurnDamage(); // 顯示燃燒符石傷害

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
    QPoint pressedIndex;
    QVector<QPoint> passedCells;

    bool isDragging = false;
    int totalComboCount = 0;
    QMap<QString, int> totalNcarMap;
    bool isComboResolving = false;
    bool comboCycleFinished = false;

    QPoint getCellFromPosition(int x, int y) const;
    bool areAdjacent(QPoint a, QPoint b) const;
    void swapGems(QPoint a, QPoint b);

    Player* player = nullptr;
    QVector<Enemy*> enemies;

    QList<Gem*> gemsToDelete;
    void markGemForDelete(Gem* gem);


    int comboCount;
    QMap<QString, int> ncarMap;
};
