#include "formoperator.h"
#include "ui_formoperator.h"

#include <QFormLayout>
#include <QLabel>
#include <QPushButton>

#include <helper.h>
#include <matchrow.h>

FormOperator::FormOperator(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormOperator)
{
    ui->setupUi(this);

    // add matchname edit
    eMatchname = new MyLineEdit("select id, title as ctitle from matches", this);
    eMatchname->setObjectName("eMatchname");
    ui->horizontalLayout->addWidget(eMatchname);
    QWidget::setTabOrder(eMatchname, ui->bDeliver);
    QWidget::setTabOrder(ui->bDeliver, ui->cPrint);
    QWidget::setTabOrder(ui->cPrint, ui->bPreview);

    connect(eMatchname, SIGNAL(returnPressed()), this, SLOT(selectMatch()));
}

FormOperator::~FormOperator()
{
    delete ui;
}

void FormOperator::selectMatch()
{
    ui->newMatchButtons->setEnabled(eMatchname->value() != "");

    if (eMatchname->value() != "")
        ui->bDeliver->setFocus();
}

void FormOperator::select(QString uid)
{
    userId = uid;

    ui->newMatchButtons->setEnabled(false);
    eMatchname->setText("");
    eMatchname->setFocus();

    // clean gMatches
    QLayoutItem *child;
    if (ui->gMatches->layout())
    while ((child = ui->gMatches->layout()->takeAt(0)) != 0)
         delete child->widget();

    QSqlQuery qry;
    qry.exec(QString("select match_id, matches.title from answers inner join matches on answers.match_id = matches.id where user_id = %1 and received_at is null;").arg(userId));

    for (int i = 1; qry.next(); i++)
    {
        MatchRow* row = new MatchRow(userId, qry.value(0).toString(), qry.value(1).toString(), ui->gMatches);
        ui->gMatches->layout()->addWidget(row);
    }

    // space filler
    ui->gMatches->layout()->addWidget(new QWidget);
}

void FormOperator::on_bDeliver_clicked()
{
    if (eMatchname->value() != "")
    {
        MMatches::deliver(userId, eMatchname->value());
        select(userId);
    }
}