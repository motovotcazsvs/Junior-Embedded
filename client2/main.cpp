#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle("Client2");
    w.show();

    //після отримання файла завершити роботу

    return a.exec();
}
