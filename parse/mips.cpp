//
//  mips.cpp
//  C0compiler
//
#include "globalData.h"
//定义变量表以及函数表的存储数据的类型
#define VOID 0
#define INT 1
#define CHAR 2
//数据的类型，加入变量表中
#define GLOBAL 0
#define LOCALCONST 1
#define LOCALVAR 2
#define PARA 3

VariableItem variableTable[MAXVARNUM];  //变量表
FunctionItem functionTable[MAXFUNCNUM]; //函数表
int sp, fp, gp;  //栈指针(相对地址)，帧指针，全局变量指针
int variableTableTop = 0;  //变量表的栈顶指针
int functionTableTop = 0;  //函数表的栈顶指针
int midCodeTablePtr = 0;  //四元式表的指针，从 0 开始一直分析到 MidCodeTableTop
int constEndAddress = -1;  //记录全局数据在变量表中的结束位置
bool isGlobal = false;  //在查找时，记录当前变量是否为全局变量
bool isMain = false;  //记录当前处理的函数是否为main主函数
int CompareLabel = 0;	//比较语句生成的标签的标号
int functionNum = 0; //用于生成函数结束时候的标签，用于return语句之后的跳转
bool useCountRegOpt = false;  //是否使用计数方法优化寄存器
ofstream outFile;   //程序的变量表信息的打印文件
vector<CountStruct> CountVector;  //存放CountStruct结构体的容器，存储变量出现的次数
vector<RegStruct> RegVector;  //存放RegStruct结构体的容器，存储变量分配的寄存器
map<string, vector<RegStruct>> FunctionRegMap; //每个函数名字为键，对应的寄存器分配容器为值
string NowFuncName = ""; //现在处理的函数名字
//bool hasCounted = false;
//将中间指令转换为mips指令
void midCode2Mips() {
	//再次初始化,因为调用了两次（优化前、优化后）
	variableTableTop = 0;
	functionTableTop = 0;
	midCodeTablePtr = 0;
	constEndAddress = -1;
	isGlobal = false;
	isMain = false;
	CompareLabel = 0;
	functionNum = 0;
	sp = fp = gp = 0;
	outFile.open("outDataInTable.txt");
	//先进行初始化，包括帧指针、全局变量存放地址以及保存$s0-$s7寄存器内容的地址（用$t9寄存器存放全局变量的起始地址）
	outMipsCode << ".text" << endl;
	outMipsCode << "addi\t$fp\t$sp\t0\t# fp = sp" << endl; //addi $fp $sp 0
	//outMipsCode << "li\t$t9\t0x7fffeffc\t#global stack bottom" << endl;  //全局变量存放地址
	outMipsCode << "add\t$t9\t$gp\t$0\t#global stack bottom" << endl;  //全局变量存放地址，从$gp开始  add $t9 $gp $0 
	//outMipsCode << "li\t$t9\t0x10007ffc\t#global stack bottom" << endl;  //全局变量存放地址，从$gp开始  li $t9 0x10007ffc
	//outMipsCode << "li\t$t8\t0x10010000\t#$s0-$s7 save address" << endl;  //保存$s0-$s7寄存器内容的地址
	sp = gp = variableTableTop = midCodeTablePtr = 0;  //初始化所有指针
	while (midCodeTablePtr < MidCodeTableTop)  //一直分析到四元式的结束
	{
		//处理全局常量的四元式
		while (MidCodeTable[midCodeTablePtr].op == "const") {
			globalConst2Mips();
		}
		//处理全局变量（非数组与数组一起处理）
		while (MidCodeTable[midCodeTablePtr].op == "int" || MidCodeTable[midCodeTablePtr].op == "char" ||
			MidCodeTable[midCodeTablePtr].op == "intArr" || MidCodeTable[midCodeTablePtr].op == "charArr")
		{
			//处理全局变量（非数组）
			if (MidCodeTable[midCodeTablePtr].op == "int" || MidCodeTable[midCodeTablePtr].op == "char") {
				globalIntOrChar2Mips();
			}
			//处理全局变量（数组）
			else {
				globalIntOrCharArray2Mips();
			}
		}
		//处理完全局数据后后，跳转到_main标签，开始主函数执行
		outMipsCode << "j\t_main" << endl;  // j _main
		//记录此时全局数据的结束之处在变量表中的位置，便于以后确定查找的变量的初始地址是相对于函数fp还是全局地址$t9
		constEndAddress = variableTableTop;
		//处理函数,将其加入函数表中，由于函数不存储数值，故无需加入运行栈
		//func, int / char / void, , 函数名字
		while (MidCodeTable[midCodeTablePtr].op == "func") {
			functionNum++;   //增加函数的数量
			funcExplain2Mips();
			//处理函数中的语句
			funcContent2Mips();
		}
	}
	/*for (map<string, vector<RegStruct>>::iterator iter = FunctionRegMap.begin(); iter != FunctionRegMap.end(); iter++) {
		cout << "funcName : " << iter->first << endl;
		for (vector<RegStruct>::iterator it = (iter->second).begin(); it != (iter->second).end(); it++) {
			cout << "name = " << (*it).name  << " number = " << (*it).number << " address = " << (*it).address <<
				 " regNum = " << (*it).regNum << endl;
			}
	}*/
}
//将传入的数据压入栈中,压入sp或者gp指针位置
void pushIntoStack(string value, int size, bool isGlobalData) {
	//若传入的size为1，则应该sw存入数值进栈
	if (size == 1) {  //仅存储一个非数组变量 数值存储在传入的value中
		//全局存入gp
		if (isGlobalData) {
			outMipsCode << "li\t$t0\t" << value << "\t#" << MidCodeTable[midCodeTablePtr].part_3 << endl; // li $t0 value 加上（常）变量名字的注释
			outMipsCode << "sw\t$t0\t0($gp)" << endl;  // sw $t0 0($gp)
		}
		//否则存入sp
		else {
			outMipsCode << "li\t$t0\t" << value << "\t#" << MidCodeTable[midCodeTablePtr].part_3 << endl; // li $t0 value 加上（常）变量名字的注释
			outMipsCode << "sw\t$t0\t0($sp)" << endl;  // sw $t0 0($sp)
		}
	}
	//全局则减少gp
	if (isGlobalData) {
		outMipsCode << "subi\t$gp\t$gp\t" << 4 * size << endl; //subi $gp $gp 4 * size
		gp -= 4 * size;
	}
	//否则减少sp
	else {
		outMipsCode << "subi\t$sp\t$sp\t" << 4 * size << endl; //subi $sp $sp 4 * size
		sp -= 4 * size;
	}
}

