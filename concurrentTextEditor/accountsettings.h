#ifndef ACCOUNTSETTINGS_H
#define ACCOUNTSETTINGS_H

#include <QDialog>
#include <QInputDialog>
#include <QRegularExpression>
#include <QFileDialog>
#include <QMessageBox>

#include "workerclient.h"

namespace Ui {
class accountSettings;
}

class accountSettings : public QDialog
{
    Q_OBJECT

public:
    explicit accountSettings(QWidget *parent = nullptr, WorkerClient *worker = nullptr);
    ~accountSettings();

private slots:
    void on_pushButton_U_clicked();

    void on_pushButton_EA_clicked();

    void on_pushButton_PP_clicked();

private:
    Ui::accountSettings *ui;

    // Icons path
    //QString _defaultIconPath = "/home/albo/Documents/repos/master/concurrentTextEditor/concurrentTextEditorServer/Icons/";
    QString _defaultIconPath = "C:/Users/giorg/Documents/GitHub/concurrentTextEditor/concurrentTextEditorServer/Icons/";
    //QString _defaultIcon = "C:/Users/silvi/Google Drive/Politecnico/Magistrale/ProgettoDefinitivo/concurrentTextEditor/concurrentTextEditorServer/Icons/";

    QString _defaultIcon=  _defaultIconPath+ "male_icon.png";
};

#endif // ACCOUNTSETTINGS_H