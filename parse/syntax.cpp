//
//  syntax.cpp
//  C0compiler
//

#include "globalData.h"
int funcTotalNum = 0; //代表符号表中分程序的总数，全局变量
int symbolTableTop = 0; //当前符号表的栈顶位置(将要存放表项的位置)
signTable symbolTable[maxTableSize]; //符号表
int displayTable[maxTableSize];		//分程序索引数组
int mainNum = 0;//最终分析结束，mainNum应为1

#define INT 1
#define CHAR 2
string name = "";   //标识符名字
int  kind = -1;      //标识符的类别
int  value = -1;     //数值存储，若为函数，则有返回值存储1，无返回值存储0
int arraySize = -1; //如果是数组的话，存储数组的size大小，其他情况size为0；
int  address = -1;   //变量的地址；函数返回值的地址
int  paraNum = -1;   //若为函数，代表参数个数，否则为0；
int factorType = -1;  //因子类型，以及记录函数是否有返回值
string OperationResult = "";  //表达式的运算结果的存储寄存器或者数值
int callNoReturnFunc = 0;	 //函数调用属于有返回值还是无返回值调用 //调用函数的时候用
bool hasReturnValue = false;  //函数是否有返回数值的语句，（返回空则此数据为false） //插入符号表的时候用
//string switchResult = ""/*, default_label_final = ""*/;  //存储switch后面表达式的计算结果以及default后面之行结束后的标签
//int SwitchVarType = -1;
string forVarName = "";  //存储for循环变量的name
bool hasForVarName= false, needToJudge = false; //若在条件的表达式中出现了for循环变量的内容，则设置为true;若需要在表达式中判断，则设置needTojudge为true
stack<int> ParaType;

//将语法分析程序分析到的成分输出到文件中
void writeToFile(string writeString) {
	//cout << "This is a " << writeString << " !" << endl;
	outfile << "This is a " << writeString << " !" << endl;
}

//将数字转换为字符串
string convertNumToString(int num) {
	stringstream numStream;
	numStream << num;
	return numStream.str();
}

//打印符号表到文件
void printSymtableTable() {
	outSymbolTable << "=============================================================" << endl;
	signTable sign;
	for (int i = displayTable[funcTotalNum]; i <symbolTableTop ; i++)
	{
		sign = symbolTable[i];
		outSymbolTable << sign.name << "\t" << sign.value << "\t" << sign.kind << "\t" << sign.arraySize << "\t" <<
			sign.paraNum << "\t" << sign.address << "\t" <<endl;
	}
}
//将中缀表达式写入文件
void writeToInfixExpressionFile(string str) {
	OutInfixExpression << str << endl;
}
//插入符号表中，需要检查是否重名
int insertIntoSymTable(string name, int kind, int value, int arraySize,int address,int paraNum) {
	int index;
	//需要判断符号表中是否存在该插入项，存在报错
	if (symbolTableTop >= maxTableSize) { //符号表已经满了
		error(TableFull);  //符号表已经满了的错误
		//exit(-1);  //直接处理完错误后退出程序，因为这种情况需要终止程序
		return -1;
	}
	//函数插入与其余类型数据不同，因为函数判断是否已经存在需要在分程序表里面查找
	//而其他类型数据在当前分程序符号表中查询即可
	if (kind == FUNCTION) {
		//函数重名问题涉及两方面：一个是函数名字与全局变量相同报错；另一个是函数与任意的函数名字相同报错
		//函数名字与全局变量相同报错
		for (index = 0; index < displayTable[1] - 1; index++)
		{
			if (symbolTable[index].name == name) {
				error(SymbolNameConflict);  //函数名字与全局变量相同，报错,表冲突
				return -1;
			}
		}
		//从第一个分程序存储的函数项索引开始查找，与任意的函数名字相同报错
		for (index = 1; index <= funcTotalNum; index++)
		{
			if (symbolTable[displayTable[index]].name == name) {
				error(SymbolNameConflict);  //重复定义函数，报错
				return -1;
			}
		}
		//未重定义，入符号表
		//第0个分程序存储的全局常量、变量最后一个标识符存储位置的索引,
		//funcTotalNum初值为0，故要进行++funcTotalNum
		displayTable[++funcTotalNum] = symbolTableTop;
	}
	//其他类型数据在当前分程序符号表中查询
	else {
		//除了函数以外的类型：若为全局变量，则在第0个分程序存储；若为局部变量，可以与函数名重复
		if (funcTotalNum == 0) {  //全局变量
			for (index = displayTable[0]; index < symbolTableTop; index++)
			{
				if (symbolTable[index].name == name) {
					error(SymbolNameConflict);  //重复定义数据，报错
					return -1;
				}
			}
		}
		else {  //局部变量，可以与函数名重复,故inde = displayTable[funcTotalNum]+1
			for (index = displayTable[funcTotalNum]+1; index < symbolTableTop; index++)
			{
				if (symbolTable[index].name == name) {
					error(SymbolNameConflict);  //重复定义数据，报错
					return -1;
				}
			}
		}
	}
	symbolTable[symbolTableTop].name = name;
	symbolTable[symbolTableTop].kind = kind;
	symbolTable[symbolTableTop].value = value;
	symbolTable[symbolTableTop].arraySize = arraySize;
	symbolTable[symbolTableTop].address = symbolTableTop;  //address设置为当前符号表栈顶指针
	symbolTable[symbolTableTop].paraNum = paraNum;
	symbolTableTop++;
	return 1;
}

//该函数在某个函数符号表处理结束后进行调用，除了函数名字和函数参数类型，其余属性全部删除掉
void flushTable() {  
	//全局变量不要删除
	signTable sign;
	int count = 0, tempParaNum = symbolTable[displayTable[funcTotalNum]].paraNum;
	for (int  i = displayTable[funcTotalNum] + tempParaNum +1; i < symbolTableTop; i++)
	{
		sign = symbolTable[i]; //清空所有的属性，其实也可以不做，因为重新插入符号表的时候所有属性都已经初始化
		sign.name.clear();
		sign.kind = 0;
		sign.arraySize = 0;
		sign.paraNum = 0;
		sign.value = 0;
		sign.address = 0;
		count++;
	}
	symbolTableTop = symbolTableTop - count;
}

//在符号表中查找标识符或者函数是否存在
int findInSymbolTable(string name,int searchMode) {
	//searchMode为0，代表搜索标识符名字（在当前分程序以及总的全局变量区中搜索）；
	//searchMode为1，代表搜索函数名字（在分程序索引表中搜索）
	int index, paraType;
	if (searchMode == 1) { //函数搜索，还需要判断参数的个数与类型
		vector<int> tempVector;
		for ( index = 1; index <= funcTotalNum; index++)
		{
			if (symbolTable[displayTable[index]].name == name) {
				//cout << "符号表找到函数名字" << endl;
				break;
			}
		}
		if (index > funcTotalNum) { // 未声明的函数
			error(FunctionNotDefine);  //未声明的函数，报错
			cout << "未声明函数调用" << endl;
			return -1;
		}
		if (symbolTable[displayTable[index]].paraNum != paraNum) {  //函数参数个数不匹配
			error(FormalParaNumNotMatch); //函数参数不匹配，报错
			/*cout << "name" << name << endl;
			cout << symbolTable[displayTable[index]].paraNum << "   " << paraNum << endl;
			cout << "函数参数不匹配" << endl;*/
			return -1;
		}
		//参数类型存入到容器tempVector中(此时为倒序，后续比较需要反过来)
		for (int  i = 1; i <= symbolTable[displayTable[index]].paraNum; i++)//类型需要检查
		{
			//cout << ParaType.size() << endl;
			//参数的类型存储在value属性中
			//paraType = symbolTable[displayTable[index] + i].value == intsy ? INT : CHAR;
			tempVector.push_back(ParaType.top());
			ParaType.pop();
			//cout << "bbbbbbbbbbb" << ParaType.size() << endl;
			//if(paraType != ParaType[i-1]){
			//	error(ParaTypeNotMatch);  //参数类型不符合
			//	//cout << "gggggggggggggggggg" << endl;
			//	return -1;
			//}
		}
		//参数类型比较
		int size = tempVector.size();
		for (int i = 1; i <= symbolTable[displayTable[index]].paraNum; i++)//类型需要检查
		{
			paraType = symbolTable[displayTable[index] + i].value == intsy ? INT : CHAR;
			if(paraType != tempVector[size - i]){
				error(ParaTypeNotMatch);  //参数类型不符合
				//cout << "gggggggggggggggggg" << endl;
				return -1;
			}
		}
		if (symbolTable[displayTable[index]].value == 0) {  //无返回值函数调用
			callNoReturnFunc = 1;
		}
		return symbolTable[displayTable[index]].address;
	}
	else {  //标识符搜索
		//先在当前分程序符号表中搜索
		for ( index = displayTable[funcTotalNum]; index < symbolTableTop; index++)
		{
			if (symbolTable[index].name == name) {
				break;
			}
		}
		if (index == symbolTableTop) {  //未在当前分程序符号表中找到，则去全局变量区查找
			for ( index = 0; index < displayTable[1]; index++)
			{
				if (symbolTable[index].name == name) {
					break;
				}
			}
			if (index == displayTable[1]) {  //未在全局变量区找到，报错，标识符未定义
				error(VariableNotDefine);  //报错，标识符未定义
				return -1;
			}
		}
		return symbolTable[index].address;
	}
}

