#ifndef QUESTIONMODULE_H
#define QUESTIONMODULE_H

#include <QWidget>

namespace Ui {
    class QuestionModule;
}

class QuestionModule : public QWidget
{
    Q_OBJECT

public:
    explicit QuestionModule(QString q, QString a, QWidget *parent = 0);
    ~QuestionModule();

    void refresh(bool collapse = false);

    QString question, answer;

    void select();

private:
    Ui::QuestionModule *ui;

private slots:
    void on_bAnswer_clicked();
};

#endif // QUESTIONMODULE_H
