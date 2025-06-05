#pragma once

#include <QWidget>

class QLabel;
class QPushButton;

class FinishStageWidget : public QWidget {
    Q_OBJECT

public:
    explicit FinishStageWidget(QWidget *parent = nullptr);
    void setResult(bool win);

signals:
    void restartGame();
    void exitGame();

private:
    QLabel *resultLabel;
    QPushButton *restartButton;
    QPushButton *exitButton;
};
