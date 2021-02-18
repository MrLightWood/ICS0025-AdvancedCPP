#ifndef MYSOCKET_H
#define MYSOCKET_H

#include <QObject>
#include <QString>
#include <QThread>
#include <QDebug>
#include <QWaitCondition>
#include <QMutex>
#include <QLocalSocket>
#include <QByteArray>
#include <QTextCodec>
#include <QDataStream>
#include <QTextBrowser>

class MySocket:public QObject
{
Q_OBJECT
public:
    MySocket(QObject *parent = nullptr):QObject(parent){};
    ~MySocket();

private:
    QString pipePath = "\\\\.\\pipe\\ICS0025";
    QLocalSocket *socket;

    QString plotFunction;
    int plotPoints, plotOrder;
    double plotX0, plotXn;

public slots:
    void slotStart();
    void slotWrite(QString, int, int, double, double);
    void slotRead();
    void slotDisconnect();
    void slotError(QLocalSocket::LocalSocketError error);
signals:
    //void slotConnected(QString info = "Connected");
    void signalReadSuccessful(QVector<double>, QVector<double>);
    void signalCalculationFailed(QString);
    void signalInfo(QString);
    void signalError(QString);
    void signalConnected();
    void signalDisconnected();

private slots:
    QByteArray storeStringToByteArray(QString);
};

#endif // MYSOCKET_H
