#include "mpvplayer.hpp"
#include "qthelper.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QWidget>

#include <mpv/client.h>

#include <sstream>
#include <stdexcept>

namespace Mpv {

static void wakeup(void *ctx)
{
    // This callback is invoked from any mpv thread (but possibly also
    // recursively from a thread that is calling the mpv API). Just notify
    // the Qt GUI thread to wake up (so that it can process events with
    // mpv_wait_event()), and return as quickly as possible.
    auto w = (MpvPlayer *) ctx;
    QMetaObject::invokeMethod(w, "onMpvEvents", Qt::QueuedConnection);
}

class MpvPlayer::MpvPlayerPrivate
{
public:
    MpvPlayerPrivate(MpvPlayer *parent)
        : owner(parent)
    {}

    ~MpvPlayerPrivate() { destroy(); }

    void init()
    {
        destroy();
        mpv = mpv_create();
        if (!mpv) {
            throw std::runtime_error("can't create mpv instance");
        }
    }

    void destroy()
    {
        if (mpv) {
            mpv_terminate_destroy(mpv);
            mpv = nullptr;
        }
    }

    void handle_mpv_event(mpv_event *event)
    {
        switch (event->event_id) {
        case MPV_EVENT_PROPERTY_CHANGE: {
            mpv_event_property *prop = (mpv_event_property *) event->data;
            if (strcmp(prop->name, "time-pos") == 0) {
                if (prop->format == MPV_FORMAT_DOUBLE) {
                    position = *(double *) prop->data;
                    emit owner->positionChanged(position);
                } else if (prop->format == MPV_FORMAT_NONE) {
                    // The property is unavailable, which probably means playback
                    // was stopped.
                    position = 0;
                    emit owner->positionChanged(position);
                }
            } else if (strcmp(prop->name, "chapter-list") == 0) {
                // Dump the properties as JSON for demo purposes.
                if (prop->format == MPV_FORMAT_NODE) {
                    auto v = mpv::qt::node_to_variant((mpv_node *) prop->data);
                    // Abuse JSON support for easily printing the mpv_node contents.
                    auto d = QJsonDocument::fromVariant(v);
                    emit owner->mpvLogMessage("Change property " + QString(prop->name) + ":\n");
                    emit owner->mpvLogMessage(d.toJson());
                }
            } else if (strcmp(prop->name, "track-list") == 0) {
                // Dump the properties as JSON for demo purposes.
                if (prop->format == MPV_FORMAT_NODE) {
                    auto v = mpv::qt::node_to_variant((mpv_node *) prop->data);
                    // Abuse JSON support for easily printing the mpv_node contents.
                    auto d = QJsonDocument::fromVariant(v);
                    emit owner->mpvLogMessage("Change property " + QString(prop->name) + ":\n");
                    emit owner->mpvLogMessage(d.toJson());

                    audioTrackList.clear();
                    subTrackList.clear();
                    auto array = d.array();
                    for (auto iter = array.cbegin(); iter != array.cend(); iter++) {
                        auto obj = (*iter).toObject();
                        auto traskInfo = TraskInfo(obj);
                        if (obj.value("type") == "audio") {
                            audioTrackList.append(traskInfo);
                        } else if (obj.value("type") == "sub") {
                            subTrackList.append(traskInfo);
                        }
                    }
                    emit owner->trackChanged();
                }
            }
            break;
        }
        case MPV_EVENT_VIDEO_RECONFIG: {
            // Retrieve the new video size.
            int64_t w, h;
            if (mpv_get_property(mpv, "dwidth", MPV_FORMAT_INT64, &w) >= 0
                && mpv_get_property(mpv, "dheight", MPV_FORMAT_INT64, &h) >= 0 && w > 0 && h > 0) {
                // Note that the MPV_EVENT_VIDEO_RECONFIG event doesn't necessarily
                // imply a resize, and you should check yourself if the video
                // dimensions really changed.
                // mpv itself will scale/letter box the video to the container size
                // if the video doesn't fit.
                std::stringstream ss;
                ss << "Reconfig: " << w << " " << h << "\n";
                emit owner->mpvLogMessage(QString::fromStdString(ss.str()));
            }
            break;
        }
        case MPV_EVENT_LOG_MESSAGE: {
            struct mpv_event_log_message *msg = (struct mpv_event_log_message *) event->data;
            std::stringstream ss;
            ss << "[" << msg->prefix << "] " << msg->level << ": " << msg->text;
            emit owner->mpvLogMessage(QString::fromStdString(ss.str()));
            break;
        }
        case MPV_EVENT_SHUTDOWN:
            mpv_terminate_destroy(mpv);
            mpv = NULL;
            break;
        case MPV_EVENT_FILE_LOADED: emit owner->fileLoaded(); break;
        default:
            break;
            // Ignore uninteresting or unknown events.
        }
    }

