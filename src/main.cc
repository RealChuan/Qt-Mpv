#include "mainwindow.hpp"

#include <QApplication>
#include <QStyle>

#include <clocale>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(a.style()->standardIcon(QStyle::SP_MediaPlay));

    // Qt sets the locale in the QApplication constructor, but libmpv requires
    // the LC_NUMERIC category to be set to "C", so change it back.
    std::setlocale(LC_NUMERIC, "C");

    MainWindow w;
    w.show();

    return a.exec();
}
