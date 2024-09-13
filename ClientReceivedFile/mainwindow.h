#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFile>
#include <QMainWindow>
#include <QTcpSocket>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void slotReadyRead();
signals:
    void signalExit();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::MainWindow *ui;
    QTcpSocket *socket;
    quint16 size_information;
    QString path_save_file;
    bool information_already_read;
    qint64 size_received_file;
    qint64 total_size;
    QFile *file;
    QString currentFolder;
};

#endif // MAINWINDOW_H
