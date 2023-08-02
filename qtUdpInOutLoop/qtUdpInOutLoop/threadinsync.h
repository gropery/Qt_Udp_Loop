#ifndef THREADINSYNC_H
#define THREADINSYNC_H

#include <QThread>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QtNetwork>


class ThreadInSync : public QThread
{
    Q_OBJECT
public:
    ThreadInSync();
    void stopThread(); //结束线程
    bool setUdpSocket(QUdpSocket *udpSocket, QHostAddress targetAddr, quint16 targetPort);
    bool inSyncOnce(QByteArray &ba);

    long m_inBytes=0;
    long m_inSeccesses=0;
    long m_inFailures=0;

protected:
    void run() Q_DECL_OVERRIDE;

private:
    bool m_stop=false; //停止线程
    QUdpSocket *m_udpSocket;
    QHostAddress m_peerUdpAddr;   //暂存对接的地址--暂未使用
    quint16 m_peerUdpPort;        //暂存对接的端口--暂未使用
    QHostAddress m_targetAddr;    //目标IP地址
    quint16 m_targetPort;         //目标端口
};

#endif // THREADINSYNC_H
