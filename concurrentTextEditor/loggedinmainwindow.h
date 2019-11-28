#ifndef LOGGEDINMAINWINDOW_H
#define LOGGEDINMAINWINDOW_H

#include <QMainWindow>
#include "workerclient.h"
#include "editor.h"

namespace Ui {
class loggedinmainwindow;
}

class loggedinmainwindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit loggedinmainwindow(QWidget *parent, WorkerClient* worker);
    ~loggedinmainwindow();
    void requestFileList();

public slots:
    void showFiles(QStringList list);

private slots:
//    void on_pushButtonSettings_clicked();
    void on_pushButtonLogout_clicked();
    void on_pushButtonOpenFile_2_clicked();

public:



private:
    Ui::loggedinmainwindow *ui;
    Editor *_e;
    WorkerClient* _workerClient;
};

#endif // LOGGEDINMAINWINDOW_H