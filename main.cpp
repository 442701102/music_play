#include "mainwindow.h"

#include <QApplication>
#include <QTextCodec>
/**
 * Describe:Original cc Audio Player
 * Start Date:2022/01/31
 * End Date:2022/02/09
**/
int main(int argc, char *argv[])
{
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf8"));

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
