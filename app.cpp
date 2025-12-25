#include "app.h"
#include "ui_app.h"

using nlohmann::json;

App::App(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::App)
{
    ui->setupUi(this);

    initUI();
    scanPort();
    initThread();
    initTimer();
    connectSignal();
}

void App::connectSignal()
{
        // 连接/断开
    connect(ui->pushButton, &QPushButton::clicked, this, [this](bool isCheck) {
        if (isCheck) {
            int index = ui->comboBox->currentIndex();
            uint32_t baud = ui->comboBox_2->currentText().toUInt();
            serial::bytesize_t bytesize = (serial::bytesize_t)ui->comboBox_3->currentText().toInt();
            serial::stopbits_t stopbits = (serial::stopbits_t)ui->comboBox_4->currentText().toInt();
            serial::parity_t parity = (serial::parity_t)ui->comboBox_5->currentIndex();
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
            ui->textBrowser->append(QString("<span style='color: #cdcdcd;'>%1</span>").arg("串口已连接"));
        } else {
            ser.close();
            setEnPortEdit(true);
            ui->pushButton->setText("打开串口");
            ui->textBrowser->append(QString("<span style='color: #cdcdcd;'>%1</span>").arg("串口关闭"));
        }
    });
    // 发送
    connect(ui->pushButton_2, &QPushButton::clicked, this, [this]() {
        if (!ser.isOpen()) {
            QMessageBox::warning(this, "发送失败", QString("串口未连接"));
            return;
        }

        std::string sendStr;
        QString displayStr;
        // ASCII
        if (ui->radioButton->isChecked()) {
            sendStr = ui->textEdit->toPlainText().toStdString();
            displayStr ="[ASC]: " + ui->textEdit->toPlainText();
        // HEX
        } else if (ui->radioButton_2->isChecked()) {
            QStringList hexStrList = ui->textEdit->toPlainText().split(" ");
            QString originalString;
            displayStr = "[HEX]:";
            for (const QString &hex : hexStrList) {
                bool ok = false;
                uchar value = hex.toUShort(&ok, 16);
                if (hex.isEmpty()) {
                    continue;
                } else if (hex.size() > 2) {
                    ok = false;
                }
                if (!ok) {
                    QMessageBox::warning(this, "发送失败", QString("发送数据有误\n[%1]无法解析为HEX").arg(hex));
                    return;
                }
                originalString.append(value);
                displayStr += QString(" %1").arg(static_cast<int>(value), 2, 16, QChar('0')).toUpper();  // "01"
            }
            sendStr = originalString.toStdString();
        }

        std::lock_guard<std::mutex> _lock(sendMutex);
        sendMsg.append(sendStr);

        QTime currentTime = QTime::currentTime();
        QString curTimeStr = currentTime.toString("hh:mm:ss.zzz");
        displayStr = curTimeStr + " 发送: <br>" + displayStr;
        ui->textBrowser->append(QString("<span style='color: blue;'>%1</span>").arg(displayStr));
    });
    // 清空
    connect(ui->pushButton_3, &QPushButton::clicked, this, [this]() {
        ui->textBrowser->clear();
    });

    connect(ui->textEdit, &QTextEdit::textChanged, this, [this]() {
        if (!ui->radioButton_2->isChecked()) {
            return;
        }
        QString text = ui->textEdit->toPlainText();
        QTextCursor cursor = ui->textEdit->textCursor();
        // HEX文本检测和格式化
        QString fText;
        QString errChar;
        char cIdx = 0;
        for (auto &c : text) {
            if (c == ' ') {
                continue;
            }
            if ((c < '0' || c > '9') &&
                (c < 'A' || c > 'F') &&
                (c < 'a' || c > 'f')) {
                errChar += c;
                QToolTip::showText(QCursor::pos(), QString("<span style='color: red;'>%1</span>").arg("[HEX]模式下，只能输入十六进制文本（错误的输入：%1）").arg(errChar), this, QRect());
                continue;
            }

            if (cIdx == 2) {
                fText += ' ';
                cIdx = 0;
            }
            fText += c.toUpper();
            cIdx += 1;
        }
        ui->textEdit->blockSignals(true);
        ui->textEdit->setText(fText);
        ui->textEdit->blockSignals(false);
        cursor.movePosition(QTextCursor::End);
        ui->textEdit->setTextCursor(cursor);
    });
}

void App::initTimer()
{
    QTimer *_time1 = new QTimer(this);
    connect(_time1, &QTimer::timeout, this, &App::update);
    _time1->start(UI_PERIOD_MS);

    serial::Timeout timeout = serial::Timeout::simpleTimeout(SERIAL_TIMEOUT_MS);
    ser.setTimeout(timeout);
}

void App::initThread()
{
    std::thread _writeThread(&App::writeSerial, this);
    std::thread _readThread(&App::readSerial, this);
    _writeThread.detach();
    _readThread.detach();
}

void App::scanPort()
{
    allPorts = serial::list_ports();
    for (auto &port : allPorts) {
        QString portName = QString::fromStdString(port.port + port.description);
        allPortsDesc.push_back(portName);
    }
    QStringList strL = QStringList::fromVector(allPortsDesc);
    ui->comboBox->clear();
    ui->comboBox->addItems(strL);
}

void App::initUI()
{
    // 设置 QTextBrowser 支持 HTML 格式
    ui->textBrowser->setHtml("");
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
        std::this_thread::sleep_for(std::chrono::milliseconds(THREAD_PERIOD_MS));
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
        std::this_thread::sleep_for(std::chrono::milliseconds(THREAD_PERIOD_MS));
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

    QString displayStr;
    // ASCII
    if (ui->radioButton->isChecked()) {
        displayStr = "[ASC]: "  + QString::fromStdString(receiveMsg);
    // HEX
    } else if (ui->radioButton_2->isChecked()) {
        displayStr = "[HEX]: "  + stringToHexStr(receiveMsg);
    }
    
    QTime currentTime = QTime::currentTime();
    QString curTimeStr = currentTime.toString("hh:mm:ss.zzz");
    displayStr = curTimeStr + " 接收: <br>" + displayStr;

    ui->textBrowser->append(QString("<span style='color: green;'>%1</span>").arg(displayStr));

    std::lock_guard<std::mutex> _lock(recMutex);
    receiveMsg.clear();
}


QString App::stringToHexStr(std::string str)
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

    return spacedHexString.toUpper();
}
