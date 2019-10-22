#include "dialogsignup.h"
#include "ui_dialogsignup.h"

dialogsignup::dialogsignup(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dialogsignup)
{
    ui->setupUi(this);
}

dialogsignup::~dialogsignup()
{
    delete ui;
}

void dialogsignup::on_pushButton_clicked()
{
    QString usr = ui->lineEdit_Usr->text();
    QString pwd1 = ui->lineEdit_PwdIns->text();
    QString pwd2 = ui->lineEdit_PwdConf->text();


    if(pwd1 != pwd2){
        QMessageBox pwd_not_eq;
        pwd_not_eq.setText("Passwords do not match.");
        pwd_not_eq.exec();
    }

    QJsonObject signup;

    signup["type"] = "signup";
    signup["username"] = usr;
    signup["password"] = pwd1;
}