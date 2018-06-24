#include "vmulator.h"
#include "ui_vmulator.h"

bool execDone = false; //denoting that whether all the instructions have been executed
int insLen[32] = {4,4,2,2,2,1,1,1,1,2,2,2,2,1,1,1,2,2,2,1,2,2,2,4,4,4,4,1,1,1,1,1}; //the length of each instruction(How many bytes)

//stacks to store the states before execution, using to roll back to the last instruction
std::stack<int> changedMemAddr;
std::stack<int> changedMemWord;
std::stack< std::vector<byte> >DisplayMemoryStack;
std::stack<VM_State> lastState;

bool recRunningMode = Stepping;        //record running mode when starting wait input
QStringList insList;    //record instruction list when an interrupt occurs

void VMulator::wordTo4Byte(int word, int addr)  //store a word in memory as 4 bytes
{
    int value;
    for(int i = 0;i < 4;i++)
    {
        value = word >> (24 - 8*i);
        ramMonitor->changeMem(addr,value);  //show memory change in ram monitor
        Memory[addr++] = value;
    }
}
int VMulator::byte4ToWord(int addr)   //transfer 4 consecutive bytes in memory to a word
{
    int word = 0;
    for(int i = 0;i < 4;i++)
        word += Memory[addr++] << (24 - 8*i);
    return word;
}

void VMulator::insFetch()   //fetch instructions from memory to IR
{
    int op = (Memory[PC] >> 3) & 0x1f;  //get the first 5 bits of a byte
    for(int i = 0;i < insLen[op];i++)   //write instruction to IR
        IR = (IR << 8) + Memory[PC++];
    IR = IR << ((4-insLen[op]) * 8);      //make IR starts from the leftmost bit
}

void VMulator::execInput()  //execute hardware interrupt routine to get input
{
    static int cnt = 0; //count the number of executed instructions, which is used as the row number of current instruction

    ui->InstructionList->item(cnt)->setBackgroundColor(Qt::white);  //clear the background color of current instruction

    if(cnt == 0)    //STO IT INR
    {
        Memory[IT] = INR;        //store input
        ramMonitor->changeMem(IT,INR);  //show memory change in ram monitor
        InputCacheQueue += (unsigned char)INR;    //input cache queue enqueue
        PC += 2;

        ui->textBrowser_InputQueue->setText(InputCacheQueue);   //display input cache queue
        ui->InstructionList->item(cnt+1)->setBackgroundColor(QColor(175,238,238));  //highlight the next instruction to be executed
    }
    else if(cnt == 1)   //INC IT
    {
        IT++;
        PC++;
        ui->InstructionList->item(cnt+1)->setBackgroundColor(QColor(175,238,238));  //highlight the next instruction to be executed
    }
    else if(cnt == 2)   //RETI
    {
        PC = IAR;   //restore PC
        IM = 0; //unblock interrupt
        Stat = AOK;

        //restore the instruction list before
        ui->InstructionList->clear();
        ui->InstructionList->addItems(insList);
        insList.clear();
        ui->InstructionList->setCurrentRow(addrInsMap[PC]);
        if(addrInsMap[PC] != ui->InstructionList->count())  //if not done
            ui->InstructionList->currentItem()->setBackgroundColor(QColor(175,238,238));
    }

    cnt = (cnt + 1) % 3;

    display();
}

