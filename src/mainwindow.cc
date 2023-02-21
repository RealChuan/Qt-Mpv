#include "mainwindow.hpp"
#include "controlwidget.hpp"
#include "mpvlogwindow.hpp"
#include "mpvplayer.hpp"
#include "mpvwidget.hpp"
#include "openwebmediadialog.hpp"
#include "playlistmodel.h"
#include "playlistview.hpp"
#include "previewwidget.hpp"
#include "qmediaplaylist.h"
#include "titlewidget.hpp"
#include "trackinfo.hpp"

#include <QtWidgets>

static bool isPlaylist(const QUrl &url) // Check for ".m3u" playlists.
{
    if (!url.isLocalFile()) {
        return false;
    }
    const QFileInfo fileInfo(url.toLocalFile());
    return fileInfo.exists()
           && !fileInfo.suffix().compare(QLatin1String("m3u"), Qt::CaseInsensitive);
}

class MainWindow::MainWindowPrivate
{
public:
    MainWindowPrivate(MainWindow *parent)
        : owner(parent)
    {
        mpvWidget = new Mpv::MpvWidget(owner);
        mpvWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        mpvWidget->setAcceptDrops(true);

        mpvPlayer = new Mpv::MpvPlayer(owner);
        mpvPlayer->initMpv(mpvWidget);
        mpvPlayer->setPrintToStd(true);

        logWindow = new Mpv::MpvLogWindow(owner);
        logWindow->setMinimumSize(500, 325);
        logWindow->show();

        controlWidget = new ControlWidget(owner);
        titleWidget = new TitleWidget(owner);

        playlistModel = new PlaylistModel(owner);
        playlistView = new PlayListView(owner);
        playlistView->setModel(playlistModel);
        playlistView->setCurrentIndex(
            playlistModel->index(playlistModel->playlist()->currentIndex(), 0));
        playlistView->setMaximumWidth(250);

        menu = new QMenu(owner);
        gpuAction = new QAction(QObject::tr("GPU Decode"), owner);
        gpuAction->setCheckable(true);
        audioTracksMenu = new QMenu(QObject::tr("Select audio track"), owner);
        subTracksMenu = new QMenu(QObject::tr("Select subtitle track"), owner);
        audioTracksGroup = new QActionGroup(owner);
        audioTracksGroup->setExclusive(true);
        subTracksGroup = new QActionGroup(owner);
        subTracksGroup->setExclusive(true);

        playListMenu = new QMenu(owner);

        initShortcut();
    }

    void initShortcut()
    {
        new QShortcut(QKeySequence::MoveToNextChar, owner, owner, [this] {
            mpvPlayer->seekRelative(5);
            titleWidget->setText(tr("Fast forward: 5 seconds"));
            titleWidget->setAutoHide(3000);
            setTitleWidgetGeometry(true);
        });
        new QShortcut(QKeySequence::MoveToPreviousChar, owner, owner, [this] {
            mpvPlayer->seekRelative(-5);
            titleWidget->setText(tr("Fast return: 5 seconds"));
            titleWidget->setAutoHide(3000);
            setTitleWidgetGeometry(true);
        });
        new QShortcut(QKeySequence::MoveToPreviousLine, owner, owner, [this] {
            controlWidget->setVolume(controlWidget->volume() + 10);
        });
        new QShortcut(QKeySequence::MoveToNextLine, owner, owner, [this] {
            controlWidget->setVolume(controlWidget->volume() - 10);
        });
        new QShortcut(Qt::Key_Space, owner, owner, [this] { mpvPlayer->pause(); });
    }

    void setControlWidgetGeometry(bool show = true)
    {
        auto margain = 10;
        auto geometry = mpvWidget->geometry();
        auto p1 = QPoint(geometry.x() + margain, geometry.bottomLeft().y() - 100 - margain);
        auto p2 = QPoint(geometry.topRight().x() - margain, geometry.bottomLeft().y() - margain);
        globalControlWidgetGeometry = {owner->mapToGlobal(p1), owner->mapToGlobal(p2)};
        controlWidget->setFixedSize(globalControlWidgetGeometry.size());
        controlWidget->setGeometry(globalControlWidgetGeometry);
        controlWidget->setVisible(show);
    }

