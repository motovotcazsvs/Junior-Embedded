#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QVector>
#include <QFile>

class server : public QTcpServer
{
    Q_OBJECT

public:
    server();
    QTcpSocket *socket;

private:
    QVector <QTcpSocket*> Sockets;
    void sendFileToClient(QFile &);

public slots:
    void incomingConnection(qintptr socketDescriptor);



};

#endif // SERVER_H
