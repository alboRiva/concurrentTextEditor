#include "editorcontroller.h"
#include <QKeyEvent>
#include <QRandomGenerator>
#include <QClipboard>
#include <QApplication>

EditorController::EditorController(QWidget *parent) : QTextEdit(parent)
{    
    _crdt = Crdt();

}

void EditorController::keyPressEvent(QKeyEvent *key)
{
    int pressed_key = key->key();
    int cursorPosition = this->textCursor().position();
    int anchor = this->textCursor().anchor();
    int deltaPositions = abs(cursorPosition - anchor);
    int start, end;

    if(deltaPositions != 0){
        start = anchor > cursorPosition ? cursorPosition : anchor;
        end = start == anchor ? cursorPosition : anchor;
    }

    if(key->matches(QKeySequence::Paste)){

        QClipboard* clipboard = QApplication::clipboard();
        QString clipText = clipboard->text();

        // Write clipboard text into crdt and broadcast edit
        for(int writingIndex = 0; writingIndex <  clipText.length(); writingIndex++){
            _crdt.handleLocalInsert(clipText[writingIndex], cursorPosition);
            emit broadcastEditWorker(_crdt.getFileName(), _crdt._lastChar, _crdt._lastOperation, cursorPosition, _isPublic);
            cursorPosition++;
        }

        //this->textCursor().insertText(clipText);

        QTextEdit::keyPressEvent(key);
        return;
    }

    if(key->matches(QKeySequence::Copy)){
        QTextEdit::keyPressEvent(key);
        return;
    }

    // Handle Char insert or return
    if( (pressed_key >= 0x20 && pressed_key <= 0x0ff) || pressed_key == Qt::Key_Return){


        this->textCursor().insertText(key->text().data()[0],highlight);
        emit broadcastEditWorker(_crdt.getFileName(), _crdt._lastChar, _crdt._lastOperation, cursorPosition, _isPublic);
        return;
    }


    // Handle selection deletion with backspace or delete key
    if((pressed_key == Qt::Key_Backspace || pressed_key == Qt::Key_Delete) && deltaPositions != 0) {

        //Iterate over characters to be removed
        for(int floatingCursor =  end; floatingCursor > start; floatingCursor--) {
            _crdt.handleLocalDelete(floatingCursor - 1);
            emit broadcastEditWorker(_crdt.getFileName(), _crdt._lastChar, _crdt._lastOperation, floatingCursor - 1, _isPublic);
        }
    }

    // Handle backspace deletion
    if(pressed_key == Qt::Key_Backspace && (cursorPosition -1) != -1 && deltaPositions == 0) {

        _crdt.handleLocalDelete(cursorPosition -1);
        emit broadcastEditWorker(_crdt.getFileName(), _crdt._lastChar, _crdt._lastOperation, cursorPosition -1, _isPublic);
    }


    QTextCursor lastIndex = this->textCursor();
    lastIndex.movePosition(QTextCursor::End);

    // Handle "delete" deletion
    if(pressed_key == Qt::Key_Delete && this->textCursor() != lastIndex && deltaPositions == 0) {

        _crdt.handleLocalDelete(cursorPosition);
        emit broadcastEditWorker(_crdt.getFileName(), _crdt._lastChar, _crdt._lastOperation, cursorPosition, _isPublic);
    }

    // Let the editor do its thing on current text if no handler is found
    QTextEdit::keyPressEvent(key);
}

//Scrive sull'editor il testo parsato
void EditorController::write(){

    QString _textBuffer = _crdt.getTextBuffer();
    if(_textBuffer.isNull()){
        //throw exception
    }

    this->append(_textBuffer);
}


//TODO: Tutte ste eccezioni vanno catchate nell Editor


//Wrappers for crdt methods
QString EditorController::getFileName(){

    return _crdt.getFileName();
}

QUuid EditorController::getSiteID() {
    return _crdt.getSiteID();
}

bool EditorController::parseCteFile(QJsonDocument unparsedFile){
    return _crdt.parseCteFile(unparsedFile);
}

void EditorController::handleRemoteEdit(const QJsonObject &qjo) {

    int index;
    QTextCursor editingCursor;
    QTextCursor cursorBeforeEdit;

    EditType edit = static_cast<EditType>(qjo["editType"].toInt());

    switch(edit) {

        case EditType::insertion:

            index = _crdt.handleRemoteInsert(qjo);
            editingCursor = this->textCursor();
            cursorBeforeEdit= this->textCursor();
            editingCursor.setPosition(index);
            this->setTextCursor(editingCursor);
            // Write
            this->textCursor().insertText(QString(_crdt.getChar(qjo["content"].toObject())._value));
            // Set cursor back to original position (before editing)
            this->setTextCursor(cursorBeforeEdit);

            break;

        case EditType::deletion:

            index = _crdt.handleRemoteDelete(qjo);
            editingCursor = this->textCursor();
            cursorBeforeEdit = this->textCursor();
            editingCursor.setPosition(index + 1);
            this->setTextCursor(editingCursor);
            this->textCursor().deletePreviousChar();
            this->setTextCursor(cursorBeforeEdit);

            break;

        default:
            //handle exception
            break;
    }
}

void EditorController::setAccess(bool isPublic){

    _isPublic = isPublic;
}
