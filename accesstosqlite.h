#ifndef ACCESSTOSQLITE_H
#define ACCESSTOSQLITE_H

#include <jalali.h>
#include <connector.h>
#include <musers.h>
#include <mmatches.h>

#include <QtCore/QCoreApplication>
#include <QDate>
#include <QStringList>
#include <QVariant>
#include <QFile>
#include <QDebug>
#include <QImage>
#include <QDir>

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

bool buildSqliteDb(bool library = false)
{
    QString filename = ":/resources/sqlite.sql";
    if (library)
        filename = ":/resources/library.sql";

    QFile file(filename);
    if (! file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "sql file not found";
        return false;
    }

    sqliteDb.transaction();
    if (! library)
        foreach (QString table, sqliteDb.tables(QSql::Tables))
        {
            if (! table.startsWith("sqlite_") && ! sqliteQry.exec("drop table if exists " + table + ";"))
                qDebug() << sqliteQry.lastError();
        }

    QString trigger = "";
    foreach (QString q, QTextStream(&file).readAll().split(";"))
    {
        if (q.contains("trigger", Qt::CaseInsensitive))
            trigger = q + ";";
        else if (! trigger.isEmpty())
        {
            trigger += q + ";";
            if (trigger.contains("end;"))
            {
                if (! sqliteQry.exec(trigger))
                {
                    qDebug() << "sql file error: " << sqliteQry.lastError();
                    return false;
                }
                trigger = "";
            }
        }
        else if (q.contains("create", Qt::CaseInsensitive) || q.contains("insert", Qt::CaseInsensitive) || q.contains("update", Qt::CaseInsensitive))
            if (! sqliteQry.exec(q))
            {
                qDebug() << "sql file error: " << sqliteQry.lastError();
                return false;
            }
    }
    sqliteDb.commit();

    if (! library)
    {
        QDir d;
        d.mkdir("data");
        d.mkdir("data/files");
    }

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
    if (tn == "QString")
        return refineText(value.toString()).replace('', "");
    else
        return value;
}

// fields.size() : operatorId, fields.size()+1 : time
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
        else
            insertLog(table, "insert", sqliteQry.lastInsertId().toString(), accessQry.value(fields.size()).toString(), QDateTime(toGregorian(accessQry.value(fields.size()+1).toString())));
    }
    sqliteDb.commit();

    qDebug() << "+ " << table;
    return true;
}

