#ifndef APP_H
#define APP_H

#include <QWidget>
#include <QVector>
#include <QTimer>
#include <QMessageBox>
#include <QDateTime>

#include <vector>
#include <thread>
#include <mutex>

#include "3rdParty/serial/serial.h"


#define THREAD_PERIOD_MS  (10)
#define UI_PERIOD_MS      (20)
#define SERIAL_TIMEOUT_MS (100)


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

    void setEnPortEdit(bool isEn);
    void writeSerial();
    void readSerial();
    void update();
    QString stringToHexStr(std::string str);

};
#endif // APP_H