//将传入的变量插入到变量表中，便于之后的寻址操作
void insertIntoVariableTable(int kind, int DataType, int address, bool isGlobalData) {
	//数组变量名字此时存储的地址是数组开始位置的地址，之后索引需要变换一下
	variableTable[variableTableTop].kind = kind;
	//存储地址（相对地址）
	if (address == -1) {  
		//全局存入相对于gp的偏移量
		if (isGlobalData) {
			variableTable[variableTableTop].address = gp;
		}
		//否则存储相对于sp偏移量
		else {
			variableTable[variableTableTop].address = sp;
		}
	}
	else {
		variableTable[variableTableTop].address = address;
	}
	//存储名字	使用当前四元式索引
	variableTable[variableTableTop].name = MidCodeTable[midCodeTablePtr].part_3;
	variableTable[variableTableTop].DataType = DataType;
	variableTableTop++;
}

//将传入的函数加入函数表中
void insertIntoFuncTable(int kind, string name) {
	functionTable[functionTableTop].kind = kind;
	functionTable[functionTableTop].name = name;
	functionTableTop++;
}
//查找输入的变量名字是否在变量表中（没找到返回-1，找到返回地址（-4的倍数））
int findVarInVariableTable(string name) {
	isGlobal = false;  //初始化
					   //若为整型数字，则直接返回 -1
	if (isdigit(name[0]) || name[0] == '+' || name[0] == '-') {
		return -1;
	}
	//一定要从顶部向下查找，保证同名的变量会取到最近的那一个变量！！！
	for (int start = variableTableTop - 1; start >= 0; start--)
	{
		if (variableTable[start].name == name) {
			if (start < constEndAddress) {
				isGlobal = true;  //找到的数据为全局数据
			}
			return variableTable[start].address;
		}
	}
	return -1;
}
/*局部常量四元式转换为mips指令
形如 ： const, int/char, 数值, 常量标识符名字
*/
void const2Mips() {
	int kind = MidCodeTable[midCodeTablePtr].part_1 == "int" ? INT : CHAR;
	insertIntoVariableTable(kind, LOCALCONST);
	pushIntoStack(MidCodeTable[midCodeTablePtr].part_2);
	midCodeTablePtr++;
}
//全局常量声明转换为mips指令
void globalConst2Mips() {
	int kind = MidCodeTable[midCodeTablePtr].part_1 == "int" ? INT : CHAR;
	insertIntoVariableTable(kind, GLOBAL, -1, true);
	pushIntoStack(MidCodeTable[midCodeTablePtr].part_2, 1, true);
	midCodeTablePtr++;
}
/*局部变量（非数组）四元式转换为mips指令
形如 ： int/char,  ,  , 变量名字
*/
void intOrChar2Mips() {
	int kind = MidCodeTable[midCodeTablePtr].op == "int" ? INT : CHAR;
	insertIntoVariableTable(kind, LOCALVAR);
	pushIntoStack("0");  //变量此处存储数值 0 即可，作为初始化
	midCodeTablePtr++;
}
//全局变量（非数组）转换为mips指令
void globalIntOrChar2Mips() {
	int kind = MidCodeTable[midCodeTablePtr].op == "int" ? INT : CHAR;
	insertIntoVariableTable(kind, GLOBAL, -1, true);
	pushIntoStack("0", 1, true);  //变量此处存储数值 0 即可，作为初始化
	midCodeTablePtr++;
}
/*局部变量（数组）四元式转换为mips指令
形如 ： intArr/charArr, 0, 数组大小, 数组名字
*/
void intOrCharArray2Mips() {
	int kind = MidCodeTable[midCodeTablePtr].op == "intArr" ? INT : CHAR;
	insertIntoVariableTable(kind, LOCALVAR);
	pushIntoStack("0", atoi(MidCodeTable[midCodeTablePtr].part_2.c_str()));  //变量此处存储数值 0 即可，作为初始化,传入数组的大小
	midCodeTablePtr++;
}
//全局变量（数组）转换为mips指令
void globalIntOrCharArray2Mips() {
	int kind = MidCodeTable[midCodeTablePtr].op == "intArr" ? INT : CHAR;
	insertIntoVariableTable(kind, GLOBAL, -1, true);
	pushIntoStack("0", atoi(MidCodeTable[midCodeTablePtr].part_2.c_str()), true);  //变量此处存储数值 0 即可，作为初始化,传入数组的大小
	midCodeTablePtr++;
}
/*函数声明四元式转换为mips指令
形如 ： func,  int/char/void,  , 函数名字
*/
void funcExplain2Mips() {
	int kind = MidCodeTable[midCodeTablePtr].part_1 == "int" ? INT :
		MidCodeTable[midCodeTablePtr].part_1 == "char" ? CHAR : VOID;
	string name = MidCodeTable[midCodeTablePtr].part_3;
	insertIntoFuncTable(kind, name);
	NowFuncName = name;
	//若处理到main函数,生成_main标签
	if (name == "main") {
		isMain = true;
		outMipsCode << "_main:" << endl;
	}
	//否则生成函数名字的标签
	else {
		outMipsCode << name << ":" << endl;
	}
	midCodeTablePtr++;
}
/*处理函数中的部分，转换为mips指令
由于函数的结束声明为 end, 	, 	, 函数名字 ， 故处理到end即可结束该函数
函数首先要保存现场，即保存上一个函数的fp、保存返回地址，而后再开始处理函数内容*/
void funcContent2Mips() {
	int tempPtr = midCodeTablePtr;  //临时变量，用于扫描该函数中的所有中间代码
	sp = 0;  //重置 sp，因为数据采用相对地址来获取
			 //保存$s0-$s7寄存器的数值，上一个函数的fp、保存返回地址
	//outMipsCode << "#save $s0-$s7 Registers" << endl;
	//saveSRegisters();
	outMipsCode << "#save last Function $fp" << endl;
	outMipsCode << "sw\t$fp\t0($sp)" << endl;  // sw fp 0($sp)
	outMipsCode << "addi\t$fp\t$sp\t0\t# fp = sp" << endl; //addi $fp $sp 0 令 fp = sp
	outMipsCode << "subi\t$sp\t$sp\t" << 4 << endl; //subi $sp $sp 4  sp = sp - 4
	sp -= 4;
	outMipsCode << "#save last Function $ra" << endl;
	outMipsCode << "sw\t$ra\t0($sp)" << endl;  // sw ra 0($sp)
	outMipsCode << "subi\t$sp\t$sp\t" << 4 << endl; //subi $sp $sp 4  sp = sp - 4
	sp -= 4;
	outMipsCode << "#save last Function $fp and $ra Finished" << endl;
	//先扫描一遍函数所有四元式，将里面出现的所有临时变量加入变量表中（四元式生成的临时变量用$_X表示）
	while (MidCodeTable[midCodeTablePtr].op != "end") {
		string part_1 = "", part_2 = "", part_3 = "";
		part_1 = MidCodeTable[midCodeTablePtr].part_1;
		part_2 = MidCodeTable[midCodeTablePtr].part_2;
		part_3 = MidCodeTable[midCodeTablePtr].part_3;
		//所有的临时变量（四元式产生的）加入变量表
		if (part_1.length() > 0 && part_1[0] == '$' && findVarInVariableTable(part_1) == -1) { //不在变量表中，则加入变量表
			insertIntoVariableTable(INT, LOCALVAR);  //part_1的临时变量是运算的结果，故数据类型为INT
			pushIntoStack("0");
		}
		if (part_2.length() > 0 && part_2[0] == '$' && findVarInVariableTable(part_2) == -1) { //不在变量表中，则加入变量表
			insertIntoVariableTable(INT, LOCALVAR);  //part_2的临时变量是运算的结果，故数据类型为INT
			pushIntoStack("0");
		}
		if (part_3.length() > 0 && part_3[0] == '$' && findVarInVariableTable(part_3) == -1) { //不在变量表中，则加入变量表
			//part_3的临时变量，对于数组和函数调用，需要判断数组元素类型以及函数返回值类型
			if (MidCodeTable[midCodeTablePtr].op == "getArr") {  //需要得到数组的类型
				//getArr, 数组名字， 数组下标， 结果存储的寄存器
				insertIntoVariableTable(0, LOCALVAR);
			}
			//调用函数语句 call, 函数名字，	，寄存器变量（或者为空）
			else if (MidCodeTable[midCodeTablePtr].op == "call") {
				//call, 函数名字，	，寄存器变量（或者为空）
				insertIntoVariableTable(0, LOCALVAR);
			}
			else {  //其他临时变量，类型直接设置为INT即可
				insertIntoVariableTable(INT, LOCALVAR);
			}
			pushIntoStack("0");  //寄存器变量的大小为4个字节，先初始化变量为 0 
		}
		midCodeTablePtr++;
	}
	midCodeTablePtr = tempPtr; //还原四元式的索引，处理代码中的变量
	//再次扫描，得到函数的寄存器分配情况，将该函数的寄存器分配方案加入FunctionRegMap中
	while (MidCodeTable[midCodeTablePtr].op != "end") {
		//将变量（包括参数，数组）全部加入变量表中
		while (MidCodeTable[midCodeTablePtr].op == "const" || MidCodeTable[midCodeTablePtr].op == "int" || MidCodeTable[midCodeTablePtr].op == "char"
			|| MidCodeTable[midCodeTablePtr].op == "intArr" || MidCodeTable[midCodeTablePtr].op == "charArr" || MidCodeTable[midCodeTablePtr].op == "para") {
			//函数参数（定义）   para,  int/char,  ,参数名字
			if (MidCodeTable[midCodeTablePtr].op == "para") {
				para2Mips();
			}
			//常量定义
			else if (MidCodeTable[midCodeTablePtr].op == "const") {
				const2Mips();
			}
			//变量定义（非数组）
			else if (MidCodeTable[midCodeTablePtr].op == "int" || MidCodeTable[midCodeTablePtr].op == "char") {
				intOrChar2Mips();
			}
			//变量定义（数组）
			else if (MidCodeTable[midCodeTablePtr].op == "intArr" || MidCodeTable[midCodeTablePtr].op == "charArr") {
				intOrCharArray2Mips();
			}
			else {
				break;
			}
		}
		break;
	}

	//若要进行优化
	//此处使用引用计数方法，因为变量已经加入了变量表中，所以此处进行统计工作
	if (useCountRegOpt) {
		//进行变量的寄存器分配
		CountReg();
		/*for (int i = 0; i < RegVector.size(); i++)
		{
			cout << "name = " << RegVector[i].name << " number = " << RegVector[i].number <<
				" address = " << RegVector[i].address << " regNum = " << RegVector[i].regNum << endl;
		}*/
		//cout << "end" << endl;
		//for (int i = 0; i < CountVector.size(); i++)
		//{
		//	//cout << RegVector.size() << endl;
		//	cout << "name = " << CountVector[i].name << " number = " << CountVector[i].number <<
		//		" count = " << CountVector[i].count << endl;
		//}
		//cout << "hhhhh" << endl;
		//将该函数的寄存器分配方案加入FunctionRegMap中
		FunctionRegMap[NowFuncName] = RegVector;
		CountVector.clear();
		RegVector.clear();

	}
	//再次扫描函数所有四元式，处理所有的四元表达式
	//midCodeTablePtr = tempPtr; //还原四元式的索引
	while (MidCodeTable[midCodeTablePtr].op != "end") {

		//加减乘除 + - * /
		if (MidCodeTable[midCodeTablePtr].op == "+" || MidCodeTable[midCodeTablePtr].op == "-" ||
			MidCodeTable[midCodeTablePtr].op == "*" || MidCodeTable[midCodeTablePtr].op == "/") {
			int type = MidCodeTable[midCodeTablePtr].op == "+" ? 0 : MidCodeTable[midCodeTablePtr].op == "-" ? 1 :
				MidCodeTable[midCodeTablePtr].op == "*" ? 2 : MidCodeTable[midCodeTablePtr].op == "/" ? 3 : -1;
			calculation2Mips(type);
		}
		//比较运算符	< <= > >= == != 
		if (MidCodeTable[midCodeTablePtr].op == "<" || MidCodeTable[midCodeTablePtr].op == "<=" ||
			MidCodeTable[midCodeTablePtr].op == ">" || MidCodeTable[midCodeTablePtr].op == ">=" ||
			MidCodeTable[midCodeTablePtr].op == "!=" || MidCodeTable[midCodeTablePtr].op == "==") {
			int type = MidCodeTable[midCodeTablePtr].op == "<" ? 0 : MidCodeTable[midCodeTablePtr].op == "<=" ? 1 :
				type = MidCodeTable[midCodeTablePtr].op == ">" ? 2 : MidCodeTable[midCodeTablePtr].op == ">=" ? 3 :
				type = MidCodeTable[midCodeTablePtr].op == "!=" ? 4 : MidCodeTable[midCodeTablePtr].op == "==" ? 5 : -1;
			compare2Mips(type);
		}
		//实参语句 funcPa， ， ，实参存储的寄存器(或者为数字)
		if (MidCodeTable[midCodeTablePtr].op == "funcPa") {
			funcPa2Mips();
		}
		//得到数组元素 getArr, 数组名字， 数组下标， 结果存储的寄存器
		if (MidCodeTable[midCodeTablePtr].op == "getArr") {
			getArr2Mips();
		}
		//赋值语句（非数组） =， 要赋的数值， ， 变量标识符名字
		if (MidCodeTable[midCodeTablePtr].op == "=") {
			Assign2Mips();
		}
		//赋值语句（数组）	[]=, 数组名字， 数组下标， 要存储的数值
		if (MidCodeTable[midCodeTablePtr].op == "[]=") {
			AssignArr2Mips();
		}
		//读语句	scf， int/char， ， 变量标识符
		if (MidCodeTable[midCodeTablePtr].op == "scf") {
			scf2Mips();
		}
		//写语句		ptf，字符串(可以为空)， 结果寄存器(可以为空)，char/int/或者为空
		if (MidCodeTable[midCodeTablePtr].op == "ptf") {
			ptf2Mips();
		}
		//返回语句 ret， ， ，保存结果的寄存器（也可以为整数）（或者为空）
		if (MidCodeTable[midCodeTablePtr].op == "ret") {
			ret2Mips();
		}
		//调用函数语句 call, 函数名字，	，寄存器变量（或者为空）
		if (MidCodeTable[midCodeTablePtr].op == "call") {
			call2Mips();
		}
		//无条件跳转语句	jump， ， ，label
		if (MidCodeTable[midCodeTablePtr].op == "jump") {
			jump2Mips();
		}
		//条件不满足跳转语句	jne， ， ，label
		if (MidCodeTable[midCodeTablePtr].op == "jne") {
			jne2Mips();
		}
		//标签语句	genLabel， ， ，label
		if (MidCodeTable[midCodeTablePtr].op == "genLabel") {
			genLabel2Mips();
		}
	}
	//打印每个函数结束时候的变量表
	outFile << "=======================================" << endl;
	outFile << "name\tkind\taddress\tDataType" << endl;
	for (int i = 0; i < variableTableTop; i++)
	{
		VariableItem variableitem = variableTable[i];
		outFile << variableitem.name << "\t" << variableitem.kind << "\t" << variableitem.address << "\t" << variableitem.DataType << endl;
	}
	//outFile << "=======================================" << endl;
	variableTableTop = constEndAddress;
	//生成函数结束时候的结束标签
	outMipsCode << "FuncEndLabel_" << functionNum << ":" << endl;  //FuncEndLabel_x:
	//恢复现场，退回到调用该函数前的状态
	outMipsCode << "lw\t$ra\t-4($fp)" << endl;   //lw $ra -4($fp),返回地址填入$ra
	outMipsCode << "addi\t$sp\t$fp\t0" << endl;//addi $sp $fp 0	 //恢复调用该函数的函数$sp	 $sp = $fp												  
	outMipsCode << "lw\t$fp\t0($fp)" << endl;//lw $fp 0($fp)//恢复调用该函数的函数$fp
	//loadSRegisters();
	if (isMain) {  //若处理的是主函数，则处理结束后直接结束
		outMipsCode << "li\t$v0\t10" << endl;   //li $v0 10
		outMipsCode << "syscall" << endl;  //syscall 程序结束
	}
	else { // 否则跳转到$ra存储的地址
		outMipsCode << "jr\t$ra" << endl;   //jr $ra
	}
	midCodeTablePtr++;
}

