#include "loggedinmainwindow.h"
#include "ui_loggedinmainwindow.h"

loggedinmainwindow::loggedinmainwindow(QWidget *parent, WorkerClient* worker) :
    QMainWindow(parent),
    ui(new Ui::loggedinmainwindow),
    _workerClient(worker)
{
    ui->setupUi(this);
    ui->statusbar->showMessage("Concurrent Text Editor, version 1.0.0");
    ui->pushButtonOpenFile_2->setEnabled(false);
    ui->PublicFileListTable->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->PrivatefileListTable->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->welcomeLabel->setText("Welcome, "+ _workerClient->getUser()); //used to show Username in home window

    QString icDir = QDir::currentPath().append("/IconsBar");
    ui->pushButtonLogout_2->setIcon(QIcon(QPixmap(icDir+"/logout.png")));
    connect(_workerClient, &WorkerClient::genericError, this, &loggedinmainwindow::errorDisplay);
    connect(_workerClient, &WorkerClient::ifFileOpenOk, this, &loggedinmainwindow::isFileOpenOkay);
    connect(_workerClient, &WorkerClient::newUsernameOk, this, &loggedinmainwindow::newUsernameOk);
    connect(ui->PublicFileListTable, &QTableView::customContextMenuRequested, this, &loggedinmainwindow::provideContextMenuPub);
    connect(ui->PrivatefileListTable, &QTableView::customContextMenuRequested, this, &loggedinmainwindow::provideContextMenuPri);
    _workerClient->getCurrentIconFromServer();
}

loggedinmainwindow::~loggedinmainwindow()
{
    delete ui;
}

void loggedinmainwindow::requestFileList(){
    _workerClient->getFileList("all");
}

void loggedinmainwindow::showFiles(QStringList filesList, QStringList createdList, QStringList ownerList, bool isPublic){

    QTableWidget* table = isPublic ? ui->PublicFileListTable : ui->PrivatefileListTable;

    // Empty list
    table->clear();

    // Repopulate list
    table->setRowCount(filesList.count());
    table->setColumnCount(2);

    QStringList headers;
    headers.push_back("Filename");
    headers.push_back("Created");
    table->setHorizontalHeaderLabels(headers);    

    int cnt = 0;

    foreach(auto file, filesList){
        auto fileItem = new QTableWidgetItem(filesList.at(cnt));
        auto creationItem = new QTableWidgetItem(createdList.at(cnt));

        // Disable selection for creation item
        creationItem->setFlags(Qt::ItemFlag::NoItemFlags);

        table->setItem(cnt, 0, fileItem);
        table->setItem(cnt, 1, creationItem);

        cnt++;
    }

}

void loggedinmainwindow::errorDisplay(QString str){
    QMessageBox::information(this, tr("Error"), str);
}

void loggedinmainwindow::on_pushButtonLogout_2_clicked()
{
    ui->statusbar->showMessage("Logging out...");
    this->_workerClient->disconnectFromServer();
    this->close();
    this->parentWidget()->show();
    this->deleteLater();
}

void loggedinmainwindow::on_pushButtonOpenFile_2_clicked()
{

    QString fileName;
    bool publicEmpty = ui->PublicFileListTable->selectedItems().isEmpty();
    bool privateEmpty = ui->PrivatefileListTable->selectedItems().isEmpty();

    if( publicEmpty && privateEmpty) {
        errorDisplay("Please select a file by clicking on it.");
        return;
    }    

    ui->statusbar->showMessage("Opening file...");

    if(publicEmpty){
        fileName = ui->PrivatefileListTable->selectedItems().first()->text();
        _e = new Editor(this, _workerClient, fileName, false, false);
        _e->show();
    }
    else if(privateEmpty){
        fileName = ui->PublicFileListTable->selectedItems().first()->text();
        _e = new Editor(this, _workerClient, fileName, true, false);
        _e->show();
    }

    // Clear selection
    ui->PublicFileListTable->clearSelection();
    ui->PrivatefileListTable->clearSelection();
    ui->statusbar->showMessage("File opened!");
}


