#include "userform.h"
#include "ui_userform.h"

#include <QMessageBox>

#include <jalali.h>
#include <connector.h>
#include <uihelper.h>

UserForm::UserForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UserForm)
{
    ui->setupUi(this);

    // add username edit
    connect(ui->eUser, SIGNAL(select()), this, SLOT(selectUser()));
    connect(ui->eUser, SIGNAL(cancel()), this, SLOT(cancelUser()));
    fillComboBox(ui->cAccount, MUsers::accounts());

    ui->eFirstname->setFocus();
}

UserForm::~UserForm()
{
    delete ui;
}

void UserForm::editMode(bool edit)
{
    if (ui->eUser->text().isEmpty())
        cancelUser();
    else
        ui->eUser->setText("");

    ui->gUser->setVisible(edit);
    ui->gData->setEnabled(! edit);
    ui->buttonBox->setEnabled(! edit);

    if (edit)
        ui->eUser->setFocus();
    else
        ui->eLabel->setText(MUsers::getNewLabel());
    checkReadOnly();
}

void UserForm::checkReadOnly()
{
    bool original = true;
    if (! ui->eUser->value().isEmpty())
        original = ui->eUser->value().startsWith(App::instance()->libraryId);

    ui->eFirstname->setEnabled(original);
    ui->eLastname->setEnabled(original);
    ui->gGender->setEnabled(original);
    ui->eBirthDate->setEnabled(original);
    ui->eNationalId->setEnabled(original);
    ui->ePhone->setEnabled(original);
    ui->eAddress->setEnabled(original);
    ui->eDescription->setEnabled(original);
}

void UserForm::on_buttonBox_rejected()
{
    emit closeForm();
}

void UserForm::on_buttonBox_accepted()
{
    StrMap user;
    user["firstname"] = ui->eFirstname->text();
    user["lastname"] = ui->eLastname->text();
    user["national_id"] = ui->eNationalId->text();
    user["address"] = ui->eAddress->text();
    user["phone"] = ui->ePhone->text();
    user["description"] = ui->eDescription->text();
    user["birth_date"] = toGregorian(ui->eBirthDate->text());
    user["gender"] = ui->rMale->isChecked() ? "male" : "female";
    user["account_id"] = ui->cAccount->itemData(ui->cAccount->currentIndex());
    user["label"] = ui->eLabel->text();

    QString msg = MUsers::set(ui->eUser->value(), user);

    // there isn't any error
    if (msg == "")
    {
        emit closeForm();

        if (ui->eUser->value().isEmpty())
        {
            MUsers::get(user["id"].toString(), user);
            msg = tr("%1 registered with %2 label.").arg(user["firstname"].toString() +" "+ user["lastname"].toString()).arg(user["label"].toString());
            QMessageBox::information(this, QObject::tr("Ketabkhaane"), msg);
        }
    }
    else
        QMessageBox::critical(this, QObject::tr("Ketabkhaane"), msg);
}

void UserForm::selectUser()
{
    if (! ui->eUser->value().isEmpty())
    {
        StrMap user;
        MUsers::get(ui->eUser->value(), user);
        ui->eFirstname->setText(user["firstname"].toString());
        ui->eLastname->setText(user["lastname"].toString());
        ui->eNationalId->setText(user["national_id"].toString());
        ui->eAddress->setText(user["address"].toString());
        ui->ePhone->setText(user["phone"].toString());
        ui->eDescription->setText(user["description"].toString());
        ui->eLabel->setText(user["label"].toString());
        ui->cAccount->setCurrentIndex(ui->cAccount->findData(user["account_id"]));

        ui->eBirthDate->setText(toJalali(user["birth_date"].toDate()));

        if (user["gender"] == "male")
            ui->rMale->setChecked(true);
        else
            ui->rFemale->setChecked(true);

        ui->gData->setEnabled(true);
        ui->buttonBox->setEnabled(true);
        ui->eFirstname->setFocus();
    }
    checkReadOnly();
}

void UserForm::cancelUser()
{
    ui->eUser->setQuery(MUsers::getUsersQuery());

    ui->eLabel->setText("");
    ui->eFirstname->setText("");
    ui->eLastname->setText("");
    ui->eNationalId->setText("");
    ui->eAddress->setText("");
    ui->ePhone->setText("");
    ui->eDescription->setText("");
    ui->eBirthDate->setText("");
    ui->rMale->setChecked(true);
    ui->cAccount->setCurrentIndex(0);

    ui->gData->setEnabled(false);
    ui->buttonBox->setEnabled(false);

    ui->eUser->setFocus();
}