//引用计数方法
void CountReg() {
	int tempPointer = midCodeTablePtr;
	//直到处理到一个函数的末尾end为止
	while (MidCodeTable[tempPointer].op != "end")
	{
		fourExpression tempExp = MidCodeTable[tempPointer];
		if (tempExp.op == "+" || tempExp.op == "-" || tempExp.op == "*" || tempExp.op == "/" || tempExp.op == "=") {
			countNumOfVar(tempExp.part_1);
			countNumOfVar(tempExp.part_2);
			countNumOfVar(tempExp.part_3);
		}
		tempPointer++;
	}
	//处理结束后，对CountStruct按照count字段进行排序，从大到小，选择前面的8个（如不足8个，则全部选择）分配寄存器
	if (!CountVector.empty()) {
		sort(CountVector.begin(), CountVector.end(), compareVector);
		//只分配$s0 - $s7寄存器
		for (int i = 0; i < CountVector.size(); i++)
		{
			RegStruct regStruct;
			regStruct.number = CountVector[i].number;
			regStruct.address = CountVector[i].address;
			regStruct.name = CountVector[i].name;
			regStruct.regNum = i;
			RegVector.push_back(regStruct);
			//若没有寄存器可以分配，则结束分配
			if (i == 7) {
				break;
			}
		}
	}
}
//自定义比较函数(从大到小)
bool compareVector(const CountStruct &a, const CountStruct &b) {
	return a.count > b.count;
}
//为传入的变量计数
void countNumOfVar(string varName) {
	//若处理的不是变量，则直接返回即可
	if (!isVarName(varName)) {
		return;
	}
	int temp = variableTableTop - 1;
	//constEndAddress记录全局数据的顶部位置，此处计数局部变量故temp要 >= constEndAddress才可以循环
	//只考虑局部变量的情况，即DataType为LOCALVAR
	while (temp >= constEndAddress)
	{
		VariableItem tempItem = variableTable[temp];
		//如果在变量表中查找到该变量，temp此时为改变量在变量表中的位置索引
		if (tempItem.name == varName && tempItem.DataType == LOCALVAR) {
			bool NotInVec = true;
			//在记录变量的CountVector中查找变量
			for (vector<CountStruct>::iterator iter = CountVector.begin(); iter != CountVector.end(); iter++)
			{
				//容器中有该变量记录，则数量+1 即可
				if ((*iter).number == temp) {
					(*iter).count += 1;
					NotInVec = false;
					break;
				}
			}
			//若不在，则添加
			if (NotInVec) {
				CountStruct countStruct;
				countStruct.number = temp;
				countStruct.count = 1;
				countStruct.address = tempItem.address;
				countStruct.name = tempItem.name;
				//添加到容器中
				CountVector.push_back(countStruct);
			}
		}
		temp--;
	}
	
}
//判断是否为变量的名字，如果是空的或者是数字，则返回false，否则是变量名字,返回true
bool isVarName(string name) {
	if (name == "") {
		return false;
	}
	if (name.length() > 0 && isNumber(name)) {
		return false;
	}
	//临时变量不计算
	if (name.length() > 0 && name[0] == '$') {
		return false;
	}
	return true;
}
//保存$s0-$s7寄存器中的数值
void saveSRegisters(string functionName) {
	//若当前函数不需要使用寄存器，直接返回
	for (int i = 0; i < FunctionRegMap[functionName].size() ; i++)
	{
		int address = FunctionRegMap[functionName][i].address;
		int regNum = FunctionRegMap[functionName][i].regNum;
		//将寄存器中的数值存储到对应的地址空间中
		outMipsCode << "sw\t$s" << regNum << "\t" << address << "($fp)" << endl;  // sw $s0 address($fp)
	}
}
//恢复$s0-$s7寄存器中的数值
void loadSRegisters(string functionName) {
	//若将要运行的函数不需要使用寄存器，直接返回
	for (int i = 0; i < FunctionRegMap[functionName].size(); i++)
	{
		int address = FunctionRegMap[functionName][i].address;
		int regNum = FunctionRegMap[functionName][i].regNum;
		//将寄存器中的数值存储到对应的地址空间中
		outMipsCode << "lw\t$s" << regNum << "\t" << address << "($fp)" << endl;  // lw $s0 address($fp)
	}
}
//在函数的寄存器分配map中查找，若存在该变量，则使用分配的寄存器即可，无需再lw或者sw
bool VarUseRegister(string FuncName, string varName, string tempRegister, bool needSw) {
	vector<RegStruct> tempVec = FunctionRegMap[FuncName];
	for (vector<RegStruct>::iterator iter = tempVec.begin(); iter != tempVec.end(); iter++)
	{
		//能够用寄存器来代替地址操作
		if (iter->name == varName) {
			//取值操作
			if (!needSw) {
				outMipsCode << "addi\t" << tempRegister << "\t$s" << iter->regNum << "\t0" << endl;  //addi $t0 $s0 0
			}
			//赋值操作
			else {
				outMipsCode << "addi\t$s" << iter->regNum << "\t" << tempRegister << "\t0" << endl;  //addi $s0 $t0 0
			}
			return true;
		}
	}
	return false;
}
/*加减乘除运算符转换为mips指令
包括 + - * /
形如 ： + a, b, c
*/
void calculation2Mips(int type) {
	string op = (type == 0) ? "add" : type == 1 ? "sub" : type == 2 ? "mul" : type == 3 ? "div" : "";
	int addr_1, addr_2, addr_3;
	string part_1 = "", part_2 = "", part_3 = "";
	part_1 = MidCodeTable[midCodeTablePtr].part_1;
	part_2 = MidCodeTable[midCodeTablePtr].part_2;
	part_3 = MidCodeTable[midCodeTablePtr].part_3;
	//先分析part_1是不是整数
	if (part_1.length() > 0 && isdigit(part_1[0]) || part_1[0] == '+' || part_1[0] == '-') {
		outMipsCode << "li\t$t0\t" << part_1 << endl;  //li $t0 number
	}
	else {
	//先在函数的寄存器分配map中查找，若存在该变量，则使用分配的寄存器即可，无需再lw或者sw
		addr_1 = findVarInVariableTable(part_1);  //得到该变量对应的相对地址
		if (isGlobal) {
			//相对于全局变量起始位置的偏移
			outMipsCode << "lw\t$t0\t" << addr_1 << "($t9)" << endl;  //lw $t0 addr_1($t9) 
		}
		else {
			//先在函数的寄存器分配map中查找，若存在该变量，则使用分配的寄存器即可，无需再lw或者sw
			//如果没有用到，则进行lw 或者sw操作
			if (useCountRegOpt && VarUseRegister(NowFuncName, part_1, "$t0")) {
				;
			}
			else {
				//相对于函数fp的偏移
				outMipsCode << "lw\t$t0\t" << addr_1 << "($fp)" << endl;  //lw $t0 addr_1($fp) 
			}
		}
	}
	//先分析part_2是不是整数
	if (part_2.length() > 0 && isdigit(part_2[0]) || part_2[0] == '+' || part_2[0] == '-') {
		outMipsCode << "li\t$t1\t" << part_2 << endl;  //li $t0 number
	}
	else {
		addr_2 = findVarInVariableTable(part_2);  //得到该变量对应的相对地址
		if (isGlobal) {
			//相对于全局变量起始位置的偏移
			outMipsCode << "lw\t$t1\t" << addr_2 << "($t9)" << endl;  //lw $t0 addr_2($t9) 
		}
		else {
			if (useCountRegOpt && VarUseRegister(NowFuncName, part_2, "$t1")) {
				;
			}
			else {
				//相对于函数fp的偏移
				outMipsCode << "lw\t$t1\t" << addr_2 << "($fp)" << endl;  //lw $t0 addr_2($fp)
			}
		}
	}
	addr_3 = findVarInVariableTable(part_3);  //得到该变量对应的相对地址
	outMipsCode << op << "\t$t0\t$t0\t$t1" << endl;  //$t0 = $t0 op $t1
	if (isGlobal) {
		outMipsCode << "sw\t$t0\t" << addr_3 << "($t9)" << endl;  //sw $t0 addr_3($t9)
	}
	else {
		if (useCountRegOpt && VarUseRegister(NowFuncName, part_3, "$t0", true)) {
			;
		}
		else {
			outMipsCode << "sw\t$t0\t" << addr_3 << "($fp)" << endl;  //sw $t0 addr_3($fp)
		}
	}
	midCodeTablePtr++;
}

