#include "mainwindow.h"
#include "ui_mainwindow.h"
QString redtext("font: 75 11pt \"楷体\";color: red;");
QString bluetext("font: 75 11pt \"楷体\";color: rgb(0, 0, 255);");
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setAutoFillBackground(true);
    audio_thread_init();
    Ui_init();

    aworker->deallog();
}
void MainWindow::funButtoninit()
{
    //log button
 // ui->tabWidget->setCurrentIndex(0);
    ui->bigtabs->setStyleSheet(readQss(":/qss/Public.qss"));
}
void MainWindow::Ui_init()
{
    funControl_qss_init();
    funButtoninit();
}
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    qDebug()<<obj->objectName()<<event<<endl;
    return true;
}
MainWindow::~MainWindow()
{
    delete ui;
    delete aworker;
}
/********************************************************
Function:    slotTotalTimeChanged
Description: 音乐总长度更新
Input:
OutPut:
Return:void
Others:通知更新进度条
Author: Zhiyu Liu
Date:   2021-07-10
*********************************************************/
void MainWindow::slotTotalTimeChanged(qint64 uSec)
{
   int Sec = uSec/1000000;

   ui->horizontalSlider_time->setRange(0,Sec);
   ui->horizontalSlider_time->setSingleStep(1);
   QString alltimestr;

   ui->label_totaltime->setText(alltimestr.sprintf("%2.2d:%2.2d",(int)Sec/60,(int)Sec%60));
}
/********************************************************
Function:    slotCurrentTimeChanged
Description: 音乐当前长度更新
Input:
OutPut:
Return:void
Others: 通知更新画面
Author: Zhiyu Liu
Date:   2022-02-01
*********************************************************/
void MainWindow::slotCurrentTimeChanged()
{
    static quint32 slider_event_timeout = 0 ;
    if (QObject::sender() == mTimer)
    {
        qint64 Sec = aworker->getCurrentTime();
        if(seek_flag != true)
        {
          ui->horizontalSlider_time->setValue(Sec);
          slider_event_timeout = 0;
        }
        else
        {
          ++slider_event_timeout;
          if(SliderPageStepflag == true)
          {
            ++SliderPageSteptimeout;
            if(SliderPageSteptimeout >2)
            {
               SliderPageSteptimeout = 0;
               SliderPageStepflag = false;
               ui->label_text_remind->setStyleSheet(redtext);
               qDebug()<< QString("[SliderPageStep]move time:%1").arg(ui->horizontalSlider_time->value())<<endl;
               ui->label_text_remind->setText(QString("正在跳转至 %1 s").arg(ui->horizontalSlider_time->value()));
               aworker->seek((qint64)ui->horizontalSlider_time->value() * AV_TIME_BASE);
            }
          }
          else if(slider_event_timeout >=20 )
          {
              seek_flag = false;
          }
        }
        QString alltimestr;
        ui->label_currenttime->setText(alltimestr.sprintf("%2.2d:%2.2d",(int)Sec/60,(int)Sec%60));
    }
}
/********************************************************
 * Function:	 slotSliderMoved() and slotSliderPressed()
 * Description: 进度条滑动事件处理,按压和释放
 * Return:void
 * Date:2021-08-10
*********************************************************/

