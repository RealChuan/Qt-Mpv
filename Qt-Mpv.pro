QT       += core gui multimedia network widgets

CONFIG += c++17

QT_CONFIG -= no-pkg-config
CONFIG += link_pkgconfig debug
#PKGCONFIG += mpv

INCLUDEPATH += C:/3rd/mpv/include
LIBS += C:/3rd/mpv/libmpv.dll.a

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
