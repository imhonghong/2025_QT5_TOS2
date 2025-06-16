#pragma once

#include <QLabel>
#include <QString>
#include <QPixmap>

class Gem : public QLabel
{
    Q_OBJECT

public:
    explicit Gem(QWidget* parent = nullptr);

    void setAttr(const QString& attr);     // e.g., "Fire", "Water"
    void setState(const QString& state);   // e.g., "Normal", "Burning", "Weathered"
    QString getAttr() const;
    QString getState() const;

    void updateIcon();  // 根據屬性與狀態設定圖片
    void randomize();   // 隨機設定 attr 和 state 為 normal

private:
    QString attr;   // 水火木光暗心
    QString state;  // Normal / Burning / Weathered
};
