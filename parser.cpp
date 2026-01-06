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

        if (converted_date.length() < 2) {
            continue;
        };

        result.append("BEGIN:VEVENT\r\n");

        result.append("UID:" + QUuid::createUuid().toString(QUuid::WithoutBraces) + "\r\n");

        result.append("DTSTAMP:" + current_time_str + "\r\n");
        
        result.append("DTSTART;TZID=Europe/Warsaw:" + converted_date[0] + "\r\n");

        result.append("DTEND;TZID=Europe/Warsaw:" + converted_date[1] + "\r\n");
        
        if ((! type.isEmpty()) && (! subject.isEmpty())) {
            result.append("SUMMARY:" + type + " | " + subject + "\r\n");
        } else if (! subject.isEmpty()) {
            result.append("SUMMARY:" + subject + "\r\n");
        } else if (! type.isEmpty()) {
            result.append("SUMMARY:" + type + " | ------" + "\r\n");
        } else {
            result.append("SUMMARY:------\r\n");
        };
        
        if (! teacher.isEmpty()) {
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

    result.prepend("BEGIN:VTIMEZONE\r\nTZID:Europe/Warsaw\r\nLAST-MODIFIED:20251211T094730Z\r\nTZURL:https://www.tzurl.org/zoneinfo/Europe/Warsaw\r\nX-LIC-LOCATION:Europe/Warsaw\r\nX-PROLEPTIC-TZNAME:LMT\r\nBEGIN:STANDARD\r\nTZNAME:WMT\r\nTZOFFSETFROM:+0124\r\nTZOFFSETTO:+0124\r\nDTSTART:18800101T000000\r\nEND:STANDARD\r\nBEGIN:STANDARD\r\nTZNAME:CET\r\nTZOFFSETFROM:+0124\r\nTZOFFSETTO:+0100\r\nDTSTART:19150805T000000\r\nEND:STANDARD\r\nBEGIN:DAYLIGHT\r\nTZNAME:CEST\r\nTZOFFSETFROM:+0100\r\nTZOFFSETTO:+0200\r\nDTSTART:19160430T230000\r\nRDATE:19400623T020000\r\nRDATE:19430329T020000\r\nRDATE:19440403T020000\r\nRDATE:19450429T000000\r\nRDATE:19460414T000000\r\nRDATE:19470504T020000\r\nRDATE:19480418T020000\r\nRATE:19490410T020000\r\nRDATE:19570602T010000\r\nRDATE:19580330T010000\r\nRDATE:19590531T010000\r\nRDATE:19600403T010000\r\nEND:DAYLIGHT\r\nBEGIN:STANDARD\r\nTZNAME:CET\r\nTZOFFSETFROM:+0200\r\nTZOFFSETTO:+0100\r\nDTSTART:19161001T010000\r\nRDATE:19170917T030000\r\nRDATE:19220601T000000\r\nRDATE:19421102T030000\r\nRDATE:19431004T030000\r\nRDATE:19441004T020000\r\nRDATE:19451101T000000\r\nRDATE:19461007T030000\r\nRDATE:19770925T020000\r\nRDATE:19781001T020000\r\nEND:STANDARD\r\nBEGIN:DAYLIGHT\r\nTZNAME:CEST\r\nTZOFFSETFROM:+0100\r\nTZOFFSETTO:+0200\r\nDTSTART:19170416T020000\r\nRRULE:FREQ=YEARLY;BYMONTH=4;BYDAY=3MO;UNTIL=19180415T010000Z\r\nEND:DAYLIGHT\r\nBEGIN:STANDARD\r\nTZNAME:EET\r\nTZOFFSETFROM:+0200\r\nTZOFFSETTO:+0200\r\nDTSTART:19180916T030000\r\nEND:STANDARD\r\nBEGIN:DAYLIGHT\r\nTZNAME:EEST\r\nTZOFFSETFROM:+0200\r\nTZOFFSETTO:+0300\r\nDTSTART:19190415T020000\r\nEND:DAYLIGHT\r\nBEGIN:STANDARD\r\nTZNAME:EET\r\nTZOFFSETFROM:+0300\r\nTZOFFSETTO:+0200\r\nDTSTART:19190916T030000\r\nEND:STANDARD\r\nBEGIN:DAYLIGHT\r\nTZNAME:CEST\r\nTZOFFSETFROM:+0200\r\nTZOFFSETTO:+0200\r\nDTSTART:19441001T000000\r\nEND:DAYLIGHT\r\nBEGIN:STANDARD\r\nTZNAME:CET\r\nTZOFFSETFROM:+0200\r\nTZOFFSETTO:+0100\r\nDTSTART:19471005T030000\r\nRRULE:FREQ=YEARLY;BYMONTH=10;BYDAY=1SU;UNTIL=19491002T010000Z\r\nEND:STANDARD\r\nBEGIN:STANDARD\r\nTZNAME:CET\r\nTZOFFSETFROM:+0200\r\nTZOFFSETTO:+0100\r\nDTSTART:19570929T020000\r\nRRULE:FREQ=YEARLY;BYMONTH=9;BYDAY=-1SU;UNTIL=19580928T000000Z\r\nEND:STANDARD\r\nBEGIN:STANDARD\r\nTZNAME:CET\r\nTZOFFSETFROM:+0200\r\nTZOFFSETTO:+0100\r\nDTSTART:19591004T020000\r\nRRULE:FREQ=YEARLY;BYMONTH=10;BYDAY=1SU;UNTIL=19611001T000000Z\r\nEND:STANDARD\r\nBEGIN:DAYLIGHT\r\nTZNAME:CEST\r\nTZOFFSETFROM:+0100\r\nTZOFFSETTO:+0200\r\nDTSTART:19610528T010000\r\nRRULE:FREQ=YEARLY;BYMONTH=5;BYDAY=-1SU;UNTIL=19640531T000000Z\r\nEND:DAYLIGHT\r\nBEGIN:STANDARD\r\nTZNAME:CET\r\nTZOFFSETFROM:+0200\r\nTZOFFSETTO:+0100\r\nDTSTART:19620930T020000\r\nRRULE:FREQ=YEARLY;BYMONTH=9;BYDAY=-1SU;UNTIL=19640927T000000Z\r\nEND:STANDARD\r\nBEGIN:STANDARD\r\nTZNAME:CET\r\nTZOFFSETFROM:+0100\r\nTZOFFSETTO:+0100\r\nDTSTART:19770101T000000\r\nRDATE:19880101T000000\r\nEND:STANDARD\r\nBEGIN:DAYLIGHT\r\nTZNAME:CEST\r\nTZOFFSETFROM:+0100\r\nTZOFFSETTO:+0200\r\nDTSTART:19770403T010000\r\nRRULE:FREQ=YEARLY;BYMONTH=4;BYDAY=1SU;UNTIL=19800406T000000Z\r\nEND:DAYLIGHT\r\nBEGIN:STANDARD\r\nTZNAME:CET\r\nTZOFFSETFROM:+0200\r\nTZOFFSETTO:+0100\r\nDTSTART:19790930T020000\r\nRRULE:FREQ=YEARLY;BYMONTH=9;BYDAY=-1SU;UNTIL=19870927T000000Z\r\nEND:STANDARD\r\nBEGIN:DAYLIGHT\r\nTZNAME:CEST\r\nTZOFFSETFROM:+0100\r\nTZOFFSETTO:+0200\r\nDTSTART:19810329T010000\r\nRRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=-1SU;UNTIL=19870329T000000Z\r\nEND:DAYLIGHT\r\nBEGIN:DAYLIGHT\r\nTZNAME:CEST\r\nTZOFFSETFROM:+0100\r\nTZOFFSETTO:+0200\r\nDTSTART:19880327T020000\r\nRRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=-1SU\r\nEND:DAYLIGHT\r\nBEGIN:STANDARD\r\nTZNAME:CET\r\nTZOFFSETFROM:+0200\r\nTZOFFSETTO:+0100\r\nDTSTART:19880925T030000\r\nRRULE:FREQ=YEARLY;BYMONTH=9;BYDAY=-1SU;UNTIL=19950924T010000Z\r\nEND:STANDARD\r\nBEGIN:STANDARD\r\nTZNAME:CET\r\nTZOFFSETFROM:+0200\r\nTZOFFSETTO:+0100\r\nDTSTART:19961027T030000\r\nRRULE:FREQ=YEARLY;BYMONTH=10;BYDAY=-1SU\r\nEND:STANDARD\r\nEND:VTIMEZONE\r\n");
    
    result.prepend("BEGIN:VCALENDAR\r\nVERSION:2.0\r\nPRODID:-//UEK//UEK//EN\r\n");

    result.append("END:VCALENDAR\r\n");

    return result;
}

QNetworkRequest Parser::CreateRequest(QString id, QString username, QString password) {
    QNetworkRequest request(QUrl(Parser::url_part_1 + id + Parser::url_part_2));

    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());

    QByteArray creds = QByteArray(QString(username + ":" + password).toUtf8()).toBase64();

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

    QString start_result(start_datetime.date().toString("yyyyMMdd") + "T" + start_datetime.time().toString("hhmmss"));
    QString end_result(end_datetime.date().toString("yyyyMMdd") + "T" + end_datetime.time().toString("hhmmss"));

    return {start_result, end_result};
}

QString Parser::url_part_1 = "https://planzajec.uek.krakow.pl/index.php?typ=G&id=";
QString Parser::url_part_2 = "&okres=2";
