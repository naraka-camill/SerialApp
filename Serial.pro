QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += src \
    src/3rdParty

SOURCES += \
    main.cpp \
    app.cpp \
    src/3rdParty/serial/impl/list_ports/list_ports_linux.cpp \
    src/3rdParty/serial/impl/list_ports/list_ports_osx.cpp \
    src/3rdParty/serial/impl/list_ports/list_ports_win.cpp \
    src/3rdParty/serial/impl/unix.cpp \
    src/3rdParty/serial/impl/win.cpp \
    src/3rdParty/serial/serial.cpp

HEADERS += \
    app.h \
    src/3rdParty/nlohmann/json.hpp \
    src/3rdParty/serial/impl/unix.h \
    src/3rdParty/serial/impl/win.h \
    src/3rdParty/serial/serial.h \
    src/3rdParty/serial/v8stdint.h

FORMS += \
    app.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

LIBS += -lsetupapi

RC_FILE = windows_icon.rc
