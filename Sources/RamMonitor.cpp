#include "RamMonitor.h"
#include "ui_RamMonitor.h"

RamMonitor::RamMonitor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RamMonitor)
{
    ui->setupUi(this);

    setWindowIcon(QIcon(":/Icons/Icons/Ram monitor.ico"));

    QStringList horizontalHeader, verticalHeader;
    for(int i = 0;i < 16;i++)
        horizontalHeader << QString::number(i,16).toUpper();

    QString s;
    for(int i = 0;i <= 0xfff;i++)
        verticalHeader << s.sprintf("0x%03X",i);

    ui->RamTable->setHorizontalHeaderLabels(horizontalHeader);
    ui->RamTable->setVerticalHeaderLabels(verticalHeader);
    ui->RamTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->RamTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
}

RamMonitor::~RamMonitor()
{
    delete ui;
}

void RamMonitor::changeMem(int addr, int value) //change the value of Memory[addr]
{
    QString s;
    s.sprintf("%02X",value);
    QTableWidgetItem *Memdata = new QTableWidgetItem(s);
    Memdata->setTextAlignment(Qt::AlignCenter);
    ui->RamTable->setItem(addr/16, addr%16, Memdata);
}