//＜程序＞    :: = ［＜常量说明＞］［＜变量说明＞］{ ＜有返回值函数定义＞ | ＜无返回值函数定义＞ }＜主函数＞
void programme() {
	int tempToken, tempCharCounter;
	char tempCh;
	insymbol();
	//const开头，则为常量说明
	if (token == constsy) {
		constExplain();
	}
	//int 或者 char开头，需要向后预读符号来判断是变量说明还是函数
	while (token == intsy || token == charsy) {  //变量说明部分,变量说明需要循环执行
		//存放临时变量中，便于之后遇到函数进行指针回移
		tempToken = token;
		tempCh = ch;
		tempCharCounter = charCounter;
		insymbol();
		if (token == ident) {
			insymbol();
			if (token == comma || token == semicolon || token == lmidbrack) {  //变量（包括数组）说明，回退
				token = tempToken;
				ch = tempCh;
				charCounter = tempCharCounter;
				variableExplain();
			}
			else {  //不是变量说明，回退指针，循环跳出
				token = tempToken;
				ch = tempCh;
				charCounter = tempCharCounter;
				break;
			}
		}
		else {
			error(LackIdent, 0);  //应该为标识符，报错
			//return;
		}
	}
	if (token == constsy) {
		error(ConstDefAfterVarDef, 0); //常量说明在变量说明后面，报错
		//return;
	}
	//写入文件中
	outSymbolTable << "符号表中全局变量:" << endl;
	printSymtableTable();
	while (token == intsy || token == charsy || token == voidsy) {
		RegisterCount = 0;
		//处理函数定义部分，有返回值函数定义，无返回值函数定义，main定义
		if (token == intsy || token == charsy) {  //有返回值函数定义
			returnFuncDefine();
		}
		else {  //void部分，包括无返回值函数定义，main定义 需要预读
			tempToken = token;
			tempCh = ch;
			tempCharCounter = charCounter;
			insymbol();
			if (token == ident) {		//无返回值函数定义
				token = tempToken;
				ch = tempCh;
				charCounter = tempCharCounter;
				noReturnFuncDefine();
			}
			else if (token == mainsy) {	//main定义，在处理完这个函数块后，若之后还有内容则报错，终止程序
				token = tempToken;
				ch = tempCh;
				charCounter = tempCharCounter;
				MainDefine();
				//return; //主函数结束，则退出programme
			}
			else {
				error(LackIdent, 0);  //应该为标识符
				//return;
			}
		}
	}
	//writeToFile("programme");
}

//＜整数＞        :: = ［＋｜－］＜无符号整数＞｜０  //不支持+ - 0
//＜常量说明＞ :: = const＜常量定义＞; { const＜常量定义＞; }
void constExplain() {
	int type;
	while (token == constsy)
	{
		insymbol();
		if (token == intsy || token == charsy) {
			kind = (token == intsy) ? intconst : charconst;
			arraySize = 0;
			address = 0;
			paraNum = 0;
			type = token;
			insymbol();
			constDefine(type);
			while (token == comma) {
				insymbol();
				constDefine(type);
			}
			if (token == semicolon) {	//处理分号,若不是报错
				insymbol();
			}
			else {
				error(LackSemicolon, 0);  //应该为分号
				//return;
			}
		}
		else {
			error(NotIntOrChar, 0); //既不是int 又不是char的类型标识符错误
		}
	}
	writeToFile("constExplain");
}

//＜常量定义＞   :: = int＜标识符＞＝＜整数＞{ ,＜标识符＞＝＜整数＞ } | char＜标识符＞＝＜字符＞{ ,＜标识符＞＝＜字符＞ }
void constDefine(int type) { //type要么为int，要么为char
	int Sign = -1;
	if (token == ident) {
		name = id; //记录标识符名字
		insymbol();
		if (token == becomes) { //判断是否为等号
			insymbol();
			//有符号整数
			if (token == plusSign || token == minusSign) {
				Sign = token;
				insymbol();
				if (token == intconst && type == intsy) {
					if (inum == 0) {
						error(SignBeforeZero, 0);
						return;
					}
					if (Sign == plusSign) {
						value = inum;
					}
					else {
						value = -inum;
					}
					if (insertIntoSymTable(name, kind, value, arraySize, address, paraNum) > 0) {
						insertMiddleCode("const", "int", convertNumToString(value), name);
						writeToInfixExpressionFile("const int " + name + " = " + convertNumToString(value));
					}
					insymbol();
				}
				else {
					error(DataTypeNotMatch); //类型不匹配或者token不为intconst
					return;
				}
			}
			//无符号整数
			else if (token == intconst && type == intsy) {
				value = inum;
				//存入符号表
				if (insertIntoSymTable(name, kind, value, arraySize, address, paraNum) > 0){
					insertMiddleCode("const", "int", convertNumToString(value), name);
					writeToInfixExpressionFile("const int " + name + " = " + convertNumToString(value));
				}
				insymbol();
			}
			//字符
			else if (token == charconst && type == charsy) {
				value = inum;
				//存入符号表
				if (insertIntoSymTable(name, kind, value, arraySize, address, paraNum) > 0) {
					insertMiddleCode("const", "char", convertNumToString(value), name); //char类型，value存储其ASCII码
					writeToInfixExpressionFile("const char " + name + " = " + convertNumToString(value));
				}
				insymbol();
			}
			else {
				error(DataTypeNotMatch); //类型不匹配
				return;
			}
		}
		else {
			error(LackBecome, 1); //应该为 =
			return;
		}
	}
	else {
		error(LackIdent, 1);  //应该为标识符,这个错误不能省略，在调用该函数之前没有判断token
		return;
	}
}

//＜声明头部＞   :: = int＜标识符＞ | char＜标识符＞
void declareHead() {
	if (token == intsy || token == charsy) {
		insymbol();
		if (token == ident) {
			name = id;
			insymbol();
		}
		else {
			error(LackIdent, 3); //应为标识符
			return;
		}
	}
	else {
		error(NotIntOrChar, 1); //应为int 或者char
		return;
	}
}

