#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "mysocket.h"
#include <algorithm>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QString plotFunction;
    int plotPoints, plotOrder;
    double plotX0, plotXn;
private slots:
    void exit();
    void establishConnection();
    void drawInfo(QString);
    void drawSocketError(QString);
    void drawCalculationError(QString);
    //void writeSomething();
    void drawPlot(QVector<double>, QVector<double>);

    void slotConnectedToSocket();
    void slotDisconnectedFromSocket();
    //void testing();
    void slotStartWriting();

private:
    Ui::MainWindow *ui;
    //Connector *m_pConnector = nullptr;
    //QThread *m_pConnectorThread = nullptr;

    MySocket *m_pSocket = nullptr;
    QThread *m_pSocketThread = nullptr;

    //Writer *m_pWriter = nullptr;
    //QThread *m_pWriterThread = nullptr;
signals:
    void setInfo();
    void signalStartedWriting(QString, int, int, double, double);
    void signalStopConnection();
    //void startConnection();
    //void startedWriting(int points, int order, double x0, double xn);
};

#endif // MAINWINDOW_H
