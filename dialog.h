#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QTimer>
namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

public slots:
  void OnUpdateUI();
    
private slots:
  void on_pushButton_clicked();

private:
    Ui::Dialog *ui;
    QTimer *UpdateUiTimer;
};

#endif // DIALOG_H
