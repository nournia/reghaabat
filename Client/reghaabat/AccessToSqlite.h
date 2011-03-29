#ifndef ACCESSTOSQLITE_H
#define ACCESSTOSQLITE_H

#include <Jalali.h>
#include <connector.h>

#include <QtCore/QCoreApplication>
#include <QDate>
#include <QStringList>
#include <QVariant>
#include <QFile>
#include <QDebug>

QSqlDatabase accessDb, sqliteDb;
QSqlQuery accessQry, sqliteQry;

QSqlDatabase connectAccess(QString filename)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC", "AccessDb");
    db.setDatabaseName("Driver={Microsoft Access Driver (*.mdb)};DSN='';DBQ=" + filename);
    db.setPassword("abrdmkazhdpkzsrst");

    if (! db.open())
        qDebug() << "access db connection error : " << db.lastError();

    return db;
}

bool buildSqliteDb()
{
    QFile file("../sqlite.sql");
    if (! file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "sql file not found";
        return false;
    }

    sqliteDb.transaction();
    foreach (QString table, sqliteDb.tables(QSql::Tables))
    {
        if (! table.startsWith("sqlite_") && ! sqliteQry.exec("drop table if exists " + table + ";"))
            qDebug() << sqliteQry.lastError();
    }

    foreach (QString q, QTextStream(&file).readAll().split(";"))
    {
        if (q.contains("create", Qt::CaseInsensitive) || q.contains("insert", Qt::CaseInsensitive))
            if (! sqliteQry.exec(q))
            {
            qDebug() << "sql file error: " << sqliteQry.lastError();
            return false;
        }
    }
    sqliteDb.commit();

    return true;
}

QString getInsertQuery(QString table, QStringList fields)
{
    QString params = ") values (";

    for (int i = 0; i < fields.size(); i++)
    {
        params = params + ':' + fields[i];
        if (i < fields.size()-1)
            params += ',';
    }

    return "insert into " + table + " (" + fields.join(",") + params + ")";
}

QVariant refineValue(QVariant value)
{
    QString tn = value.typeName();
    if (tn == "QString" &&  value.toString().contains(''))
        return value.toString().replace('', "");
        //return value.toString().replace('ي', 'ی').replace('ك', 'ک').replace('', ""); // replace specific charcters
    else
        return value;
}

bool importTable(QString table, QString query, QStringList fields)
{
    if (! accessQry.exec(query))
    {
        qDebug() << "access select query error : " << accessQry.lastError();
        return false;
    }

    QString qry = getInsertQuery(table, fields);
    if (! sqliteQry.prepare(qry))
    {
        qDebug() << "sqlite insert query error : " << sqliteDb.lastError();
        return false;
    }

    QVariant tmp;
    sqliteDb.transaction();
    while (accessQry.next())
    {
        for (int i = 0; i < fields.size(); i++)
        {
            if (! accessQry.record().fieldName(i).endsWith("date"))
                tmp = refineValue(accessQry.value(i));
            else
                tmp = getGregorianVariant(accessQry.value(i).toString());
            sqliteQry.bindValue(i, tmp);
        }

        if (! sqliteQry.exec())
            qDebug() << table << " import error : " << sqliteQry.lastError() << accessQry.value(0).toString() << accessQry.value(1).toString();
    }
    sqliteDb.commit();

    qDebug() << "+ " << table;
    return true;
}

QVariant getTitleId(QString table, QString title)
{
    QVariant null; null.clear();
    if (title == "") return null;

    QSqlQuery tmp(sqliteDb);
    tmp.exec("select id from " + table + " where title = '"+ title +"'");

    if (tmp.next())
        return tmp.value(0);
    else
    {
        tmp.exec("insert into " + table + " (title) values ('" + title + "')");
        return tmp.lastInsertId();
    }
}

