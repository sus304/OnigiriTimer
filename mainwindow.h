#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QThread>
#include <QKeyEvent>
#include <QKeySequence>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QSettings>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

#define CONST_V3_TIMESTEP 1000  // ms
#define CONST_LAP_TIMESTEP 1000 // ms


class TimerWorker : public QObject {
    Q_OBJECT

signals:
    void timeoutForControlThread();

public:
    TimerWorker(QObject *parent = nullptr);
    ~TimerWorker();

    void setTimeStep(unsigned short ms);

public slots:
    void start();  // for control thread
    void onThreadStarted();

    void onTimeout();

    void stop();
    void onThreadStoped();

private:
    QTimer* timer_p;
    QThread* thread_p;

    unsigned short timestep_ms;
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *obj, QEvent *event);


private slots:
    void updateTimerV3();
    void updateTimerLap();

    void startTimerV3();
    void startTimerLap();

    void on_pushButton_timer_v3_start_clicked();

    void on_pushButton_timer_lap_clicked();

    void on_pushButton_30s_sound_file_dialog_clicked();

    void on_pushButton_timer_v3_shortcut_fix_clicked();

    void on_pushButton_timer_lap_shortcut_fix_clicked();

    void on_radioButton_30s_sound_enable_clicked();

private:
    Ui::MainWindow *ui;

    TimerWorker* timer_worker_v3;
    TimerWorker* timer_worker_lap;

    QTimer* timer_v3_ms_p;
    QTimer* timer_lap_p;

    unsigned long time_v3;
    unsigned long time_lap;

    unsigned long lap_log[5];

    QKeySequence timer_v3_shortcut_keyseq;
    QKeySequence timer_lap_shortcut_keyseq;

    QString sound_file_path;
    QAudioOutput* audio_output_p;
    QMediaPlayer* sound_player_p;

    QString strCfgFile;

    QString convertCountupSec(unsigned long msec);
    QString convertCountdownSec(unsigned long msec);
    QString convertMinSec(unsigned long msec);

};



#endif // MAINWINDOW_H
