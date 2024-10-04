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

static const QString groupbox_header("QLabel {");
static const QString groupbox_footer("}");
static const QString background_color("color: rgb(63, 63, 63);");
static const QString color_while("color: rgb(255, 255, 255);");
static const QString bgc_clear("background-color: rgba(255, 255, 255, 0);");
static const QString bgc_green("background-color: limegreen;");
static const QString bgc_yellow("background-color: gold;");
static const QString bgc_red("background-color: rgb(220, 20, 60);");
static const QString groupbox_clear_style = groupbox_header + color_while + bgc_clear + groupbox_footer;
static const QString groupbox_green_style = groupbox_header + color_while + bgc_green + groupbox_footer;
static const QString groupbox_yellow_style = groupbox_header + color_while + bgc_yellow + groupbox_footer;
static const QString groupbox_red_style = groupbox_header + color_while + bgc_red + groupbox_footer;



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

    void on_pushButton_timer_v3_reverse_clicked();

    void on_pushButton_timer_v3_reverse_shortcut_fix_clicked();

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
    QKeySequence timer_v3_reverse_shortcut_keyseq;
    QKeySequence timer_lap_shortcut_keyseq;

    QString sound_file_path;
    QAudioOutput* audio_output_p;
    QMediaPlayer* sound_player_p;

    QString strCfgFile;

    QString convertCountupSec(unsigned long msec);
    QString convertCountdownSec(unsigned long msec);
    QString convertMinSec(unsigned long msec);

    void setV3GreenStyle();
    void setV3YellowStyle();
    void setV3RedStyle();

};



#endif // MAINWINDOW_H