//＜有返回值函数定义＞  :: = ＜声明头部＞‘(’＜参数＞‘)’ ‘ { ’＜复合语句＞‘ }’
void returnFuncDefine() {
	string funcReturnType, funcName;
	if (token == intsy || token == charsy) {
		value = token;	//有返回值函数,存储value为返回值类型
		funcReturnType = token == intsy ? "int" : "char";
		declareHead();//声明头部,设置name
		funcName = name;
		if (token == lsmallbrack) {
			insymbol();
		}
		else {
			error(LackLSmallBrack, 0);  //应该为左小括号
			return;
		}
		kind = FUNCTION;	//函数类型
		arraySize = 0;
		paraNum = 0;
		address = 0;
		if(insertIntoSymTable(funcName, kind, value, arraySize, address, paraNum) > 0) {   //函数在读到左括号后即加入符号表中
			insertMiddleCode("func", funcReturnType, "", funcName);
			writeToInfixExpressionFile(funcReturnType + " " + funcName + "()");
		}
		parameterTable();  //重新写入符号表函数项的paraNum
		if (token == rsmallbrack) {
			insymbol();
		}
		else {
			error(LackRSmallBrack, 0);   //应该为右小括号
			return;
		}
		if (token == lbigbrack) {
			insymbol();
		}
		else {
			error(LackLBigBrack, 0);   //应该为左大括号
			return;
		}
		complexStatement();
		if (token == rbigbrack) {
			insymbol();
		}
		else {
			error(LackRBigBrack, 0);   //应该为右大括号
			return;
		}
		if (hasReturnValue) {
			hasReturnValue = false;
		}
		else {
			error(LackReturnValue);  //有返回值函数没有返回值语句
			return;
		}
		insertMiddleCode("end", "", "", funcName); //函数声明结束
		outSymbolTable << "符号表删除函数某些数据之前：" << endl;
		printSymtableTable();
		flushTable();  //删除符号表中函数的其他项
		outSymbolTable << "符号表删除函数某些数据之后：" << endl;
		printSymtableTable();
	}
	writeToFile("returnFuncDefine");
}

//＜参数表＞    :: = ＜类型标识符＞＜标识符＞{ ,＜类型标识符＞＜标识符＞ } | ＜空＞
void parameterTable() {  //处理参数表，最后将参数个数回填到函数符号表项的属性中
	int tempParaNum = 0;
	string stringParaType, tempName;
	while (token == intsy || token == charsy) {  //若此条件不满足，则参数表为空的情况，直接结束函数即可
		arraySize = 0;
		stringParaType = token == intsy ? "int" : "char"; //记录参数类型，生成四元式使用
		value = token;  //value存储参数类型
		address = 0;
		paraNum = 0;
		insymbol();
		if (token == ident) {
			name = id;
			tempName = id;
			kind = PARAMETER;
			//存入符号表
			if (insertIntoSymTable(name, kind, value, arraySize, address, paraNum) > 0) {
				insertMiddleCode("para", stringParaType, "", tempName);
				writeToInfixExpressionFile("para " + stringParaType + " " + tempName);
			}
			insymbol();
		}
		else {
			error(LackIdent, 2); //应该为标识符
			return;
		}
		tempParaNum++;
		if (token == comma) {
			insymbol();
			continue;
		}
	}
	paraNum = tempParaNum;
	symbolTable[displayTable[funcTotalNum]].paraNum = paraNum; //插入函数的参数个数
}

//＜无返回值函数定义＞  :: = void＜标识符＞‘(’＜参数＞‘)’‘ { ’＜复合语句＞‘ }’
void noReturnFuncDefine() {
	string funcReturnType, funcName;
	if (token == voidsy) {
		value = token;	//无返回值函数
		funcReturnType = "void";
		insymbol();
		name = id;	//前面已经判断过了，若不是标识符或者main，在前面已经报错，此处均为合法情况
		funcName = name;
		insymbol();
		if (token == lsmallbrack) {
			insymbol();
		}
		else {
			error(LackLSmallBrack, 0);  //应该为左小括号
			return;
		}
		kind = FUNCTION;	//函数类型
		arraySize = 0;
		paraNum = 0;
		address = 0;
		if (insertIntoSymTable(funcName, kind, value, arraySize, address, paraNum) > 0) { //函数在读到左括号后即加入符号表中
			insertMiddleCode("func", funcReturnType, "", funcName);
			writeToInfixExpressionFile(funcReturnType + " " + funcName + "()");

		}
		parameterTable();  //重新写入符号表函数项的paraNum
		if (token == rsmallbrack) {
			insymbol();
		}
		else {
			error(LackRSmallBrack, 0);   //应该为右小括号
			return;
		}
		if (token == lbigbrack) {
			insymbol();
		}
		else {
			error(LackLBigBrack, 0);   //应该为左大括号
			return;
		}
		complexStatement();
		if (token == rbigbrack) {
			insymbol();
		}
		else {
			error(LackRBigBrack, 0);   //应该为右大括号
			return;
		}
		if (hasReturnValue) {  //无返回值函数有返回值语句
			hasReturnValue = false;
			error(NoReturnFuncWithReturn);   //无返回值函数有返回值语句，报错
			return;
		}
		insertMiddleCode("end", "", "", funcName);
		outSymbolTable << "符号表删除函数某些数据之前：" << endl;
		printSymtableTable();
		flushTable();  //删除符号表中函数的其他项
		outSymbolTable << "符号表删除函数某些数据之后：" << endl;
		printSymtableTable();
	}
	writeToFile("noReturnFuncDefine");
}

//＜主函数＞    :: = void main‘(’‘)’ ‘ { ’＜复合语句＞‘ }’
void MainDefine() {
	string funcReturnType, funcName;
	if (token == voidsy) {
		value = token;	//无返回值函数
		funcReturnType = "void";
		insymbol();
		name = id;	//前面已经判断过了，若不是标识符或者main，在前面已经报错，此处均为合法情况,此处name 为 main
		funcName = name;
		insymbol();
		if (token == lsmallbrack) {
			insymbol();
		}
		else {
			error(LackLSmallBrack, 0);  //应该为左小括号
			return;
		}
		kind = FUNCTION;	//函数类型
		arraySize = 0;
		paraNum = 0;
		address = 0;
		if (insertIntoSymTable(funcName, kind, value, arraySize, address, paraNum) > 0) { //主函数在读到左括号后即加入符号表中
			insertMiddleCode("func", funcReturnType, "", funcName);
			writeToInfixExpressionFile(funcReturnType + " " + funcName + "()");
		}
		if (token == rsmallbrack) {
			insymbol();
		}
		else {
			error(LackRSmallBrack, 0);   //应该为右小括号
			return;
		}
		if (token == lbigbrack) {
			insymbol();
		}
		else {
			error(LackLBigBrack, 0);   //应该为左大括号
			return;
		}
		complexStatement();
		if (token == rbigbrack) {
			insymbol();
		}
		else {
			error(LackRBigBrack, 0);   //应该为右大括号
			return;
		}
		if (hasReturnValue) {  //main无返回值函数有返回值语句
			hasReturnValue = false;
			error(NoReturnFuncWithReturn);   //无返回值函数main 有返回值语句，报错
			return;
		}
		mainNum++;
		insertMiddleCode("end", "", "", funcName);
	}
	outSymbolTable << "符号表主函数的数据：" << endl;
	printSymtableTable();
	writeToFile("main");
}

//＜变量说明＞  :: = ＜变量定义＞; {＜变量定义＞; } 由于变量说明与有返回值函数的前部分相同，故需在调用
//变量说明或者有返回值函数定义之前进行判断，故变量说明每次只执行到一个分号即结束，下次是否调用要依据之后判断是否
//为变量声明，此处variableExplain不可递归，也不可利用循环，(只要是int或者char就开始variableExplain是错误的)
void variableExplain() {
	if (token == intsy || token == charsy) {
		arraySize = 0; //全都初始化
		value = 0;
		address = 0;
		paraNum = 0;
		kind = token; //因为kind为全局变量，故无需传入variableDefine
		insymbol();
		variableDefine();
		while (token == comma) {
			insymbol();
			variableDefine();
		}
		if(token == semicolon) {
			insymbol();
		}
		else {
			error(LackSemicolon, 1);  //应该为分号
		}
	}
	writeToFile("variableExplain");
}

