#include "vmulator.h"
#include "ui_vmulator.h"

void VMulator::display()    //display registers, VMulator state and monitor
{
    switch(Stat)    //display VMulator state
    {
        case AOK:   ui->label_Stat->setText("运行正常"); break;
        case HLT:   ui->label_Stat->setText("已停机");  break;
        case ADR:   ui->label_Stat->setText("地址错误"); break;
        case INS:   ui->label_Stat->setText("指令错误"); break;
        case DIV:   ui->label_Stat->setText("除零错误"); break;
        case STE:   ui->label_Stat->setText("出栈错误"); break;
        case STF:   ui->label_Stat->setText("入栈错误"); break;
        case WAI:   ui->label_Stat->setText("等待输入"); break;
        case INT:   ui->label_Stat->setText("中断执行中"); break;
    }
    //display condition-code registers
    if(CF == 1)
        ui->label_CF->setText("1");
    else
        ui->label_CF->setText("0");
    if(CMF == 1)
        ui->label_CMF->setText("1");
    else
        ui->label_CMF->setText("0");
    if(OF == 1)
        ui->label_OF->setText("1");
    else
        ui->label_OF->setText("0");
    if(IRR == 1)
        ui->label_IRR->setText("1");
    else
        ui->label_IRR->setText("0");
    if(IM == 1)
        ui->label_IM->setText("1");
    else
        ui->label_IM->setText("0");

    char str[20];
    //display special purpose registers
    sprintf(str,"0x%08X",PC);
    ui->label_PC->setText(str);
    sprintf(str,"0x%08X",IR);;
    ui->label_IR->setText(str);
    sprintf(str,"0x%08X",AR);;
    ui->label_AR->setText(str);
    sprintf(str,"0x%08X",SP);;
    ui->label_SP->setText(str);
    sprintf(str,"0x%08X",IAR);;
    ui->label_IAR->setText(str);
    sprintf(str,"0x%08X",INR);;
    ui->label_INR->setText(str);
    sprintf(str,"0x%08X",IH);;
    ui->label_IH->setText(str);
    sprintf(str,"0x%08X",IT);;
    ui->label_IT->setText(str);

    //display general purpose registers
    sprintf(str,"0x%08X",R[0]);;
    ui->label_R0->setText(str);
    sprintf(str,"0x%08X",R[1]);;
    ui->label_R1->setText(str);
    sprintf(str,"0x%08X",R[2]);;
    ui->label_R2->setText(str);
    sprintf(str,"0x%08X",R[3]);;
    ui->label_R3->setText(str);
    sprintf(str,"0x%08X",R[4]);;
    ui->label_R4->setText(str);
    sprintf(str,"0x%08X",R[5]);;
    ui->label_R5->setText(str);
    sprintf(str,"0x%08X",R[6]);;
    ui->label_R6->setText(str);
    sprintf(str,"0x%08X",R[7]);;
    ui->label_R7->setText(str);

    //display monitor
    QString s;
    for(int i = 0;i < DisplayMemory.size();i++)
        s += DisplayMemory[i];
    ui->textBrower_Monitor->setText(s);
}
