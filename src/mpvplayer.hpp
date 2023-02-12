#ifndef MPVPLAYER_HPP
#define MPVPLAYER_HPP

#include "trackinfo.hpp"

#include <QObject>

namespace Mpv {

class MpvPlayer : public QObject
{
    Q_OBJECT
public:
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
    void setSubTrack(int sid);

    void setVolume(int value);
    void seek(qint64 seconds);
    void seekRelative(qint64 seconds);
    void setSpeed(double speed);
    void pause();

signals:
    void mpv_events();
    void fileLoaded();
    void trackChanged();
    void positionChanged(double position); // ms
    void mpvLogMessage(const QString &log);

private slots:
    void onMpvEvents();

private:
    class MpvPlayerPrivate;
    QScopedPointer<MpvPlayerPrivate> d_ptr;
};

} // namespace Mpv

#endif // MPVPLAYER_HPP
