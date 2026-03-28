#include "app.h"
#include "ui_app.h"

using nlohmann::json;

void to_json(json &j, const AppCfg cfg)
{
    j["serPort"]     = cfg.serPort;
    j["baud"]        = cfg.baud;
    j["bytesize"]    = cfg.bytesizeIdx;
    j["stopIdx"]     = cfg.stopIdx;
    j["parityIdx"]   = cfg.parityIdx;
    j["isASCII"]     = cfg.isASCII;

    qInfo("To Json is OK");
}

void from_json(const json &j, AppCfg &cfg)
{

    if (j.contains("serPort"))
        j.at("serPort").get_to(cfg.serPort);
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
    loadShortcuts();
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
            ui->pushButton->setChecked(false);
            // 检查串口配置是否有效（是否为空）
            if (ui->comboBox->currentText().isEmpty()) {
                QMessageBox::warning(this, "串口打开失败", QString("串口号为空！请填入串口号再试"));
                return;
            }
            
            if (ser.isOpen()) {
                qInfo("Serial already open");
                return;
            }
            
            int index = ui->comboBox->currentIndex();
            uint32_t baud = ui->comboBox_2->currentText().toUInt();
            serial::bytesize_t bytesize = (serial::bytesize_t)ui->comboBox_3->currentText().toInt();
            serial::stopbits_t stopbits = (serial::stopbits_t)ui->comboBox_4->currentText().toInt();
            serial::parity_t parity = (serial::parity_t)ui->comboBox_5->currentIndex();
            serial::Timeout timeout = serial::Timeout::simpleTimeout(SERIAL_TIMEOUT_MS);
            ser.setPort(allPorts[index].port);
            ser.setBaudrate(baud);
            ser.setBytesize(bytesize);
            ser.setStopbits(stopbits);
            ser.setParity(parity);
            ser.setTimeout(timeout);

            try {
                ser.open();
            } catch(const serial::IOException &e) {
                ui->pushButton->setChecked(false);
                QMessageBox::warning(this, "串口连接出错", QString("串口可能被占用, 错误代码:\n%1").arg(e.what()));
                return;
            }

            qInfo("Serial open");
            ui->pushButton->setText("关闭串口");
            setEnPortEdit(false);
            ui->textBrowser->append(QString("<span style='color: #cdcdcd;'>%1</span>").arg("串口已连接"));
            ui->pushButton->setChecked(true);
        } else {
            ui->pushButton->setChecked(true);

            if (ser.isOpen()) {
                std::lock_guard<std::mutex> _lock(serMutex);
                ser.close();
            }
            
            setEnPortEdit(true);
            qInfo("Serial close");
            ui->pushButton->setText("打开串口");
            ui->textBrowser->append(QString("<span style='color: #cdcdcd;'>%1</span>").arg("串口关闭"));
            ui->pushButton->setChecked(false);
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
        QString text = ui->textEdit->toPlainText();
        if (!text.isEmpty() && text.back().cell() == '\n') {
            ui->pushButton_2->click();
        }
        if (!ui->radioButton_2->isChecked()) {
            return;
        }
        QTextCursor cursor = ui->textEdit->textCursor();
        // HEX文本检测和格式化
        QString fText;
        QString errChar;
        char cIdx = 0;
        for (auto &c : text) {

            if (c == ' ') {
                continue;
            } else if ((c < '0' || c > '9') &&
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
        QString portName = QString::fromStdString(port.port);
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

    ui->comboBox->setCurrentText(QString::fromStdString(appCfg.serPort));
    ui->comboBox_2->setCurrentText(QString::number(appCfg.baud));
    ui->comboBox_3->setCurrentIndex(appCfg.bytesizeIdx);
    ui->comboBox_4->setCurrentIndex(appCfg.stopIdx);
    ui->comboBox_5->setCurrentIndex(appCfg.parityIdx);
    ui->radioButton->setChecked(appCfg.isASCII);
}

App::~App()
{
    qInfo("Delete Serial App");
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
        if (!ser.isOpen()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(THREAD_PERIOD_MS));
            continue;
        }
        if (sendMsg.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(THREAD_PERIOD_MS));
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
        std::string _recvStr;
        {
            std::lock_guard<std::mutex> _lock(serMutex);
            _recvStr = ser.read(200);
        }
            if (_recvStr.empty()) {
            continue;
        }
        {
            std::lock_guard<std::mutex> _lock(recMutex);
            receiveMsg.append(_recvStr);
        }
    }
}

