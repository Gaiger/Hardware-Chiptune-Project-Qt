QT += widgets
QT += multimedia

CONFIG += c++11 #console
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        ../chip.c \
        AudioPlayer.cpp \
        HardwareChiptunePanelWidget.cpp \
        SongPlainTextEdit.cpp \
        TrackPlainTextEdit.cpp \
        WaveGenerator.cpp \
        main.cpp \
        song_manager.c

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    ../stuff.h \
    AudioPlayer.h \
    HardwareChiptunePanelWidget.h \
    SongPlainTextEdit.h \
    TrackPlainTextEdit.h \
    WaveGenerator.h \
    song_manager.h

FORMS += \
    HardwareChiptunePanelWidget.ui
