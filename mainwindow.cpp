#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <iostream>
#include <QIntValidator>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QIntValidator* intValidator = new QIntValidator(this);
    ui->lineEdit->setValidator(intValidator);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_lineEdit_returnPressed()
{
    QString enteredText = ui->lineEdit->text();
    Loadability::setPeriod(enteredText.toInt());
}

QFont fontErrorText("Arial", 12);
QFont fontDesription("Arial", 5);
QFont fontWait("Arial", 30);

void MainWindow::paintEvent(QPaintEvent *event){
    QPainter painter(this);

//    // 0 ram, 1- vram, 2 - cpu, 3 -
    auto generelInfo = Loadability::getResources();
    if (generelInfo.empty()) {
        painter.setFont(fontWait);
        painter.setPen(Qt::blue);
        QFontMetrics fm(fontWait);

        int x = (width() - fm.horizontalAdvance("Wait gaining the data")) / 2;
        int y = (height() + fm.height()) / 2;
        painter.drawText(x, y, "Wait gaining the data");
        return;
    }

    update();
//    std::cout << generelInfo[0] << std::endl;
//    std::cout << generelInfo[1] << std::endl;
//    std::cout << generelInfo[2] << std::endl;
//    std::cout << generelInfo[3] << std::endl;

//     Memory RAM
    for (int i = 0; i < generelInfo.size(); i++)
    {
        if (Loadability::getOrderAndVisibOfInfo()[i].second) {
            if (generelInfo[i] != -1.0)
            {
                int height = (generelInfo[i] * 200) / 100;
                painter.setBrush(Qt::red);
                painter.drawRect(QRect(200 * (i + 1), 120, 50, 200));

                painter.setBrush(Qt::gray);
                painter.drawRect(QRect(200 * (i + 1), 120, 50, height));
            } else {
                painter.setFont(fontErrorText);
                painter.setPen(Qt::blue);

                QPointF position(170 * (i + 1), 120);
                painter.drawText(position, "Cannot get " + QString::fromStdString(Loadability::getOrderAndVisibOfInfo()[i].first));
            }
        }
    }
}

void MainWindow::on_checkBox_0_stateChanged(int arg1)
{
    arg1 ? Loadability::getOrderAndVisibOfInfo()[0].second = true : Loadability::getOrderAndVisibOfInfo()[0].second = false;
}


void MainWindow::on_checkBox_1_stateChanged(int arg1)
{
    arg1 ? Loadability::getOrderAndVisibOfInfo()[1].second = true : Loadability::getOrderAndVisibOfInfo()[1].second = false;
}


void MainWindow::on_checkBox2_stateChanged(int arg1)
{
    arg1 ? Loadability::getOrderAndVisibOfInfo()[2].second = true : Loadability::getOrderAndVisibOfInfo()[2].second = false;
}

void MainWindow::on_checkBox_3_stateChanged(int arg1)
{
    arg1 ? Loadability::getOrderAndVisibOfInfo()[3].second = true : Loadability::getOrderAndVisibOfInfo()[3].second = false;
}
