#include <QCoreApplication>

#include <QSettings>

#include <QtLogging>

#include <QHostAddress>

#include "server.hpp"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    a.setOrganizationName("uekscraper");
    a.setApplicationName("uekscraper");
    a.setApplicationVersion("0.1");

    QSettings settings("/etc/uekscraper.conf", QSettings::IniFormat);

    if (settings.value("username").isNull() || settings.value("password").isNull()) {
        qFatal() << "Username and/or password has not been provided!\nWrite them to \"/etc/uekscraper.conf\" file,\nusing INI format:\n username=<your_username>\n password=<your_password>";

        QCoreApplication::exit(1);
    };

    if (settings.value("port").isNull()) {
        settings.setValue("port", 9000);
    } else if (settings.value("port").toInt() < 1 || settings.value("port").toInt() < 65536) {
        settings.setValue("port", 9000);
    };

    if (settings.value("listenip").isNull()) {
        settings.setValue("listenip", QHostAddress(QHostAddress::Any).toString());

        qWarning() << "Listening address set to all addresses.";
    } else if (QHostAddress(settings.value("listenip").toString()).isNull()) {
        settings.setValue("listenip", QHostAddress(QHostAddress::Any).toString());

        qWarning() << "Saved listening address is invalid, setting to all addresses.";
    };

    Server server(QHostAddress(settings.value("listenip").toString()), settings.value("port").toInt(), settings.value("username").toString(), settings.value("password").toString());

    return a.exec();
}
