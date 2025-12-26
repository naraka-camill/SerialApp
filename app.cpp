#include "app.h"
#include "ui_app.h"

using nlohmann::json;

void to_json(json &j, const AppCfg cfg)
{
    j["serPortIdx"]  = cfg.serPortIdx;
    j["baud"]        = cfg.baud;
    j["bytesize"]    = cfg.bytesizeIdx;
    j["stopIdx"]     = cfg.stopIdx;
    j["parityIdx"]   = cfg.parityIdx;
    j["isASCII"]     = cfg.isASCII;

    qInfo("To Json is OK");
}

void from_json(const json &j, AppCfg &cfg)
{

    if (j.contains("serPortIdx"))
        j.at("serPortIdx").get_to(cfg.serPortIdx);
    if (j.contains("baud"))
        j.at("baud").get_to(cfg.baud);
    if (j.contains("bytesize"))
        j.at("bytesize").get_to(cfg.bytesizeIdx);
    if (j.contains("stopIdx"))
        j.at("stopIdx").get_to(cfg.stopIdx);
    if (j.contains("parityIdx"))
        j.at("parityIdx").get_to(cfg.parityIdx);
    if (j.contains("isASCII"))
        j.at("isASCII").get_to(cfg.isASCII);

    qInfo("From Json is OK");

}

App::App(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::App)
{
    ui->setupUi(this);

    scanPort();
    initAppCfg();
    initUI();
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

    ui->comboBox->setCurrentIndex(appCfg.serPortIdx);
    ui->comboBox_2->setCurrentText(QString::number(appCfg.baud));
    ui->comboBox_3->setCurrentIndex(appCfg.bytesizeIdx);
    ui->comboBox_4->setCurrentIndex(appCfg.stopIdx);
    ui->comboBox_5->setCurrentIndex(appCfg.parityIdx);
    ui->radioButton->setChecked(appCfg.isASCII);
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
    updateUI();
    autosave();
}

void App::updateUI()
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
        displayStr = "[ASC]: " + QString::fromStdString(receiveMsg);
        // HEX
    } else if (ui->radioButton_2->isChecked()) {
        displayStr = "[HEX]: " + stringToHexStr(receiveMsg);
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

void App::initAppCfg()
{
    std::ifstream ifile(CONFIG_PATH);
    if (!ifile.is_open()) {
        autosave(true);
        return;
    }
    json j;
    try {
        j = json::parse(ifile);
    } catch(const nlohmann::json::exception& e) {
        QMessageBox::critical(this, "配置文件出错",QString("配置文件可能被外部修改出错, 已恢复默认配置\n(错误代码：%1)").arg(e.what()));
        autosave(true);
    }
    ifile.close();

    if (!j.is_null()) {
        appCfg = j;
    }
}

void App::autosave(bool isForce)
{
    static int cnt = 0;
    static AppCfg _prev = appCfg;
    
    if (isForce) {
        goto FORCE_SAVE;
    }

    if (++cnt <= AUTO_SAVE_PERIOD_MS_CNT) {
        return;
    }

FORCE_SAVE:
    cnt = 0;
    appCfg.serPortIdx = ui->comboBox->currentIndex();
    appCfg.baud = ui->comboBox_2->currentText().toInt();
    appCfg.bytesizeIdx = ui->comboBox_3->currentIndex();
    appCfg.stopIdx = ui->comboBox_4->currentIndex();
    appCfg.parityIdx = ui->comboBox_5->currentIndex();
    appCfg.isASCII = ui->radioButton->isChecked();

    if (_prev == appCfg) {
        return;
    }
    _prev = appCfg;

    json j = appCfg;
    std::ofstream ofile(CONFIG_PATH);
    if (ofile.is_open()) {
        ofile  << j/*.dump(4)*/ << std::endl;  
        ofile.close();
    }
}