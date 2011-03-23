#include <QtCore/QCoreApplication>

#include <AccessToSqlite.h>
#include <syncer.h>
#include <connector.h>

#include <QCryptographicHash>

#include <QDebug>
int main(int argc, char *argv[])
{    
    QCoreApplication a(argc, argv);

//    convertAccessDbToSqliteDb("D:\\Flash\\Project\\Match\\Match\\ForConvert.mdb", "reghaabat.db");

    Syncer syncer;
    Sender sender;

    QString json = syncer.getChunk();
    //while (! json.isNull())
    {
        sender.send(QUrl("http://localhost/server.php"), json);
    }

    return a.exec();
}

