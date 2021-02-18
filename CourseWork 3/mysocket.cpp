#include "mysocket.h"

QByteArray MySocket::storeStringToByteArray(QString str)
{
    QByteArray result;
    for(int i = 0;i < str.size(); i++)
    {
        QChar lol = str.at(i);
        result.append(reinterpret_cast<const char*>(&lol), sizeof(wchar_t));
    }
    result.append("\0", sizeof(wchar_t));
    return result;
}

MySocket::~MySocket()
{
    delete socket;
}

void MySocket::slotStart()
{
    qDebug() << "slotStart start";
    socket = new QLocalSocket;

    connect(socket, QOverload<QLocalSocket::LocalSocketError>::of(&QLocalSocket::error),
        this, &MySocket::slotError);
    connect(socket, SIGNAL(connected()), this, SIGNAL(signalConnected()));
    connect(socket, SIGNAL(disconnected()), this, SIGNAL(signalDisconnected()));
    connect(socket, &QLocalSocket::readyRead, this, &MySocket::slotRead);

    socket->connectToServer("\\\\.\\pipe\\ICS0025");

    qDebug() << "slotStart end";
}

void MySocket::slotDisconnect()
{
    if(socket->state() == QLocalSocket::ConnectedState)
    {
        QByteArray message;
        int msgSize;

        QByteArray baFunction = storeStringToByteArray("Stop");
        message.append(baFunction);
        QByteArray array(reinterpret_cast<const char*>(&plotX0), sizeof(plotX0));
        msgSize = message.size()+4;
        array = QByteArray(reinterpret_cast<const char*>(&msgSize), sizeof(int));
        message.prepend(array);

        socket->write(message);
        //socket->flush();
    }
    emit signalDisconnected();
}

void MySocket::slotWrite(QString func,int points, int order, double x0, double xn)
{
    qDebug() << "slotWrite start";

    plotPoints = points;
    plotOrder = order;
    plotX0 = x0;
    plotXn = xn;
    plotFunction = func;
    QByteArray message;
    message.clear();
    int msgSize = 0;

    QByteArray baFunction = storeStringToByteArray(plotFunction);
    message.append(baFunction);

    QByteArray array(reinterpret_cast<const char*>(&plotX0), sizeof(plotX0));
    message.append(array);
    array = QByteArray(reinterpret_cast<const char*>(&plotXn), sizeof(plotXn));
    message.append(array);
    array = QByteArray(reinterpret_cast<const char*>(&plotPoints), sizeof(plotPoints));
    message.append(array);
    array = QByteArray(reinterpret_cast<const char*>(&plotOrder), sizeof(plotOrder));
    message.append(array);
    msgSize = message.size()+4;
    array = QByteArray(reinterpret_cast<const char*>(&msgSize), sizeof(int));
    message.prepend(array);

    qDebug() << message.size();

    socket->write(message);

    qDebug() << "slotWrite end";
}

void MySocket::slotRead()
{
    qDebug() << "slotRead start";

    QByteArray result;
    QString message;

    result = socket->readAll();
    const wchar_t *dataChars = reinterpret_cast<const wchar_t*>(result.constData());

    for(int i = 0; i < result.size(); i++)
    {
        if(*(dataChars+2+i) == L'\0')
            break;
        message += (*(dataChars+2+i));
    }

    if(message != "Curve")
    {
        emit signalCalculationFailed(message);
        return;
    }
    emit signalInfo("Calculation was successful");

    result.remove(0, 16);
    QVector<double> coordsX, coordsY;
    QVector<double> data;
    const double *dataDoubles = reinterpret_cast<const double*>(result.constData());
    for(int i = 0; i < plotPoints*2; i++)
    {
        if(i % 2 == 0)
        {
            coordsX.push_back(*(dataDoubles+i));
            i++;
            coordsY.push_back(*(dataDoubles+i));
        }
    }

    emit signalReadSuccessful(coordsX, coordsY);

    qDebug() << "slotRead end";
}


void MySocket::slotError(QLocalSocket::LocalSocketError error)
{
    QString strError =
        "Error: " + (error == QLocalSocket::ServerNotFoundError ?
                     "The server was not found." :
                     error == QLocalSocket::PeerClosedError ?
                     "The server is closed." :
                     error == QLocalSocket::ConnectionRefusedError ?
                     "The connection was refused." :
                     QString(socket->errorString()));
    emit signalError(strError);
}
