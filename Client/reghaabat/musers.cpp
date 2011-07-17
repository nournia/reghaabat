
#include <musers.h>

bool MUsers::get(QString userId, StrMap& user)
{
    QSqlQuery qry;
    qry.exec("select * from users where id = "+ userId);
    if (! qry.next()) return false;

    user = getRecord(qry);
    return true;
}

QString MUsers::getAgeClassQuery(QString userId)
{
    QSqlQuery qry;
    qry.exec("select id, beginage, endage from ageclasses order by id");

    QString field = "current_date - birth_date";
    QString caseSt = "case";
    while (qry.next())
        caseSt += QString(" when %1 between %2 and %3 then %4").arg(field).arg(qry.value(1).toString()).arg(qry.value(2).toString()).arg(qry.value(0).toString());
    caseSt += " else -10 end";

    return QString("select %1 as ageclass from users where id = %2").arg(caseSt).arg(userId);
}

QString MUsers::getAgeClass(QString userId)
{
    QSqlQuery qry;
    qry.exec(getAgeClassQuery(userId));
    qry.next();
    return qry.value(0).toString();
}

QString MUsers::set(QString userId, StrMap user, QString importedId)
{
    QSqlQuery qry;

    // validation

    // basic
    if (user["firstname"].toString().isEmpty() || user["lastname"].toString().isEmpty())
        return QObject::tr("User name is required.");
    if (! user["national_id"].toString().isEmpty() && user["national_id"].toInt() == 0)
        return QObject::tr("National id is not valid.");
    if (! user["birth_date"].toDate().isValid())
        return QObject::tr("Birth date is not valid.");

    /*
    // used name
    user["firstname"] = refineText(user["firstname"].toString().trimmed());
    user["lastname"] = refineText(user["lastname"].toString().trimmed());
    qry.exec(QString("select id, firstname ||' '|| lastname as name from users where name = '%1'").arg(user["firstname"].toString() +" "+ user["lastname"].toString()));
    if (qry.next())
        if (qry.value(0).toString() != userId)
            return QObject::tr("There is another user with this name.");
    */

    // imported
    if (! importedId.isEmpty())
    {
        qry.exec(QString("select * from users where id = %1").arg(importedId));
        if (qry.next())
            return QObject::tr("You imported this user before.");
    }

    // used national id
    qry.exec("select id, firstname ||' '|| lastname from users where national_id = "+ user["national_id"].toString());
    if (qry.next())
        if (qry.value(0).toString() != userId)
            return QObject::tr("%1 has exact same national id.").arg(qry.value(1).toString());


    // store
    if (! qry.exec(getReplaceQuery("users", user, userId)))
        return qry.lastError().text();

    QString tmpUID = qry.lastInsertId().toString();

    if (! importedId.isEmpty())
        qry.exec(QString("update users set id = %1 where id = %2").arg(importedId).arg(tmpUID));

    if (userId.isEmpty())
    {
        userId = tmpUID;
        insertLog("users", "insert", userId);

        if (qry.exec(QString("insert into scores (user_id) values (%1)").arg(userId)))
            insertLog("scores", "insert", userId);
    }
    else
        insertLog("users", "update", userId);

    return "";
}

bool MUsers::login(QString userId, QString password, StrMap& user)
{
    QSqlQuery qry;
    QString upassword = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha1).toHex();
    QString query = QString("select users.id, firstname||' '||lastname as name, gender, permission from users inner join permissions on users.id = permissions.user_id where users.id = %1 and upassword = '%2'").arg(userId).arg(upassword);
    qry.exec(query);

    if (qry.next())
    {
        user = getRecord(qry);
        return true;
    }
    else
        return false;
}

QString MUsers::changePassword(QString userId, QString oldPass, QString newPass, QString rePass)
{
    QSqlQuery qry;
    qry.exec("select upassword from users where id = "+ userId);
    if (! qry.next()) return false;

    if (qry.value(0).toString() != QCryptographicHash::hash(oldPass.toUtf8(), QCryptographicHash::Sha1).toHex())
        return QObject::tr("Invalid Old Password.");

    if (newPass != rePass)
        return QObject::tr("New password must exact matches with retyped one.");

    QString msg = setPassword(userId, newPass);
    if (! msg.isEmpty())
        return msg;

    return QObject::tr("Password Changed.");
}

bool validPassword(QString pass)
{
    if (pass.length() < 6)
        return false;
    else if (pass.toInt() != 0)
        return false;
    return true;
}

QString MUsers::setPassword(QString userId, QString newPass)
{
    QSqlQuery qry;

    if (newPass.trimmed().isEmpty())
        return QObject::tr("Password is required.");

//        if (! validPassword(newPass))
//            return QObject::tr("Password phrase length must be greater than 6 characters and mustn't be a pure number.");

    QString password = QCryptographicHash::hash(newPass.toUtf8(), QCryptographicHash::Sha1).toHex();
    if (! qry.exec(QString("update users set upassword = '%1' where id = %2").arg(password).arg(userId)))
        return qry.lastError().text();

    insertLog("users", "update", userId);

    return "";
}

QString MUsers::setPermission(QString userId, QString permission)
{
    QSqlQuery qry;

    qry.exec("select id, permission from permissions where user_id = "+ userId);
    if (qry.next())
    {
        // validation
        QString id = qry.value(0).toString();

        if (qry.value(1).toString() == "master")
        {
            qry.exec("select user_id from permissions where permission = 'master' and user_id != "+ userId);
            if (! qry.next())
                return QObject::tr("You must have at least one master user.");
        }


        // store

        if (! qry.exec(QString("update permissions set permission = '%1' where id = %2").arg(permission).arg(id)))
            return qry.lastError().text();

        insertLog("permissions", "update", id);
    }
    else
    {
        if (! qry.exec(QString("insert into permissions (user_id, permission) values (%1, 'user')").arg(userId)))
            return qry.lastError().text();

        insertLog("permissions", "insert", qry.lastInsertId());
    }

    return "";
}

QString MUsers::pay(QString userId, int score)
{
    QSqlQuery qry;

    qry.exec(QString("select score - ifnull((select sum(payment) as payed_score from payments ts where user_id = %1 and payed_at > (select started_at from library)), 0) as residue from scores where user_id = %1").arg(userId));
    if (!qry.next())
        return qry.lastError().text();

    if (qry.value(0).toInt() - score < 0)
        return QObject::tr("You have not sufficent score.");

    if (!qry.exec(QString("insert into payments (user_id, payment, payed_at) values (%1, %2, '%3')").arg(userId).arg(score).arg(formatDateTime(QDateTime::currentDateTime()))))
        return qry.lastError().text();

    insertLog("payments", "insert", qry.lastInsertId());

    return "";
}
