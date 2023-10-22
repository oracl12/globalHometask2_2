#include "mainwindow.h"
#include "./ui_mainwindow.h"

static const QFont fontErrorText("Arial", 12);
static const QFont fontDesription("Arial", 10);
static const QFont fontWait("Arial", 30);

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    QIntValidator* intValidator = new QIntValidator(this);
    ui->lineEdit->setValidator(intValidator);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_lineEdit_returnPressed() {
    QString enteredText = ui->lineEdit->text();
    Loadability::setPeriod(enteredText.toInt());
}

void MainWindow::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    std::vector<double> generelInfo = Loadability::getResources();;     // 0 ram, 1- vram, 2 - cpu, 3 - network

    if (generelInfo.empty()) {
        drawWaitMessage(painter);
        std::cout << "Empty" << std::endl;
        return;
    }

    update();

    for (int i = 0; i < generelInfo.size() - 1; i++) {
        if (Loadability::getOrderAndVisibOfInfo()[i].second) {
            drawResourceBar(painter, generelInfo[i], i);
        }
    }

    if (Loadability::getOrderAndVisibOfInfo()[3].second) {
        drawNetworkInfo(painter, generelInfo[3]);
    }
}

void MainWindow::drawWaitMessage(QPainter& painter) {
    painter.setFont(fontWait);
    painter.setPen(Qt::blue);
    QFontMetrics fm(fontWait);

    int x = (width() - fm.horizontalAdvance("Wait gaining the data")) / 2;
    int y = (height() + fm.height()) / 2;
    painter.drawText(x, y, "Wait gaining the data");
}

void MainWindow::drawResourceBar(QPainter& painter, double value, int index) {
    if (value != -1.0) {
        int height = (value * 200) / 100;
        int x = 200 * (index + 1) + (50 * index);
        QRect barRect(x, 120, 50, 200);

        painter.setBrush(Qt::red);
        painter.drawRect(barRect);

        painter.setBrush(Qt::gray);
        painter.drawRect(barRect.adjusted(0, 0, 0, -200 + height));
    } else {
        painter.setFont(fontErrorText);
        painter.setPen(Qt::blue);

        QPointF position(170 * (index + 1) + (50 * index), 120);
        painter.drawText(position, "Cannot get " + QString::fromStdString(Loadability::getOrderAndVisibOfInfo()[index].first));
    }
}

void MainWindow::drawNetworkInfo(QPainter& painter, double value) {
    QPointF position(170 * 3 + 90 + (50 * 5), 120);

    if (value != -1.0) {
        painter.setFont(fontDesription);
        painter.setPen(Qt::black);

#ifdef __linux__
        std::string metric = " KB";
#else
        std::string metric = " packets";
#endif

        painter.drawText(position, QString::fromStdString(std::to_string((int)value) + metric + "/second"));
    } else {
        painter.setFont(fontErrorText);
        painter.setPen(Qt::blue);

        painter.drawText(position, "Cannot get " + QString::fromStdString(Loadability::getOrderAndVisibOfInfo()[3].first));
    }
}

void MainWindow::updateCheckBoxState(int index, int arg1) {
    arg1 ? Loadability::getOrderAndVisibOfInfo()[index].second = true : Loadability::getOrderAndVisibOfInfo()[index].second = false;
}

void MainWindow::on_checkBox_0_stateChanged(int arg1) {
    updateCheckBoxState(0, arg1);
}

void MainWindow::on_checkBox_1_stateChanged(int arg1) {
    updateCheckBoxState(1, arg1);
}

void MainWindow::on_checkBox2_stateChanged(int arg1) {
    updateCheckBoxState(2, arg1);
}

void MainWindow::on_checkBox_3_stateChanged(int arg1) {
    updateCheckBoxState(3, arg1);
}
