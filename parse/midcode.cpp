//
//  midcode.cpp
//  C0compiler
//
#include "globalData.h"
int RegisterCount = 0;  //临时变量个数
int labelCount = 0;   //标签个数
fourExpression MidCodeTable[MAXMIDECODESIZE];
int MidCodeTableTop = 0;  //中间代码指针

//插入四元式到四元式数组
void insertMiddleCode(string op, string str_a, string str_b, string str_c) {
	//if (op == "func") {  //打印函数之间的分界符号
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
		error(TableFull);  //四元式中间代码表满了
		return;
	}
}

//生成新的临时变量，临时变量的开头为$，便于之后转换成mips的目标代码
string genNewRegister() { 
	string newRegister;
	stringstream ss;
	ss << RegisterCount;
	newRegister = "$_" + ss.str();
	//生成新的临时变量
	RegisterCount++;//临时变量数量增加
	return newRegister;
}

//生成新的标签
string genNewLabel() { 
	string newLabel;
	stringstream ss;
	ss << labelCount;
	newLabel = "LABEL_" + ss.str();
	//生成新的标签
	labelCount++;//标签数量增加
	return newLabel;
}