void App::update()
{
    updateUI();
    autosave();
}

void App::updateUI()
{
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

/// @brief 当配置被修改后，自动保存
/// @param isForce 强制保存（缺省默认为false）
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
    appCfg.serPort = ui->comboBox->currentText().toStdString();
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

void App::on_pushButton_shortcuts_clicked()
{
    ShortcutsDialog dialog(this);
    dialog.setShortcuts(shortcuts);
    if (dialog.exec() == QDialog::Accepted) {
        shortcuts = dialog.getShortcuts();
        saveShortcuts();
        createShortcutButtons();
    }
}

void App::loadShortcuts()
{
    std::ifstream ifile(SHORTCUTS_PATH);
    if (!ifile.is_open()) {
        return;
    }

    json j;
    try {
        j = json::parse(ifile);
    } catch(const nlohmann::json::exception& e) {
        qInfo("Failed to parse shortcuts file: %s", e.what());
        ifile.close();
        return;
    }
    ifile.close();

    shortcuts.clear();
    if (j.is_array()) {
        for (const auto& item : j) {
            ShortcutCommand cmd;
            if (item.contains("name")) cmd.name = QString::fromStdString(item["name"]);
            if (item.contains("type")) cmd.type = QString::fromStdString(item["type"]);
            if (item.contains("data")) cmd.data = QString::fromStdString(item["data"]);
            shortcuts.append(cmd);
        }
    }
}

void App::saveShortcuts()
{
    json j = json::array();
    for (const auto& cmd : shortcuts) {
        json item;
        item["name"] = cmd.name.toStdString();
        item["type"] = cmd.type.toStdString();
        item["data"] = cmd.data.toStdString();
        j.push_back(item);
    }

    std::ofstream ofile(SHORTCUTS_PATH);
    if (ofile.is_open()) {
        ofile << j.dump(4) << std::endl;
        ofile.close();
    }
}

void App::sendShortcutCommand(const ShortcutCommand &cmd)
{
    if (!ser.isOpen()) {
        QMessageBox::warning(this, "发送失败", QString("串口未连接"));
        return;
    }

    std::string sendStr;
    QString displayStr;
    // ASCII
    if (cmd.type == "ASCII") {
        sendStr = cmd.data.toStdString();
        displayStr = "[ASC]: " + cmd.data;
    // HEX
    } else if (cmd.type == "HEX") {
        QStringList hexStrList = cmd.data.split(" ");
        QString originalString;
        displayStr = "[HEX]";
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
            displayStr += QString(" %1").arg(static_cast<int>(value), 2, 16, QChar('0')).toUpper();
        }
        sendStr = originalString.toStdString();
    }

    std::lock_guard<std::mutex> _lock(sendMutex);
    sendMsg.append(sendStr);

    QTime currentTime = QTime::currentTime();
    QString curTimeStr = currentTime.toString("hh:mm:ss.zzz");
    displayStr = curTimeStr + " 发送: <br>" + displayStr;
    ui->textBrowser->append(QString("<span style='color: blue;'>%1</span>").arg(displayStr));
}

void App::createShortcutButtons()
{
    // 清空现有的按钮
    QLayoutItem *item;
    while ((item = ui->verticalLayout_shortcuts->takeAt(0)) != nullptr) {
        QWidget *widget = item->widget();
        if (widget) {
            widget->deleteLater();
        }
        delete item;
    }

    // 为每个快捷指令创建按钮
    for (int i = 0; i < shortcuts.size(); ++i) {
        const ShortcutCommand &cmd = shortcuts[i];
        QPushButton *button = new QPushButton(cmd.name, this);
        button->setToolTip(QString("类型: %1\n数据: %2").arg(cmd.type).arg(cmd.data));
        
        // 连接按钮点击信号
        connect(button, &QPushButton::clicked, this, [this, cmd]() {
            sendShortcutCommand(cmd);
        });
        
        ui->verticalLayout_shortcuts->addWidget(button);
        qInfo("Created shortcut button: %s, type: %s, data: %s", cmd.name.toStdString().c_str(), cmd.type.toStdString().c_str(), cmd.data.toStdString().c_str());
    }

    qInfo("Shortcuts updated: %d commands", shortcuts.size());
}