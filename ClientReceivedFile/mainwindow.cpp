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

// обробляємо дані від сервера, включно з передачею папок
void MainWindow::slotReadyRead() //
{
    socket = (QTcpSocket*)sender();

    // інформація про отримані дані
    if (information_already_read == false) {
        QString type;
        QDataStream in(socket);
        in.setVersion(QDataStream::Qt_5_7);

        if (in.status() == QDataStream::Ok) {
            if (size_information == 0) {
                if (socket->bytesAvailable() < 2) return;
                in >> size_information;
            }
            if (socket->bytesAvailable() < size_information) return;

            in >> type; // Читаємо тип передачі: файл чи папка

            // Якщо початок папки
            if (type == "START_FOLDER") {
                QString folderName;
                in >> folderName;

                // Створюємо нову папку
                currentFolder = path_save_file + folderName;
                QDir().mkdir(currentFolder);

                qDebug() << "START_FOLDER:" << currentFolder;
                return; // Не продовжуємо обробку як для файлів
            }

            // Якщо завершення папки
            if (type == "END_FOLDER") {
                QString folderName;
                in >> folderName;

                // Закінчуємо передачу папки
                currentFolder = ""; // Скидаємо шлях до поточної папки

                qDebug() << "END_FOLDER:" << folderName;
                return; // Не продовжуємо обробку як для файлів
            }

            // Якщо це файл
            if (type == "FILE") {
                QString fileName;
                in >> fileName >> size_received_file;

                qDebug() << "size_information" << size_information;
                qDebug() << "fileName" << fileName;
                qDebug() << "size_file" << size_received_file;

                size_information = 0;
                information_already_read = true;

                // Створюємо файл у поточній папці
                file = new QFile(currentFolder + "/" + fileName);
                file->open(QIODevice::WriteOnly | QIODevice::Append);
            }
        }
    }

    // Читаємо дані для файла
    QByteArray data_block_file;
    while (socket->bytesAvailable()) {
        QByteArray arr = socket->readAll();
        if (total_size < size_received_file) {
            data_block_file.append(arr);
            total_size += arr.size();
        }
    }

    if (total_size > size_received_file) {
        QByteArray temp = data_block_file.mid(0, size_received_file);
        data_block_file = temp;
    }

    // Записуємо блок даних у файл
    file->write(data_block_file);
    file->waitForBytesWritten(50000);

    float p = float(total_size) / size_received_file;
    qDebug() << int(p * 100);

    // Якщо передача файлу завершена
    if (total_size == size_received_file) {
        file->close(); // Закриваємо файл
        total_size = 0;
        information_already_read = false; // Очікуємо на наступний файл чи папку
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

