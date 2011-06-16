#include "connector.h"

#include <QDebug>
#include <QSettings>
#include <QFile>

#include "helper.h"

QSqlDatabase Connector::connectDb()
{
    if (! QSqlDatabase::contains())
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(dataFolder() + "/reghaabat.dat");

        if (! db.open())
            qDebug() << "reghaabat db connection error : " << db.lastError();
        return db;
    } else
        return QSqlDatabase::database();
}

QSqlDatabase Connector::connectLibrary(bool& ok)
{
    QSqlDatabase db;
    ok = false;

    QSettings settings("Rooyesh", "Reghaabat");
    QString library = settings.value("LibraryAddress", "").toString();

    if (library.isEmpty() || ! QFile::exists(library))
        return db;

    if (! QSqlDatabase::contains("Library"))
    {
        db = QSqlDatabase::addDatabase("QODBC", "Library");
        db.setDatabaseName("Driver={Microsoft Access Driver (*.mdb)};DSN='';DBQ=" + library);
        db.setPassword("abrdmkazhdpkzsrst");

        if (! db.open())
            qDebug() << "library db connection error : " << db.lastError();
        else
            ok = true;
    } else {
        db = QSqlDatabase::database("Library");
        ok = true;
    }

    return db;
}
