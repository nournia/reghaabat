#ifndef SENDER_H
#define SENDER_H

#include <QNetworkAccessManager>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>

class Sender : public QObject
{
    Q_OBJECT

    QNetworkAccessManager qnam;
    QNetworkReply* reply;
    void send(QString url, QMap<QString, QString>& posts, QStringList& files);

public:
    Sender(QObject *parent = 0);
    void sync();

private slots:
    void httpFinished();
};

#endif // SENDER_H
