#ifndef PREVIEWWIDGET_HPP
#define PREVIEWWIDGET_HPP

#include "mpvwidget.hpp"

namespace Mpv {

class PreviewWidget : public MpvWidget
{
public:
    PreviewWidget(QWidget *parent = nullptr);
    ~PreviewWidget();

    void startPreview(const QString &filepath, int timestamp);
    void clearAllTask();

private:
    class PreviewWidgetPrivate;
    QScopedPointer<PreviewWidgetPrivate> d_ptr;
};

} // namespace Mpv

#endif // PREVIEWWIDGET_HPP
