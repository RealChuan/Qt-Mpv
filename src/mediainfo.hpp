#ifndef MEDIAINFO_HPP
#define MEDIAINFO_HPP

#include <QtCore>

namespace Mpv {

struct TraskInfo
{
    TraskInfo() = default;
    explicit TraskInfo(const QJsonObject &obj);

    [[nodiscard]] auto text() const -> QString;

    bool albumart = false;
    QString codec;
    bool isDefault = false;
    bool dependent = false;
    bool external = false;
    int ff_index = 0;
    bool forced = false;
    bool hearing_impaired = false;
    int id = 0;
    bool image = false;
    QString lang;
    bool selected = false;
    int src_id = 0;
    QString title;
    QString type;
    bool visual_impaired = false;

    QSize size = QSize(0, 0);
    double fps = 0.0;

    int channelCount = 0;
    int samplerate = 0;
};

using TraskInfoList = QList<TraskInfo>;

struct Chapter
{
    Chapter() = default;
    explicit Chapter(const QJsonObject &obj);

    QString title;
    qint64 milliseconds;
};

using ChapterList = QList<Chapter>;

} // namespace Mpv

Q_DECLARE_METATYPE(Mpv::TraskInfo)

#endif // MEDIAINFO_HPP