QString refineContent(QString content)
{
    content.replace(QRegExp("[\r\n]+"), "</p><p>");
    content = "<p>" + content + "</p>";
    content.replace("<p></p>", "");
    return refineText(content);
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
    if (! accessQry.exec("select id, designerid, title, "+ ageField +" as ageclass, groupid, content, pictureconfiguration, author, publication, librarybookid from matches"))
        qDebug() << accessQry.lastError();

    // resource and match insertion
    sqliteQry.prepare(getInsertQuery("matches", QStringList() << "id" << "designer_id" << "title" << "ageclass" << "resource_id" << "category_id" << "content"));

    QVariant resourceId;
    QSqlQuery resourceQry(sqliteDb);
    resourceQry.prepare(getInsertQuery("resources", QStringList() << "kind" << "author_id" << "publication_id" << "title" << "ageclass"));

    QSqlQuery objectQry(sqliteDb);
    objectQry.prepare(getInsertQuery("objects", QStringList() << "resource_id" << "label"));

    sqliteDb.transaction();
    while (accessQry.next())
    {
        sqliteQry.bindValue(":id", accessQry.value(0));
        sqliteQry.bindValue(":designer_id", accessQry.value(1));
        sqliteQry.bindValue(":title", refineText(accessQry.value(2).toString()));
        sqliteQry.bindValue(":ageclass", accessQry.value(3));

        if (accessQry.value(0).toString().startsWith("31") || accessQry.value(0).toString().startsWith("34"))
        {
            sqliteQry.bindValue(":category_id", null);
            sqliteQry.bindValue(":content", null);

            if (accessQry.value(0).toString().startsWith("31"))
                resourceQry.bindValue(":kind", "book");
            else
                resourceQry.bindValue(":kind", "multimedia");

            resourceQry.bindValue(":author_id", insertTitleEntry("authors", accessQry.value(7).toString()));
            resourceQry.bindValue(":publication_id", insertTitleEntry("publications", accessQry.value(8).toString()));
            resourceQry.bindValue(":title", refineText(accessQry.value(2).toString()));
            resourceQry.bindValue(":ageclass", refineText(accessQry.value(3).toString()));

            if (! resourceQry.exec())
                qDebug() << "resource " << resourceQry.lastError() << accessQry.value(0).toString();
            else
            {
                resourceId = resourceQry.lastInsertId();
                insertLog("resources", "insert", resourceId);
            }

            sqliteQry.bindValue(":resource_id", resourceId);

            objectQry.bindValue(":resource_id", resourceId);
            objectQry.bindValue(":label", accessQry.value(9));
            if (! objectQry.exec())
                qDebug() << "object " << objectQry.lastError();
            else
                insertLog("objects", "insert", objectQry.lastInsertId());
        }
        else {
            sqliteQry.bindValue(":category_id", groups[accessQry.value(4).toInt()]);
            sqliteQry.bindValue(":content", refineContent(accessQry.value(5).toString()));
            sqliteQry.bindValue(":resource_id", null);
        }

        if (! sqliteQry.exec())
            qDebug() << "match " << sqliteQry.lastError() << accessQry.value(1).toString();
        else
            insertLog("matches", "insert", sqliteQry.lastInsertId());
    }
    sqliteDb.commit();
    qDebug() << "+ " << QString("matches");

    return true;
}

void importImages()
{
    QString sql = "select id, 'jpg', picture, '', '' from pictures where id > 10000 or id < 1000";

    importTable("files", sql, QStringList() << "id" << "extension");

    accessQry.exec(sql);
    while (accessQry.next())
    {
        QString id = accessQry.value(0).toString();
        QString filename = QString("data/files/%1.jpg").arg(id);
        QImage::fromData(accessQry.value(2).toByteArray(), "jpg").save(filename, "jpg");

        sqliteQry.exec("update matches set content = content || '<p><img "+ QString(id.startsWith("331") ? "width=\"100%\" " : "") +"src=\""+ id +".jpg\"></p>' where id = "+ id);
        insertLog("matches", "update", id);
    }
}

void importMatchDb(QString accessFilename)
{
    sqliteDb = Connector::connectDb();
    accessDb = connectAccess(accessFilename);

    sqliteQry = QSqlQuery();
    accessQry = QSqlQuery(accessDb);

    if (! buildSqliteDb())
        return;

    sqliteQry.exec("pragma foreign_keys = on");

    importTable("users", "select id, firstname, lastname, birthdate, address, phone, iif(man = true, 'male', 'female'), description, '', registerdate from users",
                QStringList() << "id" << "firstname" << "lastname" << "birth_date" << "address" << "phone" << "gender" << "description");

    insertLog("permissions", "insert", "1");
    insertLog("library", "insert", "1");

    importTable("scores", "select id, registerdate, 1, '', '' from users",
                QStringList() << "user_id" << "participated_at" << "confirm");

    importMatches();

    importImages();

    importTable("supports", "select id, designerid, maxscore, iif(maxscore = 0, 'disabled', iif(state = 0, 'active', iif(state = 1, 'disabled', iif(state = 2 , 'imported', NULL)))), '', '' from matches",
                QStringList() << "match_id" << "corrector_id" << "score" << "current_state");

    importTable("questions", "select matchid, question, answer, '', '' from questions",
                QStringList() << "match_id" << "question" << "answer");

    importTable("answers", "select userid, matchid, iif(deliverdate is null, '1300/01/01', deliverdate) as ddate, iif(deliverdate is null, '1300/01/01', receivedate) as rdate, iif(deliverdate is null, '1300/01/01', scoredate) as sdate, iif(scoredate is null, null, transactions.score/matches.maxscore) as rate, operatorid, iif(deliverdate is null, '1300/01/01', deliverdate) from transactions inner join matches on transactions.matchid = matches.id",
                QStringList() << "user_id" << "match_id" << "delivered_at" << "received_at" << "corrected_at" << "rate");

    importTable("payments", "select userid, score, scoredate, operatorid, scoredate from payments",
                QStringList() << "user_id" << "payment" << "payed_at");

    importTable("open_scores", "select userid, 0, title, score, operatorid, scoredate from freescores",
                QStringList() << "user_id" << "category_id" << "title" << "score");

    sqliteQry.exec("pragma foreign_keys = off");

    qDebug() << "match import finished";
}

