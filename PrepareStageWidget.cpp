#include "PrepareStageWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QIcon>
#include <QPixmap>
#include <QVariant>
#include <QMessageBox>
PrepareStageWidget::PrepareStageWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 初始化 hero pool  /hero/data/hero/
    heroPool.append(new Hero(1, "Water", 0, 1, 0, ":/hero/data/hero/ID1.png"));
    heroPool.append(new Hero(2, "Fire", 0, 1, 0, ":/hero/data/hero/ID2.png"));
    heroPool.append(new Hero(3, "Earth", 0, 1, 0, ":/hero/data/hero/ID3.png"));
    heroPool.append(new Hero(4, "Light", 0, 1, 0, ":/hero/data/hero/ID4.png"));
    heroPool.append(new Hero(5, "Dark", 0, 1, 0, ":/hero/data/hero/ID5.png"));

    mainLayout->addStretch();

    // (1) Player's Character: 標題
    QLabel *labelTitle = new QLabel("Player's Character:", this);
    QFont font1;
    font1.setPointSize(18);
    labelTitle->setFont(font1);
    mainLayout->addWidget(labelTitle, 0, Qt::AlignLeft);


    // 建立 6 格 ComboBox（可為空）
    QHBoxLayout *heroRow = new QHBoxLayout;
    heroRow -> setSpacing(5);

    for (int i = 0; i < 6; ++i) {
        QComboBox *box = new QComboBox(this);
        box->addItem("Empty", QVariant::fromValue<Hero*>(nullptr));
        for (Hero* h : heroPool) {
            box->addItem(QIcon(h->iconPath), QString("ID %1").arg(h->id), QVariant::fromValue(h));
            box->setIconSize(QSize(60, 60));
        }

        heroBoxes.append(box);
        selectedHeroes.append(nullptr);  // 初始為空

        // 更新選角時修改 selectedHeroes
        connect(box, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int){
            Hero* h = box->currentData().value<Hero*>();
            selectedHeroes[i] = h;
        });

        heroRow->addWidget(box);
    }

    mainLayout->addLayout(heroRow);
    mainLayout->addStretch();


    // Mission title
    QLabel *labelMission = new QLabel("Game Mission:", this);
    QFont font2;
    font2.setPointSize(18);
    labelMission->setFont(font2);
    mainLayout->addWidget(labelMission, 0, Qt::AlignLeft);

    // Mission spinbox
    missionSpinBox = new QSpinBox(this);
    missionSpinBox->setRange(1, 1); // 目前僅 mission 1
    missionSpinBox->setFixedSize(100, 40); // 目前僅 mission 1
    mainLayout->addWidget(missionSpinBox, 0, Qt::AlignHCenter);

    mainLayout->addStretch();

    // Start 按鈕
    QPushButton *startBtn = new QPushButton("Start", this);
    startBtn->setFixedSize(200, 60);
    mainLayout->addWidget(startBtn, 0, Qt::AlignHCenter);

    mainLayout->addSpacing(20);


    connect(startBtn, &QPushButton::clicked, this, &PrepareStageWidget::startGame);
}

QVector<Hero*> PrepareStageWidget::getSelectedHeroes() const {
    return selectedHeroes;
}

int PrepareStageWidget::getSelectedMission() const {
    return missionSpinBox->value();
}
