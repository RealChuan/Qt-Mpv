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
    }

    ControlWidget *owner;

    Slider *slider;
    QLabel *positionLabel;
    QLabel *durationLabel;
    Slider *volumeSlider;
    QComboBox *speedCbx;
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
    connect(d_ptr->slider, &Slider::onHover, this, &ControlWidget::hoverPosition);
    connect(d_ptr->slider, &Slider::onLeave, this, &ControlWidget::leavePosition);
    connect(d_ptr->volumeSlider, &QSlider::valueChanged, this, &ControlWidget::volumeChanged);

    connect(d_ptr->speedCbx, &QComboBox::currentIndexChanged, this, &ControlWidget::onSpeedChanged);
}