/*比较运算符转换为mips指令
包括 < <= > >= == !=
形如 ： lss a, b,
*/
void compare2Mips(int type) {
	int addr_1, addr_2;
	string part_1 = "", part_2 = "";
	string label_1 = "", label_2 = "";
	part_1 = MidCodeTable[midCodeTablePtr].part_1;
	part_2 = MidCodeTable[midCodeTablePtr].part_2;
	//先分析part_1是不是整数
	if (part_1.length() > 0 && isdigit(part_1[0]) || part_1[0] == '+' || part_1[0] == '-') {
		outMipsCode << "li\t$t0\t" << part_1 << endl;  //li $t0 number
	}
	else {
		addr_1 = findVarInVariableTable(part_1);  //得到该变量对应的相对地址
		if (isGlobal) {
			//相对于全局变量起始位置的偏移
			outMipsCode << "lw\t$t0\t" << addr_1 << "($t9)" << endl;  //lw $t0 addr_1($t9) 
		}
		else {
			if (useCountRegOpt && VarUseRegister(NowFuncName, part_1, "$t0")) {
				;
			}
			else {
				//相对于函数fp的偏移
				outMipsCode << "lw\t$t0\t" << addr_1 << "($fp)" << endl;  //lw $t0 addr_1($fp) 
			}
		}
	}
	//先分析part_2是不是整数
	if (part_2.length() > 0 && isdigit(part_2[0]) || part_2[0] == '+' || part_2[0] == '-') {
		outMipsCode << "li\t$t1\t" << part_2 << endl;  //li $t0 number
	}
	else {
		addr_2 = findVarInVariableTable(part_2);  //得到该变量对应的相对地址
		if (isGlobal) {
			//相对于全局变量起始位置的偏移
			outMipsCode << "lw\t$t1\t" << addr_2 << "($t9)" << endl;  //lw $t0 addr_2($t9) 
		}
		else {
			if (useCountRegOpt && VarUseRegister(NowFuncName, part_2, "$t1")) {
				;
			}
			else {
				//相对于函数fp的偏移
				outMipsCode << "lw\t$t1\t" << addr_2 << "($fp)" << endl;  //lw $t0 addr_2($fp)
			}
			 
		}
	}
	switch (type)
	{  //最终保证 $t0存储比较结果，若为0则条件不成立；为1 条件成立
	case 0:  // < a < b
		outMipsCode << "slt\t$t0\t$t0\t$t1" << endl;  //slt $t0 $t0 $t1 
		break;
	case 1:  // <=  if(a <= b)  转换成 a > b 则 用 （1 - 结果） 即可用 $t0存储比较结果
		outMipsCode << "slt\t$t0\t$t1\t$t0" << endl;  //slt $t0 $t1 $t0
		outMipsCode << "li\t$t1\t1" << endl;  //li $t1 1 
		outMipsCode << "sub\t$t0\t$t1\t$t0" << endl;  //sub $t0 $t1 $t0
		break;
	case 2:  // > a > b
		outMipsCode << "slt\t$t0\t$t1\t$t0" << endl;  //slt $t0 $t1 $t0 
		break;
	case 3:  // >=  if(a >= b)  转换成 a < b 则 用 （1 - 结果） 即可用 $t0存储比较结果
		outMipsCode << "slt\t$t0\t$t0\t$t1" << endl;  //slt $t0 $t0 $t1
		outMipsCode << "li\t$t1\t1" << endl;  //li $t1 1 
		outMipsCode << "sub\t$t0\t$t1\t$t0" << endl;  //sub $t0 $t1 $t0
		break;
	case 4:  // !=  if(a != b)  $t0 = 1 else  $t0 = 0
		label_1 = genCompareLabel();
		label_2 = genCompareLabel();
		outMipsCode << "beq\t$t0\t$t1\t" << label_1 << endl;  //若相等则跳转label_1，赋值$t0 = 0
		outMipsCode << "li\t$t0\t1" << endl;  //赋值$t0 = 1
		outMipsCode << "j\t" << label_2 << endl;//跳转标签label_2
		outMipsCode << label_1 << ":" << endl;  //生成标签 label_1
		outMipsCode << "li\t$t0\t0" << endl;  //赋值$t0 = 0
		outMipsCode << label_2 << ":" << endl;  //生成标签 label_2
		break;
	case 5:  // ==  if(a == b)  $t0 = 1 else  $t0 = 0
		label_1 = genCompareLabel();
		label_2 = genCompareLabel();
		outMipsCode << "bne\t$t0\t$t1\t" << label_1 << endl;  //若不等则跳转label_1，赋值$t0 = 0
		outMipsCode << "li\t$t0\t1" << endl;  //赋值$t0 = 1
		outMipsCode << "j\t" << label_2 << endl;//跳转标签label_2
		outMipsCode << label_1 << ":" << endl;  //生成标签 label_1
		outMipsCode << "li\t$t0\t0" << endl;  //赋值$t0 = 0
		outMipsCode << label_2 << ":" << endl;  //生成标签 label_2
		break;
	default:
		break;
	}
	midCodeTablePtr++;
}
//为执行的跳转语句生成跳转标签并返回
string genCompareLabel() {
	stringstream ss;
	ss << CompareLabel;
	CompareLabel++;
	return "CompareLabel_" + ss.str();
}

