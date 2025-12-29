#ifndef PARSER_HPP
#define PARSER_HPP

#include <QObject>

#include <QEventLoop>

#include <QTimer>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include <QString>

#include <QRegularExpression>

#include <QDateTime>
#include <QDate>
#include <QTime>
#include <QTimeZone>

#include <QUuid>

class Parser {
    public:
        static QString ParseToiCal(QString id, QString username, QString password);

    private:
        static QNetworkRequest CreateRequest(QString id, QString username, QString password);

        static QString url_part_1;
        static QString url_part_2;

        static QStringList TimeConverter(QString date, QString time);
};

#endif // PARSER_HPP