void VMulator::execForward()   //execute the next instruction
{
    if(IRR == 1) //check hardware interrupt
    {
        IAR = PC; //record interrupt address
        IRR = 0; //clear interrupt request
        IM = 1; //mask interrupt
        PC = byte4ToWord(0);   //read interrupt vector table(the input interrupt's interrupt identification code is 0)
        Stat = INT;

        insList.clear();
        for(int i = 0;i < ui->InstructionList->count();i++) //recorde current instruction list
            insList << ui->InstructionList->item(i)->text();

        //change instruction list to insterrupt service routine
        ui->InstructionList->clear();
        ui->InstructionList->addItem("STO IT INR");
        ui->InstructionList->addItem("INC IT");
        ui->InstructionList->addItem("RETI");

        //highlight the first instruction
        ui->InstructionList->setCurrentRow(0);
        ui->InstructionList->currentItem()->setBackgroundColor(QColor(175,238,238));

        display();

        return;
    }

    if(Stat != AOK)     //show error information and stop
    {
        if(Stat == WAI)
            QMessageBox::information(this,"等待中","等待输入，无法继续执行");
        else if(Stat == HLT)
            QMessageBox::information(this,"停机","已停机");
        else if(Stat == INT)
            execInput();
        else
            QMessageBox::warning(this,"错误","状态异常，无法继续执行");
        return;
    }

    if(execDone)
    {
        QMessageBox::information(this,"执行完毕","已执行完毕所有指令");
        if(RunningMode == Continuous)
        {
            RunningMode = Stepping;
            ui->Button_RunOrPause->setIcon(QIcon(":/Icons/Icons/Run.ico"));
            timer->stop();
        }
        return;
    }

    //clear the background color of current instruction
    ui->InstructionList->setCurrentRow(addrInsMap[PC]);
    ui->InstructionList->currentItem()->setBackgroundColor(Qt::white);

    //record current state before execute instruction
    VM_State vm_state;
    vm_state.Stat = Stat;
    vm_state.CF = CF;   vm_state.CMF = CMF; vm_state.OF = OF;
    vm_state.PC = PC;   vm_state.IR = IR;   vm_state.AR = AR;   vm_state.SP = SP;   vm_state.IH = IH;
    for(int i = 0;i < 8;i++)    vm_state.R[i] = R[i];
    lastState.push(vm_state);

    insFetch(); //fetch instruction from memory to IR

    int op = (IR >> 27) & 0x1f; //get the first 5 bits of IR as operation code
    int D,S,Imm;    //D,S = the corresponding index(0~7) of registers    Imm = the immediate operand
    bool sign;      //the sign bit of the Imm in JMP JC JO JCM
    long long ovCheck;  //check overflow
    int n;  //number n in instruction INT n

    switch(op)      //execute instruction according to the operation code
    {
        case 0:     //IRMOV RD Imm
            D = (IR >> 16) & 0x7;   Imm = IR & 0xffff;
            R[D] = R[D] & 0xffff0000;
            R[D] = R[D] | Imm;
            break;
        case 1:     //IRMOVS RD Imm
            D = (IR >> 16) & 0x7;   Imm = IR & 0xffff;
            R[D] = R[D] & 0x0000ffff;
            R[D] = R[D] | (Imm << 16);
            break;
        case 2:     //RRMOV RD RS
            D = (IR >> 20) & 0x7;   S = (IR >> 16) & 0x7;
            R[D] = R[S];
            break;
        case 3:     //LAD RD RS
            D = (IR >> 20) & 0x7;   S = (IR >> 16) & 0x7;
            AR = R[S];
            if( !(AR >= InsStartPos && AR < MemorySize) )
            {
               Stat = ADR;
               QMessageBox::warning(this,"错误","地址错误，内存访问越界");
               break;
            }
            R[D] = byte4ToWord(AR);
            break;
        case 4:     //STO RD RS
            D = (IR >> 20) & 0x7;   S = (IR >> 16) & 0x7;
            AR = R[D];

            if( !(AR >= InsStartPos && AR < MemorySize) )
            {
               Stat = ADR;
               QMessageBox::warning(this,"错误","地址错误，内存访问越界");
               break;
            }

            //record the memory unit being changed
            changedMemAddr.push(AR);
            changedMemWord.push(byte4ToWord(AR));

            wordTo4Byte(R[S],AR);
            break;
        case 5:     //CLA RD
            D = (IR >> 24) & 0x7;
            R[D] = 0;
            break;
        case 6:     //SET RD
            D = (IR >> 24) & 0x7;
            R[D] = 0xffffffff;
            break;
        case 7:     //PUSH RD
            if(SP == InsStartPos-1) //stack full
            {
                QMessageBox::warning(this,"错误","堆栈区满，无法执行入栈操作");
                Stat = STF;
                break;
            }

            //record the memory unit being changed
            changedMemAddr.push(SP+1);
            changedMemWord.push(byte4ToWord(SP+1));

            D = (IR >> 24) & 0x7;
            wordTo4Byte(R[D],SP+1);
            SP += 4;
            break;
        case 8:     //POP RD
            if(SP == StackAddr) //stack empty
            {
                QMessageBox::warning(this,"错误","堆栈区为空，无法执行出栈操作");
                Stat = STE;
                break;
            }
            D = (IR >> 24) & 0x7;
            R[D] = byte4ToWord(SP-3);
            SP -= 4;
            break;
        case 9:     //ADD RD RS
            D = (IR >> 20) & 0x7;   S = (IR >> 16) & 0x7;

            if( (unsigned int)R[D] + (unsigned int)R[S] < (unsigned int)R[D])   //check if the operation generates a carry
                CF = 1;
            else
                CF = 0;

            ovCheck = (long long)R[D] + (long long)R[S];
            if(ovCheck > INT_MAX || ovCheck < INT_MIN)  //check overflow
                OF = 1;
            else
                OF = 0;

            R[D] += R[S];
            break;
        case 10:     //SUB RD RS
            D = (IR >> 20) & 0x7;   S = (IR >> 16) & 0x7;

            if( (unsigned int)R[D] < (unsigned int)R[S])   //check if the operation generates a borrow
                CF = 1;
            else
                CF = 0;

            ovCheck = (long long)R[D] - (long long)R[S];
            if(ovCheck > INT_MAX || ovCheck < INT_MIN)  //check overflow
                OF = 1;
            else
                OF = 0;

            R[D] -= R[S];
            break;
        case 11:     //MUL RD RS
            D = (IR >> 20) & 0x7;   S = (IR >> 16) & 0x7;

            ovCheck = (long long)R[D] * (long long)R[S];
            if(ovCheck > INT_MAX || ovCheck < INT_MIN)  //check overflow
                OF = 1;
            else
                OF = 0;

            R[D] *= R[S];
            break;
        case 12:     //DIV RD RS
            D = (IR >> 20) & 0x7;   S = (IR >> 16) & 0x7;
            if(R[S] == 0)
            {
                QMessageBox::warning(this,"错误","运算错误，除数为零");
                Stat = DIV;
                break;
            }
            R[D] /= R[S];
            break;
        case 13:     //NEG RD
            D = (IR >> 24) & 0x7;
            R[D] = -R[D];
            break;
        case 14:     //INC RD
            D = (IR >> 24) & 0x7;
            R[D]++;
            break;
        case 15:     //DEC RD
            D = (IR >> 24) & 0x7;
            R[D]--;
            break;
        case 16:     //CMP RD RS
            D = (IR >> 20) & 0x7;   S = (IR >> 16) & 0x7;
            if(R[D] < R[S])
                CMF = 1;
            else
                CMF = 0;
            break;
        case 17:     //AND RD RS
            D = (IR >> 20) & 0x7;   S = (IR >> 16) & 0x7;
            R[D] = R[D] & R[S];
            break;
        case 18:     //OR RD RS
            D = (IR >> 20) & 0x7;   S = (IR >> 16) & 0x7;
            R[D] = R[D] | R[S];
            break;
        case 19:     //NOT RD
            D = (IR >> 24) & 0x7;
            R[D] = ~R[D];
            break;
        case 20:     //XOR RD RS
            D = (IR >> 20) & 0x7;   S = (IR >> 16) & 0x7;
            R[D] = R[D] ^ R[S];
            break;
        case 21:     //SHL RD RS
            D = (IR >> 20) & 0x7;   S = (IR >> 16) & 0x7;
            R[D] = R[D] << R[S];
            break;
        case 22:     //SHR RD RS
            D = (IR >> 20) & 0x7;   S = (IR >> 16) & 0x7;
            R[D] = R[D] >> R[S];
            break;
        case 23:     //JMP Imm
            sign = (IR >> 16) & 1;
            Imm = IR & 0xffff;
            if(sign == 1)
                PC -= Imm;
            else
                PC += Imm;
            break;
        case 24:     //JC Imm
            if(CF == 0)     break;

            sign = (IR >> 16) & 1;
            Imm = IR & 0xffff;
            if(sign == 1)
                PC -= Imm;
            else
                PC += Imm;
            break;
        case 25:     //JO Imm
            if(OF == 0)     break;

            sign = (IR >> 16) & 1;
            Imm = IR & 0xffff;
            if(sign == 1)
                PC -= Imm;
            else
                PC += Imm;
            break;
        case 26:     //JCM Imm
            if(CMF == 0)     break;

            sign = (IR >> 16) & 1;
            Imm = IR & 0xffff;
            if(sign == 1)
                PC -= Imm;
            else
                PC += Imm;
            break;
        case 27:     //INT n
            n = (IR >> 24) & 0x7;

            IAR = PC; //record interrupt address
            PC = byte4ToWord(n*4);   //read interrupt vector table(the input interrupt's interrupt identification code is 0)
            IM = 1; //mask interrupt

            //push state bit registers to stack segment
            Memory[++SP] = CF;      ramMonitor->changeMem(SP, CF);
            Memory[++SP] = CMF;     ramMonitor->changeMem(SP, CMF);
            Memory[++SP] = OF;      ramMonitor->changeMem(SP, OF);
            //push general purpose registers
            for(int i = 0; i < 8;i++)
            {
                wordTo4Byte(R[i],SP+1);
                SP += 4;
            }

            insList.clear();
            for(int i = 0;i < ui->InstructionList->count();i++) //record current instruction list
                insList << ui->InstructionList->item(i)->text();

            //show instruction list of interrupt routines
            ui->InstructionList->clear();
            ui->InstructionList->addItems(intRoutineLists[n]);

            break;
        case 28:     //RETI
            if(insList.empty()) //if not interrupted
            {
                QMessageBox::warning(this,"错误", "非中断状态下无法执行中断返回指令");
                Stat = INS;
                break;
            }

            PC = IAR;   //restore PC
            IM = 0; //unblock interrupt

            //restore general purpose registers from stack segment
            for(int i = 7;i >= 0;i--)
            {
                R[i] = byte4ToWord(SP-3);
                SP -= 4;
            }
            //pop state bit registers
            OF = Memory[SP--];
            CMF = Memory[SP--];
            CF = Memory[SP--];

            //restore the instruction list before interrupt
            ui->InstructionList->clear();
            ui->InstructionList->addItems(insList);
            ui->InstructionList->setCurrentRow(addrInsMap[PC]);
            if(addrInsMap[PC] != ui->InstructionList->count())  //if not done
                ui->InstructionList->currentItem()->setBackgroundColor(QColor(175,238,238));
            break;
        case 29:     //HLT
            Stat = HLT;
            break;
        case 30:     //IN RD
            D = (IR >> 24) & 0x7;
            if(IH == IT)    //input cache queue empty
            {
                recRunningMode = RunningMode;
                Stat = WAI;
                PC--;
            }
            else
            {
                R[D] = Memory[IH++];
                InputCacheQueue.remove(0,1);
                ui->textBrowser_InputQueue->setText(InputCacheQueue);
            }
            break;
        case 31:     //OUT RD
            D = (IR >> 24) & 0x7;
            if((R[D] & 0xff) == 0)        //check the last 8 bits
            {
                DisplayMemoryStack.push(DisplayMemory);
                DisplayMemory.clear();
            }
            else
                DisplayMemory.push_back(R[D] & 0xff);
            break;
    }

    if(addrInsMap.count(PC) == 0)  //if reached undefined position
    {
        QMessageBox::warning(this,"错误","跳转错误，PC指向未装载指令的地址");
        Stat = ADR;
    }
    else if(addrInsMap[PC] == ui->InstructionList->count()) //if reached the ending position of instrucitons
        execDone = true;

    if(Stat != AOK) //highlight the error instruction
    {
        if(Stat == WAI)
            ui->InstructionList->currentItem()->setBackgroundColor(Qt::yellow);  //set the background color of current instruction red to denote waiting
        else if(Stat == HLT)
            ui->InstructionList->currentItem()->setBackgroundColor(Qt::gray);  //set the background color of current instruction gray to denote hlt
        else
            ui->InstructionList->currentItem()->setBackgroundColor(Qt::red);  //set the background color of current instruction red to denote error
        if(RunningMode == Continuous)
        {
            RunningMode = Stepping;
            ui->Button_RunOrPause->setIcon(QIcon(":/Icons/Icons/Run.ico"));
            timer->stop();
        }
    }
    else if(!execDone)                        //if not done, highlight the next instruction to be executed
    {
        ui->InstructionList->setCurrentRow(addrInsMap[PC]);
        ui->InstructionList->currentItem()->setBackgroundColor(QColor(175,238,238));
    }

    display();  //show changes
}

