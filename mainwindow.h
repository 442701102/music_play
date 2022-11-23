#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ddaudio.h"
#include "env.h"
#include <QMainWindow>
#include <QAbstractSlider>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void on_pushButton_test_clicked();
    void on_pushButton_play_clicked();
    void on_pushButton_file_clicked();
    void on_pushButton_stop_clicked();
    void on_pushButton_pause_clicked();

    void slotTotalTimeChanged(qint64 uSec);
    void slotCurrentTimeChanged();
    void slotaudiostatusChanged(int);

    void slotSliderReleased();
    void slotSliderPressed();
    void slotSlideractionChanged(int);



    void on_pushButton_record_clicked();

private:
    void audio_thread_init();
    void funButtoninit();
    void funControl_qss_init();
    void Ui_init();
    QString readQss(QString const &file);

    bool seek_flag = false;
    bool SliderPageStepflag = false;
    quint32 SliderPageSteptimeout = 0 ;
    QThread *athread;
    ddaudio *aworker;
    QTimer *mTimer;
    Ui::MainWindow *ui;
protected:
     bool eventFilter(QObject *obj, QEvent *event);
signals:
       void music_decoding_start(const char * ,int);
};
#endif // MAINWINDOW_H