/*参数声明四元式转换为mips指令
形如 ： para,  int/char,  ,参数名字
*/
void para2Mips() {
	//需要将参数加入变量表中,参数不入栈，只代表相对地址！
	string part_1 = "";
	int kind;
	int paraNum = 0;
	//统计参数个数，然后写入地址（该地址与调用该函数传入的实参地址对应）
	for (int i = midCodeTablePtr; i < MidCodeTableTop; i++)
	{
		if (MidCodeTable[i].op == "para") {
			paraNum++;
		}
		else {
			break;
		}
	}
	for (int i = 0; i < paraNum; i++)
	{
		part_1 = MidCodeTable[midCodeTablePtr].part_1;
		kind = part_1 == "int" ? INT : CHAR;
		insertIntoVariableTable(kind, PARA, 4 * (paraNum - i));//因为最开始的实参位于栈的最底部，故应该4 * (paraNum - i)是真正的偏移量
		midCodeTablePtr++;
	}
}

/*实参四元式转换为mips指令
形如 ： funcPa， ， ，实参存储的寄存器(或者为数字)
*/
void funcPa2Mips() {
	int addr;
	string part_3 = "";
	part_3 = MidCodeTable[midCodeTablePtr].part_3;
	//判断是否为数字
	if (part_3.length() > 0 && isdigit(part_3[0]) || part_3[0] == '+' || part_3[0] == '-') {
		outMipsCode << "li\t$t0\t" << part_3 << endl;  //li $t0 number
	}
	else {
		addr = findVarInVariableTable(part_3);
		if (isGlobal) {
			//相对于全局变量起始位置的偏移
			outMipsCode << "lw\t$t0\t" << addr << "($t9)" << endl;  //lw $t0 addr($t9) 
		}
		else {
			if (useCountRegOpt && VarUseRegister(NowFuncName, part_3, "$t0")) {
				;
			}
			else {
				//相对于函数fp的偏移
				outMipsCode << "lw\t$t0\t" << addr << "($fp)" << endl;  //lw $t0 addr($fp)
			}
			 
		}
	}
	//将实参压栈
	outMipsCode << "sw\t$t0\t0($sp)" << endl;  //sw $t0 0($sp) 
	outMipsCode << "subi\t$sp\t$sp\t4" << endl;  //subi $sp $sp 4
	sp -= 4;
	midCodeTablePtr++;

}