//＜变量定义＞  :: = ＜类型标识符＞(＜标识符＞ | ＜标识符＞‘[’＜无符号整数＞‘]’) { , ＜标识符＞ | ＜标识符＞‘[’＜无符号整数＞‘]’ }
void variableDefine() {
	int tempToken, tempCharCounter;
	char tempCh;
	string arrType, dataType;
	if (token == ident) {
		arrType = kind == intsy ? "intArr" : "charArr";
		dataType = kind == intsy ? "int" : "char";
		name = id;
		tempToken = token;
		tempCh = ch;
		tempCharCounter = charCounter;
		insymbol();
		if (token == lmidbrack) { //数组
			insymbol();
			if (token == intconst) {
				if (inum != 0) {
					arraySize = inum; // 数组大小
					insymbol();
				}
				else {
					error(ArraySizeIsZero);  //数组大小为0，错误
					//insymbol();
					return;
				}
				if (token == rmidbrack) {
					//存入符号表，数组在读到右中括号后才会入表!
					if (insertIntoSymTable(name, kind, value, arraySize, address, paraNum) > 0) {
						insertMiddleCode(arrType, "0", convertNumToString(arraySize), name);
						writeToInfixExpressionFile("var " + dataType + +" " + name + "[" + convertNumToString(arraySize) + "]");
					}
					insymbol();
				}
				else {
					error(LackRMidBrack, 0);  //应为右中括号
					return;
				}
			}
			else {
				error(ArrayIndexNotUnsignedNum);  //数组大小不为无符号整数，报错
				return;
			}
		}
		else { //为变量，且不是数组
			//存入符号表
			if(insertIntoSymTable(name, kind, value, arraySize, address, paraNum) > 0){
				insertMiddleCode(dataType, "", "", name);
				writeToInfixExpressionFile("var " + dataType + " " + name);
			}
			token = tempToken;  //读取了下一个符号，指针回移
			ch = tempCh;
			charCounter = tempCharCounter;
			insymbol();
		}
	}
	else {
		error(LackIdent, 1);  //应该为标识符,这个错误不能省略，在调用该函数之前没有判断token
		return;
	}
}

//＜复合语句＞   :: = ［＜常量说明＞］［＜变量说明＞］＜语句列＞
void complexStatement() {
	char tempCh; //需要向后读入三个符号才能判断出究竟是变量声明还是有返回值函数定义
	int tempToken, tempCharCounter;
	if (token == constsy) {  //常量说明
		constExplain();
	}
	while (token == intsy || token == charsy) { //因为变量声明每次只处理一行，故应该循环执行
		tempToken = token;
		tempCh = ch;
		tempCharCounter = charCounter;
		insymbol();
		if (token == ident) {
			name = id;
			insymbol();
			if (token == comma || token == lmidbrack || token == semicolon) { //变量声明,将指针回退
				token = tempToken;
				ch = tempCh;
				charCounter = tempCharCounter;
				variableExplain();
			}
			else {
				error(LackSemicolon, 2);  //不符合变量定义文法，应该为分号
				return;
			}
		}
		else {
			error(LackIdent, 4); //应该为标识符
			return;
		}
	}
	if (token == constsy) {  //常量说明在变量说明后面，报错
		error(ConstDefAfterVarDef, 1);  //常量说明在变量说明后面，报错
		return;
	}
	statementList(); //处理语句列
}

//＜语句列＞   :: = ｛＜语句＞｝
void statementList() {
	//语句列可以为空
	statement();
	while (token == ifsy || token == forsy || token == lbigbrack || token == intsy || token == charsy
		|| token == voidsy || token == ident || token == scanfsy || token == printfsy ||
		token == switchsy || token == returnsy || token == semicolon) {
		statement();
	}
}

/*＜语句＞    :: = ＜条件语句＞｜＜循环语句＞ | ‘{ ’＜语句列＞‘ }’｜＜有返回值函数调用语句＞;
| ＜无返回值函数调用语句＞; ｜＜赋值语句＞; ｜＜读语句＞; ｜＜写语句＞; ｜＜空＞; | ＜情况语句＞｜＜返回语句＞;*/
void statement() {
	int tempToken, tempCharCounter;
	char tempCh;
	int result;
	string tempName;
	if (token == ifsy) {  //条件语句
		ifStatement();
	}
	else if (token == forsy) {  //循环语句
		forStatement();
	}
	else if (token == lbigbrack) {  //语句列
		insymbol();
		statementList();
		if (token == rbigbrack) {
			insymbol();
		}
		else {
			error(LackRBigBrack, 2);  //应该为右大括号
			return;
		}
	}
	else if (token == ident) {	//有返回值函数调用语句,无返回值函数调用语句,赋值语句，需要预读
		tempName = id;
		tempToken = token;
		tempCh = ch;
		tempCharCounter = charCounter;
		insymbol();
		if (token == becomes || token == lmidbrack) {		//赋值语句
			token = tempToken;
			ch = tempCh;
			charCounter = tempCharCounter;
			assignStatement();
		}
		else if (token == lsmallbrack) {	//＜有返回值函数调用语句＞ :: = ＜标识符＞‘(’＜值参数表＞‘)’
			insymbol();						//＜无返回值函数调用语句＞ :: = ＜标识符＞‘(’＜值参数表＞‘)’ 
			valueOfParameterTable(); //分析值参数表，已经生成了一个记录值参数类型的容器
			if (token == rsmallbrack) {
				insymbol();
			}
			else {
				error(LackRSmallBrack, 1);  //应该为右小括号
				return;
			}
			result = findInSymbolTable(tempName, 1); //查找该函数
			if (result >= 0) {  //找到
				if (callNoReturnFunc == 1) {  //该调用函数属于无返回值调用函数
					callNoReturnFunc = 0;
					//if (tempName == "main") { // 不能调用main函数的检查！  因为在maindef中，statementList在遇到main()这个调用后就会结束maindef，之后判断右大括号报没有右大括号的错误
					//	error(callMain); 
					//	return;											
					//}
				}
				else {
					//有返回值函数调用

				}
				insertMiddleCode("call", tempName, "", ""); //函数调用的四元式 语句中的有返回值函数调用并不用将返回值结果存储到一个寄存器中！
				writeToInfixExpressionFile("call " + tempName);
			}
			else {
				/*cout << tempName << endl;
				cout << "size " << ParaType.size() << endl;
				cout << "lineNum " << lineNum << endl;
				for (int i = 0; i < ParaType.size(); i++) {
					cout << ParaType[i] << endl;
				}*/
				//cout << "ccccccccccccccccccccc" << endl;
				cout << "语句中调用的函数未定义或者参数类型不匹配或者参数个数不匹配" << endl;
				//调用的函数未定义或者参数类型不匹配或者参数个数不匹配，findInSymbolTable已经报错
				return;
			}
		}
		else {
			//有没有这种可能？？？
			//error(StatementIsIllegal);  //既不是函数调用语句，又不是赋值语句
			//return;
		}
		if (token == semicolon) {
			insymbol();
		}
		else {
			error(LackSemicolon, 3);  // 应该为分号，报错
			return;
		}
	}
	else if (token == scanfsy) {  //读语句
		scanfStatement();
		if (token == semicolon) {
			insymbol();
		}
		else {
			error(LackSemicolon, 3);  // 应该为分号，报错
			return;
		}
	}
	else if (token == printfsy) {  //写语句
		printfStatement();
		if (token == semicolon) {
			insymbol();
		}
		else {
			error(LackSemicolon, 3);  // 应该为分号，报错
			return;
		}
	}
	else if (token == semicolon) {  //空语句
		insymbol();
		writeToFile("emptyStatement");
	}
	else if(token == switchsy) {   //情况语句
		switchStatement();
	}
	else if (token == returnsy) {  //返回语句
		returnStatement();
		if (token == semicolon) {
			insymbol();
		}
		else {
			error(LackSemicolon, 3);  // 应该为分号，报错
			return;
		}
	}
	//else {
	//	error(StatementIllegalSign);  // 不符合语句组成文法
	//	return;
	//}
}


//＜值参数表＞   :: = ＜表达式＞{ ,＜表达式＞ }｜＜空＞
void valueOfParameterTable() {
	vector<string> paraVector;
	//将值参数表的每个参数的类型放入ParaType中，值参数表参数类型检查在查表的时候进行
	if (token == rsmallbrack) { //值参数表为空的情况
		paraNum = 0;
		return;
	}
	else {
		do
		{
			if (token == comma)		insymbol();
			expression();
			paraVector.push_back(OperationResult);
			ParaType.push(factorType); //类型容器
		} while (token == comma);
		paraNum = paraVector.size();
		//cout << "aaaaaaa" << paraNum << endl;
		for (int i = 0; i < paraNum; i++)  //实参表，插入funcPa四元式
		{
			insertMiddleCode("funcPa", "", "", paraVector[i]);
			writeToInfixExpressionFile("push " + paraVector[i]);
		}
		paraVector.clear();
	}

}

