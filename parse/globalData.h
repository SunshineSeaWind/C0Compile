//
//  globalData.h
//  C0compiler
//
#ifndef _GLOBAL_DATA
#define _GLOBAL_DATA
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cassert>
#include <map>
#include <set>
#include <vector>
#include <stack>
#include <algorithm>
using namespace std;

//===============================================================main.cpp
extern string inputFilePath, outputFilePath, outLexicalPath, outSymbolTablePath, outMidCodePath,
OutInfixExpressionPath, outOptCodePath, outMipsCodePath, outOptMidCodePath; //文件路径
extern ifstream infile;
extern ofstream outfile, outLexical, outSymbolTable, outMidCode, OutInfixExpression, 
outOptCode, outMipsCode, outOptMidCode;


//===============================================================lexical.cpp
extern const int keyWordsNum, signMAXLen, kmax, nmax;
extern string lineContent, id, stringContent;//id存储识别出来的标识符
extern string keyword[];
extern int keywordsy[];
extern map<char, int> sps;
extern map<char, string> spsString;
extern int charCounter, inum, token, readOver, lineNum;
extern unsigned char ch;
typedef struct {
	int num;
	string type;
	string name;
}INFO;
extern vector<INFO> infoVec;
void insymbol();
ostream &operator<< (ostream &os, const INFO & info);


//===============================================================syntax.cpp
const int maxTableSize = 512;
//声明kind的所有数值情况，代表标识符的类别
//intconst, charconst对应0,1
#define PARAMETER 2
#define FUNCTION 3
//intsy, charsy 对应24，25

typedef struct {
	string name;  //标识符名字
	int kind; //标识符的类别
	int value; //数值存储，若为函数，则有返回值存储1，无返回值存储0
	int arraySize; //如果是数组的话，存储数组的size大小，其他情况size为0；
	int address; //变量的地址；函数返回值的地址
	int paraNum; //若为函数，代表参数个数，否则为0；
} signTable; //符号表项结构

extern int funcTotalNum;	//代表符号表中分程序的总数
extern int symbolTableTop; //当前符号表的栈顶位置(将要存放表项的位置)
extern signTable symbolTable[]; //符号表
extern int displayTable[];  //分程序索引数组
extern int mainNum;

void writeToInfixExpressionFile(string str);
void writeToFile(string writeString);
string convertNumToString(int num);
void printSymtableTable();
int insertIntoSymTable(string name, int kind, int value, int arraySize, int address, int paraNum);
void flushTable();
int findInSymbolTable(string name, int searchMode);
void programme();
void constExplain();
void constDefine(int type);
void declareHead();
void returnFuncDefine();
void parameterTable();
void noReturnFuncDefine();
void MainDefine();
void variableExplain();
void variableDefine();
void complexStatement();
void statementList();
void statement();
void valueOfParameterTable();
void expression();
void item();
void factor();
void assignStatement();
void ifStatement();
void condition(int conditionType);
void forStatement();
void switchStatement();
void situationTable(string default_label_final, string switchResult, int SwitchVarType);
void situationStatement(string default_label_final, string switchResult, int SwitchVarType);
void defaultStatement();
void scanfStatement();
void printfStatement();
void returnStatement();


//===============================================================error.cpp
extern int errorpos;
extern vector<int> errors;
extern map<int, string> errorMsg;
extern int errNum;
const int maxErrorNum = 256;
/*没有该文件、缺少双引号、缺少单引号、缺少分号、缺少逗号、缺少冒号、缺少左小括号、缺少右小括号、缺少右中括号、
缺少左大括号、缺少右大括号、缺少标识符、缺少返回值、缺少加减号、缺少数字的声明、缺少 =号，不是int或者char的类型标识符、main函数个数不是1、
字符由非法字符组成、字符串由非法字符组成、使用的函数未定义、使用的变量未定义、关键字错误、表满了（多种表，符号表，分程序索引表，四元式表等）、
符号表插入数据重名、main函数后还有代码、形参个数不匹配、参数类型不匹配、无返回值函数作为当做右值、给常量赋值、整数范围过大、赋值语句两边类型不对、
数字以0开头且不是0这个整数、读取到的字符长度大于1、词法分析程序读取到非法的字符（不符合任何单词的构成规则）、常量声明在变量后面、0前面有符号、
字符前面有符号、变量或者常量声明时候的类型不匹配、无返回值函数却有返回值、（定义时）数组下标不为无符号整数、数组大小为0、
句子不合法（例如某句子应该为赋值或者函数调用语句，却都不是等）、语句处理时均不符合条件、数组下标越界、因子处理时均不符合条件、类型不为数组、
数组调用时表达式结果不为整型、for循环判断条件与循环变量无关、for循环更新条件与循环变量无关、步长为0、不为常量、switch表达式类型与case判断的类型不匹配、
数组名字赋值、除数为0

*/
enum errorType {
	NoSuchFile, LackDoubleQuotes, LackSingleQuote, LackSemicolon, LackComma, LackColon, LackLSmallBrack,
	LackRSmallBrack, LackRMidBrack, LackLBigBrack, LackRBigBrack, LackIdent,
	LackReturnValue, LackPlusOrMinus, LackNum, LackBecome, NotIntOrChar, MainNumNotOne,
	CharIllegal, StringIllegal, FunctionNotDefine, VariableNotDefine,
	KeywordError, TableFull, SymbolNameConflict, CodeAfterMain, FormalParaNumNotMatch, ParaTypeNotMatch,
	AssignWithNoReturnFunc, AssignConst, NumOutOfRange, AssignTypeNotMatch, BeginWithZeroButNotZero, CharLenBigThanOne,
	LexicalIlegalCh, ConstDefAfterVarDef, SignBeforeZero, SignBeforeChar, DataTypeNotMatch, NoReturnFuncWithReturn, ArrayIndexNotUnsignedNum,
	ArraySizeIsZero, StatementIsIllegal, StatementIllegalSign, ArrayIndexOfRange, FactorIllegalSign, TypeNotArray, ArrayIndexNotIntType,
	forConditionNotContainVar, forUpdateNotContainVar, StepIsZero, notConst, SwitchTypeNotMatch, AssignArray, DivideZero
};
//存储错误的种类以及行数的结构体
typedef struct {
	int lineNum;
	int ErrorType;
}ErrorItem;
//错误处理的操作
enum errorOperation {
	Quit, DoNothing, IntCharVoid, ConstIntChar, CommaSemicolon, LSmallBrack, RBigBrack,
	IfForLBigBrackICVISPSRS, PlusMinusTimesDivSemicolon, SemiColon, RSmallBrack, CaseDefaultRBigBrack
};
void error(int num, int dealType = -1);
void dealWithError(int operationType);
bool isNotInTable(ErrorItem erroritem, ErrorItem errortable[]);


