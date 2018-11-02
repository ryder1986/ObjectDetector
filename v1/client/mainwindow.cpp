#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "videoprocessor.h".h"
#include "detectregion_data.h"
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),tmr1(std::bind(&MainWindow::handle_output,this)),
    ui(new Ui::MainWindow)
{
    fullscreen_mode=false;
    ui->setupUi(this);
    connect(&searcher,SIGNAL(find_ip(QString)),this,SLOT(search_server_result(QString)));
    connect(&clt,SIGNAL(server_msg(QString)),this,SLOT(server_msg(QString)));
    start_udp();
    play_mode=ALL_CAM;
    tmr1.start(10);// check output every 10 ms;
}

MainWindow::~MainWindow()
{
    tmr1.stop();
    delete ui;
}

void MainWindow::search_server_result(QString ip)
{
    ui->comboBox_search->addItem(ip);
}

void MainWindow::on_pushButton_search_clicked()
{
    ui->comboBox_search->clear();
    searcher.search_device();
}

void MainWindow::on_comboBox_search_activated(const QString &arg1)
{
    clt.connect_to_server(arg1);
}
//#include "mvdprocessor.h"
void MainWindow::on_pushButton_addcam_clicked()
{
    DummyProcessorInputData di=DummyProcessorInputData::get_dummy_test_data();
    DetectRegionInputData dri=DetectRegionInputData::get_region_test_data(di.data(),LABEL_PROCESSOR_DUMMY);
    vector<DetectRegionInputData> dris;
    dris.push_back(dri);
    // CameraInputData  d=CameraInputData::get_camera_test_data(dris,"/root/test.mp4");
    //CameraInputData  d=CameraInputData::get_camera_test_data(dris,"D:/build/test.mp4");
    CameraInputData  d=CameraInputData::get_camera_test_data(dris,ui->lineEdit_addcam->text().toStdString());
    request_add_cam(cfg.CameraData.size(),d);
    //insert_into_vector(players.size(),players,new PlayerWidget(d));
    //ui->gridLayout_video->addWidget(players.back());
}

void MainWindow::on_pushButton_delcam_clicked()
{
    int del_index=deleting_index;
  //  int del_index=ui->comboBox_del_index->itemText(deleting_index).toInt();
    if(del_index==0){
        for(int i=0;i<cfg.CameraData.size();i++){
            request_del_cam(cfg.CameraData.size()-i);
        }
    }
    if(del_index>0&&del_index<=cfg.CameraData.size()){
        prt(info,"del index %d",del_index);
        request_del_cam(del_index);
    }else{
        prt(info,"del index %d out of range",del_index);
    }
    //del_from_vector(del_index,players);
}

void MainWindow::on_pushButton_load_clicked()
{
    request_get_config();
}

void MainWindow::server_msg(QString msg)
{
    //ui->plainTextEdit_recive->setPlainText(msg);//show what we got
    string str(msg.toUtf8());
    QString old_msg=ui->textEdit_netbuffer->toPlainText();
  //  ui->textEdit_netbuffer->setPlainText(msg);
    ui->textEdit_netbuffer->setPlainText(old_msg.append("------\n").append(msg));
    ReplyPkt event(str);
    switch(event.Operation){
    case AppInputData::Operation::GET_CONFIG:
    {
        cfg=DeviceConfigData(event.Data).DeviceConfig;
        int cam_sz=cfg.CameraData.size();
        ui->comboBox_play_index->setEditable(true);
        ui->comboBox_play_index->setEnabled(true);
        ui->comboBox_play_index->clear();
        ui->comboBox_play_index->addItem("all");
        for(int i=0;i<cam_sz;i++){
            ui->comboBox_play_index->addItem(QString::number(i+1).append("(").append(cfg.CameraData[i].Url.data()).append(")"));
        }

        ui->comboBox_del_index->clear();
        ui->comboBox_del_index->addItem("all");
        for(int i=0;i<cam_sz;i++){
          //  ui->comboBox_del_index->addItem(QString::number(i+1));
            ui->comboBox_del_index->addItem(QString::number(i+1).append("(").append(cfg.CameraData[i].Url.data()).append(")"));

        }
        prt(info,"%s",cfg.data().str().data());
        break;
    }
    case AppInputData::Operation::INSERT_CAMERA:
    {
        break;
    }
    default:break;
    }
}

void MainWindow::widget_append_camera(CameraInputData d)
{
    int rows_count=3;
    PlayerWidget *player=new PlayerWidget(d);
    connect(player,SIGNAL(camera_request(RequestPkt,PlayerWidget *)),this,SLOT(camera_request(RequestPkt,PlayerWidget*)));
    connect(player,SIGNAL(double_click_event(PlayerWidget *)),this,SLOT(cameras_show_mode(PlayerWidget*)));


    players.push_back(player);
    ui->gridLayout_video->addWidget(player,(players.size()-1)/rows_count,(players.size()-1)%rows_count,1,1);
}
void MainWindow::widget_remove_camera(QWidget *w)
{
    ui->gridLayout_video->removeWidget(w);
}

void MainWindow::cameras_show_mode(PlayerWidget *wgt)
{
    prt(info,"set full screen");
    if(fullscreen_mode==false){
        prt(info,"set full screen");
        fullscreen_mode=true;
        ui->widget_text->hide();
        ui->widget_video_text->hide();

    }else{
        prt(info,"quit full screen");
        fullscreen_mode=false;
        ui->widget_text->show();
        ui->widget_video_text->show();
    }
}

void MainWindow::on_pushButton_play_clicked()
{
    stop_config();
    start_config();
}

void MainWindow::on_pushButton_stop_clicked()
{
    stop_config();
}

void MainWindow::on_pushButton_clear_buffer_clicked()
{
#if 1
    ui->textEdit_netbuffer->clear();
#endif
}

void MainWindow::on_pushButton_connect_clicked()
{
    clt.connect_to_server(ui->lineEdit_ip->text());
}

void MainWindow::on_comboBox_play_index_activated(const QString &arg1)
{
//    prt(info,"%s selected",arg1.toStdString().data());
#if 0
    if(arg1=="all"){
        play_all_cam();
    }else{
        play_one_cam(arg1.toInt());
    }
#endif
}

void MainWindow::on_comboBox_del_index_activated(int index)
{
    deleting_index=index;
}

void MainWindow::on_checkBox_showoutput_clicked(bool checked)
{
    for(PlayerWidget *w:players){
        w->show_output_data(checked);
    }
}

void MainWindow::on_checkBox_showinput_clicked(bool checked)
{
    for(PlayerWidget *w:players){
        w->show_input_data(checked);
    }
}

void MainWindow::on_pushButton_waitms_clicked()
{
    int ms=(ui->lineEdit_wait_ms->text().toInt());
    if(ms>0){
        ms=(ms<10000)?ms:10000;
    }
    for(PlayerWidget *w:players){
        w->set_delay(ms/40);
    }

}

void MainWindow::on_checkBox_show_text_clicked(bool checked)
{
    ClientConfig::show_processor_text=checked;
    //    for(PlayerWidget *w:players){
    //        w->show_text_info(checked);
    //    }
}

void MainWindow::on_checkBox_show_info_clicked(bool checked)
{
    ClientConfig::show_camera_state=checked;
}

void MainWindow::on_comboBox_play_index_activated(int index)
{

    if(index==0){
        play_all_cam();
    }else{
        play_one_cam(index);
    }
}