//＜表达式＞    :: = ［＋｜－］＜项＞{ ＜加法运算符＞＜项＞ }
void expression() {
	string register_1, register_2, register_final, op;
	int Sign = -1;
	factorType = -1; //初始化因子类型
	if (token == plusSign || token == minusSign) {
		factorType = INT;
		Sign = token;
		insymbol();
	}
	if (Sign == -1) { // 没有 加减运算符的情况
		item();
		register_final = OperationResult;  //先存储分析一个项后的结果
	}
	else if (Sign == plusSign) {
		item();
		//Sign + 与 第一个item()结合相当于item()的返回结果未改变，故无需另外的指令操作
		register_final = OperationResult;
	}
	else if (Sign == minusSign) {
		item();
		//Sign - 与 第一个item()结合
		register_1 = OperationResult; //存储临时结果
		register_final = genNewRegister(); //生成新的寄存器存储register_1与负号结合的结果
		insertMiddleCode("-", "0", register_1, register_final);		//register_final = 0 - register_1 = -register_1
		writeToInfixExpressionFile(register_final + " = 0 - " + register_1);
	}
	//还有其他项要处理
	while (token == plusSign || token == minusSign)
	{
		factorType = INT;
		register_1 = register_final; //register_1存储分析完的表达式部分的结果
		op = token == plusSign ? "+" : "-";
		insymbol();
		item();
		register_2 = OperationResult;
		register_final = genNewRegister(); //生成新的寄存器存储register_1与register_2结合的结果
		insertMiddleCode(op, register_1, register_2, register_final);
		writeToInfixExpressionFile(register_final + " = " + register_1 + " " + op + " " + register_2);

	}
	//均处理结束后，表达式的最终结果存储在register_final中，通过OperationResult返回
	OperationResult = register_final;
}

//＜项＞     :: = ＜因子＞{ ＜乘法运算符＞＜因子＞ }
void item() {
	string register_1, register_2, register_final, op;  //同理于表达式的处理
	factor();
	register_final = OperationResult; //存储项的结果
	while (token == timesSign || token == idivSign) {
		factorType = INT;
		register_1 = register_final;
		op = token == timesSign ? "*" : "/";
		insymbol();
		factor();
		register_2 = OperationResult;
		register_final = genNewRegister();
		if (op == "/" && register_2 == "0") {
			//传入除数为0的错误，继续照常处理
			error(DivideZero);
			// return;
		}
		insertMiddleCode(op, register_1, register_2, register_final);
		writeToInfixExpressionFile(register_final + " = " + register_1 + " " + op + " " + register_2);
	}
	//均处理结束后，项的最终结果存储在register_final中，通过OperationResult返回
	OperationResult = register_final;
}

//＜因子＞    :: = ＜标识符＞｜＜标识符＞‘[’＜表达式＞‘]’｜＜整数＞ | ＜字符＞
//｜＜有返回值函数调用语句＞ | ‘(’＜表达式＞‘)’
void factor() {
	int result, Sign = -1, tempType;
	int tempToken, tempCharCounter;
	char tempCh;
	string tempName, op;
	string tempRegister;
	if (token == ident) {
		tempName = id;
		tempToken = token;  //需要预读
		tempCh = ch;
		tempCharCounter = charCounter;
		insymbol();
		if (token == lsmallbrack) {  //有返回值函数调用
			insymbol();
			valueOfParameterTable();
			if (token == rsmallbrack) {
				result = findInSymbolTable(tempName,1);
				if (result >= 0) {  //在符号表中查找该函数是否存在,若返回值大于等于0，则代表该函数存在，且参数个数与参数类型相同
					if (callNoReturnFunc != 1) {  //有返回值函数调用
						//cout << "因子调用有返回值函数" << endl;
						tempRegister = genNewRegister(); //存储返回值的寄存器
						insertMiddleCode("call", tempName, "", tempRegister);
						writeToInfixExpressionFile("call " + tempName);
						writeToInfixExpressionFile(tempRegister + " = RET");
						insymbol();
						OperationResult = tempRegister; //项的结果存储
						factorType = (symbolTable[result].value == intsy ? INT : CHAR);		//函数的value存储返回类型
					}
					else {
						callNoReturnFunc = 0;  //重置callNoReturnFunc！
						error(AssignWithNoReturnFunc);  //调用了无返回值函数，错误
						//cout << "因子调用无返回值函数" << endl;
						return;
					}
				}
				else {   //findInSymbolTable已经报错
					//cout << "bbbbbbbbbbbbbbbbbbbbb" << endl;
					cout << "因子调用的函数未声明或者函数参数个数不匹配，或者类型不匹配" << endl;
					return;
				}
			}
			else {
				error(LackRSmallBrack, 2);  //应该为右小括号
				return;
			}
		}
		else if (token == lmidbrack) { //数组变量
			insymbol();
			tempType = factorType;  //存储之前的变量类型，因为在调用数组的下标表达式后factorType会改变，临时保存起来之后还原，下面表达式确定的factorType来判断下标
			expression();//表达式结果存储在OperationResult中
			factorType = tempType;  //还原factorType
			result = findInSymbolTable(tempName,0);  // 查找数组名字
			if (result >= 0) {  //在符号表中查找该数组是否存在
								//还应该判断数组下标是否越界，判断数组下标类型！
				if (symbolTable[result].arraySize == 0) {
					error(TypeNotArray, 0);   //找到的变量不为数组类型，这个需要判断吗????
					return;
				}
				if (factorType == -1) {  //若只有一个数组因子，则应该设置factorType为该数组元素的类别
					factorType = symbolTable[result].kind == intsy ? INT : CHAR;
				}
				if (isdigit(OperationResult[0])) {
					if (atoi(OperationResult.c_str()) >= symbolTable[result].arraySize) {
						//数组下标越界，报错
						error(ArrayIndexOfRange, 0);
						//return;
					}
				}
				// else {
				// 	error();  //数组下标可以为表达式，若为无符号整数，则判断是否越界，运行时确定？不用报错
				// }
			}
			else {
				//未定义该数组,在查找函数中已经报错
				return;
			}
			if (token == rmidbrack) {
				//得到该数组的元素，四元表达式生成
				insymbol();
				tempRegister = genNewRegister(); //生成存储寄存器
				insertMiddleCode("getArr", tempName, OperationResult, tempRegister); //OperationResult此时存储数组下标
				writeToInfixExpressionFile(tempRegister + " = " + tempName + "[" + OperationResult + "]");
				OperationResult = tempRegister; //OperationResult之后更新
			}
			else {
				error(LackRMidBrack, 1);  // 应该为右中括号
				return;
			}
		}
		else {  //变量标识符判断 ,需要指针回移动
			token = tempToken;
			ch = tempCh;
			charCounter = tempCharCounter;
			result = findInSymbolTable(tempName,0);  // 查找变量名字
			if (result >= 0) {  //找到
				//cout << "找到了" << name << " result = " << result << endl;
				OperationResult = tempName;
				if (symbolTable[result].kind == intconst || symbolTable[result].kind == charconst) {  //常量的情况
					if (factorType != INT) {
						factorType = (symbolTable[result].kind == intconst ? INT : CHAR);
					}
				}
				else if (symbolTable[result].kind == PARAMETER) {  //函数参数的情况
					if (factorType != INT) {
						factorType = (symbolTable[result].value == intsy ? INT : CHAR);
					}
				}
				else {
					if (factorType != INT) {
						factorType = symbolTable[result].kind == intsy ? INT : CHAR;
					}
				}
				insymbol();
				//下面是进行for循环的条件语句中是否存在循环变量标识符的语句
				if (needToJudge && (tempName == forVarName)) {  //若需要被判断，则在名字相等时设置hasForVarName为true
					hasForVarName = true;
				}
			}
			else {
				//没找到，未定义，已经在查找函数中报错
				return;
			}
		}
	}
	else if (token == intconst || token == plusSign || token == minusSign) {  //整数分析
		if (token == plusSign || token == minusSign) {
			Sign = token;
			insymbol();
		}
		if (token == intconst) {
			if (Sign != -1) { //前面含有符号
				if (inum == 0) {
					error(SignBeforeZero, 2);  //0前面有 + 或者 - 符号
					return;
				}
				else {
					//处理常数的情况
					op = Sign == plusSign ? "" : "-";
					OperationResult = op + convertNumToString(inum);
				}
			}
			else {	
				//前面不含有符号的整数
				OperationResult = convertNumToString(inum);
			}
			factorType = INT;  //设置为int类型
			insymbol();
		}
		else {
			error(LackNum, 0);  //应该为整数,整数缺失，报错
			return;
		}
	}
	else if (token == charconst) {
		//处理字符的情况
		//仅有一个字符的情况，设置为char；若有操作符 + - * / 等，则该表达式类型就为int，当类型为int时就不能转换回char了
		if (factorType != INT) {
			factorType = CHAR;  //仅有一个字符的情况，设置为char类型
		}
		OperationResult = convertNumToString(inum);  //此时 inum 存储字符的ASCII码
		insymbol();
	}
	else if (token == lsmallbrack) {  //处理 ‘(’表达式‘)’ 的情况
		insymbol();
		expression();
		//tempRegister = OperationResult;
		if (token == rsmallbrack) {
			//OperationResult = tempRegister;
			insymbol();
		}
		else {
			error(LackRSmallBrack, 2);  //应该为右小括号
			return;
		}
	}
	else {
		error(FactorIllegalSign); //不符合因子的构成文法
		return;
	}
}

