#include "server.hpp"

Server::Server(QHostAddress listenip, qint64 port, QString username, QString password) {
    this -> listenip = listenip;
    this -> port = port;
    this -> username = username;
    this -> password = password;

    this -> server = new QTcpServer;

    connect(this -> server, &QTcpServer::pendingConnectionAvailable, this, &Server::NewConnection);

    QTimer::singleShot(0, this, &Server::Startup);
}

Server::~Server() {
    delete this -> server;
}

void Server::Startup() {
    this -> server -> listen(this -> listenip, this -> port);

    if (this -> server -> isListening()) {
        qInfo() << "Server is listening on:" << this -> server -> serverAddress().toString() << "port" << this -> server -> serverPort();
    } else {
        qCritical() << "Server cannot bind to:" << this -> listenip << "port" << this -> port;

        QCoreApplication::exit(3);
    };
}

void Server::NewConnection() {
    QTcpSocket * connection = this -> server -> nextPendingConnection();

    connect(connection, &QTcpSocket::readyRead, this, &Server::ProcessConnection);
}

void Server::ProcessConnection() {
    QTcpSocket * connection = qobject_cast<QTcpSocket *>(sender());

    if (connection == nullptr) {
        return;
    }

    QString request(connection -> readAll());

    QString data;

    if (request.isEmpty()) {
        data = "HTTP/1.1 404 Not Found\r\nconnection: close";

        connection -> write(data.toUtf8());
        connection -> flush();
        connection -> disconnectFromHost();
        connection -> deleteLater();

        return;
    };

    request = request.split("\n").first();
    request = request.split(" ")[1];
    request = request.sliced(1);

    if (request.length() != 6 || request.toInt() == 0) {
        data = "HTTP/1.1 404 Not Found\r\nconnection: close";

        connection -> write(data.toUtf8());
        connection -> flush();
        connection -> disconnectFromHost();
        connection -> deleteLater();

        return;
    };

    data = Parser::ParseToiCal(request, this -> username, this -> password);

    if (data.isEmpty()) {
        data = "HTTP/1.1 404 Not Found\r\nconnection: close";
    } else {
        data.prepend("content-type: text/calendar; charset=UTF-8\r\ncontent-length:" + QString::number(data.toUtf8().length()) +"\r\nconnection: close\r\n\r\n");
        data.prepend("HTTP/1.1 200 OK\r\n");
    };

    connection -> write(data.toUtf8());

    connection -> flush();
    connection -> disconnectFromHost();
    connection -> deleteLater();
}
