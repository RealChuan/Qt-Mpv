QT       += core gui multimedia network widgets openglwidgets

CONFIG += c++17 sdk_no_version_check

QT_CONFIG -= no-pkg-config
CONFIG += link_pkgconfig debug
#PKGCONFIG += mpv

win32 {
    INCLUDEPATH += C:/3rd/mpv/include
    LIBS += C:/3rd/mpv/libmpv.dll.a
}

macx {
    INCLUDEPATH += /usr/local/include
    LIBS += -L/usr/lib -L/usr/local/lib
    LIBS += -lmpv
}

unix:!macx{
    LIBS += -lmpv
}

contains(QT_ARCH, i386) {
    BIN = bin-32
}else{
    BIN = bin-64
}

CONFIG(debug, debug|release) {
    APP_OUTPUT_PATH = $$PWD/$$BIN/Debug
}else{
    APP_OUTPUT_PATH = $$PWD/$$BIN/Release
}

DESTDIR = $$APP_OUTPUT_PATH

include(src/src.pri)

DISTFILES += \
    README.md
