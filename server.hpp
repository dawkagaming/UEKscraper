#ifndef SERVER_HPP
#define SERVER_HPP

#include <QObject>

#include <QCoreApplication>

#include <QtLogging>

#include <QTimer>

#include <QTcpServer>
#include <QHostAddress>

#include <QHttpServer>
#include <QHttpServerRequest>
#include <QHttpServerResponder>

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

        QTcpServer * tcp_server;
        QHttpServer * http_server;

    private slots:
        void Startup();

        void Answer(QString id, QHttpServerResponder &responder);
};

#endif // SERVER_HPP
