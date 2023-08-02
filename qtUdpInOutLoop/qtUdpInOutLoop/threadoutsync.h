#ifndef THREADOUTSYNC_H
#define THREADOUTSYNC_H

#include <QThread>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QtNetwork>


class ThreadOutSync : public QThread
{
    Q_OBJECT
public:
    ThreadOutSync();
    void stopThread(); //结束线程
    bool setUdpSocket(QUdpSocket *udpSocket, QHostAddress targetAddr, quint16 targetPort);
    bool outSyncOnce(QByteArray ba);

    long m_outBytes=0;
    long m_outSeccesses=0;
    long m_outFailures=0;

protected:
    void run() Q_DECL_OVERRIDE;

private:
    bool m_stop=false; //停止线程
    QHostAddress m_targetAddr;    //目标IP地址
    quint16 m_targetPort;         //目标端口
    QUdpSocket *m_udpSocket;


};

#endif // THREADOUTSYNC_H