    void setTitleWidgetGeometry(bool show = true)
    {
        auto margain = 10;
        auto geometry = mpvWidget->geometry();
        auto p1 = QPoint(geometry.x() + margain, geometry.y() + margain);
        auto p2 = QPoint(geometry.topRight().x() - margain, geometry.y() + margain + 80);
        globalTitlelWidgetGeometry = {owner->mapToGlobal(p1), owner->mapToGlobal(p2)};
        titleWidget->setFixedSize(globalTitlelWidgetGeometry.size());
        titleWidget->setGeometry(globalTitlelWidgetGeometry);
        titleWidget->setVisible(show);
    }

    MainWindow *owner;

    Mpv::MpvPlayer *mpvPlayer;
    Mpv::MpvWidget *mpvWidget;
    QScopedPointer<Mpv::PreviewWidget> previewWidgetPtr;
    Mpv::MpvLogWindow *logWindow;

    ControlWidget *controlWidget;
    QRect globalControlWidgetGeometry;
    TitleWidget *titleWidget;
    QRect globalTitlelWidgetGeometry;

    PlayListView *playlistView;
    PlaylistModel *playlistModel;

    QMenu *menu;
    QAction *gpuAction;
    QMenu *audioTracksMenu;
    QMenu *subTracksMenu;
    QActionGroup *audioTracksGroup;
    QActionGroup *subTracksGroup;

