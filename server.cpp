#include "server.hpp"

Server::Server(QHostAddress listenip, qint64 port, QString username, QString password) {
    this -> listenip = listenip;
    this -> port = port;
    this -> username = username;
    this -> password = password;

    this -> tcp_server = new QTcpServer;
    this -> http_server = new QHttpServer;

    QTimer::singleShot(0, this, &Server::Startup);
}

Server::~Server() {
    delete this -> http_server;
    delete this -> tcp_server;
}

void Server::Startup() {
    this -> tcp_server -> listen(this -> listenip, this -> port);

    if (! this -> tcp_server -> isListening()) {
        qCritical() << "Server cannot bind to:" << this -> listenip << "port" << this -> port;

        QCoreApplication::exit(3);
    };

    this -> http_server -> route("/<arg>", QHttpServerRequest::Method::Get, this, &Server::Answer);

    this -> http_server -> bind(this -> tcp_server);

    qInfo() << "Server is listening on:" << this -> tcp_server -> serverAddress().toString() << "port" << this -> tcp_server -> serverPort();
}

void Server::Answer(QString id, QHttpServerResponder &responder) {
    if (id.length() != 6) {
        responder.write(QHttpServerResponder::StatusCode::NotFound);
    } else {
        QString data(Parser::ParseToiCal(id, this -> username, this -> password));

        if (data.isEmpty()) {
            responder.write(QHttpServerResponder::StatusCode::NoContent);
        } else {
            responder.write(data.toUtf8(), QString("text/calendar").toUtf8(), QHttpServerResponder::StatusCode::Ok);
        };
    }

}
