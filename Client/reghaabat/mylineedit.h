#ifndef MyLineEdit_H
#define MyLineEdit_H

#include <QLineEdit>
#include <QTreeWidget>
#include <QSqlQuery>

class MyLineEdit;

class MyCompleter : public QObject
{
    Q_OBJECT

public:
    MyCompleter(MyLineEdit *parent = 0);
    ~MyCompleter();
    bool eventFilter(QObject *obj, QEvent *ev);
    void showCompletion(const QStringList &choices, const QStringList &hits);

public slots:
    void doneCompletion();
    void updateSuggestions();

    void setQuery(QString q)
    {
        query = q;
    }

private:
    QString query; // cid, clabel, ctitle

    MyLineEdit *editor;
    QTreeWidget *popup;
    QSqlQuery *qry;
};


class MyLineEdit: public QLineEdit
{
    Q_OBJECT

public:
    MyLineEdit(QString q, QWidget *parent = 0);

    void setValue(QString val);

    QString value()
    {
        return valueId;
    }

    void setQuery(QString q)
    {
        completer->setQuery(q);
    }

signals:
    void select();
    void cancel();

private:
    QString valueId;
    MyCompleter *completer;
};


#endif // MyLineEdit_H