    QMenu *playListMenu;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , d_ptr(new MainWindowPrivate(this))
{
    setupUI();
    buildConnect();
    initMenu();
    initPlayListMenu();

    setAttribute(Qt::WA_Hover);
    d_ptr->mpvWidget->installEventFilter(this);
    d_ptr->playlistView->installEventFilter(this);
    installEventFilter(this);

    resize(1000, 650);
}

MainWindow::~MainWindow() {}

void MainWindow::onOpenLocalMedia()
{
    const auto path = QStandardPaths::standardLocations(QStandardPaths::MoviesLocation)
                          .value(0, QDir::homePath());
    const auto filePaths
        = QFileDialog::getOpenFileUrls(this,
                                       tr("Open File"),
                                       path,
                                       tr("Audio Video (*.mp3 *.mp4 *.mkv *.rmvb)"));
    if (filePaths.isEmpty()) {
        return;
    }
    addToPlaylist(filePaths);
}

void MainWindow::onOpenWebMedia()
{
    OpenWebMediaDialog dialog(this);
    dialog.setMinimumSize(size() / 2.0);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
    addToPlaylist({QUrl(dialog.url())});
}

void MainWindow::onFileLoaded()
{
    setWindowTitle(d_ptr->mpvPlayer->filename());
    d_ptr->controlWidget->onDurationChanged(d_ptr->mpvPlayer->duration());
}

void MainWindow::onTrackChanged()
{
    auto audioTrackList = d_ptr->mpvPlayer->audioTrackList();
    qDeleteAll(d_ptr->audioTracksGroup->actions());
    if (audioTrackList.size() > 1) {
        for (const auto &item : qAsConst(audioTrackList)) {
            auto action = new QAction(item.text(), this);
            action->setData(QVariant::fromValue(item));
            action->setCheckable(true);
            d_ptr->audioTracksMenu->addAction(action);
            d_ptr->audioTracksGroup->addAction(action);
            if (item.selected) {
                action->setChecked(true);
            }
        }
    }

    auto subTrackList = d_ptr->mpvPlayer->subTrackList();
    qDeleteAll(d_ptr->subTracksGroup->actions());
    if (subTrackList.size() > 1) {
        for (const auto &item : qAsConst(subTrackList)) {
            auto action = new QAction(item.text(), this);
            action->setData(QVariant::fromValue(item));
            action->setCheckable(true);
            d_ptr->subTracksMenu->addAction(action);
            d_ptr->subTracksGroup->addAction(action);
            if (item.selected) {
                action->setChecked(true);
            }
        }
    }
}

void MainWindow::onRenderChanged(QAction *action)
{
    auto value = static_cast<Mpv::MpvPlayer::GpuApiType>(action->data().toInt());
    d_ptr->mpvPlayer->initMpv(d_ptr->mpvWidget);
    d_ptr->mpvPlayer->setGpuApi(value);
    d_ptr->mpvPlayer->setPrintToStd(true);
    d_ptr->mpvPlayer->setUseGpu(d_ptr->gpuAction->isChecked());
    auto index = d_ptr->playlistModel->playlist()->currentIndex();
    if (index > -1) {
        playlistPositionChanged(d_ptr->playlistModel->playlist()->currentIndex());
    }
}

void MainWindow::onPreview(int pos, int value)
{
    auto url = d_ptr->mpvPlayer->filepath();
    if (url.isEmpty()) {
        return;
    }
    if (d_ptr->previewWidgetPtr.isNull()) {
        d_ptr->previewWidgetPtr.reset(new Mpv::PreviewWidget);
        d_ptr->previewWidgetPtr->setWindowFlags(d_ptr->previewWidgetPtr->windowFlags() | Qt::Tool
                                                | Qt::FramelessWindowHint
                                                | Qt::WindowStaysOnTopHint);
    }
    d_ptr->previewWidgetPtr->startPreview(url, value);
    int w = 320;
    int h = 200;
    d_ptr->previewWidgetPtr->setFixedSize(w, h);
    auto gpos = d_ptr->controlWidget->sliderGlobalPos() + QPoint(pos, 0);
    d_ptr->previewWidgetPtr->move(gpos - QPoint(w / 2, h + 15));
    d_ptr->previewWidgetPtr->show();
}

void MainWindow::onPreviewFinish()
{
    if (!d_ptr->previewWidgetPtr.isNull()) {
        d_ptr->previewWidgetPtr->hide();
        d_ptr->previewWidgetPtr->clearAllTask();
    }
}

void MainWindow::playlistPositionChanged(int currentItem)
{
    d_ptr->playlistView->setCurrentIndex(d_ptr->playlistModel->index(currentItem, 0));
    d_ptr->mpvPlayer->openMedia(d_ptr->playlistModel->playlist()->currentMedia().toString());
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == d_ptr->mpvWidget) {
        switch (event->type()) {
        case QEvent::DragEnter: {
            auto e = static_cast<QDragEnterEvent *>(event);
            e->acceptProposedAction();
        } break;
        case QEvent::DragMove: {
            auto e = static_cast<QDragMoveEvent *>(event);
            e->acceptProposedAction();
        } break;
        case QEvent::Drop: {
            auto e = static_cast<QDropEvent *>(event);
            QList<QUrl> urls = e->mimeData()->urls();
            if (!urls.isEmpty()) {
                addToPlaylist(urls);
            }
        } break;
        case QEvent::ContextMenu: {
            auto e = static_cast<QContextMenuEvent *>(event);
            d_ptr->menu->exec(e->globalPos());
        } break;
        case QEvent::Resize:
            QMetaObject::invokeMethod(
                this,
                [=] {
                    d_ptr->setControlWidgetGeometry(d_ptr->controlWidget->isVisible());
                    d_ptr->setTitleWidgetGeometry(d_ptr->titleWidget->isVisible());
                },
                Qt::QueuedConnection);
            break;
            //        case QEvent::MouseButtonPress: {
            //            auto e = static_cast<QMouseEvent *>(event);
            //            if (e->button() & Qt::LeftButton) {
            //                d_ptr->mpvPlayer->pause();
            //            }
            //        } break;
        case QEvent::MouseButtonDblClick:
            if (isFullScreen()) {
                showNormal();
            } else {
                d_ptr->playlistView->hide();
                showFullScreen();
            }
            break;
        default: break;
        }
    } else if (watched == d_ptr->playlistView) {
        switch (event->type()) {
        case QEvent::ContextMenu: {
            auto e = static_cast<QContextMenuEvent *>(event);
            d_ptr->playListMenu->exec(e->globalPos());
        } break;
        default: break;
        }
    } else if (watched == this) {
        switch (event->type()) {
        case QEvent::Show:
        case QEvent::Move:
            QMetaObject::invokeMethod(
                this,
                [=] {
                    d_ptr->setControlWidgetGeometry(d_ptr->controlWidget->isVisible());
                    d_ptr->setTitleWidgetGeometry(d_ptr->titleWidget->isVisible());
                },
                Qt::QueuedConnection);
            break;
        case QEvent::Hide: d_ptr->controlWidget->hide(); break;
        case QEvent::HoverMove:
            if (d_ptr->globalControlWidgetGeometry.isValid()) {
                auto e = static_cast<QHoverEvent *>(event);
                bool contain = d_ptr->globalControlWidgetGeometry.contains(
                    e->globalPosition().toPoint());
                bool isVisible = d_ptr->controlWidget->isVisible();
                if (contain && !isVisible) {
                    d_ptr->controlWidget->show();
                } else if (!contain && isVisible) {
                    d_ptr->controlWidget->hide();
                }
            }
            if (d_ptr->globalTitlelWidgetGeometry.isValid() && isFullScreen()) {
                auto e = static_cast<QHoverEvent *>(event);
                bool contain = d_ptr->globalTitlelWidgetGeometry.contains(
                    e->globalPosition().toPoint());
                bool isVisible = d_ptr->titleWidget->isVisible();
                if (contain && !isVisible) {
                    d_ptr->titleWidget->setText(windowTitle());
                    d_ptr->titleWidget->show();
                } else if (!contain && isVisible) {
                    d_ptr->titleWidget->hide();
                }
            }
            break;
        default: break;
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::keyPressEvent(QKeyEvent *ev)
{
    QMainWindow::keyPressEvent(ev);

    qDebug() << ev->key();
    switch (ev->key()) {
    case Qt::Key_Escape:
        if (isFullScreen()) {
            showNormal();
        } else {
            showMinimized();
        }
        break;
    case Qt::Key_Q: qApp->quit(); break;
    default: break;
    }
}

void MainWindow::setupUI()
{
    auto widget = new QWidget(this);
    auto layout = new QHBoxLayout(widget);
    layout->setContentsMargins(QMargins());
    layout->setSpacing(0);
    layout->addWidget(d_ptr->mpvWidget);
    layout->addWidget(d_ptr->playlistView);

    setCentralWidget(widget);
}

void MainWindow::buildConnect()
{
    connect(d_ptr->mpvPlayer, &Mpv::MpvPlayer::fileLoaded, this, &MainWindow::onFileLoaded);
    connect(d_ptr->mpvPlayer, &Mpv::MpvPlayer::trackChanged, this, &MainWindow::onTrackChanged);
    connect(d_ptr->mpvPlayer,
            &Mpv::MpvPlayer::positionChanged,
            d_ptr->controlWidget,
            &ControlWidget::onPositionChanged);
    connect(d_ptr->mpvPlayer,
            &Mpv::MpvPlayer::mpvLogMessage,
            d_ptr->logWindow,
            &Mpv::MpvLogWindow::onAppendLog);

    connect(d_ptr->controlWidget, &ControlWidget::seek, d_ptr->mpvPlayer, [this](int value) {
        d_ptr->mpvPlayer->seek(value);
        d_ptr->titleWidget->setText(
            tr("Fast forward to: %1")
                .arg(QTime::fromMSecsSinceStartOfDay(value * 1000).toString("hh:mm:ss")));
        d_ptr->titleWidget->setAutoHide(3000);
        d_ptr->setTitleWidgetGeometry(true);
    });
    connect(d_ptr->controlWidget, &ControlWidget::hoverPosition, this, &MainWindow::onPreview);
    connect(d_ptr->controlWidget, &ControlWidget::leavePosition, this, &MainWindow::onPreviewFinish);
    connect(d_ptr->controlWidget, &ControlWidget::volumeChanged, d_ptr->mpvPlayer, [this](int value) {
        d_ptr->mpvPlayer->setVolume(value);
        d_ptr->titleWidget->setText(tr("Volume: %1%").arg(value));
        d_ptr->titleWidget->setAutoHide(3000);
        d_ptr->setTitleWidgetGeometry(true);
    });
    auto max = d_ptr->mpvPlayer->volumeMax();
    d_ptr->controlWidget->setVolumeMax(max);
    d_ptr->controlWidget->setVolume(max / 2);
    connect(d_ptr->controlWidget,
            &ControlWidget::speedChanged,
            d_ptr->mpvPlayer,
            [this](double value) {
                d_ptr->mpvPlayer->setSpeed(value);
                d_ptr->titleWidget->setText(tr("Speed: %1").arg(value));
                d_ptr->titleWidget->setAutoHide(3000);
                d_ptr->setTitleWidgetGeometry(true);
            });
    connect(d_ptr->controlWidget, &ControlWidget::showList, d_ptr->playlistView, [this] {
        d_ptr->playlistView->setVisible(!d_ptr->playlistView->isVisible());
    });

    connect(d_ptr->playlistModel->playlist(),
            &QMediaPlaylist::currentIndexChanged,
            this,
            &MainWindow::playlistPositionChanged);
    connect(d_ptr->playlistView, &QAbstractItemView::activated, this, &MainWindow::jump);
}

void MainWindow::initMenu()
{
    d_ptr->menu->addAction(tr("Open Local Media"), this, &MainWindow::onOpenLocalMedia);
    d_ptr->menu->addAction(tr("Open Web Media"), this, &MainWindow::onOpenWebMedia);

    d_ptr->menu->addAction(d_ptr->gpuAction);
    connect(d_ptr->gpuAction, &QAction::toggled, d_ptr->mpvPlayer, [this](bool checked) {
        d_ptr->mpvPlayer->setUseGpu(checked);
    });
    d_ptr->gpuAction->setChecked(true);

    auto menu = new QMenu(tr("Render"), this);
    auto actionGroup = new QActionGroup(this);
    actionGroup->setExclusive(true);
    auto metaEnum = QMetaEnum::fromType<Mpv::MpvPlayer::GpuApiType>();
    for (int i = 0; i < metaEnum.keyCount(); i++) {
        auto value = metaEnum.value(i);
        auto action = new QAction(metaEnum.valueToKey(value), this);
        action->setData(value);
        action->setCheckable(true);
        actionGroup->addAction(action);
        menu->addAction(action);
        action->setChecked(i == 0);
    }
    connect(actionGroup, &QActionGroup::triggered, this, &MainWindow::onRenderChanged);
    d_ptr->menu->addMenu(menu);

    d_ptr->menu->addMenu(d_ptr->audioTracksMenu);
    d_ptr->menu->addMenu(d_ptr->subTracksMenu);
    connect(d_ptr->audioTracksGroup, &QActionGroup::triggered, this, [this](QAction *action) {
        auto data = action->data().value<Mpv::TraskInfo>();
        d_ptr->mpvPlayer->setAudioTrack(data.id);
    });
    connect(d_ptr->subTracksGroup, &QActionGroup::triggered, this, [this](QAction *action) {
        auto data = action->data().value<Mpv::TraskInfo>();
        d_ptr->mpvPlayer->setSubTrack(data.id);
    });
}

void MainWindow::initPlayListMenu()
{
    d_ptr->playListMenu->addAction(tr("Open Local Media"), this, &MainWindow::onOpenLocalMedia);
    d_ptr->playListMenu->addAction(tr("Open Web Media"), this, &MainWindow::onOpenWebMedia);
    d_ptr->playListMenu
        ->addAction(tr("Remove the currently selected item"), d_ptr->playlistView, [this] {
            auto indexs = d_ptr->playlistView->selectedAllIndexs();
            std::sort(indexs.begin(), indexs.end(), [&](QModelIndex left, QModelIndex right) {
                return left.row() > right.row();
            });
            for (const auto &index : qAsConst(indexs)) {
                d_ptr->playlistModel->playlist()->removeMedia(index.row());
            }
        });
    d_ptr->playListMenu->addAction(tr("Clear"), d_ptr->playlistView, [this] {
        d_ptr->playlistModel->playlist()->clear();
    });
}

void MainWindow::addToPlaylist(const QList<QUrl> &urls)
{
    auto playlist = d_ptr->playlistModel->playlist();
    const int previousMediaCount = playlist->mediaCount();
    for (auto &url : urls) {
        if (isPlaylist(url)) {
            playlist->load(url);
        } else {
            playlist->addMedia(url);
        }
    }
    if (playlist->mediaCount() > previousMediaCount) {
        auto index = d_ptr->playlistModel->index(previousMediaCount, 0);
        d_ptr->playlistView->setCurrentIndex(index);
        jump(index);
    }
}

void MainWindow::jump(const QModelIndex &index)
{
    if (index.isValid()) {
        d_ptr->playlistModel->playlist()->setCurrentIndex(index.row());
    }
}
