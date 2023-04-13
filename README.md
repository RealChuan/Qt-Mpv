# Qt-Mpv

- [简体中文](README.md)
- [English](README.en.md)

## 编译环境

1. QT6；
2. Windows：[libmpv](https://sourceforge.net/projects/mpv-player-windows/files/libmpv/)；
3. MacOS：`brew install mpv`，先升级brew；

## 预览窗口在使用4K视频时，会占用大量内存，因为多开了一个mpv实例，内存double；

**预览窗口使用opengl渲染存在问题，会创建出一个mpv原生播放窗口！！！**

## MacOS好像只能使用[QOpenglWidget](https://github.com/mpv-player/mpv-examples/tree/master/libmpv/qt_opengl)渲染；

```
[vo/gpu] opengl cocoa backend is deprecated, use vo=libmpv instead
```

但是使用 `vo=libmpv`也无法正常显示视频；

使用opengl的版本大于3，性能更好；

```cpp
QSurfaceFormat surfaceFormat;
surfaceFormat.setVersion(3, 3);
surfaceFormat.setProfile(QSurfaceFormat::CoreProfile);
setFormat(surfaceFormat);
QSurfaceFormat::setDefaultFormat(surfaceFormat);
```

## MacOS打包需要[install_name_tool](/mac/change_lib_dependencies.rb)，依赖拷贝脚本文件来自[iina](https://github.com/iina/iina/blob/develop/other/change_lib_dependencies.rb)；

```shell
./mac/change_lib_dependencies.rb "$(brew --prefix)" "$(brew --prefix mpv-iina)/lib/libmpv.dylib"
```

依赖会拷贝到 `packet/Qt-Mpv.app/Contents/Frameworks/`；

<div align=center><img src="doc/player.jpeg"></div>