bool importMatches()
{
    // init
    QVariant null; null.clear();

    sqliteQry.exec("select id, beginage, endage from ageclasses order by id");
    QString ageField = "", estr;
    while (sqliteQry.next())
    {
        ageField += "iif(age >= "+ sqliteQry.value(1).toString() +" and age <= "+ sqliteQry.value(2).toString() + ", " + sqliteQry.value(0).toString() + ", ";
        estr += ")";
    }
    ageField += "NULL" + estr;

    QMap<int, int> groups;
    groups.insert(21, 4);
    groups.insert(22, 3);
    groups.insert(23, 2);
    groups.insert(31, 1);
    groups.insert(32, 0);

    // access select query
    if (! accessQry.exec("select id, designerid, title, "+ ageField +" as ageclass, groupid, content, pictureconfiguration, author, publication from matches"))
        qDebug() << accessQry.lastError();

    // resource and match insertion
    sqliteQry.prepare(getInsertQuery("matches", QStringList() << "id" << "designer_id" << "title" << "ageclass" << "resource_id" << "category_id" << "content" << "configuration"));

    QSqlQuery resourceQry(sqliteDb);
    resourceQry.prepare(getInsertQuery("resources", QStringList() << "kind" << "author_id" << "publication_id" << "title" << "ageclass"));

    sqliteDb.transaction();
    while (accessQry.next())
    {
        sqliteQry.bindValue(":id", accessQry.value(0));
        sqliteQry.bindValue(":designer_id", accessQry.value(1));
        sqliteQry.bindValue(":title", accessQry.value(2));
        sqliteQry.bindValue(":ageclass", accessQry.value(3));

        if (accessQry.value(0).toString().startsWith("31") || accessQry.value(0).toString().startsWith("34"))
        {
            sqliteQry.bindValue(":category_id", null);
            sqliteQry.bindValue(":content", null);
            sqliteQry.bindValue(":configuration", null);

            if (accessQry.value(0).toString().startsWith("31"))
                resourceQry.bindValue(":kind", "book");
            else
                resourceQry.bindValue(":kind", "multimedia");

            resourceQry.bindValue(1, getTitleId("authors", accessQry.value(7).toString()));
            resourceQry.bindValue(2, getTitleId("publications", accessQry.value(8).toString()));
            resourceQry.bindValue(3, accessQry.value(2));
            resourceQry.bindValue(4, accessQry.value(3));

            if (! resourceQry.exec())
                qDebug() << resourceQry.lastError();

            sqliteQry.bindValue(":resource_id", resourceQry.lastInsertId());
        }
        else {
            sqliteQry.bindValue(":resource_id", null);

            sqliteQry.bindValue(":category_id", groups[accessQry.value(4).toInt()]);
            sqliteQry.bindValue(":content", accessQry.value(5));
            sqliteQry.bindValue(":configuration", accessQry.value(6));
        }

        if (! sqliteQry.exec())
            qDebug() << sqliteQry.lastError() << accessQry.value(1).toString();
    }
    sqliteDb.commit();
    qDebug() << "+ " << QString("matches");

    return true;
}

void convertAccessDbToSqliteDb(QString accessFilename)
{
    sqliteDb = Connector::connectDb();
    accessDb = connectAccess(accessFilename);

    sqliteQry = QSqlQuery();
    accessQry = QSqlQuery(accessDb);

    if (! buildSqliteDb())
        return;

    sqliteQry.exec("pragma foreign_keys = on");

    importTable("users", "select id, firstname, lastname, birthdate, address, phone, iif(man = true, 'male', 'female'), description, registerdate, registerdate as udate from users",
                QStringList() << "id" << "firstname" << "lastname" << "birth_date" << "address" << "phone" << "gender" << "description"  << "created_at"  << "updated_at");

    importMatches();

    importTable("questions", "select matchid, question, answer from questions",
                QStringList() << "match_id" << "question" << "answer");

    importTable("answers", "select userid, matchid, iif(deliverdate is null, '1300/01/01', receivedate) as rdate, iif(deliverdate is null, '1300/01/01', scoredate) as sdate, round(transactions.score/matches.maxscore, 2) as rate, iif(deliverdate is null, '1300/01/01', deliverdate) as ddate, iif(deliverdate is null, '1300/01/01', deliverdate) as udate from transactions left join matches on transactions.matchid = matches.id",
                QStringList() << "user_id" << "match_id" << "received_at" << "corrected_at" << "rate" << "created_at" << "updated_at");

    importTable("supports", "select id, designerid, maxscore, iif(state = 0, 'active', iif(state = 1, 'disabled', iif(state = 2 , 'imported', NULL))) from matches",
                QStringList() << "match_id" << "corrector_id" << "score" << "current_state");

    importTable("payments", "select userid, score, scoredate, scoredate as udate from payments",
                QStringList() << "user_id" << "payment" << "created_at" << "updated_at");

    importTable("open_scores", "select userid, 0, title, score, scoredate, scoredate as udate from freescores",
                QStringList() << "user_id" << "category_id" << "title" << "score" << "created_at" << "updated_at");

//    importTable("pictures", "select id, picture, iif(id > 100000, 'match', iif(id > 1000, 'user', 'library')) from pictures",
//                QStringList() << "reference_id" << "picture" << "kind");

    sqliteQry.exec("pragma foreign_keys = off");

    accessQry.finish();

    qDebug() << "import finished";
}



#endif // ACCESSTOSQLITE_H