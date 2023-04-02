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

    information_already_read = false;//флаг считана или нет информация
    size_information = 0;//размер информации
    total_size = 0;//счетчик байт для файла

    //ui->lineEdit->setText("192.168.1.106");

}

MainWindow::~MainWindow()
{
    delete ui;
}

//получаєм файл від сервера та завершаєм роботу
void MainWindow::slotReadyRead()//
{
    socket = (QTcpSocket*)sender();
//информация о читаемых данных
    if(information_already_read == false){
        QString name_file;
        QDataStream in(socket);
        in.setVersion(QDataStream::Qt_5_7);
        if(in.status() == QDataStream::Ok){
            if(size_information == 0){
                if(socket->bytesAvailable() < 2) return;
                in >> size_information;
            }
            if(socket->bytesAvailable() < size_information) return;

            in >> name_file >> size_received_file;

            qDebug() << "size_information" << size_information;
            qDebug() << "name_file" << name_file;
            qDebug() << "size_file" << size_received_file;

        }
        size_information = 0;
        information_already_read = true;

//создаем файл с таким именем
        file = new QFile(path_save_file + name_file);
        file->open(QIODevice::WriteOnly | QIODevice::Append);
    }

//читаем з сокета данные для файла
    QByteArray data_block_file;
    while(socket->bytesAvailable()){
        QByteArray arr = socket->readAll();
        if(total_size < size_received_file){
            data_block_file.append(arr);
            total_size += arr.size();
        }
    }
    if(total_size > size_received_file){//проверяем если лишнее
        QByteArray temp = data_block_file.mid(0, size_received_file);//то не записываем у файл
        data_block_file = temp;
    }

    //qDebug() << "data_block_file" << data_block_file.size();

//записываем блок данных у файл
    file->write(data_block_file);
    file->waitForBytesWritten(30000);

    float p = float(total_size) / size_received_file;
    qDebug() << int(p * 100);

//если считано столько, сколько блоков принято в информации о размере файла
    if(total_size == size_received_file){
        file->close();//закрываем файл
        emit signalExit();//закрываем програму
    }


}

//підключаємся до сервера
void MainWindow::on_pushButton_clicked()
{
    socket->connectToHost(ui->lineEdit->text(), 44);
}

//задаєм папку на жорстком диску, куди буде збережено файл
void MainWindow::on_pushButton_2_clicked()
{
    path_save_file = QFileDialog::getExistingDirectory(this, tr("Save file to folder"), QDir::currentPath() ,QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    path_save_file.append("/");
    //qDebug() << path_save_file;
}