/*获取数组元素转换为mips指令
形如 ： getArr, 数组名字， 数组下标， 结果存储的寄存器
*/
void getArr2Mips() {
	int addr_1, addr_2, addr_3;
	string part_1 = "", part_2 = "", part_3 = "";
	part_1 = MidCodeTable[midCodeTablePtr].part_1;
	part_2 = MidCodeTable[midCodeTablePtr].part_2;
	part_3 = MidCodeTable[midCodeTablePtr].part_3;
	//得到part_1的地址 
	//数组的地址存储最高位，故涉及到数组读取需要将下标数值取反！！！！
	addr_1 = findVarInVariableTable(part_1);
	//临时存储该数组是否为全局变量
	bool part_1_isGlobal = isGlobal;
	//判断数组下标是否为整数，得到偏移量存入$t0中
	if (part_2.length() > 0 && isdigit(part_2[0]) || part_2[0] == '+') {
		outMipsCode << "li\t$t0\t" << part_2 << endl;  //li $t0 number
	}
	else {
		addr_2 = findVarInVariableTable(part_2);  //得到该变量对应的相对地址
		if (isGlobal) {
			//相对于全局变量起始位置的偏移
			outMipsCode << "lw\t$t0\t" << addr_2 << "($t9)" << endl;  //lw $t0 addr_2($t9) 
		}
		else {
			if (useCountRegOpt && VarUseRegister(NowFuncName, part_2, "$t0")) {
				;
			}
			else {
				//相对于函数fp的偏移
				outMipsCode << "lw\t$t0\t" << addr_2 << "($fp)" << endl;  //lw $t0 addr_2($fp) 
			}
		}
	}
	outMipsCode << "mul\t$t0\t$t0\t4" << endl;  //mul $t0 $t0 4 
												//数组的地址存储最高位，故涉及到数组读取需要将下标数值取反！！！！
	outMipsCode << "sub\t$t0\t$0\t$t0" << endl;	 //sub $t0 $0 $t0 // $t0 = 0 - $t0
	outMipsCode << "addi\t$t0\t$t0\t" << addr_1 << endl;	 //addi $t0 $t0 addr_1 // $t0 = $t0 + addr_1（此时$t0已经是相对地址了！负数数值或者0！）
															 //此时$t0存储真正偏移量（字节）
															 //先把数组中元素lw取出来，再去sw到指定寄存器中
															 //得到要赋值的数组的位置
	if (part_1_isGlobal) {
		outMipsCode << "add\t$t0\t$t0\t$t9" << endl;	 //add $t0 $t0 $t9 // $t0 = $t0 + $t9
	}
	else {
		outMipsCode << "add\t$t0\t$t0\t$fp" << endl;	 //add $t0 $t0 $fp // $t0 = $t0 + $fp
	}
	//先把数组中元素lw取出来到$t1
	outMipsCode << "lw\t$t1\t0($t0)" << endl;  //lw $t1 0($t0)
											   //再去sw到指定寄存器中
											   //得到结果存储的寄存器地址
	addr_3 = findVarInVariableTable(part_3);
	if (isGlobal) {
		//相对于全局变量起始位置的偏移
		outMipsCode << "sw\t$t1\t" << addr_3 << "($t9)" << endl;  //sw $t1 addr_3($t9)
	}
	else {
		if (useCountRegOpt && VarUseRegister(NowFuncName, part_3, "$t1", true)) {
			;
		}
		else {
			//相对于函数fp的偏移			
			outMipsCode << "sw\t$t1\t" << addr_3 << "($fp)" << endl;  //sw $t1 addr_3($fp)
		}
	}
	midCodeTablePtr++;

}

