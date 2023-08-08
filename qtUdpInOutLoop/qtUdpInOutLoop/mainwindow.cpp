#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QDebug"
#include <windows.h>


#define     MIN_DATA_NUM         1
#define     MAX_DATA_NUM         1472

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Qt Udp Loop");

    // Label数据更新-定时器
    timerUpdateLabel = new QTimer;
    timerUpdateLabel->start(1000);
    connect(timerUpdateLabel, &QTimer::timeout, this, [=](){
        slot_timerUpdateLabel_timeout();
    });

    // 新建UDP socket
    udpSocket = new QUdpSocket(this);
    udpSocket->setReadBufferSize(16*1024*1024);
    connect(udpSocket,SIGNAL(stateChanged(QAbstractSocket::SocketState)),this,SLOT(slot_udpSocket_stateChanged(QAbstractSocket::SocketState)));
    slot_udpSocket_stateChanged(udpSocket->state());    //更新Socket状态

    qDebug()  << "mainwindow Thread ID:"<< QThread::currentThreadId();
    //SetThreadAffinityMask(GetCurrentThread(), 0x10); //强制分配到第8个CPU核运行

}

// 显示本地所有IPv4地址
void MainWindow::on_pushButtonShowLocalIp_clicked()
{
    QList<QNetworkInterface> list=QNetworkInterface::allInterfaces();
    for(int i=0;i<list.count();i++)
    {
        QNetworkInterface aInterface=list.at(i);
        if (!aInterface.isValid())
           continue;

        ui->plainTextEditBufferIn->appendPlainText("设备名称："+aInterface.humanReadableName());
        ui->plainTextEditBufferIn->appendPlainText("硬件地址："+aInterface.hardwareAddress());
        QList<QNetworkAddressEntry> entryList=aInterface.addressEntries();
        for(int j=0;j<entryList.count();j++)
        {
            QNetworkAddressEntry aEntry=entryList.at(j);
            if(!(aEntry.ip().protocol() == QAbstractSocket::IPv4Protocol))  //只打印IPv4
                continue;

            ui->plainTextEditBufferIn->appendPlainText("   IP 地址："+aEntry.ip().toString());
            ui->plainTextEditBufferIn->appendPlainText("   子网掩码："+aEntry.netmask().toString());
            ui->plainTextEditBufferIn->appendPlainText("   广播地址："+aEntry.broadcast().toString()+"\n");
        }
        ui->plainTextEditBufferIn->appendPlainText("\n");
    }
}

// UDP绑定端口
void MainWindow::on_pushButtonBindPort_clicked()
{
    if(ui->pushButtonBindPort->isChecked()){
        quint16 port=ui->spinBoxLocalPort->value(); //本机UDP端口

        if (udpSocket->bind(port)){
            ui->pushButtonBindPort->setChecked(true);   //按下绑定按钮
            ui->spinBoxLocalPort->setEnabled(false);    //屏蔽本地端口
            ui->comboBoxTargetIP->setEnabled(false);    //屏蔽目标IP
            ui->spinBoxTargetPort->setEnabled(false);   //屏蔽目标端口
            ui->pushButtonBindPort->setText("解除绑定");

            QString targetIP = ui->comboBoxTargetIP->currentText();   //目标IP
            QHostAddress targetAddr_tmp(targetIP);
            targetAddr = targetAddr_tmp;
            targetPort = ui->spinBoxTargetPort->value();              //目标port
        }
    }
    else{
        udpSocket->abort();
        ui->pushButtonBindPort->setChecked(false);  //释放绑定按钮
        ui->spinBoxLocalPort->setEnabled(true);     //使能本地端口
        ui->comboBoxTargetIP->setEnabled(true);     //使能目标IP
        ui->spinBoxTargetPort->setEnabled(true);    //使能目标端口
        ui->pushButtonBindPort->setText("绑定端口");
    }
}

//UDP 状态变化
void MainWindow::slot_udpSocket_stateChanged(QAbstractSocket::SocketState socketState)
{
    switch(socketState)
    {
        case QAbstractSocket::UnconnectedState:
            ui->labelSocketState->setText("scoket状态：UnconnectedState");
            break;
        case QAbstractSocket::HostLookupState:
            ui->labelSocketState->setText("scoket状态：HostLookupState");
            break;
        case QAbstractSocket::ConnectingState:
            ui->labelSocketState->setText("scoket状态：ConnectingState");
            break;
        case QAbstractSocket::ConnectedState:
            ui->labelSocketState->setText("scoket状态：ConnectedState");
            break;
        case QAbstractSocket::BoundState:
            ui->labelSocketState->setText("scoket状态：BoundState");
            break;
        case QAbstractSocket::ClosingState:
            ui->labelSocketState->setText("scoket状态：ClosingState");
            break;
        case QAbstractSocket::ListeningState:
            ui->labelSocketState->setText("scoket状态：ListeningState");
    }
}


