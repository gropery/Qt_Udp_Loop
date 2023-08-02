#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QString>
#include <QTimer>
#include <QPainter>
#include <QPlainTextEdit>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QtNetwork>
#include <QThread>

#include "threadoutsync.h"
#include "threadinsync.h"
#include "threadloopsync.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButtonShowLocalIp_clicked();
    void on_pushButtonBindPort_clicked();
    void slot_udpSocket_stateChanged(QAbstractSocket::SocketState socketState); //UDP 状态变化

    void on_pushButtonOutSyncContinue_clicked();    //同步连续输出
    void on_pushButtonOutSyncOnce_clicked();        //同步单次输出
    void on_pushButtonInSyncContinue_clicked();     //同步连续输入
    void on_pushButtonInSyncOnce_clicked();         //同步单次输入
    void on_pushButtonLoopSyncContinue_clicked();   //同步连续LOOP

    void slot_timerUpdateLabel_timeout();           //统计数据更新定时器槽函数

    void on_pushButtonInTxtClear_clicked();
    void on_pushButtonOutTxtClear_clicked();





private:
    Ui::MainWindow *ui;

    //-----------------------
    // UDP
    QUdpSocket *udpSocket;
    QHostAddress peerUdpAddr;   //暂存对接的地址--暂未使用
    quint16 peerUdpPort;        //暂存对接的端口--暂未使用
    QHostAddress targetAddr;    //目标IP地址
    quint16 targetPort;         //目标端口

    ThreadOutSync m_ThreadOutSync;
    ThreadInSync  m_ThreadInSync;
    ThreadLoopSync  m_ThreadLoopSync;

    QTimer *timerUpdateLabel;

    long m_curOutBytes=0,m_lastOutByte=0,m_outRate=0;
    long m_curInBytes=0,m_lastInByte=0,m_inRate=0;
    long m_curLoopOutBytes=0,m_lastLoopOutByte=0,m_loopOutRate=0;
    long m_curLoopInBytes=0,m_lastLoopInByte=0,m_loopInRate=0;
    long m_curLoopBytes=0,m_lastLoopByte=0,m_loopRate=0;



};
#endif // MAINWINDOW_H
