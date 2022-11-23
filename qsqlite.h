#ifndef QSQLITE_H
#define QSQLITE_H

#include <QObject>
#include <QSqlRecord>
#include <env.h>
#define CREATE_TABLEDES "create table MUSCILOG(id INTEGER primary key AUTOINCREMENT,name varchar(128), starttime varchar(10), endtime varchar(10))"

class qsqlite : public QObject
{
    Q_OBJECT
public:
    //explicit
    explicit qsqlite(QObject *parent = nullptr);
    ~qsqlite();
    explicit qsqlite(QString ConnectionName,QString DatabaseName);
    explicit qsqlite(QObject *parent,QString ConnectionName,QString DatabaseName,QString UserName,QString Password);
    bool db_init();
    bool db_add(musiclog_s_t);
    bool db_update(musiclog_s_t);
    bool db_delete(int );
    bool db_queryAll(int flags);
    void add_log(musiclog_s_t);
    QList<musiclog_s_t>  get_musicloglist();

private:
    QString DB_ConnectionName;
    QString DB_DatabaseName;
    QString DB_UserName;
    QString DB_Password;
    QString DB_tabname="MUSCILOG";
    QSqlDatabase database;
    QList<musiclog_s_t>  musiclog_list;
signals:
    void sig_SqliteError(qint32 types); //获取到视频时长的时候激发此信号
};

#endif // QSQLITE_H
