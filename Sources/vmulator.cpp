#include "vmulator.h"
#include "ui_vmulator.h"
#include "assembler.h"


VMulator::VMulator(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::VMulator)
{
    ui->setupUi(this);
}

VMulator::~VMulator()
{
    delete ui;
}


void VMulator::on_actionOpen_triggered()
{   //load instructions and intialize VMulator
    init();
}
void VMulator::on_actionRamMonitor_triggered()
{
    ramMonitor->show();
}

void VMulator::on_actionHelp_triggered()
{
    help->show();
}
void VMulator::on_Button_RunOrPause_clicked()
{
    if(ui->InstructionList->count() == 0)   //no response if instruction list empty
       return;

    if( Stat != AOK && !(Stat == WAI && IRR == 1) ) //if not normal state, show error
    {
        execForward();
        return;
    }

    //Change RunningMode
    if(RunningMode == Stepping)
    {
        RunningMode = Continuous;
        ui->Button_RunOrPause->setIcon(QIcon(":/Icons/Icons/Pause.ico"));
        timer->start();
    }
    else
    {
        RunningMode = Stepping;
        ui->Button_RunOrPause->setIcon(QIcon(":/Icons/Icons/Run.ico"));
        timer->stop();
    }
}

void VMulator::on_Button_Forward_clicked()
{
    if(RunningMode == Continuous || ui->InstructionList->count() == 0)
       return;   //no response in continuous mode or instruction list empty

    execForward();
}

void VMulator::on_Button_Backward_clicked()
{
    if(RunningMode == Continuous || ui->InstructionList->count() == 0)
       return;   //no response in continuous mode or instruction list empty

    execBackward();
}



void VMulator::on_FreqScrollBar_valueChanged(int value)
{   //change clock frequency
    clockFreq = value;
    timer->setInterval(1000/clockFreq);
}

void VMulator::on_pushButton_Input_clicked()
{
    if(IM == 1) //interrupt masked
    {
        QMessageBox::information(this,"中断屏蔽","中断屏蔽中，无法输入");
        return;
    }
    if(Stat != AOK && Stat != WAI)  //error state
    {
        QMessageBox::warning(this,"错误","状态异常，无法输入");
        return;
    }

    QString s = ui->InputText->toPlainText();   //get input text
    if(s == "") //no response if input text empty
    {
        QMessageBox::warning(this,"错误","输入为空，请重新输入");
        return;
    }

    IRR = 1;
    INR = s[0].toLatin1();

    s.remove(0,1);      //input text queue dequeue
    ui->InputText->setPlainText(s); //display changed input text

    display();  //display changed register INR and IRR

    extern bool recRunningMode;
    if(Stat == WAI && recRunningMode == Continuous)     //restart execution if waiting in Continuous mode
    {
        RunningMode = Continuous;
        ui->Button_RunOrPause->setIcon(QIcon(":/Icons/Icons/Pause.ico"));
        timer->start();
    }
}
