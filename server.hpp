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

#include <QHttpHeaders>

#include <QByteArrayView>

#include <QString>

#include <parser.hpp>

class Server : public QObject {
    Q_OBJECT

    public:
        Server(QHostAddress listenip, qint64 port, QString username, QString password, bool userloginmode);
        ~Server();

    private:
        QHostAddress listenip;
        qint64 port;

        bool userloginmode;

        QString username;
        QString password;

        QTcpServer * tcp_server;
        QHttpServer * http_server;

        static QStringList ExtractAuth(QHttpHeaders headers);

    private slots:
        void Startup();

        void AnswerStandalone(QString id, QHttpServerResponder &responder);

        void AnswerUserLogin(QString id, QHttpServerResponder &responder);
};

#endif // SERVER_HPP
