#ifndef MPVOPENGLWIDGET_HPP
#define MPVOPENGLWIDGET_HPP

#include <QOpenGLWidget>

namespace Mpv {

class MpvPlayer;

class MpvOpenglWidget Q_DECL_FINAL : public QOpenGLWidget
{
    Q_OBJECT
public:
    explicit MpvOpenglWidget(Mpv::MpvPlayer *mpvPlayer, QWidget *parent = nullptr);
    ~MpvOpenglWidget();

protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;

private Q_SLOTS:
    void maybeUpdate();
    void onBeforeResize();
    void onAfterResize();

private:
    static void on_update(void *ctx);

    void buildConnect();

    class MpvOpenglWidgetPrivate;
    QScopedPointer<MpvOpenglWidgetPrivate> d_ptr;
};

} // namespace Mpv

#endif // MPVOPENGLWIDGET_HPP