/*变量赋值转换为mips指令
形如 ： =， 要赋的数值， ， 变量标识符名字
*/
void Assign2Mips() {
	int addr_1, addr_3;
	string part_1 = "", part_3 = "";
	part_1 = MidCodeTable[midCodeTablePtr].part_1;
	part_3 = MidCodeTable[midCodeTablePtr].part_3;
	//先分析part_1是不是整数
	if (part_1.length() > 0 && isdigit(part_1[0]) || part_1[0] == '+' || part_1[0] == '-') {
		outMipsCode << "li\t$t0\t" << part_1 << endl;  //li $t0 number
	}
	//写到这里了
	//////////
	else {
		addr_1 = findVarInVariableTable(part_1);  //得到该变量对应的相对地址
		if (isGlobal) {
			//相对于全局变量起始位置的偏移
			outMipsCode << "lw\t$t0\t" << addr_1 << "($t9)" << endl;  //lw $t0 addr_1($t9) 
		}
		else {
			if (useCountRegOpt && VarUseRegister(NowFuncName, part_1, "$t0")) {
				;
			}
			else {
				//相对于函数的偏移
				outMipsCode << "lw\t$t0\t" << addr_1 << "($fp)" << endl;  //lw $t0 addr_1($fp) 
			}
		}
	}
	addr_3 = findVarInVariableTable(part_3);  //得到该变量对应的相对地址
	if (isGlobal) {
		outMipsCode << "sw\t$t0\t" << addr_3 << "($t9)" << endl;  //sw $t0 addr_3($t9)
	}
	else {
		if (useCountRegOpt && VarUseRegister(NowFuncName, part_3, "$t0", true)) {
			;
		}
		else {
			outMipsCode << "sw\t$t0\t" << addr_3 << "($fp)" << endl;  //sw $t0 addr_3($fp)
		}
	}
	midCodeTablePtr++;
}

/*数组赋值转换为mips指令
形如 ： []=, 数组名字， 数组下标， 要存储的数值
*/
void AssignArr2Mips() {
	int addr_1, addr_2, addr_3;
	string part_1 = "", part_2 = "", part_3 = "";
	part_1 = MidCodeTable[midCodeTablePtr].part_1;
	part_2 = MidCodeTable[midCodeTablePtr].part_2;
	part_3 = MidCodeTable[midCodeTablePtr].part_3;
	//得到part_1的地址
	addr_1 = findVarInVariableTable(part_1);
	bool part_1_isGlobal = isGlobal;
	//判断数组下标是否为整数，得到偏移量存入$t0中
	if (part_2.length() > 0 && isdigit(part_2[0]) || part_2[0] == '+') {
		outMipsCode << "li\t$t0\t" << part_2 << endl;  //li $t0 number
	}
	else {
		addr_2 = findVarInVariableTable(part_2);  //得到该变量对应的相对地址
		if (isGlobal) {
			//相对于全局变量起始位置的偏移
			outMipsCode << "lw\t$t0\t" << addr_2 << "($t9)" << endl;  //lw $t0 addr_2($t9) 
		}
		else {
			if (useCountRegOpt && VarUseRegister(NowFuncName, part_2, "$t0")) {
				;
			}
			else {
				//相对于函数fp的偏移
				outMipsCode << "lw\t$t0\t" << addr_2 << "($fp)" << endl;  //lw $t0 addr_2($fp) 
			}
		}
	}
	outMipsCode << "mul\t$t0\t$t0\t4" << endl;  //mul $t0 $t0 4 
												//数组的地址存储最高位，故涉及到数组读取需要将下标数值取反！！！！
	outMipsCode << "sub\t$t0\t$0\t$t0" << endl;	 //sub $t0 $0 $t0 // $t0 = 0 - $t0
	outMipsCode << "addi\t$t0\t$t0\t" << addr_1 << endl;	 //addi $t0 $t0 addr_1 // $t0 = $t0 + addr_1
	 //此时$t0存储真正偏移量（字节）
	//得到要赋值的数组的位置,(最后sw偏移量应该为0)
	if (part_1_isGlobal) {
		outMipsCode << "add\t$t0\t$t0\t$t9" << endl;	 //add $t0 $t0 $t9 // $t0 = $t0 + $t9
	}
	else {
		outMipsCode << "add\t$t0\t$t0\t$fp" << endl;	 //add $t0 $t0 $fp // $t0 = $t0 + $fp
	}
	//判断要存储的数据是否为整数
	if (part_3.length() > 0 && isdigit(part_3[0]) || part_3[0] == '+' || part_3[0] == '-') {
		outMipsCode << "li\t$t1\t" << part_3 << endl;  //li $t1 number
	}
	else {
		//得到要存储的数值的地址
		addr_3 = findVarInVariableTable(part_3);
		if (isGlobal) {
			//相对于全局变量起始位置的偏移
			outMipsCode << "lw\t$t1\t" << addr_3 << "($t9)" << endl;  //lw $t1 addr_3($t9) 
		}
		else {
			if (useCountRegOpt && VarUseRegister(NowFuncName, part_3, "$t1")) {
				;
			}
			else {
				//相对于函数fp的偏移
				outMipsCode << "lw\t$t1\t" << addr_3 << "($fp)" << endl;  //lw $t1 addr_3($fp) 
			}
		}
	}
	outMipsCode << "sw\t$t1\t0($t0)" << endl;  //sw $t1 0($t0)
	midCodeTablePtr++;
}

/*读语句 转换为mips指令
形如 ： scf， int/char， ， 变量标识符
*/
void scf2Mips() {
	int addr;
	string part_1 = "", part_3 = "";
	part_1 = MidCodeTable[midCodeTablePtr].part_1;
	part_3 = MidCodeTable[midCodeTablePtr].part_3;
	//找到变量地址
	addr = findVarInVariableTable(part_3);
	int kind = part_1 == "int" ? INT : CHAR;
	if (kind == INT) {
		outMipsCode << "li\t$v0\t5" << endl;  //li $v0 5 读取整数
	}
	else {
		outMipsCode << "li\t$v0\t12" << endl;  //li $v0 12 读取字符
	}
	outMipsCode << "syscall" << endl;  //syscall   //结果存储在$v0中
	if (isGlobal) {
		outMipsCode << "sw\t$v0\t" << addr << "($t9)" << endl;  //sw $v0 addr($t9) 存储
	}
	else {
		if (useCountRegOpt && VarUseRegister(NowFuncName, part_3, "$v0", true)) {
			;
		}
		else {
			outMipsCode << "sw\t$v0\t" << addr << "($fp)" << endl;  //sw $v0 addr($fp) 存储
		}
	}
	midCodeTablePtr++;
}

