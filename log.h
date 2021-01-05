#ifndef LOG_H
#define LOG_H

#include <QObject>
#include <QIODevice>
#include <QFile>
#include <QtCore>


enum OutputDevice{console, file, both};

class Log : public QObject
{
    Q_OBJECT
public:
    Log(const bool createlogfile = false, const QString caller_name = "",
        const QString logFileName = "Logs/" + QCoreApplication::applicationName() + '_' + QDate::currentDate().toString("yyyy-MM-dd") + ".log");
    void sendMessage(const QString message, OutputDevice device = console);
    ~Log();
public slots:
    void setSaveMode(bool saveMode){this->saveToFile = saveMode;}

signals:
    void LogMessage(QString);


private:
    bool saveToFile = true;
    QFile logfile;
    QTextStream out;
    QString callername, toWidget;

};

#endif // LOG_H