void MainWindow::slotSlideractionChanged(int Slider_action)
{
//   qDebug()<< QString("[slotSliderChanged]action[%1]-time[%2]").arg(ui->horizontalSlider_time->value())<<Slider_action<<endl;
   switch(Slider_action)
   {
   case   QAbstractSlider::SliderNoAction :
       break;
   case   QAbstractSlider::SliderSingleStepAdd :
       break;
   case   QAbstractSlider::SliderSingleStepSub :
       break;
   case   QAbstractSlider::SliderPageStepAdd :
   case   QAbstractSlider::SliderPageStepSub :
       if (QObject::sender() == ui->horizontalSlider_time)
       {
           ui->label_text_remind->setText(QString("预计跳转至 %1 s").arg(ui->horizontalSlider_time->value()));
           ui->label_text_remind->setStyleSheet(bluetext);
           SliderPageStepflag = true;
           seek_flag = true;
           SliderPageSteptimeout = 0;
       }
       break;
   case   QAbstractSlider::SliderToMinimum :
       break;
   case   QAbstractSlider::SliderToMaximum :
       break;
   case   QAbstractSlider::SliderMove :
       break;
   }

}
void MainWindow::slotSliderReleased()
{
   qDebug()<< QString("[slotSliderReleased]move time:%1").arg(ui->horizontalSlider_time->value())<<endl;
   if (QObject::sender() == ui->horizontalSlider_time)
   {
       ui->label_text_remind->setText(QString("正在跳转至 %1 s").arg(ui->horizontalSlider_time->value()));
       aworker->seek((qint64)ui->horizontalSlider_time->value() * AV_TIME_BASE);
   }
}
void MainWindow::slotaudiostatusChanged(int event)
{
  qDebug()<<__FUNCTION__<<"event:"<<event<<endl;
  switch(event)
  {
    case Startfailed:
    {
      aworker->play(ui->label_filename->text());
    }break;
    case Seekend:
    {
      ui->label_text_remind->clear();
      seek_flag = false;
    }break;
    case Playend:
//      qDebug()<<"PLAY AN MUSIC:"<<endl;
 //     for(int i =0;i<aworker->musiclog_list.size();++i)
      {
        qDebug()<<aworker->musicinfo.m_name<<endl;
        qDebug()<<aworker->musicinfo.m_starttime<<endl;
        qDebug()<<aworker->musicinfo.m_endtime<<endl;
      }
     break;
    case Firstbuild:
    {
      qDebug()<<"First build loglist" <<endl;
      QVector<QStringList> templists(3);
      for(int i=0;i<aworker->musicinfos.size();++i)
      {
           templists[0] << aworker->musicinfos.at(i).m_name;
           templists[1] << aworker->musicinfos.at(i).m_starttime.toString("yyyy年MM月dd日 hh:mm:ss");
           templists[2] << aworker->musicinfos.at(i).m_endtime.toString("yyyy年MM月dd日 hh:mm:ss");
      }
      ui->listWidget_name->addItems( templists[0] );
      ui->listWidget_starttime->addItems( templists[1] );
      ui->listWidget_endtime->addItems( templists[2] );
    }break;
    default:break;
  }
}
void MainWindow::slotSliderPressed()
{
  seek_flag = true;
}
/********************************************************
 * Function: on_pushButton_xxx_clicked
 * Description: 用户按钮相关槽函数
 * Return: void
 * Date:2021-08-10
*********************************************************/
void MainWindow::on_pushButton_test_clicked()
{
    qDebug()<<"qt test"<<endl;
    qint64 Sec = aworker->getCurrentTime();
//     qDebug() <<QString("get %1").arg(Sec)<<endl;
    ui->horizontalSlider_time->setValue(Sec);
    QString alltimestr;

    qDebug()<<"2021/8/6:"<<QString(alltimestr.sprintf("%2.2d:%2.2d",(int)Sec/60,(int)Sec%60))<<endl;
}
void MainWindow::on_pushButton_play_clicked()
{
    if(ui->label_filename->text()!=nullptr)
    {
   //     qDebug()<< QString("start play %1-%2").arg(ui->label_filename->text().toLocal8Bit().data()).arg(ui->label_filename->text());
        while(aworker->status() == 5)
        {
          av_usleep(5000);
        }
        if(aworker->status()==0)
        {
//           qDebug()<< QString("play new music") <<endl;
           emit music_decoding_start((const char *)ui->label_filename->text().toLocal8Bit().data(),0);
        }
        else
        {
//         qDebug()<< QString("[BUTTON] play again") <<endl;
         aworker->play(ui->label_filename->text());
        }
    }
}
void MainWindow::on_pushButton_file_clicked()
{
    QString ccfile = QFileDialog::getOpenFileName(this,tr("选择要播放的音乐"),"E:\\Music/", tr("所有文件(*);;音频文件 (*.WMA *.WAV *.FLAC *.MP3 *.AAC *.OGG);"));
    if (!ccfile.isEmpty())
    {
        /**在linux系统中不需要做目录的替换**/
            ccfile.replace("/","\\");
            ui->label_filename->setText(ccfile);
 //            qDebug()<< QString("set play %1-%2").arg(ccfile.toLocal8Bit().data()).arg(ccfile);
    }
}
void MainWindow::on_pushButton_stop_clicked()
{
    while(aworker->status() == 1 || aworker->status() == 3)
    {
//      qDebug()<< QString("[BUTTON] stop music") <<endl;
      aworker->stop();
    }
}
void MainWindow::on_pushButton_pause_clicked()
{
    aworker->pause();
    qDebug()<<QString("[usr]pause music")<<endl;
}

