#include "mainwindow.h"
#include "ui_mainwindow.h"

MainFormBase::MainFormBase(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainFormBase)
{
    ui->setupUi(this);
}

MainFormBase::~MainFormBase()
{
    delete ui;
}
