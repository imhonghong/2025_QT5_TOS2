QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Enemy.cpp \
    FinishStageWidget.cpp \
    GameStageWidget.cpp \
    Gem.cpp \
    GemAeaWidget.cpp \
    Hero.cpp \
    PauseStageWidget.cpp \
    Player.cpp \
    PrepareStageWidget.cpp \
    main.cpp \
    MainWindow.cpp

HEADERS += \
    Enemy.h \
    FinishStageWidget.h \
    GameStageWidget.h \
    Gem.h \
    GemAreaWidget.h \
    Hero.h \
    MainWindow.h \
    PauseStageWidget.h \
    Player.h \
    PrepareStageWidget.h

FORMS += \
    mainwindow.ui

TRANSLATIONS += \
    TOS2_zh_TW.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    data.qrc