//＜赋值语句＞   :: = ＜标识符＞＝＜表达式＞ | ＜标识符＞‘[’＜表达式＞‘]’ = ＜表达式＞
void assignStatement() {
	int result;
	string tempName;
	string register_final;
	int leftType = -1, rightType = -1;
	if (token == ident) {
		tempName = id;
		insymbol();
		if (token == lmidbrack) { //数组
			result = findInSymbolTable(tempName, 0);
			if (result >= 0) {
				if (symbolTable[result].arraySize != 0) {  //确定是数组
					;
				}
				else {
					error(TypeNotArray, 1);   //找到的变量不为数组类型，这个需要判断
					return;
				}
				leftType = symbolTable[result].kind == intsy ? INT : CHAR;
			}
			else {
				//error(VariableNotDefine);  //变量未定义，已经在查找函数中报错
				return;
			}
			insymbol();
			expression();
			register_final = OperationResult; //存储表达式结果
			if (isdigit(OperationResult[0])) {
				if (atoi(OperationResult.c_str()) >= symbolTable[result].arraySize) {
					//数组下标越界，报错
					error(ArrayIndexOfRange, 1);
					//cout << "第" << lineNum << "行赋值语句数组越界！" << endl;
					return;
				}
			}
			//else {
			//	//error();  //数组下标可以为表达式，若为无符号整数，则判断是否越界,运行时确定，不用报错？
			//}
			if (factorType != INT) {  //检查下标类型
				error(ArrayIndexNotIntType);  //表达式类型不为整型，与数组下标类型不对应
				//cout << factorType << endl;
				cout << "表达式类型不为整型，与数组下标类型不对应" << endl;
				return;
			}
			if (token == rmidbrack) {
				insymbol();
			}
			else {
				error(LackRMidBrack, 2);  //应该为右中括号
				return;
			}
			if (token == becomes) {
				insymbol();
			}
			else {
				error(LackBecome, 2);  //应该为等号赋值符号
				return;
			}
			expression();
			rightType = factorType;
			if (leftType != rightType) {
				//error();  //赋值语句（数组）左边与右边的类型不匹配???
				//cout << leftType << " " << rightType << endl;
				cout << "warning:第" << lineNum << "行赋值语句（数组）左边与右边的类型不匹配!在之后的转换可能会出错！" << endl;
				//return;
			}
			insertMiddleCode("[]=", tempName, register_final, OperationResult);
			writeToInfixExpressionFile(tempName + "[" + register_final + "] = " + OperationResult);
		}
		else if (token == becomes) {
			result = findInSymbolTable(tempName, 0);
			if (result >= 0) {
				if (symbolTable[result].kind == intconst || symbolTable[result].kind == charconst) {
					error(AssignConst, 0);  //为常量赋值，报错
					//cout << "第" << lineNum << "行为常量赋值！" << endl;
					return;
				}
				//为数组赋值，错误
				else if (symbolTable[result].arraySize != 0) {
					error(AssignArray);
					return;
				}
			}
			else {
				//error(VariableNotDefine);  //变量未定义，已经在查找函数中报错
				return;
			}
			leftType = (symbolTable[result].kind == intsy || (symbolTable[result].kind == PARAMETER && symbolTable[result].value == intsy))? INT : CHAR;
			insymbol();
			expression();
			rightType = factorType;
			if (leftType != rightType) {
				//error();  //赋值语句（非数组变量）左边与右边的类型不匹配 ? 字符转换的范围如何判断？？？？
				//cout << leftType << " " << rightType << endl;
				cout << "warning:第" << lineNum << "行赋值语句左边与右边的类型不匹配!在之后的转换可能会出错！" << endl;
				//return;
			}
			insertMiddleCode("=", OperationResult, "", tempName);
			writeToInfixExpressionFile(tempName + " = " + OperationResult);
		}
	}
	writeToFile("assignStatement");
}

//＜条件语句＞  :: = if ‘(’＜条件＞‘)’＜语句＞［else＜语句＞］
void ifStatement() {
	string label_1, label_2;
	label_1 = genNewLabel();
	label_2 = genNewLabel();
	if (token == ifsy) {
		insymbol();
		if (token == lsmallbrack) {
			insymbol();
			condition(ifsy);
		}
		else {
			error(LackLSmallBrack, 1);  //应该为左小括号
			return;
		}
		if (token == rsmallbrack) {
			insertMiddleCode("jne", "", "", label_1); //若为假的，则跳转到label_1标签，否则顺序执行
			writeToInfixExpressionFile("BZ " + label_1);
			insymbol();
			statement();
			insertMiddleCode("jump", "", "", label_2); //无条件跳转到label_2,即else执行结束的地方，若没有else，则跳转到if语句结束之后的地方
			writeToInfixExpressionFile("GOTO " + label_2);

		    /*若没有else语句部分，也生成两个标签来做跳转
			 即最后的生成为 if 不成立 jump label_1
						......后面是if要执行的语句
						jump label_2
						label_1:
						label_2:后面是if语句结束后的语句		*/
			//生成第一个标签，作为if条件不成立跳转的标签
			insertMiddleCode("genLabel", "", "", label_1);
			writeToInfixExpressionFile(label_1 + ":");
			if (token == elsesy) { //还有else
				insymbol();
				statement();
			}
			insertMiddleCode("genLabel", "", "", label_2);
			writeToInfixExpressionFile(label_2 + ":");
		}
		else {
			error(LackRSmallBrack, 3);  //应该为右小括号
			return;
		}
		writeToFile("ifStatement");
	}
}

//＜条件＞    :: = ＜表达式＞＜关系运算符＞＜表达式＞｜＜表达式＞ //表达式为0条件为假，否则为真,若为for循环调用，还需要判断条件是否与循环变量
void condition(int conditionType) {
	string register_1, register_2;
	string CompareOp;
	expression();
	//将表达式的结果存储到register_1
	register_1 = OperationResult;
	if (token == lss || token == leq || token == gtr || token == geq || token == neq || token == eql) {
		switch (token)
		{
		case lss:
			CompareOp = "<";
			break;
		case leq:
			CompareOp = "<=";
			break;
		case gtr:
			CompareOp = ">";
			break;
		case geq:
			CompareOp = ">=";
			break;
		case neq:
			CompareOp = "!=";
			break;
		case eql:
			CompareOp = "==";
			break;
		default:
			break;
		}
		insymbol();
		expression();
		//表达式结果存储到register_2
		register_2 = OperationResult;
		insertMiddleCode(CompareOp, register_1, register_2, ""); //转换成mips指令应该将结果存入寄存器中
		writeToInfixExpressionFile(register_1 + " " + CompareOp + " " + register_2);
		if (conditionType == forsy) {  //只有for和if调用condition，若为for，则判断条件中是否存在identName
			if (hasForVarName)   {	//若条件中包含循环变量，重置hasForVarName
				hasForVarName = false;
			}
			else {	//若不存在identName，则报错;
				cout << "出错行数： "<< lineNum << " 循环语句的循环条件不包含循环变量！" << endl;
				error(forConditionNotContainVar);
				return;
			}
			
		}
		return;
	}
	//对于只有一个表达式的情况!
	insertMiddleCode("!=", register_1, "0", "");
	writeToInfixExpressionFile(register_1 + " != 0 ");
	return;
}

