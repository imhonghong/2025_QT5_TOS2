// PauseStageWidget.h
#ifndef PAUSESTAGEWIDGET_H
#define PAUSESTAGEWIDGET_H

#include <QWidget>
#include <QLabel>

class QPushButton;

class PauseStageWidget : public QWidget {
    Q_OBJECT

public:
    explicit PauseStageWidget(QWidget *parent = nullptr);

signals:
    void resumeGame();
    void surrender();

private:
    QLabel      *pauseTitle;
    QPushButton *resumeButton;
    QPushButton *surrenderButton;
};

#endif // PAUSESTAGEWIDGET_H