//AssignTypeNotMatch 除去该类型错误还有51个错误
//===============================================================midcode.cpp
#define MAXMIDECODESIZE 1024

void insertMiddleCode(string op, string a, string b, string c);
string genNewRegister();
string genNewLabel();
extern int RegisterCount;
extern int MidCodeTableTop;
typedef struct {
	string op;
	string part_1;
	string part_2;
	string part_3;
}fourExpression;
extern fourExpression MidCodeTable[];


//===============================================================optimize.cpp
//基本块结构
typedef struct {
	int beginIndex;
	int endIndex;
}BasicBlockItem;
typedef struct {
	vector<BasicBlockItem> beforeBlock;
	BasicBlockItem Block;
	vector<BasicBlockItem> afterBlock;
}BasicBlock;
//dag图节点结构体
struct Dagitem {
	string name;
	int num;
	vector<string> varItems;
	vector<Dagitem*> parent;
	Dagitem* lchild;
	Dagitem* rchild;
	//是否已经添加到队列中
	bool isInStack;
};
typedef Dagitem DagItem;
void divideBasicBlock();
void optAllExpression();
void mergeConstant();
//void deletePublicExpression();
void deleteLabel();
bool isNumber(string str);
void deleteVarInVector(string varName);
void dag(); 
int findInDag(string op, int i, int j);
string realName(string str, DagItem *tempItem);
bool parentIsInStack(DagItem* tempDagItem);
void exportDagToMidcode();

//===============================================================mips.cpp
#define MAXVARNUM 2048
#define MAXFUNCNUM 1024
typedef struct {
	string name;
	int kind;
	int address;
	int DataType;
} VariableItem;
typedef struct {
	string name;
	int kind;
} FunctionItem;
extern VariableItem variableTable[];  //变量表
extern FunctionItem functionTable[]; //函数表

void Assign2Mips();
void AssignArr2Mips();
void calculation2Mips(int type);
void call2Mips();
void compare2Mips(int type);
void const2Mips();
int findVarInVariableTable(string name);
void funcContent2Mips();
void funcExplain2Mips();
void funcPa2Mips();
string genCompareLabel();
void genLabel2Mips();
void getArr2Mips();
void insertIntoFuncTable(int kind, string name);
void insertIntoVariableTable(int kind, int DataType, int address = -1, bool isGlobalData = false);
void intOrChar2Mips();
void intOrCharArray2Mips();
void jne2Mips();
void jump2Mips();
void midCode2Mips();
void para2Mips();
void ptf2Mips();
void pushIntoStack(string value, int size = 1, bool isGlobalData = false);
void ret2Mips();
void scf2Mips();
void globalConst2Mips();
void globalIntOrChar2Mips();
void globalIntOrCharArray2Mips();
//引用计数方法
//对应变量在变量表中的编号以及出现数量
typedef struct {
	string name;
	int number;
	int address;//记录相对偏移量
	int count; //变量出现的个数
}CountStruct;
//对应变量在变量表中的编号以及要分配的寄存器
typedef struct {
	string name;
	int number;
	int address;//记录相对偏移量
	int regNum; //分配的寄存器编号
}RegStruct;
extern bool useCountRegOpt;
void CountReg();
bool compareVector(const CountStruct &a, const CountStruct &b);
void countNumOfVar(string varName);
bool isVarName(string name);
bool VarUseRegister(string FuncName, string varName, string tempRegister, bool needSw = false);
void loadSRegisters(string functionName);
void saveSRegisters(string functionName);
//===============================================================globalData.h
enum symbol {
	intconst, charconst, stringconst, plusSign, minusSign, timesSign, idivSign, eql, neq, gtr, geq
	, lss, leq, becomes, lsmallbrack, rsmallbrack, lmidbrack, rmidbrack, lbigbrack, rbigbrack,
	comma, semicolon, colon, constsy, intsy, charsy, ident, ifsy, elsesy, switchsy, casesy,
	defaultsy, forsy, voidsy, mainsy, returnsy, printfsy, scanfsy	//最后的下标为37
};

#endif // MAIN