//＜循环语句＞   :: = for‘(’＜标识符＞＝＜表达式＞; ＜条件＞; ＜标识符＞＝＜标识符＞(+| -)＜步长＞‘)’＜语句＞
//＜步长＞    :: = ＜非零数字＞｛＜数字＞｝
void forStatement() {
	/*条件与循环变量有关
	两个标识符必须是循环语句中的第一个标识符（循环变量）*/
	/*for语句先执行一次循环体中的语句再进行循环变量是否越界的测试
	for循环，先执行后判断需要3个label
	生成指令顺序为，执行语句，更新语句，判断条件语句*/
	/*	标识符 = 表达式;生成一条指令
	   jump label_1
	   label_2:
	   (条件)，若不满足，jne label_3
	   label_1:
	   执行语句部分
	   更新循环变量操作
	   jump label_2
	   label_3:
	   for结束之后的语句
	*/
	string SymbolName_1, SymbolName_2, SymbolName_3, op, tempRegister;
	string label_1, label_2, label_3;
	int result, Sign = -1, step;
	label_1 = genNewLabel();
	label_2 = genNewLabel();
	label_3 = genNewLabel();
	if (token == forsy) {
		insymbol();
		if (token == lsmallbrack) {
			insymbol();
		}
		else {
			error(LackLSmallBrack, 1);  //应该为左小括号
			return;
		}
		if (token == ident) {   //只能为标识符，而不能是数组的某个元素！
			SymbolName_1 = id;
			forVarName = id;  //记录该循环变量名字到全局变量中
			result = findInSymbolTable(SymbolName_1, 0);
			if (result >= 0) {
				//判断是不是常量为左值
				if (symbolTable[result].kind == intconst || symbolTable[result].kind == charconst) {
					error(AssignConst, 1);  //为常量赋值，报错
					cout << "第" << lineNum << "行为常量赋值！" << endl;
					return;
				}
				insymbol();
			}
			else {
				//error(VariableNotDefine); //未找到，查找程序已经报错，直接返回即可
				return;
			}
		}
		else {
			error(LackIdent, 5);  //应该为标识符，报错
			return;
		}
		if (token == becomes) {
			insymbol();
		}
		else {
			error(LackBecome, 0);  //应该为等号，报错
			return;
		}
		expression();
		insertMiddleCode("=", OperationResult, "", SymbolName_1);  // for(标识符 = 表达式;...;...)
		writeToInfixExpressionFile(SymbolName_1 + " = " + OperationResult);
		if (token == semicolon) {
			insymbol();
		}
		else {
			error(LackSemicolon, 4);  //应该为分号，报错
			//return;
		}
		insertMiddleCode("jump", "", "", label_1); //无条件跳转到label_1,即for的执行语句部分
		writeToInfixExpressionFile("GOTO " + label_1);
		insertMiddleCode("genLabel", "", "", label_2);  //在条件判断之前生成一个标签
		writeToInfixExpressionFile(label_2 + ":");
		needToJudge = true;//开始分析条件，设置needToJudge变量为true
		condition(forsy); // for(...; 条件 ;...)  
		needToJudge = false;  //分析条件结束，设置needToJudge变量为false
		if (token == semicolon) {
			insymbol();
		}
		else{
			error(LackSemicolon, 4);  //应该为分号，报错
			//return;
		}
		insertMiddleCode("jne", "", "", label_3); //若为假的，则跳转到label_3标签，否则顺序执行
		writeToInfixExpressionFile("BZ " + label_3);
		if (token == ident) {
			SymbolName_2 = id;
			result = findInSymbolTable(SymbolName_2, 0); //该标识符必须为之前的增量标识符！
			if (result >= 0) {
				if (SymbolName_2 == SymbolName_1) {
					insymbol();
				}
				else {  //报错，该标识符与增量标识符无关!
					error(forUpdateNotContainVar);  //报错
					return;
				}
			}
			else {
				//未找到，查找程序已经报错，直接返回即可
				return;
			}
		}
		else {
			error(LackIdent, 5);  //应该为标识符，报错
			return;
		}
		if (token == becomes) {
			insymbol();
		}
		else {
			error(LackBecome);  //应该为等号，报错
			return;
		}
		if (token == ident) {
			SymbolName_3 = id;
			result = findInSymbolTable(SymbolName_3, 0); //该标识符必须为之前的增量标识符！
			if (result >= 0) {
				if (SymbolName_3 == SymbolName_1) {
					insymbol();
				}
				else {  //报错，该标识符与增量标识符无关!
					error(forUpdateNotContainVar);  //报错
					return;
				}
			}
			else {
				//未找到，查找程序已经报错，直接返回即可
				return;
			}
		}
		else {
			error(LackIdent, 5);  //应该为标识符，报错
			return;
		}
		if (token == plusSign || token == minusSign) {
			Sign = token;
			insymbol();
		}
		else {
			error(LackPlusOrMinus);  //应该为加号或者减号
			return;
		}
		//分析步长
		if (token == intconst) {
			if (inum != 0) {
				step = inum;
				insymbol();
			}
			else {
				error(StepIsZero);  //不能为0 ，步长要以非0数字开头
				return;
			}
		}
		else {
			error(LackNum, 1);  //应该为整数常量
			return;
		}
		if (token == rsmallbrack) {
			insymbol();
			insertMiddleCode("genLabel", "", "", label_1);  //在执行语句前面生成一个标签
			writeToInfixExpressionFile(label_1 + ":");
			statement();
		}
		else {
			error(LackRSmallBrack, 3);  //应该为右小括号
			return;
		}
		op = Sign == plusSign ? "+" : "-";
		tempRegister = genNewRegister();
		insertMiddleCode(op, SymbolName_3, convertNumToString(step), tempRegister);
		writeToInfixExpressionFile(tempRegister + " = " + SymbolName_3 + " " + op + " " + convertNumToString(step));
		insertMiddleCode("=", tempRegister, "", SymbolName_2);  //更新变量语句
		writeToInfixExpressionFile(SymbolName_2 + " = " + tempRegister);
		insertMiddleCode("jump", "", "", label_2);
		writeToInfixExpressionFile("GOTO " + label_2);
		insertMiddleCode("genLabel", "", "", label_3);  //在for循环体结束后的位置生成一个标签
		writeToInfixExpressionFile(label_3 + ":");
		writeToFile("forStatement");
	}
}

