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

#include "3rdParty/serial/serial.h"
#include "3rdParty/nlohmann/json.hpp"


#define THREAD_PERIOD_MS  (10)
#define UI_PERIOD_MS      (20)
#define SERIAL_TIMEOUT_MS (100)


struct AppCfg
{
    int serPortIdx;
    int baud;
    int bytesize;
    int stopIdx;
    int parityIdx;
    bool isASCII;
    
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
    void scanPort();
    void update();
    void connectSignal();

    void setEnPortEdit(bool isEn);
    void writeSerial();
    void readSerial();
    QString stringToHexStr(std::string str);

};
#endif // APP_H
