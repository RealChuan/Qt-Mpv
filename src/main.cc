#include "mainwindow.hpp"

#include <QApplication>
#include <QStyle>
#include <QSurfaceFormat>

#include <clocale>

void setSurfaceFormatVersion(int major, int minor)
{
    auto surfaceFormat = QSurfaceFormat::defaultFormat();
    surfaceFormat.setVersion(major, minor);
    surfaceFormat.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(surfaceFormat);
}

int main(int argc, char *argv[])
{
    setSurfaceFormatVersion(3, 3);

    QApplication a(argc, argv);
    a.setWindowIcon(a.style()->standardIcon(QStyle::SP_MediaPlay));

    // Qt sets the locale in the QApplication constructor, but libmpv requires
    // the LC_NUMERIC category to be set to "C", so change it back.
    std::setlocale(LC_NUMERIC, "C");

    MainWindow w;
    w.show();

    return a.exec();
}
