#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <qprocess.h>

#include <QDir>

QString get_mount_dir()
{
    QDir mount("/media/pi");
    mount.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);
    QStringList list = mount.entryList();
    
    if( list.size() == 0 ){
      return QString();
    }
    return mount.absoluteFilePath( list[0] );
}

bool has_mount_dir()
{
  struct stat st = {};
  ::stat("/media/pi", &st);
  return  st.st_nlink >= 3 ;
}
bool umount_dir(){
  QString list = get_mount_dir();
  //  QString cmd = QString(  "sudo umount \"%1\"" ).arg(list);
  QString cmd = QString(  "gio mount -e \"%1\"" ).arg(list);
  printf("%s\n", qPrintable(cmd) ) ;
  QProcess::execute(cmd);

  return true;
}

int open_mounted_log(QString &path)
{
  QString usb_drive = get_mount_dir();
      if( usb_drive.isEmpty() ){
	return -1;
      }

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
      return fdw;
}

   
