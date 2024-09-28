#include "server.h"
#include <QDataStream>
#include <QDir>

server::server()
{
    if(this->listen(QHostAddress::Any, 44)){
        qDebug() << "start server";
    }
    else qDebug() << "error";
}

//підключення клієнта
void server::incomingConnection(qintptr socketDescriptor)
{
    socket = new QTcpSocket;
    socket->setSocketDescriptor(socketDescriptor);

    if (!socket->isValid()) {
            qDebug() << "Invalid socket. Connection failed.";
            socket->deleteLater();
            return;
        }

    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
    connect(socket, &QTcpSocket::readyRead, this, &server::slotReadyRead);


    Sockets.push_back(socket);
    qDebug() << "client connected" << socketDescriptor;

//створюєм файл для відправки клієнту
    //QFile file("testtt.7z");
    //this->sendFileToClient(file);
    //this->sendFolderToClient("Newfolder");
    //writeText();
}

//відправлення файла
void server::sendFileToClient(QFile &file)
{
    if (!socket->isValid()) {
            qDebug() << "Socket is not valid, skipping file transfer.";
            return;
        }


    // Інформація про файл, який буде передаватися
    QByteArray arr;
    QString name_file = file.fileName();
    qDebug() << "name_file" << name_file;
    // Оголошуємо тип передачі "FILE"
        QDataStream out(&arr, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_5_7);
        out << quint16(0) << QString("FILE") << name_file << file.size(); // Додаємо тип передачі "FILE"
        out.device()->seek(0);
        out << quint16(arr.size() - sizeof(quint16));
    // Надсилання мета-даних про файл всім клієнтам
    for(int i = 0; i < Sockets.size(); i++) {
        Sockets[i]->write(arr);
        Sockets[i]->waitForBytesWritten();
    }

    // Читання та надсилання файлу блоками по 512 байт
    QByteArray block;
    if (file.open(QIODevice::ReadOnly)) {
        while (!file.atEnd()) {
            block = file.read(512);  // Читаємо блок 512 байт
            for (int i = 0; i < Sockets.size(); i++) {
                Sockets[i]->write(block);
                Sockets[i]->waitForBytesWritten();
            }
        }
        file.close();
    }

    // Перевірка та виведення інформації про завершення передачі файлу
    for(int i = 0; i < Sockets.size(); i++) {
        if (Sockets[i]->bytesToWrite() == 0) {
            qDebug() << "file upload completed! client #" << i + 1;
        }
    }


}

void server::sendFolderToClient(const QString &folderPath)
{
    QDir dir(folderPath);
    if (!dir.exists()) {
        qDebug() << "Directory does not exist!";
        return;
    }

    // Отримуємо список файлів і папок у директорії
    QStringList fileNames = dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
    QStringList folderNames = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    // Відправляємо сигнал початку передачі папки
    QByteArray arr;
    QDataStream out(&arr, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_7);
    out << quint16(0) << QString("START_FOLDER") << folderPath;  // Початок передачі папки
    out.device()->seek(0);
    out << quint16(arr.size() - sizeof(quint16));
    // Відправляємо сигнал всім клієнтам про початок передачі папки
    for (int i = 0; i < Sockets.size(); i++) {
        Sockets[i]->write(arr);
        Sockets[i]->waitForBytesWritten();
        qDebug() << "send start folder";
    }

    // Відправляємо всі файли у поточній папці
    foreach (QString fileName, fileNames) {
        QFile file(dir.filePath(fileName));
        qDebug() << "foreach1";
        //QFile file2("testtt.7z");
        this->sendFileToClient(file);  // Використовуємо вже існуючу функцію для відправки файлу
    }
//    QFile file2("testtt.7z");
//    this->sendFileToClient(file2);  // Використовуємо вже існуючу функцію для відправки файлу

    // Рекурсивно обробляємо всі вкладені папки
    foreach (QString folderName, folderNames) {
        QString subFolderPath = dir.filePath(folderName);
        sendFolderToClient(subFolderPath);  // Рекурсивно відправляємо вкладені папки
    }

    // Відправляємо сигнал завершення передачі папки
    arr.clear();
    out.device()->seek(0);
    out << quint16(0) << QString("END_FOLDER") << folderPath;  // Завершення передачі папки
    out.device()->seek(0);
    out << quint16(arr.size() - sizeof(quint16));
    for (int i = 0; i < Sockets.size(); i++) {
        Sockets[i]->write(arr);
        Sockets[i]->waitForBytesWritten();
        qDebug() << "send end folder";
    }

    qDebug() << "Folder " << folderPath << " and all contents sent successfully!";
}

void server::slotReadyRead()
{
    socket = (QTcpSocket*)sender();
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_5_7);
    quint16 size;
    QString info;
    in >> size;
    in >> info;
    qDebug() << "info" << info;

}

void server::writeText()
{
    QByteArray arr;
    QDataStream out(&arr, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_7);
    out << quint16(0) << QString("END_FOLDER");//резервуєм два байта на розмір блоку(записуючи туди нулі) та поміщаєм дані в масив
    out.device()->seek(0);//переміщаємо вказівник на начало в масиві, тобто на зарезервовані два байта - розмір блоку
    out << quint16(arr.size() - sizeof(quint16));//та записуєм туди фактичний розмір даних(віднявши від масива перші два байти)
    qDebug() << arr;
//    socket->write(arr);
//    socket->waitForBytesWritten();
    for(int i = 0; i < Sockets.size(); i++) {
        Sockets[i]->write(arr);
        Sockets[i]->waitForBytesWritten();
    }
}



