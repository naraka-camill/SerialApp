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

    // 连接/断开
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
            try {
                ser.open();
            } catch(const serial::IOException &e) {
                QMessageBox::warning(this, "串口连接出错", QString("串口可能被占用, 错误代码:\n%1").arg(e.what()));
                return;
            }
            ui->pushButton->setText("关闭串口");
            setEnPortEdit(false);
        } else {
            ser.close();
            setEnPortEdit(true);
            ui->pushButton->setText("打开串口");
        }
    });
    // 发送
    connect(ui->pushButton_2, &QPushButton::clicked, this, [this](bool isCheck){
        if (!ser.isOpen()) {
            QMessageBox::warning(this, "发送失败", QString("串口未连接"));
            return;
        }
        std::string data;
        
        // ASCII
        if (ui->radioButton->isChecked()) {
            data = ui->textEdit->toPlainText().toStdString();
        // HEX
        } else if (ui->radioButton_2->isChecked()) {
            QStringList hexStrList = ui->textEdit->toPlainText().split(" ");
            QString originalString;
            for (const QString &hex : hexStrList) {
                bool ok;
                char value = hex.toInt(&ok, 16);
                if (!ok) {
                    QMessageBox::warning(this, "发送失败", QString("发送数据有误"));
                    return;
                }
                originalString.append(value);
            }
            data = originalString.toStdString();
        } else {
            data = ui->textEdit->toPlainText().toStdString();
        }
        std::lock_guard<std::mutex> _lock(sendMutex);
        sendMsg.append(data);
        ui->textBrowser->append(ui->textEdit->toPlainText());
    });
    // 清空
    connect(ui->pushButton_3, &QPushButton::clicked, this, [this](bool isCheck){
        ui->textBrowser->clear();
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
    // ASCII
    if (ui->radioButton->isChecked()) {
        ui->textBrowser->append(QString::fromStdString(receiveMsg));
    // HEX
    } else if (ui->radioButton_2->isChecked()) {
        stringToHexStr(receiveMsg);
    }
    
    std::lock_guard<std::mutex> _lock(recMutex);
    receiveMsg.clear();
}

void App::stringToHexStr(std::string str)
{
    QString data = QString::fromStdString(str);
    QByteArray array = data.toUtf8();
    QString hexString = array.toHex();
    QString spacedHexString;
    for (int i = 0; i < hexString.size(); i += 2) {
        spacedHexString.append(hexString.mid(i, 2));
        if (i < hexString.size() - 2) {
            spacedHexString.append(" ");
        }
    }

    ui->textBrowser->append(spacedHexString);
}
