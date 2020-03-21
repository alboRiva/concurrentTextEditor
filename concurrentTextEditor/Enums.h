#ifndef ENUMS_H
#define ENUMS_H

enum messageType  { login, filesRequest, invalid, signup, newFile, userList, edit, deleteFile, invite, openFile };

enum EditType { insertion, deletion, check, fileOk, format };

enum Format { plain, bold, italics, underline };

enum action { add, del, show, request };

#endif // ENUMS_H
