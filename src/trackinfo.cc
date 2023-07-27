#include "trackinfo.hpp"

namespace Mpv {

TraskInfo::TraskInfo(const QJsonObject &obj)
{
    albumart = obj.value("albumart").toBool();
    codec = obj.value("codec").toString();
    isDefault = obj.value("default").toBool();
    dependent = obj.value("dependent").toBool();
    external = obj.value("external").toBool();
    ff_index = obj.value("ff-index").toInt();
    forced = obj.value("forced").toBool();
    hearing_impaired = obj.value("hearing-impaired").toBool();
    id = obj.value("id").toInt();
    image = obj.value("image").toBool();
    lang = obj.value("lang").toString();
    selected = obj.value("selected").toBool();
    src_id = obj.value("src-id").toInt();
    title = obj.value("title").toString();
    type = obj.value("type").toString();
    visual_impaired = obj.value("visual-impaired").toBool();
}

auto TraskInfo::text() const -> QString
{
    QString str = title;
    if (str.isEmpty()) {
        str = lang;
    } else if (!lang.isEmpty()) {
        str += "-" + lang;
    }
    if (!codec.isEmpty()) {
        str += "-" + codec;
    }
    if (isDefault) {
        str += "-Default";
    }
    return str;
}

} // namespace Mpv