MainWindow::~MainWindow()
{
    if (m_ThreadOutSync.isRunning())
    {
        m_ThreadOutSync.stopThread();
        m_ThreadOutSync.wait();
    }

    if (m_ThreadInSync.isRunning())
    {
        m_ThreadInSync.stopThread();
        m_ThreadInSync.wait();
    }

    if (m_ThreadLoopSync.isRunning())
    {
        m_ThreadLoopSync.stopThread();
        m_ThreadLoopSync.wait();
    }

    udpSocket->abort();
    delete udpSocket;
    delete ui;
}


//同步连续输出
void MainWindow::on_pushButtonOutSyncContinue_clicked()
{
    if(ui->pushButtonOutSyncContinue->text()=="Start")
    {
        if(m_ThreadOutSync.setUdpSocket(udpSocket, targetAddr, targetPort)==true)
        {
           //启动线程
           m_ThreadOutSync.start();
           ui->pushButtonOutSyncContinue->setText("Stop");
        }

    }
    else{
        ui->pushButtonOutSyncContinue->setText("Start");
        //停止线程
        m_ThreadOutSync.stopThread();
        m_ThreadOutSync.wait();
    }
}

//同步单次输出
void MainWindow::on_pushButtonOutSyncOnce_clicked()
{
    long outTransferSize = ui->lineEditOutSyncOnceBytes->text().toLong();

    // 发送数据不能小于1byte
    if(outTransferSize<MIN_DATA_NUM)
    {
        outTransferSize = MIN_DATA_NUM;
        ui->lineEditOutSyncOnceBytes->setText(tr("").number(outTransferSize));
    }
    else if(outTransferSize>MAX_DATA_NUM)
    {
        outTransferSize = MAX_DATA_NUM;
        ui->lineEditOutSyncOnceBytes->setText(tr("").number(outTransferSize));
    }

    if(m_ThreadOutSync.setUdpSocket(udpSocket, targetAddr, targetPort)==true)
    {
        // 获取实际发送内容
        QString strSendData = ui->plainTextEditBufferOut->toPlainText();
        QByteArray ba = QByteArray::fromHex(strSendData.toLocal8Bit());    // GB2312编码输出
        int baLen = ba.length();

        QByteArray ba2(outTransferSize,0);
        for(int i=0; i<outTransferSize; i++)
        {
            if(i < baLen)                   //包起始
                ba2[i] = ba.at(i);
            else if(i > outTransferSize-2)  //包尾
                ba2[i] = 0xA5;
            else
                ba2[i] = 0x12;     //包填充
        }

        if(m_ThreadOutSync.outSyncOnce(ba2)==true)
        {
            qDebug()<<"out ok";
        }
    }
}

//同步连续输入
void MainWindow::on_pushButtonInSyncContinue_clicked()
{
    if(ui->pushButtonInSyncContinue->text()=="Start")
    {
        if(m_ThreadInSync.setUdpSocket(udpSocket, targetAddr, targetPort)==true)
        {
           //启动线程
           m_ThreadInSync.start();
           ui->pushButtonInSyncContinue->setText("Stop");
        }
    }
    else{
        ui->pushButtonInSyncContinue->setText("Start");
        //停止线程
        m_ThreadInSync.stopThread();
        m_ThreadInSync.wait();
    }
}

//同步单次输入
void MainWindow::on_pushButtonInSyncOnce_clicked()
{
    if(m_ThreadInSync.setUdpSocket(udpSocket, targetAddr, targetPort)==true)
    {
        QByteArray baRecvData;
        if(m_ThreadInSync.inSyncOnce(baRecvData)==true)
        {

            //已接收字节统计
            int baRecvDataSize = baRecvData.size();

            if(baRecvDataSize>0)
            {
                // 16进制显示，用空格分隔，转换为大写
                QByteArray ba = baRecvData.toHex(' ').toUpper();
                QString str = QString(ba).append(' ');

                //qstring 每16个bytes作为一行打印
                QString str2("");
                for(int j=0; j<baRecvDataSize*3; j=j+3)
                {
                    if(j%(16*3)==0)
                    {
                        str2 += tr("0x%1(%2): ").arg(int(j/3),4,16,QChar('0'))
                                                .arg(int(j/3),4,10,QChar('0'));
                        str2 += str.mid(j,16*3).append('\n');
                    }
                }

                // 在当前位置插入文本
                ui->plainTextEditBufferIn->appendPlainText(str2);
                // 移动光标到文本结尾
                ui->plainTextEditBufferIn->moveCursor(QTextCursor::End);
            }
        }
    }
}


