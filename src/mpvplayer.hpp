#ifndef MPVPLAYER_HPP
#define MPVPLAYER_HPP

#include "trackinfo.hpp"

#include <QObject>

struct mpv_handle;

namespace Mpv {

class MpvPlayer : public QObject
{
    Q_OBJECT
public:
    enum GpuApiType {
        Auto,
        Opengl,
        Vulkan,
#ifdef Q_OS_WIN
        D3d11
#endif
    };
    Q_ENUM(GpuApiType)

    explicit MpvPlayer(QObject *parent = nullptr);
    ~MpvPlayer();

    void initMpv(QWidget *widget);

    void openMedia(const QString &filePath);

    QString filename() const;
    QString filepath() const;
    double filesize() const;
    double duration() const; // s
    double position() const; // s

    TraskInfoList audioTrackList() const;
    TraskInfoList subTrackList() const;
    void setAudioTrack(int aid);
    void blockAudioTrack();
    void setSubTrack(int sid);
    void blockSubTrack();

    void setPrintToStd(bool print);
    void setCache(bool cache);
    void setUseGpu(bool use);
    void setGpuApi(GpuApiType type);
    void setVolume(int value);
    void seek(qint64 seconds);
    void seekRelative(qint64 seconds);
    void setSpeed(double speed);
    void pause();

    int volumeMax() const;

    void abortAllAsyncCommands();

    void destroy();

    mpv_handle *mpv_handle();

signals:
    void fileLoaded();
    void trackChanged();
    void positionChanged(double position); // ms
    void mpvLogMessage(const QString &log);
    void cacheSpeedChanged(int64_t);

private slots:
    void onMpvEvents();

private:
    class MpvPlayerPrivate;
    QScopedPointer<MpvPlayerPrivate> d_ptr;
};

} // namespace Mpv

#endif // MPVPLAYER_HPP