/*写语句 转换为mips指令
形如 ： ptf，字符串(可以为空)， 结果寄存器(可以为空)，char/int/或者为空
*/
void ptf2Mips() {
	int addr;
	string part_1 = "", part_2 = "", part_3 = "";
	part_1 = MidCodeTable[midCodeTablePtr].part_1;
	part_2 = MidCodeTable[midCodeTablePtr].part_2;
	part_3 = MidCodeTable[midCodeTablePtr].part_3;
	//先看字符串是否为空
	if (part_1 != "") {
		//不为空则进行打印  (调用系统调用)
		for (int i = 0; i < part_1.length(); i++)
		{
			outMipsCode << "li\t$v0\t11" << endl;  //li $v0 11
			outMipsCode << "li\t$a0\t" << int(part_1[i]) << endl;  //li $a0 字符转换为整数
			outMipsCode << "syscall" << endl;  //syscall
		}
	}
	//分析寄存器(不为空的情况下进行分析)
	if (part_2 != "") {
		if (isdigit(part_2[0]) || part_2[0] == '+' || part_2[0] == '-') {
			//字符的打印
			if (part_3 == "char") {
				outMipsCode << "li\t$v0\t11" << endl;  //li $v0 11
				outMipsCode << "li\t$a0\t" << part_2 << endl;  //li $a0 part_2(part_2为数字)
				outMipsCode << "syscall" << endl;  //syscall
			}
			//数字的打印
			else if (part_3 == "int") {
				outMipsCode << "li\t$v0\t1" << endl;  //li $v0 1
				outMipsCode << "li\t$a0\t" << part_2 << endl;  //li $a0 part_2(part_2为数字)
				outMipsCode << "syscall" << endl;  //syscall
			}
		}
		//寄存器或者变量的打印
		else {
			addr = findVarInVariableTable(part_2);
			int kind = part_3 == "int" ? INT : CHAR;
			if (kind == INT) {
				outMipsCode << "li\t$v0\t1" << endl;  //li $v0 1
			}
			else {
				outMipsCode << "li\t$v0\t11" << endl;  //li $v0 11
			}
			if (isGlobal) {
				outMipsCode << "lw\t$a0\t" << addr << "($t9)" << endl;  //lw $a0 addr($t9)
			}
			else {
				if (useCountRegOpt && VarUseRegister(NowFuncName, part_2, "$a0")) {
					;
				}
				else {
					outMipsCode << "lw\t$a0\t" << addr << "($fp)" << endl;  //lw $a0 addr($fp)
				}
			}
			outMipsCode << "syscall" << endl;  //syscall
		}
	}
	//打印换行符
	//outMipsCode << "li\t$a0\t\'\\n\'" << endl;  //li $a0 '\n'
	//outMipsCode << "li\t$v0\t11" << endl;  //li $v0 11
	//outMipsCode << "syscall" << endl;  //syscall
	midCodeTablePtr++;
}

/*返回语句 转换为mips指令
形如 ： ret， ， ，保存结果的寄存器（也可以为整数）（或者为空）
*/
void ret2Mips() {
	int addr;
	string  part_3 = "";
	part_3 = MidCodeTable[midCodeTablePtr].part_3;
	//不为空则进行返回
	if (part_3 != "") {
		//若part_3为整数
		if (isdigit(part_3[0]) || part_3[0] == '+' || part_3[0] == '-') {
			outMipsCode << "li\t$v0\t" << part_3 << endl;  //lw $v0 part_3
		}
		else {
			addr = findVarInVariableTable(part_3);
			if (isGlobal) {
				outMipsCode << "lw\t$v0\t" << addr << "($t9)" << endl;  //lw $v0 addr($t9)
			}
			else {
				if (useCountRegOpt && VarUseRegister(NowFuncName, part_3, "$v0")) {
					;
				}
				else {
					outMipsCode << "lw\t$v0\t" << addr << "($fp)" << endl;  //lw $v0 addr($fp)
				}
			}
		}
	}
	outMipsCode << "j\tFuncEndLabel_" << functionNum << endl;  //j FuncEndLabel_x
	midCodeTablePtr++;

}

/*调用函数四元式转换为mips指令
形如 ： call, 函数名字，	，寄存器变量（或者为空）
*/
void call2Mips() {  //调用函数
	//xieddasdaskdhaskdjasjdhsajdhsajk
	int addr;
	string preFuncName = NowFuncName;
	
	//若采用优化
	if (useCountRegOpt) {
		//保存S系列寄存器中的数值到地址空间中
		saveSRegisters(preFuncName);
	}
	string  part_1 = "", part_3 = "";
	part_1 = MidCodeTable[midCodeTablePtr].part_1;
	part_3 = MidCodeTable[midCodeTablePtr].part_3;
	outMipsCode << "jal\t" << part_1 << endl;  //jal 函数名字
	outMipsCode << "nop" << endl;  //插入nop指令
	
	//若采用优化
	if (useCountRegOpt) {
		//传入之前函数名字，将相关变量lw进来
		loadSRegisters(preFuncName);
	}
	//函数返回值存储在$v0中,将其存储到寄存器变量
	if (part_3 != "") {
		addr = findVarInVariableTable(part_3);
		if (isGlobal) {
			outMipsCode << "sw\t$v0\t" << addr << "($t9)" << endl; //sw $v0 addr($t9)
		}
		else {
			if (useCountRegOpt && VarUseRegister(NowFuncName, part_3, "$v0", true)) {
				;
			}
			else {
				outMipsCode << "sw\t$v0\t" << addr << "($fp)" << endl; //sw $v0 addr($fp)
			}
		}
	}
	midCodeTablePtr++;
}

/*无条件跳转 转换为mips指令
形如 ： jump， ， ，label
*/
void jump2Mips() {
	string  part_3 = MidCodeTable[midCodeTablePtr].part_3;
	outMipsCode << "j\t" << part_3 << endl;  //j part_3
	midCodeTablePtr++;
}

/*不满足条件跳转 转换为mips指令
形如 ： jne， ， ，label
*/
void jne2Mips() {  //判断$t0 的数值，为1 则不跳转，不为1 跳转
	string  part_3 = MidCodeTable[midCodeTablePtr].part_3;
	outMipsCode << "bne\t$t0\t1\t" << part_3 << endl;  //bne $t0, 1, label
	midCodeTablePtr++;
}

/*生成标签 转换为mips指令
形如 ： genLabel， ， ，label
*/
void genLabel2Mips() {
	string  part_3 = MidCodeTable[midCodeTablePtr].part_3;
	outMipsCode << part_3 << ":" << endl;  //label:
	midCodeTablePtr++;
}

