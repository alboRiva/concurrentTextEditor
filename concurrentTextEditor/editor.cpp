#include "editor.h"
#include "ui_editor.h"

#include <exception>

Editor::Editor(QWidget *parent, WorkerClient *worker, QString fileName, bool isPublic, bool shared) :
    QMainWindow(parent),
    ui(new Ui::Editor),
    _workerClient(worker)
{
    ui->setupUi(this);
    this->setWindowTitle(fileName);
    connect(_workerClient, &WorkerClient::handleFile, this, &Editor::handleFile);
    connect(_workerClient, &WorkerClient::showUser, this, &Editor::showUser);
    connect(_workerClient, &WorkerClient::deleteUser, this, &Editor::deleteUser);
    connect(ui->editorController, &EditorController::broadcastEditWorker, _workerClient, &WorkerClient::broadcastEditWorker);
    connect(_workerClient, &WorkerClient::handleRemoteEdit, ui->editorController, &EditorController::handleRemoteEdit);
    connect(_workerClient, &WorkerClient::fileDeleted, this, &Editor::fileDeleted);

    _workerClient->requestFile(fileName, ui->editorController->getSiteID(), isPublic);

    //Prende lista degli utenti attivi su quel file
    _workerClient->requestUserList(fileName); // DEVE SAPERE SE PRIVATO O PUBBLICO?
    if(shared){
        ui->editorController->setOwner(fileName.split("/")[0]);
        ui->editorController->setShared(shared);
    }
    //Notifica il server che l'utente si e' connesso a quel file
    _workerClient->userJoined(fileName, _workerClient->getUser());    
    ui->editorController->setAccess(isPublic);

    // Get Icons to show in GUI
    ui->actionCut->setIcon(_workerClient->getIcon(UiEditor::cut));
    ui->actionCopy->setIcon(_workerClient->getIcon(UiEditor::copy));
    ui->actionPaste->setIcon(_workerClient->getIcon(UiEditor::paste));
    ui->actionBold->setIcon(_workerClient->getIcon(UiEditor::bold1));
    ui->actionExport_PDF->setIcon(_workerClient->getIcon(UiEditor::pdf));
    ui->actionUnderline->setIcon(_workerClient->getIcon(UiEditor::underlined));
    ui->actionItalics->setIcon(_workerClient->getIcon(UiEditor::italics1));
}

Editor::~Editor()
{
    disconnect(_workerClient, &WorkerClient::handleFile, this, &Editor::handleFile);
    disconnect(_workerClient, &WorkerClient::showUser, this, &Editor::showUser);
    disconnect(_workerClient, &WorkerClient::deleteUser, this, &Editor::deleteUser);
    disconnect(ui->editorController, &EditorController::broadcastEditWorker, _workerClient, &WorkerClient::broadcastEditWorker);
    disconnect(_workerClient, &WorkerClient::handleRemoteEdit, ui->editorController, &EditorController::handleRemoteEdit);
    disconnect(_workerClient, &WorkerClient::fileDeleted, this, &Editor::fileDeleted);
    delete ui;
}

//Al posto di QString avremo il JSonDocument ricevuto, che corrisponde al file
void Editor::handleFile(QJsonDocument unparsedFile) {
    //Il document ricevuto viene passato a editorcontroller che lo converte il data structures
    //c++ e lo visualizza
    if(!ui->editorController->parseCteFile(unparsedFile)){
        //throw exception;
    }
    ui->editorController->write();
}

void Editor::showUser(QString user) {
    QListWidgetItem *newUser = new QListWidgetItem(user);
    QColor color = _colors.at(_colorNumber%_colors.size());
    newUser->setBackground(_colors.at(_colorNumber%_colors.size())); //creare una palette
    _colorNumber++;
    ui->editorController->setUserColor(user, color);
    ui->listWidget->addItem(newUser);
    ui->activeUsers->setText("Active users: " + QString::number(ui->listWidget->count()));
}

QString Editor::deleteUser(QString user) {
    for(int i=0; i < ui->listWidget->count(); i++) {
        if(ui->listWidget->item(i)->text()==user) {
            ui->activeUsers->setText("Active users: " + QString::number(ui->listWidget->count()-1));
            return ui->listWidget->takeItem(i)->text();
        }
    }

    //TODO: see how to manage exceptions, how to distinguish them
    throw ("Users isn't active on file, user not found");
}

