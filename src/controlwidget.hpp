#ifndef CONTROLWIDGET_HPP
#define CONTROLWIDGET_HPP

#include <QWidget>

class ControlWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ControlWidget(QWidget *parent = nullptr);
    ~ControlWidget() override;

    [[nodiscard]] auto sliderGlobalPos() const -> QPoint;

    void setPause(bool pause);
    void setVolumeMax(int max);
    void setVolume(int value);
    [[nodiscard]] auto volume() const -> int;

public slots:
    void onDurationChanged(double value);
    void onPositionChanged(double value);
    void onCacheSpeedChanged(int64_t cache_speed);

signals:
    void previous();
    void next();
    void seek(int value);
    void hoverPosition(int pos, int value);
    void leavePosition();
    void pause();
    void volumeChanged(int value);
    void speedChanged(double value);
    void modelChanged(int model);
    void showList();

private slots:
    void onSpeedChanged();
    void onModelChanged();

private:
    void buildConnect();

    class ControlWidgetPrivate;
    QScopedPointer<ControlWidgetPrivate> d_ptr;
};

#endif // CONTROLWIDGET_HPP
