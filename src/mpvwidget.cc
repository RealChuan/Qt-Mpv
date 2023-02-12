#include "mpvwidget.hpp"

namespace Mpv {

MpvWidget::MpvWidget(QWidget *parent)
    : QWidget{parent}
{
    setAttribute(Qt::WA_DontCreateNativeAncestors);
    setAttribute(Qt::WA_NativeWindow);
}

MpvWidget::~MpvWidget() {}

} // namespace Mpv