void Editor::closeEvent(QCloseEvent *event) {
    QString user;
    user = _workerClient->getUser();
    _workerClient->userLeft(ui->editorController->getFileName(), user);
    this->deleteLater();
}

void Editor::fileDeleted() {
    QMessageBox::warning(this, tr("Warning"), tr("The file has been deleted"));
    this->close();
}


void Editor::on_actionExport_PDF_triggered()
{
    QString fileName = ui->editorController->getFileName().remove(".cte");

    QFileDialog *savePDF = new QFileDialog(parentWidget(),tr("Export PDF"), QDir::current().path(), tr("PDF files (*.pdf)"));
    savePDF->setFileMode(QFileDialog::AnyFile);
    savePDF->setAcceptMode(QFileDialog::AcceptSave);
    if(savePDF->exec() != QDialog::Accepted)
        return;
    QStringList pathList = savePDF->selectedFiles();
    QString path(pathList.join("\\"));
    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPaperSize(QPrinter::A4);
    printer.setOutputFileName(path);

    QTextDocument doc;
    doc.setHtml(ui->editorController->toHtml());
    doc.setPageSize(printer.pageRect().size()); // This is necessary if you want to hide the page number
    doc.print(&printer);
}

void Editor::on_actionPaste_triggered(){

    QClipboard* clipboard = QApplication::clipboard();
    QString clipText = clipboard->text();
    if(!clipText.isNull() && !clipText.isEmpty()){
        QKeyEvent* pasteEvent  = new QKeyEvent(QEvent::KeyPress, Qt::Key_Paste, Qt::NoModifier, clipText);
        QCoreApplication::sendEvent(ui->editorController, pasteEvent);
    }
}

void Editor::on_actionBold_triggered()
{
    setFormatUi(UiEditor::bold1);

    int position = ui->editorController->textCursor().position();
    int anchor = ui->editorController->textCursor().anchor();
    ui->editorController->changeFormat(position, anchor, Format::bold);
}

void Editor::on_actionItalics_triggered()
{
    setFormatUi(UiEditor::italics1);

    int position = ui->editorController->textCursor().position();
    int anchor = ui->editorController->textCursor().anchor();
    ui->editorController->changeFormat(position, anchor, Format::italics);
}

void Editor::on_actionUnderline_triggered()
{
    setFormatUi(UiEditor::underlined);

    int position = ui->editorController->textCursor().position();
    int anchor = ui->editorController->textCursor().anchor();
    ui->editorController->changeFormat(position, anchor, Format::underline);
}

void Editor::on_actionCopy_triggered()
{
    ui->editorController->copy();
}

void Editor::on_actionCut_triggered()
{
    ui->editorController->cut();
}

void Editor::setFormatUi(UiEditor tag){
    switch (tag) {
        case UiEditor::bold1:
            if(!b) {ui->actionBold->setIcon(_workerClient->getIcon(UiEditor::boldSelected)); b = true;}
            else {ui->actionBold ->setIcon(_workerClient->getIcon(UiEditor::bold1)); b = false; }
            ui->actionItalics->setIcon(_workerClient->getIcon(UiEditor::italics1));
            ui->actionUnderline->setIcon(_workerClient->getIcon(UiEditor::underlined));
            break;
        case UiEditor::italics1:
            if(!i) {ui->actionItalics->setIcon(_workerClient->getIcon(UiEditor::italicsSelected)); i =true;}
            else {ui->actionItalics->setIcon(_workerClient->getIcon(UiEditor::italics1)); i = false;}
            ui->actionBold ->setIcon(_workerClient->getIcon(UiEditor::bold1));
            ui->actionUnderline->setIcon(_workerClient->getIcon(UiEditor::underlined));
            break;
        case UiEditor::underlined:
            if(!u) {ui->actionUnderline->setIcon(_workerClient->getIcon(UiEditor::underlinedSelected)); u=true;}
            else {ui->actionUnderline->setIcon(_workerClient->getIcon(UiEditor::underlined)); u = false;}
            ui->actionBold ->setIcon(_workerClient->getIcon(UiEditor::bold1));
            ui->actionItalics->setIcon(_workerClient->getIcon(UiEditor::italics1));
            break;
    }
}
