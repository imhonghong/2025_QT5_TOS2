#include "Gem.h"
#include <QRandomGenerator>
#include <QMap>

Gem::Gem(QWidget* parent)
    : QLabel(parent), attr("Water"), state("Normal")
{
    setFixedSize(90, 90);  // 符合盤面格子大小
    setScaledContents(true);  // 讓圖片填滿
    updateIcon();
}
Gem::Gem(QString attr, QString state, QWidget* parent)
    : QLabel(parent), attr(attr), state(state)
{
    updateIcon();
    setFixedSize(90, 90);
    setAlignment(Qt::AlignCenter);
}

void Gem::setAttr(const QString& a)
{
    attr = a;
    updateIcon();
}

void Gem::setState(const QString& s)
{
    state = s;
    updateIcon();
}

QString Gem::getAttr() const
{
    return attr;
}

QString Gem::getState() const
{
    return state;
}

void Gem::updateIcon()
{
    QMap<QString, int> attrCode = {
        {"Water", 1}, {"Fire", 2}, {"Earth", 3},
        {"Light", 4}, {"Dark", 5}, {"Heart", 6}
    };
    QMap<QString, int> stateCode = {
        {"Normal", 0}, {"Weathered", 1}, {"Burning", 2}
    };

    int a = attrCode.value(attr, 1);   // 預設為 Water
    int s = stateCode.value(state, 0); // 預設為 Normal

    QString path = QString(":/gem/data/gem/gem%1%2.png").arg(s).arg(a);
    QPixmap pix(path);

    if (!pix.isNull()) {
        setPixmap(pix);
    } else {
        setText(attr + "\n" + state);  // fallback
    }
}

void Gem::randomize()
{
    QStringList attrs = {"Water", "Fire", "Earth", "Light", "Dark", "Heart"};
    int index = QRandomGenerator::global()->bounded(attrs.size());
    attr = attrs[index];
    state = "Normal";
    updateIcon();
}
