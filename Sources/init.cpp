#include "vmulator.h"
#include "ui_vmulator.h"
#include "assembler.h"

void strToMem(QString ins, byte Memory[], int &pos) //Put a binary instruction string into memory
{
    int byteNum = ins.size()/8;
    for(int i = 0;i < byteNum;i++)
    {
        byte b = 0;
        for(int j = 0; j < 8;j++)
        {
            if(ins[i*8 + j] == '0')
                b = b << 1;
            else
                b = (b << 1) + 1;
        }
        Memory[pos++] = b;
        //qDebug() << hex << b;
    }
    //qDebug() << endl;
}

bool VMulator::load(QFile &insFile, int &pos)
{   //load instructions from file to VMulator memory
    QTextStream in(&insFile);
    QString line,ins;

    while(!in.atEnd())
    {
        line = in.readLine();
        ins = assemble(line);
        if(ins == "")   //no instruction matched
        {
            ui->InstructionList->clear();
            QMessageBox::warning(this,"指令加载失败","检测到异常指令" + line + "\n请重新加载指令");
            return false;
        }
        //qDebug() << ins << endl;
        addrInsMap[pos] = ui->InstructionList->count();
        strToMem(ins, Memory, pos);  //write instructions to memory
        ui->InstructionList->addItem(line);
    }    
    addrInsMap[pos] = ui->InstructionList->count();    //record the ending position

    insFile.close();
    return true;
}
void VMulator::initInterrupt()
{   //initialize interrupt vector table and interrupt service routines
    wordTo4Byte(0x400,0);
    int pos = 0x404;    //start position for INT 1
    for(int i = 1;i <= 3;i++)
    {
        wordTo4Byte(pos, i*4);    //set interrupt vector table
        QString s = ":/interrupt routines/INT " + QString::number(i) + ".txt";
        QFile intFile(s);
        intFile.open(QFile::ReadOnly | QFile::Text);
        load(intFile,pos);

        intRoutineLists[i].clear();
        for(int j = 0;j < ui->InstructionList->count();j++) //record the instruction list of interrupt routines
            intRoutineLists[i] << ui->InstructionList->item(j)->text();

        ui->InstructionList->clear();
    }
}
void VMulator::init()
{   //load instructions, initialize VMulator and display corresponing variables
    static bool firstInit = true;
    if(firstInit)   //interrupt vector table only need to be initialized once
    {
        initInterrupt();
        firstInit = false;
    }

    QString filename = QFileDialog::getOpenFileName(this,"打开文件","./","VMulator instructions(*.txt *.vmi)");
    QFile insFile(filename);
    if(!insFile.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox::warning(this,"警告","文件打开失败");
        return;
    }

    memset(Memory + InputQueueAddr,0,MemorySize - InputQueueAddr);    //clear memory except interrupt service routine
    ui->InstructionList->clear();       //clear current instruction list

    std::map<int,int>::iterator iter = addrInsMap.begin();
    for(; iter != addrInsMap.end();iter++)  //clear address-to-instruction map except interrupt routines
    {
        if(iter->first >= InsStartPos)
            addrInsMap.erase(iter);
    }     

    int pos = InsStartPos;

    if(!load(insFile, pos)) //load instructions
        return;

    for(int i = 0;i < MemorySize;i++)
        ramMonitor->changeMem(i,Memory[i]); //show memory data in ram monitor

    Stat = AOK;

    extern bool execDone;
    execDone = false;
    extern std::stack<VM_State> lastState;
    while(!lastState.empty())   lastState.pop();    //clear the rollback informations
    extern QStringList insList;
    insList.clear();    //clear the recorded instructions list

    DisplayMemory.clear();
    InputCacheQueue.clear();
    ui->textBrowser_InputQueue->clear();
    ui->InputText->clear();

    RunningMode = Stepping;     //reset running mode
    clockFreq = ui->lcdNumber_Freq->value();     //10 instructions per second at initial
    delete timer;
    timer = new QTimer(this);       //reset clock timer
    timer->setInterval(1000/clockFreq);
    connect(timer, SIGNAL(timeout()), this, SLOT(execForward()));

    //initialize registers
    CF = CMF = OF = IRR = IM = 0;
    PC = InsStartPos;
    SP = StackAddr;
    IAR = AR = IR = INR = 0;
    IH = IT = InputQueueAddr;
    for(int i = 0;i < 8;i++)
        R[i] = 0;

    if(ui->InstructionList->count() != 0)   //if not empty, highlight the first instruction
    {
        ui->InstructionList->setCurrentRow(0);
        ui->InstructionList->currentItem()->setBackgroundColor(QColor(175,238,238));
    }

    ui->Button_RunOrPause->setIcon(QIcon(":/Icons/Icons/Run.ico"));

    display();
}
