#ifndef RAMMONITOR_H
#define RAMMONITOR_H

#include <QDialog>

namespace Ui {
class RamMonitor;
}

class RamMonitor : public QDialog
{
    Q_OBJECT

public:
    explicit RamMonitor(QWidget *parent = 0);
    ~RamMonitor();

    void changeMem(int addr, int value);

private:
    Ui::RamMonitor *ui;
};

#endif // RAMMONITOR_H
