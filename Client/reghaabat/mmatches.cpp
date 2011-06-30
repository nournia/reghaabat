
#include <mmatches.h>
#include <musers.h>


bool MMatches::get(QString matchId, StrMap& match, QList<StrPair>& questions)
{
    QSqlQuery qry;
    qry.exec("select matches.title, matches.ageclass, matches.category_id, matches.content, resources.kind, authors.title as author, publications.title as publication, supports.current_state, supports.score, supports.corrector_id, users.firstname || ' ' || users.lastname as corrector "
             "from matches left join resources on matches.resource_id = resources.id left join authors on resources.author_id = authors.id left join publications on resources.publication_id = publications.id left join supports on matches.id = supports.match_id left join users on supports.corrector_id = users.id where matches.id = " + matchId);
    if (! qry.next()) return false;

    match = getRecord(qry);
    match["content"] = match["content"].toString().replace("src=\"", QString("src=\"%1/").arg(filesUrl()));

    qry.exec("select question, answer from questions where match_id = "+ matchId);
    while (qry.next())
        questions.append(qMakePair(qry.value(0).toString(), qry.value(1).toString()));
    return true;
}

QString MMatches::set(QString matchId, StrMap data, QList<StrPair> questions)
{
    QSqlQuery qry;

    // validation

    // basic
    if (data["title"].toString().isEmpty())
        return QObject::tr("Title is required.");
    if (data["corrector_id"].toString().isEmpty())
        return QObject::tr("Corrector is required.");
    if (data["score"].toInt() <= 0)
        return QObject::tr("Score is invalid.");

    // used title
    data["title"] = refineText(data["title"].toString().trimmed());
    qry.exec(QString("select id from matches where title = '%1'").arg(data["title"].toString()));
    if (qry.next())
        if (qry.value(0).toString() != matchId)
            return QObject::tr("There is another match with this title.");

    data["author"] = data["author"].toString().trimmed();
    data["publication"] = data["publication"].toString().trimmed();


    // store

    QSqlDatabase db = Connector::connectDb();
    db.transaction();

    StrMap match;
    match["resource_id"] = "";

    if (data["category_id"].toString() == "")
    {
        // new author & publication
        StrMap ndata;
        if (! data["author"].toString().isEmpty() && data["author"].toInt() == 0)
        {
            ndata["title"] = data["author"].toString();
            qry.exec(getReplaceQuery("authors", ndata, ""));
            data["author"] = qry.lastInsertId();
        }
        if (! data["publication"].toString().isEmpty() && data["publication"].toInt() == 0)
        {
            ndata["title"] = data["publication"].toString();
            qry.exec(getReplaceQuery("publications", ndata, ""));
            data["publication"] = qry.lastInsertId();
        }

        StrMap resource;
        resource["author_id"] = data["author"];
        resource["publication_id"] = data["publication"];
        resource["kind"] = data["kind"];
        resource["title"] = data["title"];
        resource["ageclass"] = data["ageclass"];

        if (matchId != "")
        {
            qry.exec("select resource_id from matches where id = "+ matchId);
            if (qry.next())
                match["resource_id"] = qry.value(0).toString();
        }

        // resources table
        if (! qry.exec(getReplaceQuery("resources", resource, match["resource_id"].toString())))
        {
            db.rollback();
            return qry.lastError().text();
        }
        if (match["resource_id"].toString() == "")
            match["resource_id"] = qry.lastInsertId().toString();

    } else
    {
        match["category_id"] = data["category_id"];
        match["content"] = data["content"].toString().replace(QString("src=\"%1/").arg(filesUrl()), "src=\"");
    }

    match["designer_id"] = data["corrector_id"];
    match["title"] = data["title"];
    match["ageclass"] = data["ageclass"];

    // matches table
    if (! qry.exec(getReplaceQuery("matches", match, matchId)))
    {
        db.rollback();
        return qry.lastError().text();
    }
    if (matchId == "")
       matchId = qry.lastInsertId().toString();

    QString supportId;
    StrMap support;
    support["match_id"] = matchId;
    support["corrector_id"] = data["corrector_id"];
    support["current_state"] = data["current_state"];
    support["score"] = data["score"];

    // supports table
    qry.exec("select id from supports where match_id = "+ matchId);
    if (qry.next())
        supportId = qry.value(0).toString();
    if (! qry.exec(getReplaceQuery("supports", support, supportId)))
    {
        db.rollback();
        return qry.lastError().text();
    }

    // questions table
    if (data["category_id"].toString() == "")
    {
        qry.exec("delete from questions where match_id = "+ matchId);
        for (int i = 0; i < questions.size(); i++)
        {
            StrMap question;
            question["match_id"] = matchId;
            question["question"] = questions.at(i).first;
            question["answer"] = questions.at(i).second;

            if (! qry.exec(getReplaceQuery("questions", question, "")))
            {
                db.rollback();
                return qry.lastError().text();
            }
        }
    }

    db.commit();

    return "";
}



QString MMatches::deliver(QString userId, QString matchId)
{
    QSqlQuery qry;

    // validations

    // delivered match
    qry.exec(QString("select id from answers where user_id = %1 and match_id = %2").arg(userId).arg(matchId));
    if (qry.next())
        return QObject::tr("You received this match before");

    // match state
    qry.exec(QString("select current_state from supports where match_id = %1").arg(matchId));
    if (! qry.next())
        return QObject::tr("This match is not supported.");

    if (qry.value(0).toString() != "active")
        return QObject::tr("This match is not in active state.");

    // user ageclass
    qry.exec(QString("select abs(ageclass - %1) from matches where id = %2").arg(MUsers::getAgeClass(userId)).arg(matchId));
    if (! qry.next())
        return QObject::tr("Error in user or match");

    if (qry.value(0).toInt() > 1)
        return QObject::tr("This matches' ageclass deffers more than one level with yours.");


    // deliver

    qry.prepare("insert into answers (user_id, match_id) values (?, ?)");
    qry.addBindValue(userId);
    qry.addBindValue(matchId);
    qry.exec();

    return "";
}

void MMatches::receive(QString userId, QString matchId)
{
    QSqlQuery qry;
    qry.prepare("update answers set received_at = ? where user_id = ? and match_id = ?");
    qry.addBindValue(QDateTime::currentDateTime());
    qry.addBindValue(userId);
    qry.addBindValue(matchId);
    qry.exec();
}

QString MMatches::correct(QString answerId, QString Score)
{
    QSqlQuery qry;

    qry.prepare("select score from supports inner join answers on supports.match_id = answers.match_id where answers.id = ?");
    qry.addBindValue(answerId);
    qry.exec();
    if (! qry.next())
        return QObject::tr("Invalid record selected.");

    float rate = Score.toFloat() / qry.value(0).toInt();

    if (rate < -1 || rate > 2)
        return QObject::tr("Score must be less than 2 * max score.");

    qry.prepare("update answers set rate = ?, corrected_at = current_timestamp where id = ?");
    qry.addBindValue(rate);
    qry.addBindValue(answerId);
    qry.exec();

    return "";
}