void loggedinmainwindow::on_pushButtonUpdate_2_clicked()
{
    this->
    _workerClient->getFileList("all");
}

// New PUBLIC file button
void loggedinmainwindow::on_pushButtonNewFile_2_clicked()
{
   newFile(true);
}

// New PUBLIC file button
void loggedinmainwindow::on_pushButtonNewFile_3_clicked()
{
   newFile(false);
}

void loggedinmainwindow::newFile(bool isPublic){

    bool ok = false;
    QJsonObject filename_req;

    // get filename from input dialog
    QString fileName = QInputDialog::getText(this, "New File", "Please insert new filename: ", QLineEdit::Normal, QString("FileName"), &ok);

    if(!fileName.isEmpty() && ok) {
        ui->statusbar->showMessage("Creating a new file...");
        filename_req["type"] = messageType::newFile;
        filename_req["filename"] = fileName.append(".cte");
        filename_req["access"] = isPublic;
        _workerClient->newFileRequest(filename_req);

        _e = new Editor(this, _workerClient, fileName, isPublic, false);
        _e->show();
        ui->statusbar->showMessage("File opened!");
    }
    else if(ok){
        errorDisplay("Insert a name for the file");
    }

    return;
}


void loggedinmainwindow::on_PublicFileListTable_cellDoubleClicked(int row, int column)
{
    QString fileName = ui->PublicFileListTable->item(row, 0)->text();
    ui->statusbar->showMessage("Opening file...");

    // Detect if private or public
    _e = new Editor(this, _workerClient, fileName, true, false);
    //hide();
    _e->show();
    ui->statusbar->showMessage("File opened!");
}



void loggedinmainwindow::on_PrivatefileListTable_cellDoubleClicked(int row, int column)
{
    QString fileName = ui->PrivatefileListTable->item(row, 0)->text();

    ui->statusbar->showMessage("Opening file...");
    _e = new Editor(this, _workerClient, fileName, false, false);
    _e->show();
    ui->statusbar->showMessage("File opened!");
}

void loggedinmainwindow::on_pushButtonInvite_2_clicked()
{
    // Invite makes sense only if a selected file in PrivateFilesListTable is selected
    ui->statusbar->showMessage("Generating invite link...");
    if(ui->PrivatefileListTable->selectedItems().isEmpty()) {
        errorDisplay("Can't generate invite link. Please select a private file by clicking on it.");
        ui->statusbar->showMessage("Invite link generation failed");
        return;
    }

    QString fileName = ui->PrivatefileListTable->selectedItems().first()->text();

    QString link = generateInviteLink(fileName, _workerClient->getUser());

    // Change this into a window that displays the link and has a button "copy link to clipboard"
    // that copies the link to clipboard for the user and notifies it that the operation was successful
    QClipboard* clip = QGuiApplication::clipboard();
    clip->setText(link);

    QInputDialog::getText(this, "Sharing link", "Generated link for file: " + fileName + " and copied to clipboard.", QLineEdit::Normal, link);
    ui->statusbar->showMessage("Invite link generated");
}

QString loggedinmainwindow::generateInviteLink(QString fileName, QString username){

    QJsonObject inviteLink;

    QString link = QUuid::createUuid().toString() + "/" + fileName + "/" + username;
    inviteLink["type"] = messageType::invite;
    inviteLink["user"] = username;
    inviteLink["link"] = link;
    inviteLink["operation"] = EditType::insertion;

    _workerClient->saveLinkToServer(inviteLink);

    return link;
}

void loggedinmainwindow::isFileOpenOkay(const QJsonObject& qjo){
    QString fileName = qjo["fileName"].toString();       
    _e = new Editor(this, _workerClient, fileName, false, qjo["sharing"].toBool()); //perchè è privato
    //hide();
    _e->show();

}

