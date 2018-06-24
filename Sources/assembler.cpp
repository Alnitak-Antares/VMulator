#include <QString>
#include <QRegExp>
//#include <QDebug>

QRegExp IRMOV("^IRMOV R([0-7]) 0x([0-9A-F]{4,4})$");
QRegExp IRMOVS("^IRMOVS R([0-7]) 0x([0-9A-F]{4,4})$");
QRegExp RRMOV("^RRMOV R([0-7]) R([0-7])$");
QRegExp LAD("^LAD R([0-7]) R([0-7])$");
QRegExp STO("^STO R([0-7]) R([0-7])$");
QRegExp CLA("^CLA R([0-7])$");
QRegExp SET("^SET R([0-7])$");
QRegExp PUSH("^PUSH R([0-7])$");
QRegExp POP("^POP R([0-7])$");
QRegExp ADD("^ADD R([0-7]) R([0-7])$");
QRegExp SUB("^SUB R([0-7]) R([0-7])$");
QRegExp MUL("^MUL R([0-7]) R([0-7])$");
QRegExp DIV("^DIV R([0-7]) R([0-7])$");
QRegExp NEG("^NEG R([0-7])$");
QRegExp INC("^INC R([0-7])$");
QRegExp DEC("^DEC R([0-7])$");
QRegExp CMP("^CMP R([0-7]) R([0-7])$");
QRegExp AND("^AND R([0-7]) R([0-7])$");
QRegExp OR("^OR R([0-7]) R([0-7])$");
QRegExp NOT("^NOT R([0-7])$");
QRegExp XOR("^XOR R([0-7]) R([0-7])$");
QRegExp SHL("^SHL R([0-7]) R([0-7])$");
QRegExp SHR("^SHR R([0-7]) R([0-7])$");
QRegExp JMP("^JMP (-?)0x([0-9A-F]{4,4})$");
QRegExp JC("^JC (-?)0x([0-9A-F]{4,4})$");
QRegExp JO("^JO (-?)0x([0-9A-F]{4,4})$");
QRegExp JCM("^JCM (-?)0x([0-9A-F]{4,4})$");
QRegExp INT("^INT ([1-3])$");
QRegExp RETI("^RETI$");
QRegExp HLT("^HLT$");
QRegExp IN("^IN R([0-7])$");
QRegExp OUT("^OUT R([0-7])$");