//＜情况语句＞  :: = switch ‘(’＜表达式＞‘)’ ‘ { ’＜情况表＞＜缺省＞‘ }’
void switchStatement() {
	//情况语句中，switch后面的表达式和case后面的常量只允许出现int和char类型；每个情况子语句执行完毕后，不继续执行后面的情况子语句
	/*switch (表达式) 表达式结果存储到switchResult中
	 case xxx:
		生成switchResult 与 xxx 的比较指令
		jne , , ,label指令（不等，则跳到下一个case或者default位置）
		生成语句指令
		jump default_label_final 执行该分支后，直接跳到default结束的位置
		genLabel label  生成下一个case或者default开始时候的标签
	case yyy: 
		.......
	default:
		生成语句指令
	genLabel default_label_final  //生成default结束位置的label标签
	*/
	//最终生成的default结束后位置的标签
	string default_label_final = "";
	string switchResult = "";
	int SwitchVarType = -1;
	if (token == switchsy) {
		insymbol();
		if (token == lsmallbrack) {
			insymbol();
		}
		else {
			error(LackLSmallBrack, 1);  //应该为左小括号
			return;
		}
		expression();
		switchResult = OperationResult;  //存储表达式结果
		SwitchVarType = factorType;
		if (token == rsmallbrack) {
			insymbol();
		}
		else {
			error(LackRSmallBrack, 3);  //应该为右小括号
			return;
		}
		if (token == lbigbrack) {
			insymbol();
		}
		else {
			error(LackLBigBrack, 1);  //应该为左大括号
			return;
		}
		if (token == casesy) {
			//最终生成的default结束后位置的标签
			default_label_final = genNewLabel();
			situationTable(default_label_final, switchResult, SwitchVarType);
		}
		else {
			error(KeywordError);  //应该为case关键字
			return;
		}
		if (token == defaultsy) {
			defaultStatement();
		}
		else {
			error(KeywordError);  //应该为default关键字
			return;
		}
		if (token == rbigbrack) {
			insymbol();
		}
		else {
			error(LackRBigBrack, 1);  //应该为右大括号
			return;
		}
		insertMiddleCode("genLabel", "", "", default_label_final);  //生成default结束位置的label标签
		writeToInfixExpressionFile(default_label_final + ":");
		writeToFile("switchStatement");
	}
}

//＜情况表＞   :: = ＜情况子语句＞{ ＜情况子语句＞ }
void situationTable(string default_label_final,string switchResult,int SwitchVarType) {
	while (token == casesy) {
		situationStatement(default_label_final, switchResult, SwitchVarType);
	}

}

//＜情况子语句＞  :: = case＜常量＞：＜语句＞
void situationStatement(string default_label_final, string switchResult, int SwitchVarType) {  //常量为整数或者字符，(整数可以带符号)
	string label;
	int Sign = -1, tempNum;
	label = genNewLabel();
	if (token == casesy) {
		insymbol();
		if (token == plusSign || token == minusSign) {
			Sign = token;
			insymbol();
		}
		if (token == intconst || token == charconst) {	//识别结果放在inum中
			//类型检查
			if ((token == intconst && SwitchVarType == CHAR) || (token == charconst && SwitchVarType == INT)) {
				//cout << "warning:第" << lineNum << "行的switch变量判断的类型不匹配！" << endl;
				error(SwitchTypeNotMatch);
				return;
			}
			if (Sign != -1 && inum == 0) {
				error(SignBeforeZero, 1);  //0前面有符号
				return;
			}
			else if (Sign != -1 && token == charconst) {
				error(SignBeforeChar);  //字符常量前面有符号
				return;
			}
			if (Sign == minusSign) {
				tempNum = -inum;
			}
			else {
				tempNum = inum;
			}
			insertMiddleCode("==", switchResult, convertNumToString(tempNum), ""); //常量是否相等的判断
			writeToInfixExpressionFile(switchResult + " == " + convertNumToString(tempNum));
			insertMiddleCode("jne", "", "", label);
			writeToInfixExpressionFile("BZ " + label);
			insymbol();
		}
		else {
			error(notConst);  //应该为常量
			return;
		}
		if (token == colon) {
			insymbol();
		}
		else {
			error(LackColon, 0);  //应该为冒号
			return;
		}
		statement();
		insertMiddleCode("jump", "", "", default_label_final);  //执行结束跳转到default结束后位置
		writeToInfixExpressionFile("GOTO " + default_label_final);
		insertMiddleCode("genLabel", "", "", label);  //生成下一个case或者default的label标签
		writeToInfixExpressionFile(label + ":");
	}


}

//＜缺省＞   :: = default: ＜语句＞
void defaultStatement() {
	if (token == defaultsy) {
		insymbol();
		if (token == colon) {
			insymbol();
		}
		else {
			error(LackColon, 1);  //应该为冒号
			return;
		}
		statement();
	}
}

//＜读语句＞    :: = scanf ‘(’＜标识符＞{ ,＜标识符＞ }‘)’
void scanfStatement() {
	int result;
	string scanfType;
	string tempName;
	if (token == scanfsy) {
		insymbol();
		if (token == lsmallbrack) {
			insymbol();
		}
		else {
			error(LackLSmallBrack, 1);  //应该为左小括号
			return;
		}
		do
		{
			if (token == comma) insymbol();
			if (token == ident) {
				tempName = id;
				result = findInSymbolTable(tempName, 0);  //存储结果后还需判断result
			}
			else {
				error(LackIdent, 5); //应该为标识符
				return;
			}
			if (result >= 0) { //还需判断不为常量
				//判断是不是常量为左值
				if (symbolTable[result].kind == intconst || symbolTable[result].kind == charconst) {
					error(AssignConst, 1);  //为常量赋值，报错
					cout << "第" << lineNum << "行为常量赋值！" << endl;
					return;
				}
				scanfType = symbolTable[result].kind == intsy ? "int" : "char";
				insertMiddleCode("scf", scanfType, "", tempName);
				writeToInfixExpressionFile("input " + tempName + " (" + scanfType + ")");
				insymbol();
			}
			else {
				return;
			}

		} while (token == comma);
		if (token == rsmallbrack) {
			insymbol();
		}
		else {
			error(LackRSmallBrack, 3);  //应该为右小括号
			return;
		}
		writeToFile("scanfStatement");
	}
}

//＜写语句＞    :: = printf ‘(’ ＜字符串＞, ＜表达式＞ ‘)’ | printf ‘(’＜字符串＞ ‘)’ | printf ‘(’＜表达式＞‘)’
void printfStatement() {
	string tempString = "", tempResult= "", printType = "";
	if (token == printfsy) {
		insymbol();
		if (token == lsmallbrack) {
			insymbol();
		}
		else{
			error(LackLSmallBrack, 1);  //应该为左小括号
			return;
		}
		if (token == stringconst) {
			tempString = stringContent;
			insymbol();
			if (token == comma) {
				insymbol();
				expression();
				tempResult = OperationResult;
				//cout << "printf语句中的" << factorType << endl;
				printType = factorType == INT ? "int" : "char";
			}
		}
		else { //处理表达式
			expression();
			tempResult = OperationResult;
			printType = factorType == INT ? "int" : "char";  //依据表达式结果选择打印类型
		}
		if (token == rsmallbrack) {
			insymbol();
		}
		else {
			error(LackRSmallBrack, 3);  //应该为右小括号
			return;
		}
		insertMiddleCode("ptf", tempString, tempResult, printType);
		writeToInfixExpressionFile("output " + tempString + " (string)");
		writeToInfixExpressionFile("output " + tempResult + " (" + printType + ")");
		writeToFile("printfStatement");
	}
}

//＜返回语句＞   :: = return[‘(’＜表达式＞‘)’]
void returnStatement() {
	string tempResult="";
	if (token == returnsy) {
		insymbol();
		if (token == lsmallbrack) {
			insymbol();
			expression();  //表达式结果存储
			tempResult = OperationResult;
			if (token == rsmallbrack) {
				hasReturnValue = true;
				insertMiddleCode("ret", "", "", tempResult);
				writeToInfixExpressionFile("ret " + tempResult);
				insymbol();
				return;
			}
			else {
				error(LackRSmallBrack, 3);  //应该为右小括号
				return;
			}
		}
		//只有一个return，返回即可
		insertMiddleCode("ret", "", "", "");
		writeToInfixExpressionFile("ret ");
		writeToFile("returnStatement");
		return;
	}
}

//
//附加说明：
//
//（1）char类型的表达式，用字符的ASCII码对应的整数参加运算，在写语句中输出字符
//
//（2）标识符区分大小写字母
//
//（3）写语句中的字符串原样输出
//
//（4）情况语句中，switch后面的表达式和case后面的常量只允许出现int和char类型；每个情况子语句执行完毕后，不继续执行后面的情况子语句
//
//（5）数组的下标从0开始
//
//（6）for语句先执行一次循环体中的语句再进行循环变量是否越界的测试
//for循环，先执行后判断需要3个label
//只在赋值语句中允许int到char或者char到int的类型转换，需要给出警告提示。
