# Qt-Mpv

## 预览窗口在使用4K视频时，会占用大量内存，因为多开了一个mpv实例，内存double；

## MacOS好像只能使用[QOpenglWidget](https://github.com/mpv-player/mpv-examples/tree/master/libmpv/qt_opengl)渲染；

```
[vo/gpu] opengl cocoa backend is deprecated, use vo=libmpv instead
```

但是使用 `vo=libmpv`也无法正常显示视频；

<div align=center><img src="doc/player.jpeg"></div>