QString hexToBin(QString hex)   //Convert a hexadecimal string to a binary string
{
    QString bin;
    int len = hex.size();
    for(int i = 0;i < len;i++)
    {
        if     (hex[i] == '0')  bin += "0000";
        else if(hex[i] == '1')  bin += "0001";
        else if(hex[i] == '2')  bin += "0010";
        else if(hex[i] == '3')  bin += "0011";
        else if(hex[i] == '4')  bin += "0100";
        else if(hex[i] == '5')  bin += "0101";
        else if(hex[i] == '6')  bin += "0110";
        else if(hex[i] == '7')  bin += "0111";
        else if(hex[i] == '8')  bin += "1000";
        else if(hex[i] == '9')  bin += "1001";
        else if(hex[i] == 'A')  bin += "1010";
        else if(hex[i] == 'B')  bin += "1011";
        else if(hex[i] == 'C')  bin += "1100";
        else if(hex[i] == 'D')  bin += "1101";
        else if(hex[i] == 'E')  bin += "1110";
        else if(hex[i] == 'F')  bin += "1111";
    }
    return bin;
}
QString assemble(QString s)
{
    QString ins;		// store assembled instruction 
    if(IRMOV.indexIn(s) != -1)	//IRMOV matched 
    {
        ins += "00000";		//operating code
    	ins += "0000000";	//reserved bits
        ins += hexToBin(IRMOV.cap(1));  //number of RD
        ins += hexToBin(IRMOV.cap(2));  //immediate operand
	}
    else if(IRMOVS.indexIn(s) != -1)    //IRMOVS matched
    {
        ins += "00001";		//operating code
        ins += "0000000";	//reserved bits
        ins += hexToBin(IRMOVS.cap(1));  //number of RD
        ins += hexToBin(IRMOVS.cap(2));  //immediate operand
    }
    else if(RRMOV.indexIn(s) != -1)    //RRMOVS matched
    {
        ins += "00010";		//operating code
        ins += "000";	//reserved bits
        ins += hexToBin(RRMOV.cap(1));  //number of RD
        ins += hexToBin(RRMOV.cap(2));  //number of RS
    }
    else if(LAD.indexIn(s) != -1)    //LAD matched
    {
        ins += "00011";		//operating code
        ins += "000";	//reserved bits
        ins += hexToBin(LAD.cap(1));  //number of RD
        ins += hexToBin(LAD.cap(2));  //number of RS
    }
    else if(STO.indexIn(s) != -1)    //STO matched
    {
        ins += "00100";		//operating code
        ins += "000";	//reserved bits
        ins += hexToBin(STO.cap(1));  //number of RD
        ins += hexToBin(STO.cap(2));  //number of RS
    }
    else if(CLA.indexIn(s) != -1)    //CLA matched
    {
        ins += "00101";		//operating code
        QString Rnum4 = hexToBin(CLA.cap(1)); //number of R(4 bits)
        ins += Rnum4.mid(1,3);  //number of R(3 bits)
    }
    else if(SET.indexIn(s) != -1)    //SET matched
    {
        ins += "00110";		//operating code
        QString Rnum4 = hexToBin(SET.cap(1)); //number of R(4 bits)
        ins += Rnum4.mid(1,3);  //number of R(3 bits)
    }
    else if(PUSH.indexIn(s) != -1)    //PUSH matched
    {
        ins += "00111";		//operating code
        QString Rnum4 = hexToBin(PUSH.cap(1)); //number of R(4 bits)
        ins += Rnum4.mid(1,3);  //number of R(3 bits)
    }
    else if(POP.indexIn(s) != -1)    //POP matched
    {
        ins += "01000";		//operating code
        QString Rnum4 = hexToBin(POP.cap(1)); //number of R(4 bits)
        ins += Rnum4.mid(1,3);  //number of R(3 bits)
    }
    else if(ADD.indexIn(s) != -1)    //ADD matched
    {
        ins += "01001";		//operating code
        ins += "000";	//reserved bits
        ins += hexToBin(ADD.cap(1));  //number of RD
        ins += hexToBin(ADD.cap(2));  //number of RS
    }
    else if(SUB.indexIn(s) != -1)    //SUB matched
    {
        ins += "01010";		//operating code
        ins += "000";	//reserved bits
        ins += hexToBin(SUB.cap(1));  //number of RD
        ins += hexToBin(SUB.cap(2));  //number of RS
    }
    else if(MUL.indexIn(s) != -1)    //MUL matched
    {
        ins += "01011";		//operating code
        ins += "000";	//reserved bits
        ins += hexToBin(MUL.cap(1));  //number of RD
        ins += hexToBin(MUL.cap(2));  //number of RS
    }
    else if(DIV.indexIn(s) != -1)    //DIV matched
    {
        ins += "01100";		//operating code
        ins += "000";	//reserved bits
        ins += hexToBin(DIV.cap(1));  //number of RD
        ins += hexToBin(DIV.cap(2));  //number of RS
    }
    else if(NEG.indexIn(s) != -1)    //NEG matched
    {
        ins += "01101";		//operating code
        QString Rnum4 = hexToBin(NEG.cap(1)); //number of R(4 bits)
        ins += Rnum4.mid(1,3);  //number of R(3 bits)
    }
    else if(INC.indexIn(s) != -1)    //INC matched
    {
        ins += "01110";		//operating code
        QString Rnum4 = hexToBin(INC.cap(1)); //number of R(4 bits)
        ins += Rnum4.mid(1,3);  //number of R(3 bits)
    }
    else if(DEC.indexIn(s) != -1)    //DEC matched
    {
        ins += "01111";		//operating code
        QString Rnum4 = hexToBin(DEC.cap(1)); //number of R(4 bits)
        ins += Rnum4.mid(1,3);  //number of R(3 bits)
    }
    else if(CMP.indexIn(s) != -1)    //CMP matched
    {
        ins += "10000";		//operating code
        ins += "000";	//reserved bits
        ins += hexToBin(CMP.cap(1));  //number of Ra
        ins += hexToBin(CMP.cap(2));  //number of Rb
    }
    else if(AND.indexIn(s) != -1)    //AND matched
    {
        ins += "10001";		//operating code
        ins += "000";	//reserved bits
        ins += hexToBin(AND.cap(1));  //number of RD
        ins += hexToBin(AND.cap(2));  //number of RS
    }
    else if(OR.indexIn(s) != -1)    //OR matched
    {
        ins += "10010";		//operating code
        ins += "000";	//reserved bits
        ins += hexToBin(OR.cap(1));  //number of RD
        ins += hexToBin(OR.cap(2));  //number of RS
    }
    else if(NOT.indexIn(s) != -1)    //NOT matched
    {
        ins += "10011";		//operating code
        QString Rnum4 = hexToBin(NOT.cap(1)); //number of R(4 bits)
        ins += Rnum4.mid(1,3);  //number of R(3 bits)
    }
    else if(XOR.indexIn(s) != -1)    //XOR matched
    {
        ins += "10100";		//operating code
        ins += "000";	//reserved bits
        ins += hexToBin(XOR.cap(1));  //number of RD
        ins += hexToBin(XOR.cap(2));  //number of RS
    }
    else if(SHL.indexIn(s) != -1)    //SHL matched
    {
        ins += "10101";		//operating code
        ins += "000";	//reserved bits
        ins += hexToBin(SHL.cap(1));  //number of RD
        ins += hexToBin(SHL.cap(2));  //number of RS
    }
    else if(SHR.indexIn(s) != -1)    //SHR matched
    {
        ins += "10110";		//operating code
        ins += "000";	//reserved bits
        ins += hexToBin(SHR.cap(1));  //number of RD
        ins += hexToBin(SHR.cap(2));  //number of RS
    }
    else if(JMP.indexIn(s) != -1)    //JMP matched
    {
        ins += "10111";		//operating code
        ins += "0000000000";	//reserved bits
        if(JMP.cap(1) == "-")   //check the sign bit
            ins += "1";
        else
            ins += "0";
        ins += hexToBin(JMP.cap(2));  //immediate operand
    }
    else if(JC.indexIn(s) != -1)    //JC matched
    {
        ins += "11000";		//operating code
        ins += "0000000000";	//reserved bits
        if(JC.cap(1) == "-")   //check the sign bit
            ins += "1";
        else
            ins += "0";
        ins += hexToBin(JC.cap(2));  //immediate operand
    }
    else if(JO.indexIn(s) != -1)    //JO matched
    {
        ins += "11001";		//operating code
        ins += "0000000000";	//reserved bits
        if(JO.cap(1) == "-")   //check the sign bit
            ins += "1";
        else
            ins += "0";
        ins += hexToBin(JO.cap(2));  //immediate operand
    }
    else if(JCM.indexIn(s) != -1)    //JCM matched
    {
        ins += "11010";		//operating code
        ins += "0000000000";	//reserved bits
        if(JCM.cap(1) == "-")   //check the sign bit
            ins += "1";
        else
            ins += "0";
        ins += hexToBin(JCM.cap(2));  //immediate operand
    }
    else if(INT.indexIn(s) != -1)    //INT matched
    {
        ins += "11011";		//operating code
        QString Rnum4 = hexToBin(INT.cap(1)); //number of R(4 bits)
        ins += Rnum4.mid(1,3);  //number of R(3 bits)
    }
    else if(RETI.indexIn(s) != -1)    //RETI matched
    {
        ins += "11100";		//operating code
        ins += "000";	//reserved bits
    }
    else if(HLT.indexIn(s) != -1)    //HLT matched
    {
        ins += "11101";		//operating code
        ins += "000";	//reserved bits
    }
    else if(IN.indexIn(s) != -1)    //IN matched
    {
        ins += "11110";		//operating code
        QString Rnum4 = hexToBin(IN.cap(1)); //number of R(4 bits)
        ins += Rnum4.mid(1,3);  //number of R(3 bits)
    }
    else if(OUT.indexIn(s) != -1)    //OUT matched
    {
        ins += "11111";		//operating code
        QString Rnum4 = hexToBin(OUT.cap(1)); //number of R(4 bits)
        ins += Rnum4.mid(1,3);  //number of R(3 bits)
    }
    return ins;
}
