#include "dialog.h"
#include "ui_dialog.h"
#include <pigpiod_if2.h>

extern int g_gpfd;

static const int GPIO_INPUTBUTTON = 12;


Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    UpdateUiTimer = new QTimer(this);
    connect(UpdateUiTimer, SIGNAL(timeout()), this, SLOT(OnUpdateUI()) );
    UpdateUiTimer->start(500);


    set_mode( g_gpfd, GPIO_INPUTBUTTON, PI_INPUT );
    set_pull_up_down( g_gpfd, GPIO_INPUTBUTTON, PI_PUD_DOWN);
}

Dialog::~Dialog()
{
    delete ui;
}



#include "switch.h"

int g_prev_gpio;
void Dialog::OnUpdateUI()
{
  int gpio = gpio_read( g_gpfd, GPIO_INPUTBUTTON );
  if( gpio == 1  && g_prev_gpio == 0 ){
    on_pushButton_clicked();
  }
  g_prev_gpio = gpio;

  QString maintxt = ( g_fd > 0 ) ? QString::fromUtf8("●保存中：") : QString::fromUtf8("×待機中：");

  {
    char t[10];
    sprintf(t, "%08d", current_tc() );
    char t2[32];
    sprintf(t2, "%c%ch%c%cm%c%cs",
                      t[0],t[1],t[2],t[3],t[4],t[5]);

    maintxt += t2;
    ui->label->setText(maintxt);
  }

  QLabel *labels[] = { ui->label_1, ui->label_2, ui->label_3, ui->label_4 };

  QLabel *actlabel = nullptr;
  switch( midi.pgm_a ){
  case 1: actlabel =  ui->label_1; break;
  case 2: actlabel =  ui->label_2; break;
  case 3: actlabel =  ui->label_3; break;
  case 4: actlabel =  ui->label_4; break;
  }
  if( ui->label ){
    for( int i=0; i<4; i++){
      QLabel *l  = labels[i];
      QPalette palette = l->palette();
      if( l != actlabel ){
        palette.setColor( QPalette::Window, "#ffffff");
        palette.setColor( QPalette::WindowText, "#000000");
      }
      else{
        palette.setColor( QPalette::Window, "#ff0000");
        palette.setColor( QPalette::WindowText, "#ffffff");
      }
      l->setPalette( palette );
    }
  }

}

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <QDir>

void Dialog::on_pushButton_clicked()
{
  if( g_fd > 0 ){
    int fd = g_fd;
    g_fd = 0;
    ::close(fd);
  }
  else{
    QDir mount("/media/pi");
    mount.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);
    QStringList list = mount.entryList();
    if( list.size() == 0 ){
      ui->label_path->setText("NO-USB-DRIVE");
    }
    else{
      QString usb_drive =  mount.absoluteFilePath( list[0] );
      QString path;
      {
        time_t now;
        time(&now);
        char buf[64];
        strftime(buf, sizeof buf, "%Y%m%dT%H%M%S", localtime(&now));
        path = usb_drive;
        path += "/";
        path += buf;
        path += ".log";
      }

      int fdw = ::open( path.toUtf8().constData(), O_RDWR | O_CREAT, 0600);
      if( fdw > 0 ){
        g_fd = fdw;
        ui->label_path->setText(path);
      }
      else{
        ui->label_path->setText("ERROR:" + path);
      }
    }
  }
}
