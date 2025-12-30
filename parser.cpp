#include "parser.hpp"

QString Parser::ParseToiCal(QString id, QString username, QString password) {
    if (id.length() != 6 || username.length() != 6 || password.length() < 8) {
        return QString();
    }

    // Request data

    QNetworkAccessManager accessmanager;

    QEventLoop loop;

    auto reply = accessmanager.get(Parser::CreateRequest(id, username, password));

    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);

    QTimer::singleShot(5000, &loop, &QEventLoop::quit);

    loop.exec();

    if (reply -> error() != QNetworkReply::NoError) {
        qCritical() << reply -> errorString();

        return QString();
    };

    if (reply -> isRunning()) {
        reply -> abort();

        qCritical() << "Request timed out.";

        reply -> deleteLater();

        return QString();
    };

    QString data(reply -> readAll());

    reply -> deleteLater();

    if (data.isEmpty()) {
        return QString();
    }

    // Regex setup

    QRegularExpression tr_re(R"(<tr[^>]*>(.*?)</tr>)", QRegularExpression::DotMatchesEverythingOption);
    QRegularExpression td_re(R"(<td[^>]*>(.*?)</td>)", QRegularExpression::DotMatchesEverythingOption);

    // Time and date setup

    QDateTime current_time = QDateTime::currentDateTimeUtc();
    QString current_time_str(current_time.date().toString(R"(yyyyMMdd)") + "T" + current_time.time().toString(R"(hhmmss)") + "Z");

    //

    QString result;

    // Body processing

    QRegularExpressionMatchIterator rows = tr_re.globalMatch(data);

    while (rows.hasNext()) {
        QRegularExpressionMatch row = rows.next();

        QRegularExpressionMatchIterator columns = td_re.globalMatch(row.captured(1));

        quint8 i = 1;

        QString date;
        QString time;
        QString subject;
        QString type;
        QString teacher;
        QString room;

        while (columns.hasNext()) {
            QRegularExpressionMatch column = columns.next();

            // Columns processing

            switch (i) {
                case (1):
                    date = column.captured(1).remove(QRegularExpression("<.*>"));
                    break;
                case (2):
                    time = column.captured(1).remove(QRegularExpression("<.*>"));
                    break;
                case (3):
                    subject = column.captured(1).remove(QRegularExpression("<.*>"));
                    break;
                case (4):
                    type = column.captured(1).remove(QRegularExpression("<.*>"));
                    break;
                case (5):
                    teacher = column.captured(1).remove(QRegularExpression("<.*>"));
                    break;
                case (6):
                    room = column.captured(1).remove(QRegularExpression("<.*>"));
                    break;
            };

            i++;

        }

        if (i != 7) {
            continue;
        };

        // Saving data

        QStringList converted_date = Parser::TimeConverter(date, time);

        if (converted_date.isEmpty()) {
            continue;
        };

        result.append("BEGIN:VEVENT\r\n");

        result.append("UID:" + QUuid::createUuid().toString(QUuid::WithoutBraces) + "\r\n");

        result.append("DTSTAMP:" + current_time_str + "\r\n");
        
        result.append("DTSTART:" + converted_date[0] + "\r\n");

        result.append("DTEND:" + converted_date[1] + "\r\n");
        
        if (! subject.isEmpty()) {
            result.append("SUMMARY:" + subject + "\r\n");
        };
        
        if (! type.isEmpty()) {
            result.append("DESCRIPTION:" + type);
            
            if (! teacher.isEmpty()) {
                result.append(", " + teacher);
            };
            
            result.append("\r\n");
        } else if (! teacher.isEmpty() {
            result.append("DESCRIPTION:" + teacher + "\r\n");
        };

        if (! room.isEmpty()) {
            result.append("LOCATION:" + room + "\r\n");
        };

        result.append("END:VEVENT\r\n");
    };

    //
    
    if (result.isEmpty()) {
        return QString();
    }
    
    result.prepend("BEGIN:VCALENDAR\r\nVERSION:2.0\r\n");

    result.append("END:VCALENDAR\r\n");

    return result;
}

QNetworkRequest Parser::CreateRequest(QString id, QString username, QString password) {
    QNetworkRequest request(QUrl(Parser::url_part_1 + id + Parser::url_part_2));

    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());

    QByteArray creds = QByteArray(username.toUtf8() + ":" + password.toUtf8()).toBase64();

    request.setRawHeader("Authorization", "Basic " + creds);

    request.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:109.0) Gecko/20100101 Firefox/121.0");

    return request;
}

QStringList Parser::TimeConverter(QString date, QString time) {
    QStringList date_splitted = date.split("-");

    if (date_splitted.length() < 2) {
        return {};
    };

    if (time.length() != 22) {
        return {};
    };

    time = time.sliced(3, 14);
    time = time.remove(" ");
    QStringList splitted_time = time.split("-");

    QStringList start = splitted_time[0].split(":");
    QStringList end = splitted_time[1].split(":");

    QDateTime start_datetime(QDate(date_splitted[0].toInt(), date_splitted[1].toInt(), date_splitted[2].toInt()), QTime(start[0].toInt(), start[1].toInt()), QTimeZone("Europe/Warsaw"));
    QDateTime end_datetime(QDate(date_splitted[0].toInt(), date_splitted[1].toInt(), date_splitted[2].toInt()), QTime(end[0].toInt(), end[1].toInt()), QTimeZone("Europe/Warsaw"));

    QString start_result(start_datetime.date().toString("yyyyMMdd") + "T" + start_datetime.time().toString("hhmmsstt"));
    QString end_result(end_datetime.date().toString("yyyyMMdd") + "T" + end_datetime.time().toString("hhmmsstt"));

    return {start_result, end_result};
}

QString Parser::url_part_1 = "https://planzajec.uek.krakow.pl/index.php?typ=G&id=";
QString Parser::url_part_2 = "&okres=2";
