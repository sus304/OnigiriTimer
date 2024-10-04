#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    installEventFilter(this);

    strCfgFile = "config.ini";
    QSettings settings(strCfgFile, QSettings::IniFormat);

    ui->radioButton_30s_sound_enable->setChecked(settings.value("SoundEnable", true).toBool());

    sound_file_path = settings.value("SoundFilePath", "").toString();
    ui->lineEdit_30s_sound_file_path->setText(sound_file_path);

    audio_output_p = new QAudioOutput();
    audio_output_p->setVolume(50);
    sound_player_p = new QMediaPlayer();
    sound_player_p->setAudioOutput(audio_output_p);
    sound_player_p->setSource(QUrl::fromLocalFile(sound_file_path));

    QString key;
    key = settings.value("v3ShortcutKey", "Ctrl+V").toString();
    timer_v3_shortcut_keyseq = QKeySequence(key);
    ui->lineEdit_timer_v3_shortcut_key->setText(timer_v3_shortcut_keyseq.toString());
    key = settings.value("LapShortcutKey", "Ctrl+L").toString();
    timer_lap_shortcut_keyseq = QKeySequence(key);
    ui->lineEdit_timer_lap_shortcut_key->setText(timer_lap_shortcut_keyseq.toString());

    ui->label_timer_v3_countup->setText(convertCountupSec(0));
    ui->label_timer_v3_countdown->setText(convertCountdownSec(0));
    ui->progressBar_timer_v3->setValue(100);
    ui->label_timer_lap_countup->setText(convertMinSec(0));
    ui->label_laptime_average->setText(convertMinSec(0));
    ui->label_speed->setText(QString::number(0));
    ui->label_laptime_log1->setText(convertMinSec(0));
    ui->label_laptime_log2->setText(convertMinSec(0));
    ui->label_laptime_log3->setText(convertMinSec(0));
    ui->label_laptime_log4->setText(convertMinSec(0));
    ui->label_laptime_log5->setText(convertMinSec(0));

    // timer_v3_ms_p = new QTimer(this);
    // connect(timer_v3_ms_p, &QTimer::timeout, this, &MainWindow::updateTimerV3);

    // timer_lap_p = new QTimer(this);
    // connect(timer_lap_p, &QTimer::timeout, this, &MainWindow::updateTimerLap);

    timer_worker_v3 = new TimerWorker();
    timer_worker_v3->setTimeStep(CONST_V3_TIMESTEP);
    connect(timer_worker_v3, &TimerWorker::timeoutForControlThread, this, &MainWindow::updateTimerV3);

    timer_worker_lap = new TimerWorker();
    timer_worker_lap->setTimeStep(CONST_LAP_TIMESTEP);
    connect(timer_worker_lap, &TimerWorker::timeoutForControlThread, this, &MainWindow::updateTimerLap);

    this->setFocus();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// カウントアップタイマーの文字列変換
QString MainWindow::convertCountupSec(unsigned long msec) {
    float sec;
    sec = msec / 100.0;
    sec = std::floor(sec);
    sec /= 10.0;
    return QString::number(sec);
}
// カウントダウンタイマーの文字列変換
QString MainWindow::convertCountdownSec(unsigned long msec) {
    float sec;
    sec = msec / 1000.0;
    sec = 30.0 - sec;
    sec *= 10.0;
    sec = std::floor(sec);
    sec /= 10.0;
    return QString::number(sec);
}
// ラップタイマーの文字列変換
QString MainWindow::convertMinSec(unsigned long msec) {
    unsigned short min;
    unsigned short sec;
    sec = msec / 1000;
    min = sec / 60;
    sec -= min * 60;
    std::string time_str = std::to_string(min) + ":" + std::to_string(sec);
    return QString(QString::fromStdString(time_str));
}

// timer_v3の発火イベント
void MainWindow::updateTimerV3() {
    time_v3 += CONST_V3_TIMESTEP;
    ui->label_timer_v3_countup->setText(convertCountupSec(time_v3));

    if (time_v3 <= 30000) {
        ui->label_timer_v3_countdown->setText(convertCountdownSec(time_v3));
        ui->progressBar_timer_v3->setValue(std::floor(float(30000 - time_v3) / 30000.0 * 100.0));
        if (time_v3 == 30000) {
            if (ui->radioButton_30s_sound_enable->isChecked()) {
               try {
                   sound_player_p->play();
              } catch (...) {
              }
            }
        }
    } else if (time_v3 >= 60000) {
        timer_worker_v3->stop();
    }
}

// timer_lapの発火イベント
void MainWindow::updateTimerLap() {
    time_lap += CONST_LAP_TIMESTEP;
    ui->label_timer_lap_countup->setText(convertMinSec(time_lap));
}

void MainWindow::startTimerV3(){
    // timer_v3_ms_p->stop();
    time_v3 = 0;
    // timer_v3_ms_p->start(CONST_V3_TIMESTEP);
    timer_worker_v3->start();

    ui->label_timer_v3_countup->setText(convertCountupSec(0));
    ui->label_timer_v3_countdown->setText(convertCountdownSec(0));
    ui->progressBar_timer_v3->setValue(100);
}

