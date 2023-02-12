#ifndef TRACKINFO_HPP
#define TRACKINFO_HPP

#include <QtCore>

namespace Mpv {

struct TraskInfo
{
    TraskInfo() {}
    TraskInfo(const QJsonObject &obj);

    QString text() const;

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
};

} // namespace Mpv

Q_DECLARE_METATYPE(Mpv::TraskInfo)

using TraskInfoList = QList<Mpv::TraskInfo>;

#endif // TRACKINFO_HPP
