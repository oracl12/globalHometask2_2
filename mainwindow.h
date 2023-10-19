#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPainter>
#include "loadability.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    virtual void paintEvent(QPaintEvent *event);


    int defaultPeriod = 5;
    bool defaultCpuStatus = true;
    bool defaultMemory = true;
    bool defaultVMemory = true;
    bool defaultNetwork = true;
private slots:
    void on_lineEdit_returnPressed();

    void on_checkBox2_stateChanged(int arg1);

    void on_checkBox_0_stateChanged(int arg1);

    void on_checkBox_1_stateChanged(int arg1);

    void on_checkBox_3_stateChanged(int arg1);

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
