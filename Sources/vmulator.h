#ifndef VMULATOR_H
#define VMULATOR_H

#include <QMainWindow>

#include <QColor>
#include <QMessageBox>
#include <QDebug>
#include <QTimer>
#include <QFile>
#include <QFileDialog>
#include <vector>
#include <map>
#include <stack>
#include "RamMonitor.h"
#include "help.h"

//fixed window size
#define Window_width 1280
#define Window_height 720

typedef unsigned char byte;

typedef struct VM_State //the state to record before execution, using to rollback to last state
{
    int Stat;  //The state bit
    bool CF, CMF, OF;  //condition-code registers
    unsigned int PC, IR, AR, SP, IH;    //special purpose registers
    int R[8];                   //general purpose registers
}VM_State;

//define the state of VMulatorA
#define AOK 1   //no exceptions
#define HLT 2   //instruction HLT executed
#define ADR 3   //invalid address
#define INS 4   //invalid instruction
#define DIV 5   //divided by zero
#define STE 6   //pop when stack empty
#define STF 7   //push when stack full
#define WAI 8   //waiting input
#define INT 9   //hardware interrupt

//define the execution mode of VMulator
#define Stepping 0
#define Continuous 1

#define MemorySize 0x10000  //64KB Memory

#define InsStartPos 0x5000      //start position for instruction/data segment in memory
#define InputQueueAddr 0x2000   //start position for storing the input data
#define StackAddr   0x3000      //start position for the stack pointer


namespace Ui {
class VMulator;
}

class VMulator : public QMainWindow
{
    Q_OBJECT

public:
    explicit VMulator(QWidget *parent = 0);
    ~VMulator();

private slots:
    void on_actionOpen_triggered();

    void on_actionRamMonitor_triggered();

    void on_Button_RunOrPause_clicked();

    void on_Button_Forward_clicked();

    void on_Button_Backward_clicked();

    void on_FreqScrollBar_valueChanged(int value);

    void on_pushButton_Input_clicked();

    void execForward();

    void on_actionHelp_triggered();

private:
    Ui::VMulator *ui;
    RamMonitor *ramMonitor = new RamMonitor(this);
    Help *help = new Help(this);

    byte Memory[MemorySize];
    std::vector<byte> DisplayMemory;

    byte Stat;      //denoting the execution state of VMulator(halted, waiting input etc.)
    bool RunningMode;  //Stepping or Continuous
    unsigned int clockFreq;     //This clockFreq variable simply denotes that how many instructions the cpu can executive in a second;

    //Registers
    bool CF, CMF, OF, IRR, IM;  //state bit registers
    unsigned int PC, IR, AR, SP, IAR, INR, IH, IT;    //special purpose registers
    int R[8];                   //general purpose registers

    QTimer *timer = new QTimer;     //clock timer using to control the frequency of execution in continuous running mode
    QString InputCacheQueue;
    std::map<int,int> addrInsMap;   //a map from address to the instructin number to specify the position of each instrciton in the instrction list
    QStringList intRoutineLists[7]; //the instruction lists of interrupt routine to show in interrupter state

    bool load(QFile &insFile, int &pos);
    void initInterrupt();
    void init();

    void display();

    void insFetch();
    void wordTo4Byte(int word, int addr);
    int byte4ToWord(int addr);

    void execInput();
    //void execForward();   //defined above as Qt slot
    void execBackward();

};

#endif // VMULATOR_H
