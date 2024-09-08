#include "server.h"
#include <QDataStream>

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

    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);

    Sockets.push_back(socket);
    qDebug() << "client connected" << socketDescriptor;

//створюєм файл для відправки клієнту
    QFile file("testttt.7z");
//    QFile file("response.txt");
//    QTextStream out(&file);
//    if(file.open(QIODevice::WriteOnly | QIODevice::Truncate)){
//        out << "Hello world!";
//        out << "\r\n";
//        out << "31.03.2023";
//        file.close();
//    }
    this->sendFileToClient(file);
}

//відправлення файла
void server::sendFileToClient(QFile &file)
{
    /*
//информация о данных, которые будут передаваться
    QByteArray arr;
    QString name_file = file.fileName();
    qDebug() << "name_file" << name_file;
    QDataStream out(&arr, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_7);
    out << quint16(0) << name_file << file.size();
    out.device()->seek(0);
    out << quint16(arr.size() - sizeof(quint16));
    for(int i = 0; i < Sockets.size(); i++){
        Sockets[i]->write(arr);
        Sockets[i]->waitForBytesWritten();
    }

//считываем данные с файла
    QByteArray bytes_file;
    if(file.open(QIODevice::ReadOnly)){
        while(!file.atEnd()){
            bytes_file = file.readAll();
        }
        file.close();
    }

//разделяем на блоки по 512 байт
    QVector<QByteArray> blocks;
    int pos = 0;
    int size_bytesfile = bytes_file.size();
    int size_block = 512;
    while(pos < size_bytesfile){
        QByteArray arr = bytes_file.mid(pos, size_block);
        blocks << arr;
        pos += arr.size();
    }

//отправка всех блоков данных
    foreach(QByteArray block, blocks){
        for(int i = 0; i < Sockets.size(); i++){
            //qDebug() << "block size" << block.size();
            Sockets[i]->write(block);
            Sockets[i]->waitForBytesWritten();
            if(block == blocks.last()){
                qDebug() << "file upload completed! client #" << i + 1;
            }
        }
    }
    qDebug() << "send size bytes" << size_bytesfile;

//отсоединяем клиентов
   for(int i = 0; i < Sockets.size(); i++){
       Sockets[i]->disconnectFromHost();
   }
*/


//        // Інформація про файл, який буде передаватися
//        QByteArray arr;
//        QString name_file = file.fileName();
//        qDebug() << "name_file" << name_file;
//        QDataStream out(&arr, QIODevice::WriteOnly);
//        out.setVersion(QDataStream::Qt_5_7);
//        out << quint16(0) << name_file << file.size();
//        out.device()->seek(0);
//        out << quint16(arr.size() - sizeof(quint16));

//        // Надсилання мета-даних про файл всім клієнтам
//        for (int i = 0; i < Sockets.size(); i++) {
//            Sockets[i]->write(arr);
//            Sockets[i]->waitForBytesWritten();
//        }

//        // Читання та надсилання файлу блоками по 512 байт
//        QByteArray block;
//        if (file.open(QIODevice::ReadOnly)) {
//            while (!file.atEnd()) {
//                block = file.read(512);  // Читаємо блок 512 байт
//                for (int i = 0; i < Sockets.size(); i++) {
//                    Sockets[i]->write(block);
//                    Sockets[i]->waitForBytesWritten();
//                }
//            }
//            file.close();
//        }

//        // Повідомлення про завершення передачі файлу
//        qDebug() << "file upload completed!";

//        // Відключення клієнтів
//        for (int i = 0; i < Sockets.size(); i++) {
//            Sockets[i]->disconnectFromHost();
//        }


    // Інформація про файл, який буде передаватися
        QByteArray arr;
        QString name_file = file.fileName();
        qDebug() << "name_file" << name_file;
        QDataStream out(&arr, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_5_7);
        out << quint16(0) << name_file << file.size();
        out.device()->seek(0);
        out << quint16(arr.size() - sizeof(quint16));

        // Надсилання мета-даних про файл всім клієнтам
        for (int i = 0; i < Sockets.size(); i++) {
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
        for (int i = 0; i < Sockets.size(); i++) {
            if (Sockets[i]->bytesToWrite() == 0) {
                qDebug() << "file upload completed! client #" << i + 1;
            }
        }

        // Відключення клієнтів
        for (int i = 0; i < Sockets.size(); i++) {
            Sockets[i]->disconnectFromHost();
        }
}

