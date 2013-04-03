#include "objectmanagement.h"
#include "ui_objectmanagement.h"

#include <mainwindow.h>

ObjectManagement::ObjectManagement(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ObjectManagement)
{
    ui->setupUi(this);

    connect(ui->bNewObject, SIGNAL(clicked()), parent, SLOT(newObject()));
    connect(ui->bEditObject, SIGNAL(clicked()), parent, SLOT(editObject()));

    ui->gList->setVisible(false);
    ui->gLabels->setVisible(false);
}

ObjectManagement::~ObjectManagement()
{
    delete ui;
}

void ObjectManagement::on_bPreviewList_clicked()
{
    ViewerForm* viewer = new ViewerForm((MainWindow*) parent());
    viewer->showObjectList(ui->eFromList->text(), ui->eToList->text());
    viewer->exec();
}

void ObjectManagement::on_bPreviewLabels_clicked()
{
    ViewerForm* viewer = new ViewerForm((MainWindow*) parent());
    viewer->showObjectLabels(ui->eFromLabels->text(), ui->eToLabels->text(), ui->cOnlyMatchObjects->isChecked());
    viewer->exec();
}

void ObjectManagement::on_bPrintLabels_clicked()
{
    ui->gLabels->setVisible(!ui->gLabels->isVisible());
    ui->gList->setVisible(false);

    QSqlQuery qry;
    qry.exec("select min(label), max(label) from belongs where label > '000-000'");
    if (qry.next())
    {
        ui->eFromLabels->setText(qry.value(0).toString());
        ui->eToLabels->setText(qry.value(1).toString());
    }
}

void ObjectManagement::on_bPrintList_clicked()
{
    ui->gList->setVisible(!ui->gList->isVisible());
    ui->gLabels->setVisible(false);

    QSqlQuery qry;
    qry.exec("select min(label), max(label) from objects where label > '000-000'");
    if (qry.next())
    {
        ui->eFromList->setText(qry.value(0).toString());
        ui->eToList->setText(qry.value(1).toString());
    }
}
