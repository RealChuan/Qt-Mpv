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

    void play();
    void stop();

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

    void addAudio(const QStringList &paths);
    void addSub(const QStringList &paths);

    void setPrintToStd(bool print);

    void setCache(bool cache);
    void setCacheSeconds(int seconds);
    double cacheSpeed() const;        // s
    void setCacheSpeed(double speed); // bytes / s

    void setUseGpu(bool use);
    void setGpuApi(GpuApiType type);

    void setVolume(int value);
    int volume() const;
    int volumeMax() const;

    void seek(qint64 seconds);
    void seekRelative(qint64 seconds);

    void setSpeed(double speed);
    double speed() const;

    void pause();
    bool pausing();

    void abortAllAsyncCommands();

    void destroy();

    mpv_handle *mpv_handler();

signals:
    void fileLoaded();
    void fileFinished();
    void trackChanged();
    void durationChanged(double duration); // ms
    void positionChanged(double position); // ms
    void mpvLogMessage(const QString &log);
    void pauseStateChanged(bool state);
    void cacheSpeedChanged(int64_t);

private slots:
    void onMpvEvents();

private:
    class MpvPlayerPrivate;
    QScopedPointer<MpvPlayerPrivate> d_ptr;
};

} // namespace Mpv

#endif // MPVPLAYER_HPP
