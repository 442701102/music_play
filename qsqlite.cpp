#include "qsqlite.h"

qsqlite::qsqlite(QObject *parent) : QObject(parent)
{
   qDebug()<<QString("Defualt Constructor")<<endl;
 //  qDebug()<<QString("Defualt parent:%1").arg(parent->objectName())<<endl;
}
qsqlite::qsqlite(QString ConnectionName,QString DatabaseName) :
    DB_ConnectionName(ConnectionName), \
    DB_DatabaseName(DatabaseName)
{
    qDebug()<<"Less:"<<ConnectionName<<endl<<DatabaseName<<endl;
}
qsqlite::qsqlite(QObject *parent,QString ConnectionName,QString DatabaseName,QString UserName,QString Password) :
    QObject(parent),\
    DB_ConnectionName(ConnectionName), \
    DB_DatabaseName(DatabaseName),\
    DB_UserName(UserName),\
    DB_Password(Password)
{
    qDebug()<<__FUNCTION__<<ConnectionName<<endl<<DatabaseName<<endl<<UserName<<"and"<<Password<<endl;
}
qsqlite::~qsqlite()
{

}
void qsqlite::add_log(musiclog_s_t musicinfo)
{
    musiclog_list.append(musicinfo);
}
QList<musiclog_s_t> qsqlite::get_musicloglist()
{
    return musiclog_list;
}
bool qsqlite::db_add(musiclog_s_t mdata)
{
//    QSqlDatabase db = QSqlDatabase::database(DB_DatabaseName); //建立数据库连接
    QSqlQuery query(database);
    QString insstring=QString("INSERT INTO %1 VALUES(NULL, :name,:starttime,:stoptime)").arg(DB_tabname);
#if DB_DEBUG
    qDebug()<<__FUNCTION__<<insstring <<endl;
#endif
    query.prepare(insstring);
    query.bindValue(0,mdata.m_name);
    query.bindValue(1,mdata.m_starttime.toString("yyyy年MM月dd日 hh:mm:ss"));
    query.bindValue(2,mdata.m_endtime.toString("yyyy年MM月dd日 hh:mm:ss"));

    bool qret = query.exec();
    if(qret)
    {
#if DB_DEBUG
        qDebug()<<__FUNCTION__<<":"<<"successfully"<<endl;
#endif
        return true;
    }
    else
    {
        qDebug()<<__FUNCTION__<<":"<<"failed"<<endl;
        return false;
    }
    return true;
}
bool qsqlite::db_update(musiclog_s_t mdata)
{
    qDebug()<<__FUNCTION__<<"This feature is not currently needed - "<<mdata.m_name<<endl;
/***
    QSqlDatabase db = QSqlDatabase::database(DB_DatabaseName); //建立数据库连接
    QSqlQuery query(db);
    QString updatestring=QString("update  %1 VALUES(:id, :name,:starttime,:stoptime)").arg(DB_DatabaseName);
    qDebug()<<__FUNCTION__<<updatestring <<endl;
    query.prepare(updatestring);
    query.bindValue(0,mdata.m_name);
    query.bindValue(1,mdata.m_starttime.currentDateTime().toString("yyyy年MM月dd日 hh:mm:ss"));
    query.bindValue(2,mdata.m_endtime.currentDateTime().toString("yyyy年MM月dd日 hh:mm:ss"));
    bool qret = query.exec();
    if(qret)
    {
        qDebug()<<__FUNCTION__<<":"<<"successfully"<<endl;
        return true;
    }
    else
    {
        qDebug()<<__FUNCTION__<<":failed"<<endl;
        return false;
    }
***/
   return true;
}
/*
 * Query all records
 *
 *
 */
bool qsqlite::db_queryAll(int flags)
{
    QSqlQuery query;

    bool qret = query.exec(QString("select * from %1").arg(DB_tabname));
    if(qret)
    {
#if DB_DEBUG
        qDebug()<<__FUNCTION__<<":"<<"successfully"<<endl;
#endif
    }
    else
    {
        qDebug()<<__FUNCTION__<<"failed"<<endl;
         return false;
    }
#if DB_DEBUG
        QSqlRecord rec = query.record();
        qDebug() << QObject::tr(QStinrg("MUSCILOG表的字段数为：%1个").arg(rec.count()));
#endif
    musiclog_s_t musicinfo;
//将数据库数据存储到本地链表中
    while(query.next())
    {
        if(flags>0)
        {
           musicinfo.m_name      = query.value(1).toString();
           musicinfo.m_starttime = QDateTime::fromString(query.value(2).toString(),"yyyy年MM月dd日 hh:mm:ss");
           musicinfo.m_endtime   = QDateTime::fromString(query.value(3).toString(),"yyyy年MM月dd日 hh:mm:ss");
#if DB_DEBUG
 //          qDebug()<<"db1:"<<musicinfo.m_name<<endl;
 //          qDebug()<<"db2:"<<query.value(2).toString()<<endl;
 //          qDebug()<<"db3:"<<query.value(3).toString()<<endl;
#endif
        }
         musiclog_list.append(musicinfo);
    }
    return true;
}
/*
 * Delete records based on id
 *
 *
 */
bool qsqlite::db_delete(int id)
{
    QSqlDatabase db = QSqlDatabase::database(DB_DatabaseName); //建立数据库连接
    QSqlQuery query(db);
    query.prepare(QString("delete from automobil where id = %1").arg(id));
    if(!query.exec())
    {
        qDebug() << "删除记录失败！";
        return false;
    }
    return true;
}
bool qsqlite::db_init()
{
#if DB_DEBUG
    qDebug()<<"db_init"<<DB_ConnectionName<<endl<<DB_DatabaseName<<endl<<DB_UserName<<"and"<<DB_Password<<endl;
#endif
    if (QSqlDatabase::contains(DB_ConnectionName))
    {
        database = QSqlDatabase::database(DB_ConnectionName);
    }
    else
    {
        database = QSqlDatabase::addDatabase(DB_ConnectionName);
        database.setDatabaseName(DB_DatabaseName);  // 设置db文件名称
        database.setUserName(DB_UserName);  // 设置登陆账号
        database.setPassword(DB_Password);
    }

    if (!database.open())
    {
            QMessageBox::critical(nullptr, QObject::tr("Cannot open database"),
                QObject::tr("Unable to establish a database connection.\n"),
            QMessageBox::Cancel);
            return false;
    }
    QSqlQuery query;
    bool success = query.exec(CREATE_TABLEDES);
    if(success)
    {
        qDebug() << QObject::tr("数据库表创建成功！\n");
        return true;
    }
    else
    {
#if DB_DEBUG
        qDebug() << QObject::tr("数据库表创建失败,已经存在！\n");
#endif
        return false;
    }
    return true;
}

