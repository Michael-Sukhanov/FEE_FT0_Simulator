#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    Serverlog.setSaveMode(ui->checkBox_Save->isChecked());;
    setWindowTitle(QCoreApplication::applicationName() + " v" + QCoreApplication::applicationVersion() + " alpha");
    QString green("background-color:rgb(127,233,127)"), red("background-color:rgb(234,65,85)");
    QList<QPushButton *> PM_select_list = ui->centralwidget->findChildren<QPushButton *>(QRegularExpression("pushButton_PM..."));
    connect(&Serverlog, &Log::LogMessage, this, &MainWindow::logHandler);
    connect(&IPbusServer, &Server::response_ready, this, &MainWindow::textEditshow);
    connect(this, &MainWindow::avail_PM_changed, &T0, &FEE::ChangeAvailablePMs);
    connect(ui->checkBox_Save, &QCheckBox::clicked, this, [=](bool state){Serverlog.setSaveMode(state);});
    connect(ui->checkBox_READONLY_OFF, &QCheckBox::clicked, &T0, &FEE::setReadonly);
    connect(&IPbusServer, &Server::packet_recieved, this, [=](QHostAddress senderAddress, quint16 senderPort){
        ui->label_lastConnection->setText(QString::asprintf("Last request from: %s:%u", senderAddress.toString().toLocal8Bit().data(), senderPort));});
    connect(ui->pushButton_Bind, &QPushButton::clicked, this, [=](bool checked){
        if(checked){
            ui->pushButton_Bind->setChecked(IPbusServer.start());
            if(ui->pushButton_Bind->isChecked())ui->pushButton_Bind->setText("Stop");
        }else{
            IPbusServer.stop();
            ui->pushButton_Bind->setText("Bind");}});
    foreach(QPushButton* but, PM_select_list){
        but->setFont(QFont("Arial", 12, 87));
        but->setStyleSheet(but->isChecked() ? green : red);
        QString id = but->objectName().right(2);
        quint8 PMn0 = id.right(1).toUInt() + (id.at(0) == 'A' ? 0 : 10);
        but->setText(but->objectName().right(2));
        if(but->isChecked())    emit avail_PM_changed(PMn0);
//        qDebug() << T0.getavailablePM();
        connect(but, &QPushButton::clicked, this, [=](bool checked){
            but->setStyleSheet(checked ? green : red);
            Serverlog.sendMessage(QString::asprintf("PM%s %s", id.toLocal8Bit().data(), checked ? "On" : "Off"));
            emit avail_PM_changed(PMn0);});
    }
    ui->pushButton_Bind->click();
    ui->textEdit_logs->setFont(QFont("Consolas", 10));
    ui->label_lastConnection->setFont(QFont("Arial", 10));
}

MainWindow::~MainWindow(){
    delete ui;
}

void MainWindow::logHandler(QString message){
    if(message.contains("reading") && ui->checkBox_writeTransaction_only->isChecked()) return;
    message_to_window.append(message + "\n");
}

void MainWindow::textEditshow(){
    if(ui->checkBox_writeTransaction_only->isChecked())
        if(!message_to_window.contains("writing") || message_to_window.contains("Status")){
            message_to_window.clear();
            return;
        }
    ui->textEdit_logs->append(message_to_window);
    message_to_window.clear();
}

