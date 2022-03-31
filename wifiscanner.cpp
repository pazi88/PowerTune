#include "wifiscanner.h"
#include "dashboard.h"
#include "connect.h"
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QProcess>
#include <QByteArrayMatcher>
#include <QByteArray>
#include <QNetworkInterface>


QString outputline;
QStringList result;
QString eth0ip;
QString wlanip;

WifiScanner::WifiScanner(QObject *parent)
    : QObject(parent)
    , m_dashboard(Q_NULLPTR)
{
}


WifiScanner::WifiScanner(DashBoard *dashboard, QObject *parent)
    : QObject(parent)
    , m_dashboard(dashboard)
{
}


void WifiScanner::initializeWifiscanner()
{
         QTimer *timer = new QTimer(this);
       connect(timer, &QTimer::timeout, this, &WifiScanner::getconnectionStatus);
       timer->start(3000); // Check the status of the connection every 3 Seconds to
    //connect(&findTimer, &QTimer::timeout, this, &WifiScanner::getconnectionStatus);
// Check available SSID's on wlan0 by calling iw wlan0 scan |grep SSID
    result.clear();
    QString raw;
        QProcess proc2;
        proc2.start("sh", QStringList()<<"-c"<<"iw wlan0 scan |grep SSID");
        proc2.waitForFinished();
        QString output2 = proc2.readAllStandardOutput();
        qDebug() << "output" << output2;
        m_dashboard->setSerialStat(output2);
        QStringList fields = output2.split(QRegExp("[\n]"));
        foreach (const QString &str, fields) {
                raw = str;
                raw.replace("SSID: ","");
                raw.remove(0,1); // Remove the white space before the SSID
                result += raw;
        }
        m_dashboard->setwifi(result); // Pass on wifi name list to QML Combobox


}


void WifiScanner::getconnectionStatus()
{
// displays the wlan0 and eth0 IP adresses
    // Check IP Adresses direcly via QT
    qDebug() << "get connection ";
    QNetworkInterface wlan0IP = QNetworkInterface::interfaceFromName("wlan0");
    QList<QNetworkAddressEntry> entries = wlan0IP.addressEntries();
    if (!entries.isEmpty()) {
        QNetworkAddressEntry entry = entries.first();
        wlanip = entry.ip().toString();
        wlanip.replace("QHostAddress(","");
        wlanip.remove(QChar(')'));
        wlanip.remove(QChar('"'));
        m_dashboard->setSerialStat("WLAN IP Adress : " + wlanip);
    }
    else{
        m_dashboard->setSerialStat("WLAN IP Adress: no connection");
    }

    QNetworkInterface eth0IP = QNetworkInterface::interfaceFromName("eth0");
    QList<QNetworkAddressEntry> test = eth0IP.addressEntries();
    if (!test.isEmpty()) {
        QNetworkAddressEntry entry2 = test.first();
        eth0ip = entry2.ip().toString();
        eth0ip.replace("QHostAddress(","");
        eth0ip.remove(QChar(')'));
        eth0ip.remove(QChar('"'));
        m_dashboard->setSerialStat("Ethernet IP Adress: " + eth0ip);
    }
    else{
        m_dashboard->setSerialStat("Ethernet IP Adress: no connection");
    }


}



void WifiScanner::setwifi(const QString &country,const QString &ssid1,const QString &psk1,const QString &ssid2,const QString &psk2)
{
    QFile file("/etc/wpa_supplicant/wpa_supplicant.conf");
    file.remove(); //remove file if it exists to avoid appending of existing file
    if ( file.open(QIODevice::ReadWrite) )
    {
        QTextStream out(&file);

        out     << "ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev" << "\r\n"
                << "update_config=1" << "\r\n"
                << "country="+country << "\r\n"
                << "#Primary WIFI" << "\r\n"
                << "network={" << "\r\n"
                << "ssid=" << "\"" << ssid1 << "\"" << "\r\n"
                << "psk=" << "\"" << psk1  << "\"" << "\r\n"
                << "}" << "\r\n"
                   // << "#Secondary WIFI" << "\r\n"
                   // << "network={" << "\r\n"
                   // << "ssid="<< "\"" << ssid2 << "\"" << "\r\n"
                   // << "psk=" << "\"" << psk2 << "\"" << "\r\n"
                   // << "}" << "\r\n"
                << endl;
        file.close();
    }
    QProcess restartwifi;
    restartwifi.start("sh", QStringList()<<"-c"<<"/etc/init.d/networking restart");
    restartwifi.waitForFinished();

}

