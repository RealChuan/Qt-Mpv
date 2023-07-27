#include "controlwidget.hpp"
#include "qmediaplaylist.h"
#include "slider.h"

#include <QStyle>
#include <QtWidgets>

auto bytesToString(qint64 bytes) -> QString
{
    const QStringList list = {"B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
    const int unit = 1024;
    int index = 0;
    double size = bytes;
    while (size >= unit) {
        size /= unit;
        index++;
    }
    return QString("%1 %2").arg(QString::number(size, 'f', 2), list.at(index));
}

class ControlWidget::ControlWidgetPrivate
{
public:
    ControlWidgetPrivate(ControlWidget *q)
        : q_ptr(q)
    {
        slider = new Slider(q_ptr);

        playButton = new QToolButton(q_ptr);
        playButton->setIcon(q_ptr->style()->standardIcon(QStyle::SP_MediaPlay));
        positionLabel = new QLabel("00:00:00", q_ptr);
        durationLabel = new QLabel("00:00:00", q_ptr);

        volumeSlider = new Slider(q_ptr);
        volumeSlider->setMinimumWidth(60);
        volumeSlider->setRange(0, 100);

        cacheSpeedLabel = new QLabel(q_ptr);

        speedCbx = new QComboBox(q_ptr);
        auto speedCbxView = new QListView(speedCbx);
        speedCbxView->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        speedCbxView->setTextElideMode(Qt::ElideRight);
        speedCbxView->setAlternatingRowColors(true);
        speedCbx->setView(speedCbxView);
        double i = 0.25;
        while (i <= 2) {
            speedCbx->addItem(QString::number(i), i);
            i += 0.25;
        }
        speedCbx->setCurrentText("1");

        modelButton = new QPushButton(q_ptr);
    }

    void setupUI()
    {
        auto skipBackwardButton = new QToolButton(q_ptr);
        skipBackwardButton->setToolTip(QObject::tr("Previous"));
        skipBackwardButton->setIcon(q_ptr->style()->standardIcon(QStyle::SP_MediaSkipBackward));
        QObject::connect(skipBackwardButton, &QToolButton::clicked, q_ptr, &ControlWidget::previous);

        auto skipForwardButton = new QToolButton(q_ptr);
        skipForwardButton->setToolTip(QObject::tr("Next"));
        skipForwardButton->setIcon(q_ptr->style()->standardIcon(QStyle::SP_MediaSkipForward));
        QObject::connect(skipForwardButton, &QToolButton::clicked, q_ptr, &ControlWidget::next);

        auto listButton = new QToolButton(q_ptr);
        listButton->setText(QObject::tr("List"));
        QObject::connect(listButton, &QToolButton::clicked, q_ptr, &ControlWidget::showList);

        auto volumeBotton = new QToolButton(q_ptr);
        volumeBotton->setIcon(q_ptr->style()->standardIcon(QStyle::SP_MediaVolume));

        auto controlLayout = new QHBoxLayout;
        controlLayout->setSpacing(10);
        controlLayout->addWidget(skipBackwardButton);
        controlLayout->addWidget(playButton);
        controlLayout->addWidget(skipForwardButton);
        controlLayout->addWidget(positionLabel);
        controlLayout->addWidget(new QLabel("/", q_ptr));
        controlLayout->addWidget(durationLabel);
        controlLayout->addStretch();
        controlLayout->addWidget(cacheSpeedLabel);
        controlLayout->addWidget(volumeBotton);
        controlLayout->addWidget(volumeSlider);
        controlLayout->addWidget(new QLabel(QObject::tr("Speed: "), q_ptr));
        controlLayout->addWidget(speedCbx);
        controlLayout->addWidget(modelButton);
        controlLayout->addWidget(listButton);

        auto widget = new QWidget(q_ptr);
        widget->setObjectName("wid");
        widget->setStyleSheet("QWidget#wid{background: rgba(255,255,255,0.3); border-radius:5px;}"
                              "QLabel{ color: white; }");
        auto layout = new QVBoxLayout(widget);
        layout->setSpacing(15);
        layout->addWidget(slider);
        layout->addLayout(controlLayout);

        auto l = new QHBoxLayout(q_ptr);
        l->addWidget(widget);
    }

    void initModelButton()
    {
        modelButton->setProperty("model", QMediaPlaylist::Sequential);
        QMetaObject::invokeMethod(q_ptr, &ControlWidget::onModelChanged, Qt::QueuedConnection);
    }

    ControlWidget *q_ptr;

    Slider *slider;
    QToolButton *playButton;
    QLabel *positionLabel;
    QLabel *durationLabel;
    QLabel *cacheSpeedLabel;
    Slider *volumeSlider;
    QComboBox *speedCbx;
    QPushButton *modelButton;
};

ControlWidget::ControlWidget(QWidget *parent)
    : QWidget{parent}
    , d_ptr(new ControlWidgetPrivate(this))
{
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground); //设置窗口背景透明

    d_ptr->setupUI();
    buildConnect();
    d_ptr->initModelButton();
}

