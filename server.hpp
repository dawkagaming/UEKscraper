#ifndef SERVER_HPP
#define SERVER_HPP

#include <QObject>

#include <QCoreApplication>

#include <QtLogging>

#include <QTimer>

#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>

#include <QString>

#include <parser.hpp>

class Server : public QObject {
    Q_OBJECT

    public:
        Server(QHostAddress listenip, qint64 port, QString username, QString password);
        ~Server();

    private:
        QHostAddress listenip;
        qint64 port;
        QString username;
        QString password;

        QTcpServer * server;

    private slots:
        void NewConnection();

        void Startup();

        void ProcessConnection();
};

#endif // SERVER_HPP
