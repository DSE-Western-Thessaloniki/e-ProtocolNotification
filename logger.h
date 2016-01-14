#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

class Logger : public QObject
{
Q_OBJECT

public:
    explicit Logger(QString fileName, QObject *parent=0);
    ~Logger();
    void setShowDateTime(bool value);

private:
    QFile *file;
    bool m_showDate;

public slots:
    void write(const QString &value);
};

extern Logger logger;

#endif // LOGGER_H
