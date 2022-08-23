#ifndef PCAN_QT_H
#define PCAN_QT_H

#include "include/PCANBasic.h"
#include <QMainWindow>
#include <QDebug>
#include <QTimer>
#include <QMap>
#include <QMessageBox>


QT_BEGIN_NAMESPACE
namespace Ui { class PCAN_QT; }
QT_END_NAMESPACE

struct imuData{
    float acc[3]={0};
    float gyr[3]={0};
    float eul[3]={0};
    float quat[4]={0};
    float prs=0;
};


class PCAN_QT : public QMainWindow
{
    Q_OBJECT

public:
    PCAN_QT(QWidget *parent = nullptr);
    ~PCAN_QT();

private slots:
    void pcan_read();
    void pcan_send(TPCANMsg msg);
    void calc_hz();
    void scan_channels();
    void reading_cfg();

    void on_BTN_init_clicked();
    void on_BTN_refresh_channel_clicked();
    void on_BTN_fastsdo_send_clicked();
    void on_BTN_change_baud_clicked();
    void on_BTN_change_node_id_clicked();
    void on_BTN_change_tpdo_hz_clicked();
    void on_CB_tpdo_channel_currentIndexChanged(int index);

    void on_BTN_read_config_clicked();

    void on_BTN_release_clicked();

    void on_BTN_clr_TB_fastsdo_msgbox_clicked();

private:
    Ui::PCAN_QT *ui;
    QString uchar_to_qstr(uchar *str, const int len );
    void qstr_to_uchar(QString qstr, uchar *str);
    void can_init();
    void can_uninit();


    void data_parser(TPCANMsg msg);
    void imu_parser(TPCANMsg msg);
    void pop_msgbox(QString text);
    void update_config_tpdo_hz();
    void fastsdo_readcfg();

    //current channel informations
    TPCANChannelInformation channels;
    ushort channel_handle=0;
    ushort bitrate=0;
    ushort node_id=8;

    //IMU data storage
    QMap<QString, QString> tdpo_data;
    QMap<QString, int> tdpo_hz;
    QMap<QString, int> tdpo_ctr;    
    uint config_tpdo_hz[5];

    //QT timer
    QTimer *tmr_read;
    QTimer *tmr_1000ms;

    //imu data
    imuData m_imu_data;

    bool flag_reading_cfg=false,flag_readcfg_succ=false;

};
#endif // PCAN_QT_H