void loggedinmainwindow::on_pushButtonDeleteFile_3_clicked()
{

    bool isPublic = false;
    QString fileName;

    if(ui->PublicFileListTable->selectedItems().isEmpty() && ui->PrivatefileListTable->selectedItems().isEmpty()) {
        errorDisplay("Please select a file by clicking on it.");
        return;
    }
    ui->statusbar->showMessage("Deleting file...");
    int ret = QMessageBox::warning(this, tr("Delete File"),
                                   tr("The file will be permanentely deleted, do you want to proceed?"),
                                   QMessageBox::Yes | QMessageBox::No);
    if (ret==QMessageBox::Yes)
    {
        if(!ui->PublicFileListTable->selectedItems().isEmpty()) {
            fileName = ui->PublicFileListTable->selectedItems().first()->text();
            isPublic = true;
            ui->statusbar->showMessage(fileName+" deleted");
        }
        else if (!ui->PrivatefileListTable->selectedItems().isEmpty()){
            fileName = ui->PrivatefileListTable->selectedItems().first()->text();
            isPublic = false;
            ui->statusbar->showMessage(fileName+" deleted");
        }
        _workerClient->deleteFile(fileName, isPublic);
        this->on_pushButtonUpdate_2_clicked();

    }
}

void loggedinmainwindow::on_pushButtonSettings_2_clicked()
{
    _workerClient->getCurrentIconFromServer();
    _ac = new accountSettings(this, _workerClient);
    _ac->show();
}

void loggedinmainwindow::on_pushButtonOpenSharedFile_3_clicked()
{
    bool ok;
    QString link = QInputDialog::getText(this, "Open Shared File", "Insert link for shared file", QLineEdit::Normal, "id/filename/username", &ok);
    if(ok && !link.isEmpty()) {
        _workerClient->getSharedFile(link);
    }

}

void loggedinmainwindow::on_PublicFileListTable_cellClicked(int row, int column)
{
    ui->PrivatefileListTable->clearSelection();

    ui->pushButtonOpenFile_2->setEnabled(true);

    return;
}

void loggedinmainwindow::on_PrivatefileListTable_cellClicked(int row, int column)
{
    ui->PublicFileListTable->clearSelection();

    ui->pushButtonOpenFile_2->setEnabled(true);

    return;
}

void loggedinmainwindow::newUsernameOk(){
    ui->welcomeLabel->setText("Welcome, "+ _workerClient->getUser());
}

void loggedinmainwindow::provideContextMenuPub(const QPoint &pos){
    ui->PrivatefileListTable->clearSelection();
    QMenu m;
    QAction *open;
    QAction *del = m.addAction("Delete");
    m.addSeparator();
    QAction *new_file = m.addAction("New Public File");
    m.addSeparator();

    if (!ui->PublicFileListTable->selectedItems().isEmpty()){
        open = m.addAction("Open");
    } else {
        ui->pushButtonOpenFile_2->setEnabled(false);
    }

    QAction *selected = m.exec(QCursor::pos());

    if (selected == open){
        QString filename = ui->PublicFileListTable->selectedItems().first()->text();
        _e = new Editor(this, _workerClient, filename, true, false);
        _e->show();
    }
    if(selected == new_file) {
        this->newFile(true);
    }
    if(selected == del) {
        this ->on_pushButtonDeleteFile_3_clicked();
    }
}

void loggedinmainwindow::provideContextMenuPri(const QPoint &pos){
    ui->PublicFileListTable->clearSelection();
    QMenu m;
    QAction *open;
    QAction *del = m.addAction("Delete");
    m.addSeparator();
    QAction *new_file = m.addAction("New Private File");
    m.addSeparator();

    if (!ui->PrivatefileListTable->selectedItems().isEmpty()){
            open = m.addAction("Open");
    } else {
        ui->pushButtonOpenFile_2->setEnabled(false);
    }

    QAction *selected = m.exec(QCursor::pos());

    if(selected == open){
        QString filename = ui->PrivatefileListTable->selectedItems().first()->text();
        _e = new Editor(this, _workerClient, filename, false, false);
        _e->show();
    }

    if(selected == new_file) {
        this->newFile(false);
    }
    if(selected == del) {
        this ->on_pushButtonDeleteFile_3_clicked();
    }
}
