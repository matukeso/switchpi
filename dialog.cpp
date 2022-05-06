#include "dialog.h"
#include "ui_dialog.h"
#include <pigpiod_if2.h>
#include <qprocess.h>
#include <QKeyEvent>

#include <unistd.h>
extern int g_gpfd;

static int g_fd = -1;
static char  g_firstlog[256]="";
int g_firstlen = 0;

static const int GPIO_INPUTBUTTON = 17;
static const int GPIO_INPUTBUTTON_2 = 22;
static const int GPIO_INPUTBUTTON_3 = 23;
static const int GPIO_SHUTDOWN = 27;

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    UpdateUiTimer = new QTimer(this);
    connect(UpdateUiTimer, SIGNAL(timeout()), this, SLOT(OnUpdateUI()) );
    UpdateUiTimer->start(500);


    qApp->installEventFilter(this);

    set_mode( g_gpfd, GPIO_INPUTBUTTON, PI_INPUT );
    set_pull_up_down( g_gpfd, GPIO_INPUTBUTTON, PI_PUD_UP);
    set_mode( g_gpfd, GPIO_INPUTBUTTON_2, PI_INPUT );
    set_pull_up_down( g_gpfd, GPIO_INPUTBUTTON_2, PI_PUD_UP);
    set_mode( g_gpfd, GPIO_SHUTDOWN, PI_INPUT);
    set_pull_up_down( g_gpfd, GPIO_SHUTDOWN, PI_PUD_UP);
}

Dialog::~Dialog()
{
    delete ui;
}



#include "switch.h"

bool Dialog::eventFilter(QObject *obj, QEvent *event)
{
  if (event->type() == QEvent::KeyPress)
    {
      QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
      evf_keyPressEvent( keyEvent );
      return true;
    }
  return QObject::eventFilter(obj, event);
}




void Dialog::evf_keyPressEvent(QKeyEvent * e)
{
  switch( e->key() ){
  case Qt::Key_7: send_232c_pgm(1); break;
  case Qt::Key_8: send_232c_pgm(2); break;
  case Qt::Key_9: send_232c_pgm(3); break;
  case Qt::Key_Minus: send_232c_pgm(4); break;
  case Qt::Key_4: send_232c_pst(1); break;
  case Qt::Key_5: send_232c_pst(2); break;
  case Qt::Key_6: send_232c_pst(3); break;
  case Qt::Key_Plus: send_232c_pst(4); break;
  case Qt::Key_Return:
  case Qt::Key_Enter : send_232c_ato(); break;
  }

}
bool Dialog::check_input_start(){
  static int g_prev_gpio;
  bool ret = false;
  int gpio = !gpio_read( g_gpfd, GPIO_INPUTBUTTON );
  if( gpio == 1  && g_prev_gpio == 0 ){
    on_pushButton_clicked();
    ret =  true;
  }
  g_prev_gpio = gpio;
  return ret;
}
bool Dialog::check_input_umount(){
  static int g_prev_gpio2;
  bool ret = false;
  int gpio = !gpio_read( g_gpfd, GPIO_INPUTBUTTON_2 );
  if( gpio == 1  && g_prev_gpio2 == 0 ){
    ret =  true;
  }
  g_prev_gpio2 = gpio;
  return ret;
}
bool Dialog::check_reboot_req(){
  static int g_prev_gpio;
  bool ret = false;
  int gpio = !gpio_read( g_gpfd, GPIO_SHUTDOWN );
  if( gpio == 1  && g_prev_gpio == 0 ){
    ret =  true;
  }
  g_prev_gpio = gpio;
  return ret;
}

QString get_mount_dir();
bool has_mount_dir();
bool umount_dir();
int open_mounted_log(QString &path);

bool  ensure_close_gfd()
{
  if( g_fd > 0 ){
    printf("CLOSE\n");
    int fd = g_fd;
    g_fd = 0;
    ::close(fd);
    return true;
  }
  return false;
}


void Dialog::OnUpdateUI()
{
  check_input_start();
  const bool has_usbstick = has_mount_dir();
  QString maintxt =
    ( g_fd > 0   ) ? QString::fromUtf8("●保存中：") :
    (has_usbstick) ? QString::fromUtf8("□待機中：") :
                     QString::fromUtf8("×待機中：") ;

  if( has_usbstick ){
    if( check_input_umount() ){
      ensure_close_gfd();
      umount_dir();
    }
  }
  
  if( check_reboot_req() ){
    QProcess::execute("sudo poweroff");
    QApplication::quit();
  }


  {
    char t[10];
    sprintf(t, "%08d", current_tc() );
    char t2[32];
    sprintf(t2, "%c%ch%c%cm%c%cs",
                      t[0],t[1],t[2],t[3],t[4],t[5]);

    maintxt += t2;
    ui->label->setText(maintxt);
  }

  QLabel *labels[] = { ui->label_1, ui->label_2, ui->label_3, ui->label_4, ui->label_5, ui->label_6 };

  QLabel *actlabel[2] = { nullptr, nullptr};
  switch( midi.pgm_a ){
  case 1: actlabel[0] =  ui->label_1; break;
  case 2: actlabel[0] =  ui->label_2; break;
  case 3: actlabel[0] =  ui->label_3; break;
  case 4: actlabel[0] =  ui->label_4; break;
  case 5: actlabel[0] =  ui->label_5; break;
  case 6: actlabel[0] =  ui->label_6; break;
  }
  if( midi.fader != 127 ){
    switch( midi.pst_b ){
    case 1: actlabel[1] =  ui->label_1; break;
    case 2: actlabel[1] =  ui->label_2; break;
    case 3: actlabel[1] =  ui->label_3; break;
    case 4: actlabel[1] =  ui->label_4; break;
    case 5: actlabel[1] =  ui->label_5; break;
    case 6: actlabel[1] =  ui->label_6; break;
  }
  }
  if( ui->label ){
    for( QLabel *l : labels )
    {
      QPalette palette = l->palette();
      if( l == actlabel[0] || l == actlabel[1] ){
        palette.setColor( QPalette::Window, "#ff0000");
        palette.setColor( QPalette::WindowText, "#ffffff");
      }
      else{
        palette.setColor( QPalette::Window, "#ffffff");
        palette.setColor( QPalette::WindowText, "#000000");
      }
      l->setPalette( palette );
    }
  }

}


void Dialog::on_pushButton_clicked()
{
  if( ensure_close_gfd() ){
    ;
  }
  else{
    printf("OPEN func\n");

    if(!has_mount_dir()){
      ui->label_path->setText("NO-USB-DRIVE");
      printf("NO USB\n");
    }
    else{
      QString path;
      int fdw = open_mounted_log(path);
      if(fdw < 0 ){
	ui->label_path->setText("ERR-USB-DRIVE");
	printf("ERR\n");
	return ;
      }
      else{
	char tc[12];
	sprintf(tc, "%08d", current_tc() );
	char buf[64];
	int len  =    sprintf(buf, "%c%c:%c%c:%c%c.%c%c",
			      tc[0],tc[1],tc[2],tc[3],tc[4],tc[5],tc[6],tc[7]);
	memcpy( g_firstlog, buf, len );


	::write(fdw, g_firstlog, g_firstlen );
	g_fd = fdw;


	ui->label_path->setText(path);
      }
    }
  }
}


extern "C" void output_csv( const char *msg, int bytes )
{
  int fd = g_fd;
  if( fd > 0 ) {
    ::write( fd, msg, bytes);
  }
  memcpy( g_firstlog, msg, bytes );
  g_firstlen   = bytes;
}