    MpvPlayer *owner;

    mpv_handle *mpv = nullptr;
    TraskInfoList audioTrackList;
    TraskInfoList subTrackList;
    double position = 0;
};

MpvPlayer::MpvPlayer(QObject *parent)
    : QObject{parent}
    , d_ptr(new MpvPlayerPrivate(this))
{}

MpvPlayer::~MpvPlayer() {}

void MpvPlayer::openMedia(const QString &filePath)
{
    if (!d_ptr->mpv) {
        return;
    }
    const QByteArray c_filename = filePath.toUtf8();
    const char *args[] = {"loadfile", c_filename.data(), NULL};
    mpv_command_async(d_ptr->mpv, 0, args);
}

QString MpvPlayer::filename() const
{
    return mpv::qt::get_property(d_ptr->mpv, "filename").toString();
}

QString MpvPlayer::filepath() const
{
    return mpv::qt::get_property(d_ptr->mpv, "path").toString();
}

double MpvPlayer::filesize() const
{
    return mpv::qt::get_property(d_ptr->mpv, "file-size").toDouble();
}

double MpvPlayer::duration() const
{
    return mpv::qt::get_property(d_ptr->mpv, "duration").toDouble();
}

double MpvPlayer::position() const
{
    // time-pos
    return mpv::qt::get_property(d_ptr->mpv, "playback-time").toDouble();
}

TraskInfoList MpvPlayer::audioTrackList() const
{
    return d_ptr->audioTrackList;
}

TraskInfoList MpvPlayer::subTrackList() const
{
    return d_ptr->subTrackList;
}

void MpvPlayer::setAudioTrack(int aid)
{
    qInfo() << "aid: " << aid;
    mpv::qt::set_property_async(d_ptr->mpv, "aid", aid);
}

void MpvPlayer::blockAudioTrack()
{
    mpv::qt::set_property_async(d_ptr->mpv, "aid", "no");
}

void MpvPlayer::setSubTrack(int sid)
{
    qInfo() << "sid: " << sid;
    mpv::qt::set_property_async(d_ptr->mpv, "sid", sid);
}

void MpvPlayer::blockSubTrack()
{
    mpv::qt::set_property_async(d_ptr->mpv, "sid", "no");
}

void MpvPlayer::setPrintToStd(bool print)
{
    mpv_set_option_string(d_ptr->mpv, "terminal", print ? "yes" : "no");
}

void MpvPlayer::setCache(bool cache)
{
    mpv::qt::set_property_async(d_ptr->mpv, "cahce", cache ? "auto" : "no");
}

void MpvPlayer::setUseGpu(bool use)
{
    mpv::qt::set_property_async(d_ptr->mpv, "hwdec", use ? "auto-safe" : "no");
}

void MpvPlayer::setGpuApi(GpuApiType type)
{
    QString typeStr;
    switch (type) {
    case Opengl: typeStr = "opengl"; break;
    case Vulkan: typeStr = "vulkan"; break;
#ifdef Q_OS_WIN
    case D3d11: typeStr = "d3d11"; break;
#endif
    default: typeStr = "auto"; break;
    }
    mpv::qt::set_property(d_ptr->mpv, "gpu-api", typeStr);
    qInfo() << "GpuApi: " << typeStr;
}

void MpvPlayer::setVolume(int value)
{
    qInfo() << "volume: " << value;
    mpv::qt::set_property_async(d_ptr->mpv, "volume", value);
}

void MpvPlayer::seek(qint64 percent)
{
    qInfo() << "seek: " << percent;
    mpv::qt::command_async(d_ptr->mpv, QVariantList() << "seek" << percent << "absolute");
}

void MpvPlayer::seekRelative(qint64 seconds)
{
    qInfo() << "seekRelative: " << seconds;
    mpv::qt::command_async(d_ptr->mpv, QVariantList() << "seek" << seconds << "relative");
}

void MpvPlayer::setSpeed(double speed)
{
    qInfo() << "speed: " << speed;
    mpv::qt::set_property_async(d_ptr->mpv, "speed", speed);
}

void MpvPlayer::pause()
{
    auto pause_ = !mpv::qt::get_property(d_ptr->mpv, "pause").toBool();
    qInfo() << "pause: " << pause_;
    mpv::qt::set_property_async(d_ptr->mpv, "pause", pause_);
}

int MpvPlayer::volumeMax() const
{
    return mpv::qt::get_property(d_ptr->mpv, "volume-max").toInt();
}

void MpvPlayer::abortAllAsyncCommands()
{
    mpv::qt::command_abort_async(d_ptr->mpv);
}

void MpvPlayer::onMpvEvents()
{
    // Process all events, until the event queue is empty.
    while (d_ptr->mpv) {
        auto event = mpv_wait_event(d_ptr->mpv, 0);
        if (event->event_id == MPV_EVENT_NONE) {
            break;
        }
        d_ptr->handle_mpv_event(event);
    }
}

void MpvPlayer::initMpv(QWidget *widget)
{
    d_ptr->init();

    auto raw_wid = widget->winId();
#ifdef _WIN32
    // Truncate to 32-bit, as all Windows handles are. This also ensures
    // it doesn't go negative.
    int64_t wid = static_cast<uint32_t>(raw_wid);
#else
    int64_t wid = raw_wid;
#endif
    mpv_set_property(d_ptr->mpv, "wid", MPV_FORMAT_INT64, &wid);

    // Enable default bindings, because we're lazy. Normally, a player using
    // mpv as backend would implement its own key bindings.
    mpv_set_property_string(d_ptr->mpv, "input-default-bindings", "yes");

    // Enable keyboard input on the X11 window. For the messy details, see
    // --input-vo-keyboard on the manpage.
    mpv_set_property_string(d_ptr->mpv, "input-vo-keyboard", "yes");

    // Let us receive property change events with MPV_EVENT_PROPERTY_CHANGE if
    // this property changes.
    mpv_observe_property(d_ptr->mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);

    mpv_observe_property(d_ptr->mpv, 0, "track-list", MPV_FORMAT_NODE);
    mpv_observe_property(d_ptr->mpv, 0, "chapter-list", MPV_FORMAT_NODE);

    // Request log messages with level "info" or higher.
    // They are received as MPV_EVENT_LOG_MESSAGE.
    mpv_request_log_messages(d_ptr->mpv, "info");

    mpv::qt::set_property(d_ptr->mpv, "volume-max", 200);

    // From this point on, the wakeup function will be called. The callback
    // can come from any thread, so we use the QueuedConnection mechanism to
    // relay the wakeup in a thread-safe way.
    mpv_set_wakeup_callback(d_ptr->mpv, wakeup, this);

    mpv_set_option_string(d_ptr->mpv, "msg-level", "all=v");

    if (mpv_initialize(d_ptr->mpv) < 0) {
        throw std::runtime_error("mpv failed to initialize");
    }
}

} // namespace Mpv
