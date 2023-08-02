#ifndef THREADLOOPSYNC_H
#define THREADLOOPSYNC_H

#include <QThread>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QtNetwork>


class ThreadLoopSync : public QThread
{
    Q_OBJECT
public:
    ThreadLoopSync();
    void stopThread(); //结束线程
    bool setUdpSocket(QUdpSocket *udpSocket, QHostAddress targetAddr, quint16 targetPort);

    //loop过程中in阶段
    long m_inBytes=0;
    long m_inSeccesses=0;
    long m_inFailures=0;

    //loop过程中out阶段
    long m_outBytes=0;
    long m_outSeccesses=0;
    long m_outFailures=0;

    //loop总过程
    long m_loopBytes=0;
    long m_loopSeccesses=0;
    long m_loopFailures=0;

    //PC接收到IN数据后，再发送OUT数据至FPGA的时间，此时间由FPGA计算
    quint64 m_tPcLoop=0;
    quint64 m_tMaxPcLoop=0;
    quint64 m_tAvgPcLoop=0;

protected:
    void run() Q_DECL_OVERRIDE;

private:
    bool m_stop=false;                      //停止线程
    QUdpSocket *m_udpSocket;
    QHostAddress m_peerUdpAddr;   //暂存对接的地址--暂未使用
    quint16 m_peerUdpPort;        //暂存对接的端口--暂未使用
    QHostAddress m_targetAddr;    //目标IP地址
    quint16 m_targetPort;         //目标端口

};

#endif // THREADLOOPSYNC_H
