#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::slotReadyRead);
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
    connect(this, SIGNAL(signalExit()), qApp, SLOT(quit()));

    information_already_read = false; // флаг считана или нет информация
    size_information = 0; // размер информации
    total_size = 0; // счетчик байт для файла

    ui->lineEdit->setText("192.168.0.108");

    currentFolder = ""; // зберігаємо поточну папку
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::slotReadyRead()
{
    //qDebug() << "slot read";
    socket = (QTcpSocket*)sender();
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_5_7);

    // Якщо ще не прочитали інформацію про тип
    if (!information_already_read){
        qDebug() << "!information_already_read";
        if (size_information == 0){
            qDebug() << "size_information == 0)";
            if (socket->bytesAvailable() < 2){
                qDebug() << "bytesAvailable() < 2";
                return;
            }
            in >> size_information;
        }

        if (socket->bytesAvailable() < size_information){
            qDebug() << "bytesAvailable() < size_information";
            return;
        }

        QString type;
        in >> type;
        qDebug() << "type:" << type;

        if (type == "START_FOLDER") {
            QString folderName;
            in >> folderName;
            currentFolder = path_save_file + folderName;
            QDir().mkdir(currentFolder);
            qDebug() << "START_FOLDER:" << currentFolder;
            size_information = 0;
            return;
        } else if (type == "END_FOLDER") {
            QString folderName;
            in >> folderName;
            currentFolder = "";
            qDebug() << "END_FOLDER:" << folderName;
            size_information = 0;
            //information_already_read = false;//нада перевірити,розкоментовано повинно бути
            return;
        } else if (type == "FILE") {
            QString fileName;
            in >> fileName >> size_received_file;
            qDebug() << "fileName:" << fileName << "size_file:" << size_received_file;

            size_information = 0;
            information_already_read = true;

            QFileInfo fileInfo(fileName);
            QString baseFileName = fileInfo.fileName();
            file = new QFile(currentFolder + "/" + baseFileName);
            file->open(QIODevice::WriteOnly | QIODevice::Append);
            qint64 availableData = socket->bytesAvailable();
            if(availableData){
                qDebug() << "availableData" << availableData;
            }
            else return;
        }
    }

    // Читаємо дані файлу
    qint64 availableData = socket->bytesAvailable();
    qint64 bytesToRead = qMin(availableData, size_received_file - total_size);
    QByteArray data_block_file = socket->read(bytesToRead);

    file->write(data_block_file);
    total_size += data_block_file.size();

    if (total_size >= size_received_file) {
        qDebug() << "Файл отримано повністю";
        file->close();
        total_size = 0;
        information_already_read = false;
    }

    // Якщо залишились зайві дані, які належать наступному файлу/папці
    if (availableData > bytesToRead) {
        qDebug() << "availableData > bytesToRead";
        slotReadyRead(); // викликаємо слот знову, щоб обробити зайві дані
    }
}


// Підключення до сервера
void MainWindow::on_pushButton_clicked()
{
    socket->connectToHost(ui->lineEdit->text(), 44);
}

// Вибір папки для збереження файлів
void MainWindow::on_pushButton_2_clicked()
{
    path_save_file = QFileDialog::getExistingDirectory(this, tr("Save file to folder"), QDir::currentPath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    path_save_file.append("/"); // Додаємо слеш для зручності
}


void MainWindow::on_pushButton_3_clicked()
{
    QByteArray arr;
    QDataStream out(&arr, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_7);
    out << quint16(0) << QString("hello_client_1");
    out.device()->seek(0);
    out << quint16(arr.size() - sizeof(quint16));
    qDebug() << arr;
    socket->write(arr);
    socket->waitForBytesWritten();


}
