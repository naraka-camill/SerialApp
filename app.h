#ifndef APP_H
#define APP_H

#include <QWidget>
#include <QVector>
#include <QTimer>
#include <QMessageBox>
#include <QDateTime>
#include <QEvent>
#include <QToolTip>

#include <vector>
#include <thread>
#include <mutex>
#include <fstream>
#include <iostream>

#include "3rdParty/serial/serial.h"
#include "3rdParty/nlohmann/json.hpp"


#define THREAD_PERIOD_MS     (10)
#define UI_PERIOD_MS         (20)
#define SERIAL_TIMEOUT_MS    (100)
#define AUTO_SAVE_PERIOD_MS_CNT  (1000 / UI_PERIOD_MS)
#define CONFIG_PATH (".cfg")


struct AppCfg
{
    int serPortIdx = 0;
    int baud = 19200;
    int bytesizeIdx = 0;
    int stopIdx = 0;
    int parityIdx = 0;
    bool isASCII = true;
    
    bool operator==(const AppCfg &other) {
        return (
            this->serPortIdx == other.serPortIdx &&
            this->baud == other.baud &&
            this->bytesizeIdx == other.bytesizeIdx &&
            this->stopIdx == other.stopIdx &&
            this->parityIdx == other.parityIdx &&
            this->isASCII == other.isASCII 
        );
    }

    bool operator!=(const AppCfg &other) {
        return !(*this == other);
    }
};



QT_BEGIN_NAMESPACE
namespace Ui {
class App;
}
QT_END_NAMESPACE

class App : public QWidget
{
    Q_OBJECT

public:
    App(QWidget *parent = nullptr);
    ~App();

private:
    Ui::App *ui;
    serial::Serial ser;
    std::vector<serial::PortInfo> allPorts;
    QVector<QString> allPortsDesc;
    
    std::mutex recMutex;
    std::string receiveMsg;
    std::mutex sendMutex;
    std::string sendMsg;

    AppCfg appCfg;

    void initUI();
    void initTimer();
    void initThread();
    void initAppCfg();
    void scanPort();
    void update();
    void updateUI();
    void connectSignal();
    void autosave(bool isForce = false);

    void setEnPortEdit(bool isEn);
    void writeSerial();
    void readSerial();
    QString stringToHexStr(std::string str);

};
#endif // APP_H