void MainWindow::startTimerLap() {
    lap_log[4] = lap_log[3];
    lap_log[3] = lap_log[2];
    lap_log[2] = lap_log[1];
    lap_log[1] = lap_log[0];
    lap_log[0] = time_lap;

    // timer_lap_p->stop();
    time_lap = 0;
    // timer_lap_p->start(CONST_LAP_TIMESTEP);
    timer_worker_lap->start();

    // ログの更新
    ui->label_laptime_log5->setText(ui->label_laptime_log4->text());
    ui->label_laptime_log4->setText(ui->label_laptime_log3->text());
    ui->label_laptime_log3->setText(ui->label_laptime_log2->text());
    ui->label_laptime_log2->setText(ui->label_laptime_log1->text());
    ui->label_laptime_log1->setText(ui->label_timer_lap_countup->text());

    // 5周速の更新
    unsigned long sum = 0;
    sum = lap_log[0] + lap_log[1] + lap_log[2] + lap_log[3] + lap_log[4];
    unsigned long average = sum / 5;
    ui->label_laptime_average->setText(convertMinSec(average));

    // 時速の更新
    float hour = (float)lap_log[0] / 1000.0 / 60.0 / 60.0;
    float speed = std::floor(1150.0 / hour);
    ui->label_speed->setText(QString::number(speed));

    ui->label_timer_lap_countup->setText(convertMinSec(0));
}

// キー入力検知イベント
bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    // if (event->type() == QEvent::KeyPress || event->type()== QEvent::ShortcutOverride)
    if (event->type()== QEvent::ShortcutOverride)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent->type() == QEvent::KeyRelease) {
            return true;
        }

        QKeyCombination current_keyc = QKeyCombination(keyEvent->keyCombination());
        QKeySequence current_key = QKeySequence(current_keyc);

        if (timer_v3_shortcut_keyseq.matches(current_key)) {
            startTimerV3();
            return true;
        } else if (timer_lap_shortcut_keyseq.matches(current_key)) {
            startTimerLap();
            return true;
        }
    } else {
        return QMainWindow::eventFilter(obj, event);
    }

    return true;
}

void MainWindow::on_pushButton_timer_v3_start_clicked()
{
    startTimerV3();
}


void MainWindow::on_pushButton_timer_lap_clicked()
{
    startTimerLap();
}


void MainWindow::on_pushButton_30s_sound_file_dialog_clicked()
{
    try {
        QString strFileName = QFileDialog::getOpenFileName(
            this,
            tr("ファイル選択画面"),
            QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)
            );


        ui->lineEdit_30s_sound_file_path->setText(strFileName);
        sound_file_path = strFileName;
        sound_player_p = new QMediaPlayer();
        sound_player_p->setAudioOutput(audio_output_p);
        sound_player_p->setSource(QUrl::fromLocalFile(sound_file_path));


        QSettings settings(strCfgFile, QSettings::IniFormat);
        settings.setValue("SoundFilePath", sound_file_path);
        settings.sync();
    } catch (...) {
        ui->lineEdit_30s_sound_file_path->setText(QString(" "));
    }

}


void MainWindow::on_pushButton_timer_v3_shortcut_fix_clicked()
{
    timer_v3_shortcut_keyseq = ui->keySequenceEdit_timer_v3_shortcut->keySequence();
    ui->lineEdit_timer_v3_shortcut_key->setText(timer_v3_shortcut_keyseq.toString());

    QSettings settings(strCfgFile, QSettings::IniFormat);
    settings.setValue("v3ShortcutKey", timer_v3_shortcut_keyseq.toString());
    settings.sync();
}


void MainWindow::on_pushButton_timer_lap_shortcut_fix_clicked()
{
    timer_lap_shortcut_keyseq = ui->keySequenceEdit_timer_lap_shortcut->keySequence();
    ui->lineEdit_timer_lap_shortcut_key->setText(timer_lap_shortcut_keyseq.toString());

    QSettings settings(strCfgFile, QSettings::IniFormat);
    settings.setValue("LapShortcutKey", timer_lap_shortcut_keyseq.toString());
    settings.sync();
}


void MainWindow::on_radioButton_30s_sound_enable_clicked()
{
    QSettings settings(strCfgFile, QSettings::IniFormat);
    settings.setValue("SoundEnable", ui->radioButton_30s_sound_enable->isEnabled());
    settings.sync();
}

/////////////////////////////////////////////////////////////////////////////

TimerWorker::TimerWorker(QObject *parent) : QObject(nullptr), thread_p(new QThread(parent)), timer_p(nullptr) {
    timer_p = new QTimer(this);
    connect(timer_p, &QTimer::timeout, this, &TimerWorker::onTimeout);

    moveToThread(thread_p);
    timer_p->moveToThread(this->thread());

    connect(thread_p, &QThread::started, this, &TimerWorker::onThreadStarted);
    connect(thread_p, &QThread::finished, this, &TimerWorker::onThreadStoped);


}

TimerWorker::~TimerWorker() {
    thread_p->quit();
    thread_p->exit();
}

void TimerWorker::setTimeStep(unsigned short ms) {
    timestep_ms = ms;
}

void TimerWorker::start() {
    // if (thread_p->isRunning()) {
    //     thread_p->quit();
    // }
    thread_p->start();
}

void TimerWorker::onThreadStarted() {
    timer_p->stop();
    timer_p->start(timestep_ms);
}

void TimerWorker::onTimeout() {
    emit timeoutForControlThread();
}

void TimerWorker::stop() {
    thread_p->quit();
    thread_p->exit();
}

void TimerWorker::onThreadStoped() {
    timer_p->stop();
}
