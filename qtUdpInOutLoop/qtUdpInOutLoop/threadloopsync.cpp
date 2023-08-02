#include "threadloopsync.h"
#include <QTime>
#include <QDebug>

#define NUM_DATA_LOOP 1024
ThreadLoopSync::ThreadLoopSync()
{

}

//停止线程
void ThreadLoopSync::stopThread()
{
    m_stop=true;
}

bool ThreadLoopSync::setUdpSocket(QUdpSocket *udpSocket, QHostAddress targetAddr, quint16 targetPort)
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
void ThreadLoopSync::run()
{
    QByteArray bufferInput(NUM_DATA_LOOP, 0);
    QByteArray bufferOutput(NUM_DATA_LOOP, 0);

    m_inBytes=0;
    m_inSeccesses=0;
    m_inFailures=0;
    m_outBytes=0;
    m_outSeccesses=0;
    m_outFailures=0;
    m_loopBytes=0;
    m_loopSeccesses=0;
    m_loopFailures=0;
    m_tPcLoop=0;
    m_tMaxPcLoop=0;
    m_tAvgPcLoop=0;

    long readLength=0;
    long writeLength=0;
    quint64 tSumPcLoop=0;
    quint64 cntPcLoop=0;
    uint8_t cnt=0;            //每次轮训发送数据值累加
    int STATE=0;            //状态机
    int readDummyTimes=0;   //启动线程初始，读出USB硬件端多余的IN数据计数
    int runTimes=0;         //启动线程初始，计数loop次数，除外前n次计数
    bool isRecvData;


    m_stop=false;
    //循环主体
    while(!m_stop)
    {
        switch (STATE) {
        case 0: //第一次运行读出硬件端多余数据
            while(m_udpSocket->hasPendingDatagrams())
            {
                QByteArray   datagram;
                datagram.resize(m_udpSocket->pendingDatagramSize()); // 只取出单个数据包, 如果累积了多个包可以while循环取出
                qint64 ret = m_udpSocket->readDatagram(datagram.data(),datagram.size(),&m_peerUdpAddr,&m_peerUdpPort);
                if(ret>0)
                {
                    readDummyTimes++;
                    qDebug()<<"first run read dummy data"<<readDummyTimes;
                }
            }
            STATE = 1;
            break;

        case 1: //启动Out传输
            writeLength = NUM_DATA_LOOP;
            for (int i=0; i<writeLength; i++)
            {
                bufferOutput[i] = cnt;
            }

            // 如发送成功，会返回发送的字节长度。失败，返回-1。
            if(m_udpSocket->writeDatagram(bufferOutput,m_targetAddr,m_targetPort)>0)
            {
                m_outBytes += writeLength/1024; //kByte
                m_outSeccesses++;
                STATE = 2;
            }
            else
            {
                m_outFailures++;
                STATE = 0;
            }
            break;

        case 2:  //启动In传输
            isRecvData = false;

            while(!isRecvData)
            {
                if(m_udpSocket->hasPendingDatagrams())
                {
                    QByteArray datagram;
                    datagram.resize(m_udpSocket->pendingDatagramSize()); // 只取出单个数据包, 如果累积了多个包可以while循环取出
                    qint64 ret = m_udpSocket->readDatagram(datagram.data(),datagram.size(),&m_peerUdpAddr,&m_peerUdpPort);
                    if(ret>0)
                    {
                        bufferInput = datagram;
                        readLength = bufferInput.length();
                        m_inBytes += readLength/1024; //kByte
                        m_inSeccesses++;
                        isRecvData = true;
                    }
                    else
                    {
                        m_inFailures++;
                    }
                }
            }

            STATE = 3;
            break;

        case 3:
            if((writeLength==readLength)
                &&(bufferInput[0]==bufferOutput[0])
                &&(bufferInput[50]==bufferOutput[50]))   //loop数据校验正确
            {
                 //-----------------------------------------------------
                 //大小端测试
//                quint32 buf1=0;
//                QString strbuf1("");
//                buf1 = (bufferInput[7]<<8*3)
//                      +(bufferInput[6]<<8*2)
//                      +(bufferInput[5]<<8*1)
//                      +(bufferInput[4]);
//                strbuf1 = tr("%1").arg(buf1,8,16,QChar('0'));
//                qDebug()<<"strbuf1="<<strbuf1;

                //--------------------------------------------------------
                //tPcLoop时长计算
                m_tPcLoop = ((bufferInput[11]<<8*3)
                       +(bufferInput[10]<<8*2)
                       +(bufferInput[9]<<8*1)
                       +(bufferInput[8]))/100;      //us

                //至少跳过最早的2个tPcLoop值
                //因为在第一个为之前的计算值，第二个由于中间暂停所以数值很大不正确
                //第三个已经是正确的值了，这里选择3只是为了调试
                if(runTimes>=3)
                {
                    //查找最大时间
                    if(m_tMaxPcLoop < m_tPcLoop)
                        m_tMaxPcLoop = m_tPcLoop;

                    //计算平均时间
                    tSumPcLoop += m_tPcLoop;
                    cntPcLoop++;
                    m_tAvgPcLoop = tSumPcLoop/cntPcLoop;
                }
                else
                {
                    qDebug()<<"runTimes="<<runTimes<<", m_tPcLoop(us)="<<m_tPcLoop;
                    runTimes++;
                }

                m_loopBytes += readLength/1024; //kByte
                m_loopSeccesses++;
                cnt++;      //累加loop值
                STATE = 1;
            }
            else
            {
                m_loopFailures++;
                STATE = 0;
            }
            break;

        default:
            STATE = 0;
            break;
        }

        //qDebug()<<m_inBytes;
        //msleep(500); //线程休眠500ms
    }

//  在  m_stop==true时结束线程任务
    quit();//相当于  exit(0),退出线程的事件循环
}

