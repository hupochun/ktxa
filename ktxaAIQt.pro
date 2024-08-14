QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets network

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    advancesettingdialog.cpp \
    helpdialog.cpp \
    httpreqmanager.cpp \
    main.cpp \
    mainwindow.cpp \
    ocrdrawwidget.cpp \
    ocrresult.cpp \
    qaitem.cpp \
    qthttprequestmanager.cpp \
    revise.cpp \
    selectedrect.cpp \
    selectrect.cpp \
    settings.cpp \
    util.cpp

HEADERS += \
    advancesettingdialog.h \
    common_defs.h \
    helpdialog.h \
    httpreqmanager.h \
    mainwindow.h \
    ocrdrawwidget.h \
    ocrresult.h \
    qaitem.h \
    qthttprequestmanager.h \
    revise.h \
    selectedrect.h \
    selectrect.h \
    settings.h \
    util.h

FORMS += \
    advancesettingdialog.ui \
    helpdialog.ui \
    mainwindow.ui \
    ocrdrawwidget.ui \
    qaitem.ui \
    revise.ui \
    selectedrect.ui \
    selectrect.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

LIBS += -lUser32

RESOURCES += \
    resource.qrc

RC_FILE += visualstudio.rc


TRANSLATIONS = zh_CN.ts

DESTDIR  = ../bin
