#include "server.hpp"

Server::Server(QHostAddress listenip, qint64 port, QString username, QString password, bool userloginmode) {
    this -> listenip = listenip;
    this -> port = port;

    if (! userloginmode) {
        this -> userloginmode = false;

        this -> username = username;
        this -> password = password;
    } else {
        this -> userloginmode = true;
    };

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

    if (this -> userloginmode) {
        this -> http_server -> route("/<arg>", QHttpServerRequest::Method::Get, this, &Server::AnswerUserLogin);
    } else {
        this -> http_server -> route("/<arg>", QHttpServerRequest::Method::Get, this, &Server::AnswerStandalone);
    };

    this -> http_server -> bind(this -> tcp_server);

    qInfo() << "Server is listening on:" << this -> tcp_server -> serverAddress().toString() << "port" << this -> tcp_server -> serverPort();
}

void Server::AnswerStandalone(QString id, QHttpServerResponder &responder) {
    if (id.length() != 6) {
        responder.write(QHttpServerResponder::StatusCode::NotFound);
    } else {
        QString data(Parser::ParseToiCal(id, this -> username, this -> password));

        if (data.isEmpty()) {
            responder.write(QHttpServerResponder::StatusCode::NoContent);
        } else {
            responder.write(data.toUtf8(), QString("text/calendar").toUtf8(), QHttpServerResponder::StatusCode::Ok);
        };
    };
}

void Server::AnswerUserLogin(QString id, const QHttpServerRequest &request, QHttpServerResponder &responder) {
    if (id.length() != 6 || id.toInt() == 0) {
        responder.write(QHttpServerResponder::StatusCode::NotFound);
    } else {
        QStringList auth = Server::ExtractAuth(request.headers());

        QString data(Parser::ParseToiCal(id, auth[0], auth[1]));

        if (data.isEmpty()) {
            responder.write(QHttpServerResponder::StatusCode::Unauthorized);
        } else {
            responder.write(data.toUtf8(), QString("text/calendar").toUtf8(), QHttpServerResponder::StatusCode::Ok);
        };
    };
}

QStringList Server::ExtractAuth(QHttpHeaders headers) {
    QByteArrayView auth_header = headers.value(QHttpHeaders::WellKnownHeader::Authorization);

    QByteArray data = QByteArray::fromBase64(auth_header.sliced(6).toByteArray());

    QString text = QString::fromUtf8(data);

    QStringList result = text.split(":");

    return result;
}
