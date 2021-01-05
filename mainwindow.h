#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <server.h>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
signals:
     void avail_PM_changed(const quint8 pm_no);


private:
    Ui::MainWindow *ui;
    Log Serverlog{true, "IPbusServer"};
    FEE T0;
    Server IPbusServer{&T0, &Serverlog};
    QString message_to_window = "";

private slots:
    void logHandler(QString message);
    void textEditshow();
};
#endif // MAINWINDOW_H
