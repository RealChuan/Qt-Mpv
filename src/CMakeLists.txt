set(PROJECT_SOURCES
    controlwidget.cc controlwidget.hpp
    main.cc
    mainwindow.cc mainwindow.hpp
    mpvlogwindow.cc mpvlogwindow.hpp
    mpvplayer.cc mpvplayer.hpp
    mpvwidget.cc mpvwidget.hpp
    openwebmediadialog.cc openwebmediadialog.hpp
    playlistmodel.cpp playlistmodel.h
    playlistview.cc playlistview.hpp
    previewwidget.cc previewwidget.hpp
    qmediaplaylist_p.h
    qmediaplaylist.cpp qmediaplaylist.h
    qplaylistfileparser_p.h qplaylistfileparser.cpp
    qthelper.hpp
    slider.cpp slider.h
    titlewidget.cc titlewidget.hpp
    trackinfo.cc trackinfo.hpp)

qt_add_executable(Qt-Mpv MANUAL_FINALIZATION ${PROJECT_SOURCES})
target_link_libraries(Qt-Mpv PRIVATE Qt6::Widgets Qt6::Multimedia Qt6::Network)
IF (CMAKE_HOST_WIN32)
    target_link_libraries(Qt-Mpv PRIVATE C:\\3rd\\mpv\\libmpv.dll.a)   
ENDIF ()
qt_finalize_executable(Qt-Mpv)