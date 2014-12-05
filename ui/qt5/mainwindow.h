#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
    class MainFormBase;
}

class MainFormBase : public QMainWindow {
    Q_OBJECT

public:
    explicit MainFormBase(QWidget *parent = 0);

    ~MainFormBase();

private:
    Ui::MainFormBase *ui;
};

#endif // MAINWINDOW_H