void importBranches()
{
    importTable("roots", "select sarname, iif(objecttype = 2, 1, 0), '', '' from objectsgroping where gropingcode > 100 and gropingcode < 350 group by sarname, iif(objecttype = 2, 1, 0) order by first(gropingcode)",
                QStringList() << "title" << "type_id");

    importTable("branches", "select gropingcode, 1, zirname, gropingcode, '', '' from objectsgroping where gropingcode > 100 and gropingcode < 350",
                QStringList() << "id" << "root_id" << "title" << "label");

    // fix root_id of branches
    QSqlQuery qry(sqliteDb);
    accessQry.exec("select gropingcode, sarname, iif(objecttype = 2, 1, 0) from objectsgroping");
    while (accessQry.next())
    {
        QString id = accessQry.value(0).toString();
        sqliteQry.exec("select id from roots where title = '"+ refineText(accessQry.value(1).toString()) +"' and type_id = "+ accessQry.value(2).toString());
        if (sqliteQry.next())
        {
            qry.exec("update branches set root_id = "+ sqliteQry.value(0).toString() +" where id = "+ id);
            insertLog("branches", "update", id);
        }
    }
}

void importObjects()
{
    sqliteQry.prepare(getInsertQuery("objects", QStringList() << "author_id" << "publication_id" << "type_id" << "title" << "branch_id" << "label" << "cnt"));

    sqliteDb.transaction();
    accessQry.exec("select gropingno, `object id`, `object name`, `object writer`, `object publication`, `object count`, objecttype, `date of type` from objects inner join objectsgroping on objects.gropingno = objectsgroping.gropingcode");
    while (accessQry.next())
    {
        sqliteQry.bindValue(":author_id", insertTitleEntry("authors", accessQry.value(3).toString()));
        sqliteQry.bindValue(":publication_id", insertTitleEntry("publications", accessQry.value(4).toString()));

        if (accessQry.value(6).toInt() == 0)
            sqliteQry.bindValue(":type_id", "0");
        else if (accessQry.value(6).toInt() == 2)
            sqliteQry.bindValue(":type_id", "1");

        sqliteQry.bindValue(":title", refineText(accessQry.value(2).toString()));
        sqliteQry.bindValue(":branch_id", accessQry.value(0));
        sqliteQry.bindValue(":label", accessQry.value(1));
        sqliteQry.bindValue(":cnt", accessQry.value(5));
        if (! sqliteQry.exec())
            qDebug() << "object " << sqliteQry.lastError() << accessQry.value(1).toString();
        else
            insertLog("objects", "insert", sqliteQry.lastInsertId());
    }

    // fix matches table link
    if (! sqliteQry.exec("create temporary table mb(id, designer_id, title, ageclass, category_id, content)")) qDebug() << sqliteQry.lastError();
    if (! sqliteQry.exec("insert into mb select id, designer_id, title, ageclass, category_id, content from matches")) qDebug() << sqliteQry.lastError();
    if (! sqliteQry.exec("drop table matches")) qDebug() << sqliteQry.lastError();
    if (! sqliteQry.exec("create table matches (id integer not null primary key autoincrement, designer_id integer null default null references users(id) on update cascade, title varchar(255) not null, ageclass tinyint(4) null default null, object_id integer null default null references objects(id) on update cascade, category_id tinyint(4) null default null references categories(id) on update cascade, content text null default null)")) qDebug() << sqliteQry.lastError();
    if (! sqliteQry.exec("insert into matches select id, designer_id, title, ageclass, null, category_id, content from mb")) qDebug() << sqliteQry.lastError();
    if (! sqliteQry.exec("drop table mb")) qDebug() << sqliteQry.lastError();

    if (! sqliteQry.exec("update matches set object_id = (select id from objects where trim(objects.title) = trim(matches.title)) where category_id is null")) qDebug() << sqliteQry.lastError();

    sqliteDb.commit();
    qDebug() << "+ " << QString("objects");
}

