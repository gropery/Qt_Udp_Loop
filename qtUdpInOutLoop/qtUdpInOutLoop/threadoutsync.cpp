#include "threadoutsync.h"
#include <QTime>
#include <QDebug>
#include <windows.h>

#define NUM_DATA_OUT 1024

ThreadOutSync::ThreadOutSync()
{

}

//停止线程
void ThreadOutSync::stopThread()
{
    m_stop=true;
}

bool ThreadOutSync::setUdpSocket(QUdpSocket *udpSocket, QHostAddress targetAddr, quint16 targetPort)
{
    if(udpSocket == NULL){
        return false;
    }
    else{
        m_udpSocket = udpSocket;
        m_targetAddr = targetAddr;
        m_targetPort = targetPort;

        return true;
    }
}

//线程任务
void ThreadOutSync::run()
{
    qDebug()  << "ThreadOutSync Thread ID:"<< QThread::currentThreadId();
    SetThreadAffinityMask(GetCurrentThread(), 0x80); //强制分配到第8个CPU核运行

    // 申请缓存器，用来存储OutData
    QByteArray bufferOutput(NUM_DATA_OUT, 0);
    long writeLength=0;
    uint8_t j=0;
    m_outBytes=0;
    m_outSeccesses=0;
    m_outFailures=0;

    m_stop=false;
    //循环主体
    while(!m_stop)
    {
        writeLength = NUM_DATA_OUT;
        for (int i=0; i<writeLength; i++)
        {
            bufferOutput[i] = j;
        }
        j++;


        // 如发送成功，会返回发送的字节长度。失败，返回-1。
        if (m_udpSocket->writeDatagram(bufferOutput,m_targetAddr,m_targetPort)>0)
        {
            m_outBytes += writeLength/1024; //kByte
            m_outSeccesses++;
        }
        else
        {
            qDebug()<<"threadoutsync.cpp: send err";
            m_outFailures++;
        }
        //qDebug()<<m_outBytes;
        //msleep(500); //线程休眠500ms
    }

//  在  m_stop==true时结束线程任务
    quit();//相当于  exit(0),退出线程的事件循环
}

bool ThreadOutSync::outSyncOnce(QByteArray ba)
{

    // 如发送成功，会返回发送的字节长度。失败，返回-1。
    qint64 ret = m_udpSocket->writeDatagram(ba,m_targetAddr,m_targetPort);      //发出数据报

    if(ret>0)
    {
        m_outBytes += ret/1024; //kByteS
        m_outSeccesses++;
        return true;
    }
    else
    {
        qDebug()<<"threadoutsync.cpp: send err";
        m_outFailures++;
        return false;
    }
}
