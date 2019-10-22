#ifndef HOMELOGGEDIN_H
#define HOMELOGGEDIN_H

#include <QDialog>
#include "accountsettings.h"

namespace Ui {
class homeLoggedIn;
}

class homeLoggedIn : public QDialog
{
    Q_OBJECT

public:
    explicit homeLoggedIn(QWidget *parent = nullptr, QString usrname="usr_default_value");
    ~homeLoggedIn();

private slots:
    void on_pushButtonSettings_clicked();

    void on_pushButtonLogout_clicked();

private:
    Ui::homeLoggedIn *ui;
    accountSettings *ac_st;
};

#endif // HOMELOGGEDIN_H
