#include "mpvlogwindow.hpp"

#include <QtWidgets>

namespace Mpv {

class MpvLogWindow::MpvLogWindowPrivate
{
public:
    MpvLogWindowPrivate(MpvLogWindow *parent)
        : owner(parent)
    {
        textEdit = new QTextEdit(owner);
        textEdit->setReadOnly(true);
    }

    MpvLogWindow *owner;

    QTextEdit *textEdit;
};

MpvLogWindow::MpvLogWindow(QWidget *parent)
    : QMainWindow{parent}
    , d_ptr(new MpvLogWindowPrivate(this))
{
    setWindowTitle("mpv log window");
    setCentralWidget(d_ptr->textEdit);
}

MpvLogWindow::~MpvLogWindow() {}

void MpvLogWindow::onAppendLog(const QString &log)
{
    auto cursor = d_ptr->textEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(log);
    d_ptr->textEdit->setTextCursor(cursor);
}

} // namespace Mpv