//同步连续LOOP
void MainWindow::on_pushButtonLoopSyncContinue_clicked()
{
    if(ui->pushButtonLoopSyncContinue->text()=="Start")
    {
        if(m_ThreadLoopSync.setUdpSocket(udpSocket, targetAddr, targetPort)==true)
        {
            //启动线程
            m_ThreadLoopSync.start();
            ui->pushButtonLoopSyncContinue->setText("Stop");
        }
    }
    else{
        ui->pushButtonLoopSyncContinue->setText("Start");
        //停止线程
        m_ThreadLoopSync.stopThread();
        m_ThreadLoopSync.wait();
    }
}

//定时器槽函数timeout，1s 数据更新
void MainWindow::slot_timerUpdateLabel_timeout(void)
{
    //从类中获取数据
    m_curOutBytes = m_ThreadOutSync.m_outBytes;
    m_curInBytes = m_ThreadInSync.m_inBytes;

    m_curLoopOutBytes = m_ThreadLoopSync.m_outBytes;
    m_curLoopInBytes = m_ThreadLoopSync.m_inBytes;
    m_curLoopBytes = m_ThreadLoopSync.m_loopBytes;

    //当前总计数-上次总结存暂存
    m_outRate = m_curOutBytes - m_lastOutByte;
    m_inRate = m_curInBytes - m_lastInByte;

    m_loopOutRate = m_curLoopOutBytes - m_lastLoopOutByte;
    m_loopInRate = m_curLoopInBytes - m_lastLoopInByte;
    m_loopRate = m_curLoopBytes - m_lastLoopByte;

    //暂存当前总计数
    m_lastOutByte = m_curOutBytes;
    m_lastInByte = m_curInBytes;
    m_lastLoopOutByte = m_curLoopOutBytes;
    m_lastLoopInByte = m_curLoopInBytes;
    m_lastLoopByte = m_curLoopBytes;

    //设置label
    ui->lineEditOutRate->setText(tr("%1").arg(m_outRate));
    ui->lineEditOutBytes->setText(tr("%1").arg(m_curOutBytes));
    ui->lineEditOutSuccesses->setText(tr("%1").arg(m_ThreadOutSync.m_outSeccesses));
    ui->lineEditOutFailures->setText(tr("%1").arg(m_ThreadOutSync.m_outFailures));

    ui->lineEditInRate->setText(tr("%1").arg(m_inRate));
    ui->lineEditInBytes->setText(tr("%1").arg(m_curInBytes));
    ui->lineEditInSuccesses->setText(tr("%1").arg(m_ThreadInSync.m_inSeccesses));
    ui->lineEditInFailures->setText(tr("%1").arg(m_ThreadInSync.m_inFailures));

    ui->lineEditLoopOutRate->setText(tr("%1").arg(m_loopOutRate));
    ui->lineEditLoopOutBytes->setText(tr("%1").arg(m_curLoopOutBytes));
    ui->lineEditLoopOutSuccesses->setText(tr("%1").arg(m_ThreadLoopSync.m_outSeccesses));
    ui->lineEditLoopOutFailures->setText(tr("%1").arg(m_ThreadLoopSync.m_outFailures));

    ui->lineEditLoopInRate->setText(tr("%1").arg(m_loopInRate));
    ui->lineEditLoopInBytes->setText(tr("%1").arg(m_curLoopInBytes));
    ui->lineEditLoopInSuccesses->setText(tr("%1").arg(m_ThreadLoopSync.m_inSeccesses));
    ui->lineEditLoopInFailures->setText(tr("%1").arg(m_ThreadLoopSync.m_inFailures));

    ui->lineEditLoopRate->setText(tr("%1").arg(m_loopRate));
    ui->lineEditLoopBytes->setText(tr("%1").arg(m_curLoopBytes));
    ui->lineEditLoopSuccesses->setText(tr("%1").arg(m_ThreadLoopSync.m_loopSeccesses));
    ui->lineEditLoopFailures->setText(tr("%1").arg(m_ThreadLoopSync.m_loopFailures));

    ui->lineEditPcLoop->setText(tr("%1").arg(m_ThreadLoopSync.m_tPcLoop));
    ui->lineEditMaxPcLoop->setText(tr("%1").arg(m_ThreadLoopSync.m_tMaxPcLoop));
    ui->lineEditAvgPcLoop->setText(tr("%1").arg(m_ThreadLoopSync.m_tAvgPcLoop));
}

void MainWindow::on_pushButtonInTxtClear_clicked()
{
    ui->plainTextEditBufferIn->clear();
}

void MainWindow::on_pushButtonOutTxtClear_clicked()
{
    ui->plainTextEditBufferOut->clear();
}




