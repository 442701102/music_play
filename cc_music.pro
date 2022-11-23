QT       += core gui
QT       += sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += c++11
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Media/AVPacketManager.cpp \
    Media/ffmpeg_api.cpp \
    audio_thread.cpp \
    ddaudio.cpp \
    ddaudio_info.cpp \
    main.cpp \
    mainwindow.cpp \
    qsqlite.cpp \
    ui_request.cpp

HEADERS += \
    Media/AVPacketManager.h \
    ddaudio.h \
    env.h \
    mainwindow.h \
    qsqlite.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


win32+=
{
LIBS += -LD:/LIB/SDL/lib/ -lSDL2
LIBS += -LD:/LIB/ffmpeg/lib/\
    -lavcodec \
    -lavdevice \
    -lavfilter \
    -lavformat \
    -lavutil \
    -lswresample \
    -lswscale

INCLUDEPATH += D:/LIB/SDL/include/SDL2
DEPENDPATH += D:/LIB/SDL/include/SDL2

INCLUDEPATH += D:/LIB/ffmpeg/include
DEPENDPATH += D:/LIB/ffmpeg/include

DEFINES += QT_DEPRECATED_WARNINGS \
    SDL_MAIN_HANDLED
}

RESOURCES += \
    ccResources.qrc
