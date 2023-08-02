#include "threadinsync.h"
#include <QTime>
#include <QDebug>

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
//    UCHAR *bufferInput = new UCHAR[m_totalInTransferSize];

//    m_BulkInEpt->TimeOut = m_timeoutPerTransferMilliSec;
//    m_BulkInEpt->SetXferSize(m_totalInTransferSize);

//    long readLength=0;
//    m_inBytes=0;
//    m_inSeccesses=0;
//    m_inFailures=0;

//    m_stop=false;

//    UCHAR counter=0;
//    //循环主体
//    while(!m_stop)
//    {
//        //每次重新赋值，因为xferdata可能会修改readLength
//        readLength = m_totalInTransferSize;
//        //启动BulkIn传输
//        if (m_BulkInEpt->XferData(bufferInput, readLength) == TRUE )
//        {
//            m_inBytes += readLength/1024; //kByte


//            if(readLength==m_totalInTransferSize)
//            {
//                for(int i=0; i<readLength/4096; i++)
//                {
//                    if(bufferInput[8+i*4096]==(UCHAR)(counter+1))
//                    {
//                        m_inSeccesses++;
//                    }
//                    else
//                    {
//                        m_inFailures++;
//                        qDebug()<<bufferInput[8+i*4096]<<"::"<<(UCHAR)(counter+1);
//                    }
//                    counter = bufferInput[8+i*4096];
//                }

//            }
//        }
//        else
//        {
//            m_inFailures++;
//        }

//        //qDebug()<<m_inBytes;
//        //msleep(500); //线程休眠500ms
//    }

//    delete [] bufferInput;

////  在  m_stop==true时结束线程任务
//    quit();//相当于  exit(0),退出线程的事件循环
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
            m_inFailures++;
            return false;
        }
    }
    else
    {
        m_inFailures++;
        return false;
    }
}
