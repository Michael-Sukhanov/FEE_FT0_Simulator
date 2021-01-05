QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 console

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    auxiliary/mrand.cpp \
    fee.cpp \
    ipbuspacket.cpp \
    ipbustransaction.cpp \
    log.cpp \
    main.cpp \
    mainwindow.cpp \
    server.cpp

HEADERS += \
    IPbusHeaders.h \
    RegisterMap.h \
    auxiliary/mrand.h \
    fee.h \
    gbt.h \
    ipbuspacket.h \
    ipbustransaction.h \
    log.h \
    mainwindow.h \
    server.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
