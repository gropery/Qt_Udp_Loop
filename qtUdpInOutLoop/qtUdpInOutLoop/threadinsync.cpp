#include "threadinsync.h"
#include <QTime>
#include <QDebug>

#define NUM_DATA_IN 1024
ThreadInSync::ThreadInSync()
{

}

//停止线程
void ThreadInSync::stopThread()
{
    m_stop=true;
}

bool ThreadInSync::setUdpSocket(QUdpSocket *udpSocket, QHostAddress targetAddr, quint16 targetPort)
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
void ThreadInSync::run()
{
    QByteArray bufferInput(NUM_DATA_IN, 0);

    long readLength=0;
    m_inBytes=0;
    m_inSeccesses=0;
    m_inFailures=0;
    bool checkOk = true;

    m_stop=false;

    char counter=0;
    //循环主体
    while(!m_stop)
    {
        if(m_udpSocket->hasPendingDatagrams())
        {
            QByteArray datagram;
            datagram.resize(m_udpSocket->pendingDatagramSize()); // 只取出单个数据包, 如果累积了多个包可以while循环取出
            qint64 ret = m_udpSocket->readDatagram(datagram.data(),datagram.size(),&m_peerUdpAddr,&m_peerUdpPort);
            if(ret>0)
            {
                readLength = bufferInput.length();
                bufferInput = datagram;

                if(readLength==NUM_DATA_IN)
                {
                    checkOk = true;
                    for(int i=0; i<readLength; i++)
                    {
                        if(bufferInput[i]!=counter)
                        {
                            checkOk = false;
                            qDebug()<<"i="<<i<<(uint8_t)bufferInput[i]<<"::"<<(uint8_t)(counter);
                            break;
                        }
                    }

                    counter = bufferInput[0]+1;
                    if(checkOk)
                    {
                        m_inBytes += readLength/1024; //kByte
                        m_inSeccesses++;
                    }
                    else
                    {
                        m_inFailures++;
                    }

                }
                else
                {
                    qDebug()<<"threadinsync.cpp: recv dataLehgth less";
                }

            }
            else
            {
                qDebug()<<"threadinsync.cpp: recv err";
            }
        }

        //qDebug()<<m_inBytes;
        //msleep(500); //线程休眠500ms
    }


//  在  m_stop==true时结束线程任务
    quit();//相当于  exit(0),退出线程的事件循环
}

bool ThreadInSync::inSyncOnce(QByteArray &ba)
{
    if(m_udpSocket->hasPendingDatagrams())
    {
        QByteArray   datagram;
        datagram.resize(m_udpSocket->pendingDatagramSize()); // 只取出单个数据包, 如果累积了多个包可以while循环取出
        qint64 ret = m_udpSocket->readDatagram(datagram.data(),datagram.size(),&m_peerUdpAddr,&m_peerUdpPort);
        if(ret>0)
        {
            ba = datagram;
            m_inBytes += ret/1024; //kByteS
            m_inSeccesses++;
            return true;
        }
        else
        {
            qDebug()<<"threadinsync.cpp: recv err";
            m_inFailures++;
            return false;
        }
    }
    else
    {
        qDebug()<<"threadinsync.cpp: no recv data";
        m_inFailures++;
        return false;
    }
}