void importBorrows()
{
    importTable("borrows", "select userid, objectid, date_raft as rdate, date_bargasht as bdate, login_of_raft, date_raft as cdate from (cash inner join objects on cash.objectid = objects.[object id]) inner join users on cash.userid = users.id",
                QStringList() << "user_id" << "object_id" << "delivered_at" << "received_at");

    if ( ! sqliteQry.exec("UPDATE borrows SET object_id = (SELECT id FROM objects WHERE objects.label = borrows.object_id)"))
        qDebug() << "object " << sqliteQry.lastError();
}

void importTransactions()
{
    // import payments
    sqliteQry.exec("insert into transactions select id, user_id, -1*payment, payed_at, 'pay' from payments");
    sqliteQry.exec("drop table payments");

    // import scores
    if (! sqliteQry.exec(MMatches::getScoreSql("insert")))
        qDebug() << "scores " << sqliteQry.lastError();
    sqliteQry.exec("drop table scores");

    // todo import roozname

    qDebug() << "+ " << QString("transactions");
}

void importLibraryDb(QString accessFilename)
{
    qDebug() << "library import started";

    sqliteDb = Connector::connectDb();
    accessDb = connectAccess(accessFilename);

    sqliteQry = QSqlQuery();
    accessQry = QSqlQuery(accessDb);

    // backup user passwords
    sqliteQry.exec("select id, upassword from users where upassword is not null");
    QStringList passQry;
    while(sqliteQry.next())
        passQry << "update users set upassword = '"+ sqliteQry.value(1).toString() +"' where id = "+ sqliteQry.value(0).toString();

    // build database
    sqliteQry.exec("drop table if exists users;");
    sqliteQry.exec("drop table if exists objects;");
    sqliteQry.exec("drop table if exists resources;");
    sqliteQry.exec("drop trigger if exists rate_update");
    sqliteQry.exec("drop trigger if exists rate_insert");

    if (! buildSqliteDb(true))
        return;

    importTable("users", "select id, name, family, t_t as tdate, adress +' - '+ str(block) +' - '+ str(`home no`), phon, iif(`is men` = true, 'male', 'female'), memo, id, usertypedescription, '', `reg date` from users left join usertype on users.ozviat = usertype.usertypename",
                QStringList() << "id" << "firstname" << "lastname" << "birth_date" << "address" << "phone" << "gender" << "description" << "label" << "account");

    // restore user passwords
    foreach(QString pq, passQry)
        sqliteQry.exec(pq);

    sqliteQry.exec("pragma foreign_keys = on");

    importTransactions();

    importBranches();

    sqliteQry.exec("pragma foreign_keys = off");

    importObjects();

    importBorrows();

    sqliteQry.exec("update library set version = '0.9.0'");

    qDebug() << "library import finished";
}

#endif // ACCESSTOSQLITE_H
