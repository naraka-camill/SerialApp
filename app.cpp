#include "app.h"
#include "ui_app.h"

App::App(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::App)
{
    ui->setupUi(this);

    allPorts = serial::list_ports();
    for (auto &port : allPorts) {
        QString portName = QString::fromStdString(port.port + port.description);
        allPortsDesc.push_back(portName);
    }
    QStringList strL = QStringList::fromVector(allPortsDesc);
    ui->comboBox->addItems(strL);

    // connect(ui->comboBox, QOverload<int>::of(&QComboBox::activated), this, [=](int index){
    //     ser.setPort(allPorts[index].port);
    // });
    std::thread _writeThread(&App::writeSerial, this);
    std::thread _readThread(&App::readSerial, this);
    _writeThread.detach();
    _readThread.detach();
    QTimer *_time1 = new QTimer(this);
    connect(_time1, &QTimer::timeout, this, &App::update);
    _time1->start(100);

    serial::Timeout timeout = serial::Timeout::simpleTimeout(100);
    ser.setTimeout(timeout);

    connect(ui->pushButton, &QPushButton::clicked, this, [this](bool isCheck){
        if (isCheck) {
            int index = ui->comboBox->currentIndex();
            uint32_t baud = ui->comboBox_2->currentText().toUInt();
            serial::bytesize_t bytesize = (serial::bytesize_t)ui->comboBox_3->currentText().toInt();
            serial::stopbits_t stopbits = (serial::stopbits_t)ui->comboBox_4->currentText().toInt();
            serial::parity_t parity = (serial::parity_t)ui->comboBox_4->currentIndex();
            ser.setPort(allPorts[index].port);
            ser.setBaudrate(baud);
            ser.setBytesize(bytesize);
            ser.setStopbits(stopbits);
            ser.setParity(parity);
            ser.open();
            setEnPortEdit(false);
        } else {
            ser.close();
            setEnPortEdit(true);
        }
    });
    connect(ui->pushButton_2, &QPushButton::clicked, this, [this](bool isCheck){
        std::lock_guard<std::mutex> _lock(sendMutex);
        sendMsg.append(ui->textEdit->toPlainText().toStdString());
        ui->textBrowser->append(ui->textEdit->toPlainText());
    });

}

App::~App()
{
    delete ui;
}

void App::setEnPortEdit(bool isEn)
{
    ui->comboBox->setEnabled(isEn);
    ui->comboBox_2->setEnabled(isEn);
    ui->comboBox_3->setEnabled(isEn);
    ui->comboBox_4->setEnabled(isEn);
    ui->comboBox_5->setEnabled(isEn);
}

void App::writeSerial()
{
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (!ser.isOpen()) {
            continue;
        }
        if (sendMsg.empty()) {
            continue;
        }
        std::lock_guard<std::mutex> _lock(sendMutex);
        ser.write(sendMsg);
        sendMsg.clear();
    }
}

void App::readSerial()
{
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (!ser.isOpen()) {
            continue;
        }
        std::lock_guard<std::mutex> _lock(recMutex);
        receiveMsg.append(ser.read(1024));
    }
}

void App::update()
{
    static bool isSerOpen = false;
    if (ser.isOpen() && !isSerOpen) {
        isSerOpen = true;
        qInfo("Serial open");
    } else if (!ser.isOpen() && isSerOpen) {
        isSerOpen = false;
        qInfo("Serial close");
    }
    if (receiveMsg.empty()) {
        return;
    }

    ui->textBrowser->append(QString::fromStdString(receiveMsg));
    std::lock_guard<std::mutex> _lock(recMutex);
    receiveMsg.clear();
}