/********************************************************
 * Function:	readQss(QString const &file)
 * Description: 读取qss文件,返回其格式
 * Input:  qss file name
 * Return: QString
 * Date:	2021-08-10
*********************************************************/
QString MainWindow::readQss(QString const &file)
{
    QFile qssFile(file);
    qssFile.open(QFile::ReadOnly);
    if(qssFile.isOpen())
    {
//        qDebug()<< QString("read %1 format successfully").arg(file)<<endl;
        QString qsssheet = QLatin1String(qssFile.readAll());
        qssFile.close();
        return qsssheet;
    }
    else
    {
        qDebug()<< QString("Failed to read %1 format").arg(file)<<endl;
        return nullptr;
    }
}
/********************************************************
 * Function:	 audio_thread_init()
 * Description: 启动音频线程,并绑定相关槽函数
 * Return: void
 * Date:	2021-08-13
*********************************************************/
void MainWindow::audio_thread_init()
{
 /** Instantiate the audio thread **/
 athread = new QThread(this);//Create child thread
 aworker = new ddaudio();//Do not specify the parent class
 aworker->moveToThread(athread);//Add custom threads to child threads
 /** Signal and slot function binding between classes **/
 connect(athread,&QThread::finished,aworker,&QObject::deleteLater);  //Thread suicide through deleteLater
 connect(aworker,SIGNAL(sig_TotalTimeChanged(qint64)),this,SLOT(slotTotalTimeChanged(qint64)));
 connect(aworker,SIGNAL(sig_audioevent(int)),this,SLOT(slotaudiostatusChanged(int)));
 connect(this, &MainWindow::music_decoding_start, aworker, &ddaudio::work_main); //Connect signal to thread start function
 /** Play time progress bar drag signal binding **/
 connect(ui->horizontalSlider_time,SIGNAL(sliderPressed()),this,SLOT(slotSliderPressed()));
 connect(ui->horizontalSlider_time,SIGNAL(sliderReleased()),this,SLOT(slotSliderReleased()));
 connect(ui->horizontalSlider_time,SIGNAL(actionTriggered(int)),this,SLOT(slotSlideractionChanged(int)));
 ui->horizontalSlider_time->setTracking(true);
 /** Timer - get current music time **/
 mTimer = new QTimer(this);
 connect(mTimer,SIGNAL(timeout()),this,SLOT(slotCurrentTimeChanged()));
 mTimer->setInterval(500);
 /** Turn on thread standby **/
 mTimer->start();
 athread->start();//Start the thread
}
/********************************************************
 * Function:	 funControl_qss_init()
 * Description: 通过样式表设置控件初始样式
 * Return: void
 * Date:	2021-08-13
*********************************************************/
void MainWindow::funControl_qss_init()
{
    ui->pushButton_test->setStyleSheet(readQss(":/qss/QPushButton_stylesheet.qss"));
    ui->pushButton_recommend->setStyleSheet(readQss(":/qss/QPushButton_stylesheet.qss"));
    ui->pushButton_Favorites->setStyleSheet(readQss(":/qss/QPushButton_stylesheet.qss"));
    ui->pushButton_file->setStyleSheet(readQss(":/qss/QPushButton_stylesheet.qss"));
    ui->pushButton_record->setStyleSheet(readQss(":/qss/QPushButton_stylesheet.qss"));

    ui->pushButton_play->setStyleSheet(readQss(":/qss/QIconPushButton_stylesheet.qss"));
    ui->pushButton_stop->setStyleSheet(readQss(":/qss/QIconPushButton_stylesheet.qss"));
    ui->pushButton_pause->setStyleSheet(readQss(":/qss/QIconPushButton_stylesheet.qss"));
    ui->pushButton_next->setStyleSheet(readQss(":/qss/QIconPushButton_stylesheet.qss"));
    ui->pushButton_previous->setStyleSheet(readQss(":/qss/QIconPushButton_stylesheet.qss"));
    ui->pushButton_cycle->setStyleSheet(readQss(":/qss/QIconPushButton_stylesheet.qss"));

    ui->label_text_remind->setStyleSheet(redtext);
}

void MainWindow::on_pushButton_record_clicked()
{
    ui->bigtabs->setCurrentIndex(1);
}

