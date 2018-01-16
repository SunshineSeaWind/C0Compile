//
//  midcode.cpp
//  C0compiler
//
#include "globalData.h"
int RegisterCount = 0;  //��ʱ��������
int labelCount = 0;   //��ǩ����
fourExpression MidCodeTable[MAXMIDECODESIZE];
int MidCodeTableTop = 0;  //�м����ָ��

//������Ԫʽ����Ԫʽ����
void insertMiddleCode(string op, string str_a, string str_b, string str_c) {
	//if (op == "func") {  //��ӡ����֮��ķֽ����
	//	outMidCode << "========================================================" << endl;
	//}
	if (MidCodeTableTop < MAXMIDECODESIZE) {
		outMidCode << "\t" << op << ",\t" << str_a << ",\t" << str_b << ",\t" << str_c << endl;
		MidCodeTable[MidCodeTableTop].op = op;
		MidCodeTable[MidCodeTableTop].part_1 = str_a;
		MidCodeTable[MidCodeTableTop].part_2 = str_b;
		MidCodeTable[MidCodeTableTop].part_3 = str_c;
		MidCodeTableTop++;
	}
	else {
		error(TableFull);  //��Ԫʽ�м���������
		return;
	}
}

//�����µ���ʱ��������ʱ�����Ŀ�ͷΪ$������֮��ת����mips��Ŀ�����
string genNewRegister() { 
	string newRegister;
	stringstream ss;
	ss << RegisterCount;
	newRegister = "$_" + ss.str();
	//�����µ���ʱ����
	RegisterCount++;//��ʱ������������
	return newRegister;
}

//�����µı�ǩ
string genNewLabel() { 
	string newLabel;
	stringstream ss;
	ss << labelCount;
	newLabel = "LABEL_" + ss.str();
	//�����µı�ǩ
	labelCount++;//��ǩ��������
	return newLabel;
}