ControlWidget::~ControlWidget() {}

void ControlWidget::setPause(bool pause)
{
    d_ptr->playButton->setIcon(
        style()->standardIcon(pause ? QStyle::SP_MediaPlay : QStyle::SP_MediaPause));
}

QPoint ControlWidget::sliderGlobalPos() const
{
    return d_ptr->slider->mapToGlobal(d_ptr->slider->pos());
}

void ControlWidget::setVolumeMax(int max)
{
    d_ptr->volumeSlider->setMaximum(max);
}

void ControlWidget::setVolume(int value)
{
    if (value < d_ptr->volumeSlider->minimum()) {
        value = d_ptr->volumeSlider->minimum();
    } else if (value > d_ptr->volumeSlider->maximum()) {
        value = d_ptr->volumeSlider->maximum();
    }
    d_ptr->volumeSlider->setValue(value);
}

int ControlWidget::volume() const
{
    return d_ptr->volumeSlider->value();
}

void ControlWidget::onDurationChanged(double value)
{
    auto str = QTime::fromMSecsSinceStartOfDay(value * 1000).toString("hh:mm:ss");
    d_ptr->durationLabel->setText(str);
    d_ptr->slider->blockSignals(true);
    d_ptr->slider->setRange(0, value);
    d_ptr->slider->blockSignals(false);
    onPositionChanged(0);
}

void ControlWidget::onPositionChanged(double value)
{
    auto str = QTime::fromMSecsSinceStartOfDay(value * 1000).toString("hh:mm:ss");
    d_ptr->positionLabel->setText(str);

    d_ptr->slider->blockSignals(true);
    d_ptr->slider->setValue(value);
    d_ptr->slider->blockSignals(false);
}

void ControlWidget::onCacheSpeedChanged(int64_t cache_speed)
{
    d_ptr->cacheSpeedLabel->setText(bytesToString(cache_speed) + "/S");
}

void ControlWidget::onSpeedChanged()
{
    auto data = d_ptr->speedCbx->currentData().toDouble();
    emit speedChanged(data);
}

void ControlWidget::onModelChanged()
{
    auto model = d_ptr->modelButton->property("model").toInt();

    auto metaEnum = QMetaEnum::fromType<QMediaPlaylist::PlaybackMode>();
    model += 1;
    if (model > QMediaPlaylist::Random) {
        model = QMediaPlaylist::CurrentItemOnce;
    }
    auto text = metaEnum.valueToKey(model);
    d_ptr->modelButton->setText(text);
    d_ptr->modelButton->setToolTip(text);
    d_ptr->modelButton->setProperty("model", model);

    emit modelChanged(model);
}

void ControlWidget::buildConnect()
{
    connect(d_ptr->slider, &Slider::valueChanged, this, &ControlWidget::seek);
    connect(d_ptr->slider, &Slider::onHover, this, &ControlWidget::hoverPosition);
    connect(d_ptr->slider, &Slider::onLeave, this, &ControlWidget::leavePosition);
    connect(d_ptr->playButton, &QToolButton::clicked, this, &ControlWidget::pause);
    connect(d_ptr->volumeSlider, &QSlider::valueChanged, this, &ControlWidget::volumeChanged);
    connect(d_ptr->speedCbx, &QComboBox::currentIndexChanged, this, &ControlWidget::onSpeedChanged);
    connect(d_ptr->modelButton, &QPushButton::clicked, this, &ControlWidget::onModelChanged);
}
