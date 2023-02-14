#include "previewwidget.hpp"
#include "mpvplayer.hpp"

namespace Mpv {

class PreviewWidget::PreviewWidgetPrivate
{
public:
    PreviewWidgetPrivate(PreviewWidget *parent)
        : owner(parent)
    {
        mpvPlayer = new Mpv::MpvPlayer(owner);
        mpvPlayer->initMpv(owner);
    }

    PreviewWidget *owner;

    Mpv::MpvPlayer *mpvPlayer;
};

PreviewWidget::PreviewWidget(QWidget *parent)
    : MpvWidget(parent)
    , d_ptr(new PreviewWidgetPrivate(this))
{
    d_ptr->mpvPlayer->setUseGpu(true);
    d_ptr->mpvPlayer->setCache(false);
    d_ptr->mpvPlayer->pause();
}

PreviewWidget::~PreviewWidget() {}

void PreviewWidget::startPreview(const QString &filepath, int timestamp)
{
    if (filepath != d_ptr->mpvPlayer->filepath()) {
        d_ptr->mpvPlayer->openMedia(filepath);
        d_ptr->mpvPlayer->blockAudioTrack();
        d_ptr->mpvPlayer->blockSubTrack();
    }
    d_ptr->mpvPlayer->seek(timestamp);
}

void PreviewWidget::clearAllTask()
{
    d_ptr->mpvPlayer->abortAllAsyncCommands();
}

} // namespace Mpv
