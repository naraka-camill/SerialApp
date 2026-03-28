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
#include "shortcuts_dialog.h"


#define THREAD_PERIOD_MS     (20)
#define UI_PERIOD_MS         (20)
#define SERIAL_TIMEOUT_MS    (20)
#define AUTO_SAVE_PERIOD_MS_CNT  (1000 / UI_PERIOD_MS)  // 1s
#define CONFIG_PATH (".cfg")
#define SHORTCUTS_PATH (".shortcuts")


struct AppCfg
{
    std::string serPort = "";
    int baud = 19200;
    int bytesizeIdx = 0;
    int stopIdx = 0;
    int parityIdx = 0;
    bool isASCII = true;
    
    bool operator==(const AppCfg &other) {
        return (
            this->serPort == other.serPort &&
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
    
    std::mutex serMutex;
    std::mutex recMutex;
    std::string receiveMsg;
    std::mutex sendMutex;
    std::string sendMsg;

    AppCfg appCfg;
    QList<ShortcutCommand> shortcuts;

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
    void loadShortcuts();
    void saveShortcuts();
    void sendShortcutCommand(const ShortcutCommand &cmd);
    void createShortcutButtons();

private slots:
    void on_pushButton_shortcuts_clicked();

};
#endif // APP_H
