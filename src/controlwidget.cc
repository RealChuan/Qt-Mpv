#include "controlwidget.hpp"
#include "slider.h"

#include <QtWidgets>

class ControlWidget::ControlWidgetPrivate
{
public:
    ControlWidgetPrivate(ControlWidget *parent)
        : owner(parent)
    {
        slider = new Slider(owner);
        positionLabel = new QLabel("00:00:00", owner);
        durationLabel = new QLabel("/ 00:00:00", owner);

        volumeSlider = new Slider(owner);
        volumeSlider->setRange(0, 100);

        speedCbx = new QComboBox(owner);
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

        audioTracksCbx = new QComboBox(owner);
        audioTracksCbx->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        auto audioTracksView = new QListView(audioTracksCbx);
        audioTracksView->setTextElideMode(Qt::ElideRight);
        audioTracksView->setAlternatingRowColors(true);
        audioTracksCbx->setView(audioTracksView);

        subTracksCbx = new QComboBox(owner);
        subTracksCbx->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        auto subtitleStreamsView = new QListView(subTracksCbx);
        subtitleStreamsView->setTextElideMode(Qt::ElideRight);
        subtitleStreamsView->setAlternatingRowColors(true);
        subTracksCbx->setView(subtitleStreamsView);
    }

    ControlWidget *owner;

    Slider *slider;
    QLabel *positionLabel;
    QLabel *durationLabel;
    Slider *volumeSlider;
    QComboBox *speedCbx;
    QComboBox *audioTracksCbx;
    QComboBox *subTracksCbx;
};

ControlWidget::ControlWidget(QWidget *parent)
    : QWidget{parent}
    , d_ptr(new ControlWidgetPrivate(this))
{
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground); //设置窗口背景透明

    setupUI();
    buildConnect();
}

ControlWidget::~ControlWidget() {}

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

void ControlWidget::setAudioTracks(const TraskInfoList &list)
{
    d_ptr->audioTracksCbx->blockSignals(true);
    d_ptr->audioTracksCbx->clear();
    for (const auto &audio : qAsConst(list)) {
        auto text = audio.text();
        d_ptr->audioTracksCbx->addItem(text, QVariant::fromValue(audio));
        if (audio.selected) {
            d_ptr->audioTracksCbx->setCurrentText(text);
        }
    }
    d_ptr->audioTracksCbx->blockSignals(false);
}

void ControlWidget::setSubTracks(const TraskInfoList &list)
{
    d_ptr->subTracksCbx->blockSignals(true);
    d_ptr->subTracksCbx->clear();
    for (const auto &sub : qAsConst(list)) {
        auto text = sub.text();
        d_ptr->subTracksCbx->addItem(sub.text(), QVariant::fromValue(sub));
        if (sub.selected) {
            d_ptr->subTracksCbx->setCurrentText(text);
        }
    }
    d_ptr->subTracksCbx->blockSignals(false);
}

void ControlWidget::onDurationChanged(double value)
{
    auto str = QTime::fromMSecsSinceStartOfDay(value * 1000).toString("hh:mm:ss");
    d_ptr->durationLabel->setText("/ " + str);
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

void ControlWidget::onSpeedChanged()
{
    auto data = d_ptr->speedCbx->currentData().toDouble();
    emit speedChanged(data);
}

void ControlWidget::onAudioTrackChanged()
{
    auto data = d_ptr->audioTracksCbx->currentData().value<Mpv::TraskInfo>();
    emit audioTrackChanged(data.id);
}

void ControlWidget::onSubTrackChanged()
{
    auto data = d_ptr->subTracksCbx->currentData().value<Mpv::TraskInfo>();
    emit subTrackChanged(data.id);
}

void ControlWidget::setupUI()
{
    auto processWidget = new QWidget(this);
    //processWidget->setMaximumHeight(70);
    QHBoxLayout *processLayout = new QHBoxLayout(processWidget);
    processLayout->addWidget(d_ptr->slider);
    processLayout->addWidget(d_ptr->positionLabel);
    processLayout->addWidget(d_ptr->durationLabel);

    auto listButton = new QToolButton(this);
    listButton->setText(tr("List"));
    connect(listButton, &QToolButton::clicked, this, &ControlWidget::showList);

    auto controlLayout = new QHBoxLayout;
    controlLayout->addWidget(new QLabel(tr("Volume: "), this));
    controlLayout->addWidget(d_ptr->volumeSlider);
    controlLayout->addWidget(new QLabel(tr("Speed: "), this));
    controlLayout->addWidget(d_ptr->speedCbx);
    controlLayout->addWidget(new QLabel(tr("Audio Tracks: "), this));
    controlLayout->addWidget(d_ptr->audioTracksCbx);
    controlLayout->addWidget(new QLabel(tr("Subtitle Tracks: "), this));
    controlLayout->addWidget(d_ptr->subTracksCbx);
    controlLayout->addWidget(listButton);

    auto widget = new QWidget(this);
    widget->setObjectName("wid");
    widget->setStyleSheet("QWidget#wid{background: rgba(255,255,255,0.3);}");
    auto layout = new QVBoxLayout(widget);
    layout->addWidget(processWidget);
    layout->addLayout(controlLayout);

    auto l = new QHBoxLayout(this);
    l->addWidget(widget);
}

void ControlWidget::buildConnect()
{
    connect(d_ptr->slider, &Slider::valueChanged, this, &ControlWidget::seek);
    connect(d_ptr->volumeSlider, &QSlider::valueChanged, this, &ControlWidget::volumeChanged);

    connect(d_ptr->speedCbx, &QComboBox::currentIndexChanged, this, &ControlWidget::onSpeedChanged);

    connect(d_ptr->audioTracksCbx,
            &QComboBox::currentIndexChanged,
            this,
            &ControlWidget::onAudioTrackChanged);
    connect(d_ptr->subTracksCbx,
            &QComboBox::currentIndexChanged,
            this,
            &ControlWidget::onSubTrackChanged);
}
