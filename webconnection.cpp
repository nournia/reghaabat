#include "webconnection.h"
#include "ui_webconnection.h"

#include <helper.h>

WebConnection::WebConnection(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WebConnection)
{
    ui->setupUi(this);

    syncer = new Syncer(this);
}

WebConnection::~WebConnection()
{
    delete ui;
}

void WebConnection::on_bSync_clicked()
{
    syncer->sync();
}