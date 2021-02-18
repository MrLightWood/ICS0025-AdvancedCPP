#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->w_ExitButton, &QPushButton::clicked, this, &MainWindow::exit);
    connect(ui->w_ConnectButton, &QPushButton::clicked, this, &MainWindow::establishConnection);
    ui->w_ComputeButton->setEnabled(false);
    ui->w_BreakButton->setEnabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::exit()
{
    emit signalStopConnection();
    QApplication::exit();
}

void MainWindow::drawInfo(QString info)
{
    ui->w_TextBrowser->setText(info);
}

void MainWindow::drawSocketError(QString info)
{
    ui->w_TextBrowser->setText(info);
    ui->w_ConnectButton->setEnabled(true);
}


void MainWindow::drawCalculationError(QString info)
{
    ui->w_TextBrowser->setText("Calculation has failed. The reason is: " + info + ". Please, provide another arguments");
    ui->w_ComputeButton->setEnabled(true);
}

void MainWindow::slotConnectedToSocket()
{
    ui->w_TextBrowser->setText("Connected to server");
    ui->w_ConnectButton->setEnabled(false);
    ui->w_ComputeButton->setEnabled(true);
    ui->w_BreakButton->setEnabled(true);
}

void MainWindow::slotDisconnectedFromSocket()
{
    ui->w_TextBrowser->setText("Disconnected from server");
    ui->w_ConnectButton->setEnabled(true);
    ui->w_ComputeButton->setEnabled(false);
    ui->w_BreakButton->setEnabled(false);
}

void MainWindow::slotStartWriting()
{

    bool functionOk, orderOk, pointsOk, x0Ok, xnOk;
    plotOrder = ui->w_OrderLineEdit->text().toInt(&orderOk);
    plotPoints = ui->w_PointsLineEdit->text().toInt(&pointsOk);
    plotX0 = ui->w_X0LineEdit->text().toDouble(&x0Ok);
    plotXn = ui->w_XnLineEdit->text().toDouble(&xnOk);
    plotFunction = ui->w_FunctionComboBox->currentText();

    QStringList list;
    for (int index = 0; index < ui->w_FunctionComboBox->count(); index++)
    {
        list << ui->w_FunctionComboBox->itemText(index);
    }

    for(auto it : list){
        if(it == plotFunction)
        {
            functionOk = true;
            break;
        }
        functionOk = false;
    }

    if(orderOk && pointsOk && x0Ok && xnOk && functionOk){
        ui->w_ComputeButton->setEnabled(false);
        emit signalStartedWriting(plotFunction, plotPoints, plotOrder, plotX0, plotXn);
    } else {
        drawInfo("The numerical arguments are wrong. Please, provide only numbers");
    }
}

void MainWindow::drawPlot(QVector<double> dataX, QVector<double> dataY)
{
    ui->w_Plot->addGraph();
    ui->w_Plot->graph(0)->setData(dataX, dataY);
    ui->w_Plot->xAxis->setLabel("x");
    ui->w_Plot->yAxis->setLabel("y");
    // set axes ranges, so we see all data:

    double yMin = *std::min_element(dataY.constBegin(), dataY.constEnd());
    double yMax = *std::max_element(dataY.constBegin(), dataY.constEnd());

    ui->w_Plot->xAxis->setRange(plotX0, plotXn);
    ui->w_Plot->yAxis->setRange(yMin, yMax);
    ui->w_Plot->replot();
    ui->w_ComputeButton->setEnabled(true);
    qDebug("Hi");
}

void MainWindow::establishConnection()
{
    ui->w_TextBrowser->setText("Attempting to connect to the server");
    ui->w_ConnectButton->setEnabled(false);
    m_pSocket = new MySocket();
    m_pSocketThread = new QThread();
    m_pSocket->moveToThread(m_pSocketThread);
    //m_pSocket->socket->moveToThread(m_pSocketThread);

    connect(m_pSocketThread, &QThread::started, m_pSocket, &MySocket::slotStart, Qt::DirectConnection);
    connect(m_pSocketThread, &QThread::finished, m_pSocket, &QObject::deleteLater);
    connect(m_pSocket, &MySocket::signalConnected, this, &MainWindow::slotConnectedToSocket, Qt::QueuedConnection);
    connect(m_pSocket, &MySocket::signalDisconnected, this, &MainWindow::slotDisconnectedFromSocket, Qt::QueuedConnection);
    connect(ui->w_ComputeButton, &QPushButton::clicked, this, &MainWindow::slotStartWriting, Qt::QueuedConnection);
    connect(ui->w_BreakButton, &QPushButton::clicked, m_pSocket, &MySocket::slotDisconnect, Qt::QueuedConnection);
    connect(m_pSocket, &MySocket::signalReadSuccessful, this, &MainWindow::drawPlot, Qt::QueuedConnection);
    connect(m_pSocket, &MySocket::signalCalculationFailed, this, &MainWindow::drawCalculationError, Qt::QueuedConnection);
    connect(m_pSocket, &MySocket::signalInfo, this, &MainWindow::drawInfo, Qt::QueuedConnection);

    connect(m_pSocket, &MySocket::signalError, this, &MainWindow::drawSocketError, Qt::QueuedConnection);
    connect(this, &MainWindow::signalStartedWriting, m_pSocket, &MySocket::slotWrite, Qt::QueuedConnection);
    connect(this, &MainWindow::signalStopConnection, m_pSocket, &MySocket::slotDisconnect, Qt::QueuedConnection);

    m_pSocketThread->start();
}
