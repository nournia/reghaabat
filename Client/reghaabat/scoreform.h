#ifndef SCOREFORM_H
#define SCOREFORM_H

#include <QSqlQueryModel>
#include <mmatches.h>

class SetScoreModel : public QSqlQueryModel
{
    Q_OBJECT

public:
    QString sql;
    SetScoreModel(QObject *parent = 0) : QSqlQueryModel(parent) { refresh(); }

    Qt::ItemFlags flags(const QModelIndex &index) const
    {
        Qt::ItemFlags flags = QSqlQueryModel::flags(index);
        if (index.column() == 3)
            flags |= Qt::ItemIsEditable;
        return flags;
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role)
    {
//        if (!(index.column() == 2 || index.column() == 3))
//            return false;

//        QString id = data(QSqlQueryModel::index(index.row(), 0)).toString();

//        QString msg = "";
//        if (index.column() == 2)
//            msg = MUsers::setPermission(id , value.toString());
//        else if (index.column() == 3)
//            msg = MUsers::setPassword(id, value.toString());

//        if (! msg.isEmpty())
//            QMessageBox::warning(0, QObject::tr("Reghaabat"), msg);

//        clear();

//        refresh();
//        return msg.isEmpty();
    }

    QString corrector;
    void setCorrector(QString value)
    {
        corrector = value;
        refresh();
    }

    void refresh()
    {
        QString tmp = "-1";
        if (! corrector.isEmpty())
            tmp = corrector;
        setQuery(QString("select answers.id, firstname||' '||lastname as name, matches.title, answers.rate from answers inner join supports on answers.match_id = supports.match_id inner join users on answers.user_id = users.id inner join matches on answers.match_id = matches.id where received_at is not null and corrected_at is null and corrector_id = %1 order by answers.match_id").arg(tmp));
        setHeaderData(1, Qt::Horizontal, tr("Name"));
        setHeaderData(2, Qt::Horizontal, tr("Title"));
        setHeaderData(3, Qt::Horizontal, tr("Rate"));

    }
};



#include <mylineedit.h>

#include <QWidget>

namespace Ui {
    class ScoreForm;
}

class ScoreForm : public QWidget
{
    Q_OBJECT

public:
    explicit ScoreForm(QWidget *parent = 0);
    ~ScoreForm();

    MyLineEdit *eCorrector;

    SetScoreModel* model;

private:
    Ui::ScoreForm *ui;

private slots:
    void selectCorrector();
    void cancelCorrector();
};

#endif // SCOREFORM_H
