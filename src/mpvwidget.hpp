#ifndef MPVWIDGET_HPP
#define MPVWIDGET_HPP

#include <QWidget>

namespace Mpv {

class MpvWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MpvWidget(QWidget *parent = nullptr);
    ~MpvWidget() override;
};

} // namespace Mpv

#endif // MPVWIDGET_HPP