void VMulator::execBackward()
{
    if(lastState.empty())
    {
        QMessageBox::warning(this,"回退失败","无可回退的上一条指令");
        return;
    }
    int op = (IR >> 27) & 0x1f; //get the operation code of the last executed instruction
    if((op == 4 || op == 7) && Stat == AOK)  //if the last instruction is STO or PUSH, restore the changed memory unit
    {
        int addr = changedMemAddr.top(), word = changedMemWord.top();
        wordTo4Byte(word,addr);
        changedMemAddr.pop();
        changedMemWord.pop();
    }
    else if(op == 28 && Stat != INS)   //if the last state is executing the interrupt service routine, rollback fails
    {
        QMessageBox::warning(this,"回退失败","无法回退至中断状态");
        return;
    }
    else if(op == 31)  //if the last instruction is OUT, delete the last character in display memory
    {
        if(DisplayMemory.empty())   //if DisplayMemory cleared
        {
            DisplayMemory = DisplayMemoryStack.top();
            DisplayMemoryStack.pop();
        }
        else
            DisplayMemory.pop_back();
    }

    //clear the background color of current instruction
    ui->InstructionList->setCurrentRow(addrInsMap[PC]);
    if(addrInsMap[PC] != ui->InstructionList->count())  //if not done
        ui->InstructionList->currentItem()->setBackgroundColor(Qt::white);

    //restore the state before the last instruction execution
    VM_State vm_state = lastState.top();
    Stat = vm_state.Stat;
    CF = vm_state.CF;   CMF = vm_state.CMF; vm_state.OF = vm_state.OF;
    PC = vm_state.PC;   IR = vm_state.IR;   AR = vm_state.AR;   SP = vm_state.SP;   IH = vm_state.IH;
    for(int i = 0;i < 8;i++)    R[i] = vm_state.R[i];
    lastState.pop();

    //highlight the last instruction
    ui->InstructionList->setCurrentRow(addrInsMap[PC]);
    ui->InstructionList->currentItem()->setBackgroundColor(QColor(175,238,238));

    display();
}
