#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onOpenLocalMedia();
    void onOpenWebMedia();
    void onFileLoaded();
    void onTrackChanged();

    void playlistPositionChanged(int);
    void jump(const QModelIndex &index);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void keyPressEvent(QKeyEvent *ev) override;

private:
    void setupUI();
    void buildConnect();
    void initMenu();
    void addToPlaylist(const QList<QUrl> &urls);

    class MainWindowPrivate;
    QScopedPointer<MainWindowPrivate> d_ptr;
};
#endif // MAINWINDOW_HPP
