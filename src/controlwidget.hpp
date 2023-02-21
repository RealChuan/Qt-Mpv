#ifndef CONTROLWIDGET_HPP
#define CONTROLWIDGET_HPP

#include <QWidget>

class ControlWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ControlWidget(QWidget *parent = nullptr);
    ~ControlWidget();

    QPoint sliderGlobalPos() const;

    void setVolumeMax(int max);
    void setVolume(int value);
    int volume() const;

public slots:
    void onDurationChanged(double value);
    void onPositionChanged(double value);

signals:
    void volumeChanged(int value);
    void seek(int value);
    void hoverPosition(int pos, int value);
    void leavePosition();
    void speedChanged(double);
    void showList();

private slots:
    void onSpeedChanged();

private:
    void setupUI();
    void buildConnect();

    class ControlWidgetPrivate;
    QScopedPointer<ControlWidgetPrivate> d_ptr;
};

#endif // CONTROLWIDGET_HPP
