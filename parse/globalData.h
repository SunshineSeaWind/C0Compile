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
OutInfixExpressionPath, outOptCodePath, outMipsCodePath, outOptMidCodePath; //�ļ�·��
extern ifstream infile;
extern ofstream outfile, outLexical, outSymbolTable, outMidCode, OutInfixExpression, 
outOptCode, outMipsCode, outOptMidCode;


//===============================================================lexical.cpp
extern const int keyWordsNum, signMAXLen, kmax, nmax;
extern string lineContent, id, stringContent;//id�洢ʶ������ı�ʶ��
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
//����kind��������ֵ����������ʶ�������
//intconst, charconst��Ӧ0,1
#define PARAMETER 2
#define FUNCTION 3
//intsy, charsy ��Ӧ24��25

typedef struct {
	string name;  //��ʶ������
	int kind; //��ʶ�������
	int value; //��ֵ�洢����Ϊ���������з���ֵ�洢1���޷���ֵ�洢0
	int arraySize; //���������Ļ����洢�����size��С���������sizeΪ0��
	int address; //�����ĵ�ַ����������ֵ�ĵ�ַ
	int paraNum; //��Ϊ�����������������������Ϊ0��
} signTable; //���ű���ṹ

extern int funcTotalNum;	//������ű��зֳ��������
extern int symbolTableTop; //��ǰ���ű��ջ��λ��(��Ҫ��ű����λ��)
extern signTable symbolTable[]; //���ű�
extern int displayTable[];  //�ֳ�����������
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
/*û�и��ļ���ȱ��˫���š�ȱ�ٵ����š�ȱ�ٷֺš�ȱ�ٶ��š�ȱ��ð�š�ȱ����С���š�ȱ����С���š�ȱ���������š�
ȱ��������š�ȱ���Ҵ����š�ȱ�ٱ�ʶ����ȱ�ٷ���ֵ��ȱ�ټӼ��š�ȱ�����ֵ�������ȱ�� =�ţ�����int����char�����ͱ�ʶ����main������������1��
�ַ��ɷǷ��ַ���ɡ��ַ����ɷǷ��ַ���ɡ�ʹ�õĺ���δ���塢ʹ�õı���δ���塢�ؼ��ִ��󡢱����ˣ����ֱ����ű��ֳ�����������Ԫʽ��ȣ���
���ű��������������main�������д��롢�βθ�����ƥ�䡢�������Ͳ�ƥ�䡢�޷���ֵ������Ϊ������ֵ����������ֵ��������Χ���󡢸�ֵ����������Ͳ��ԡ�
������0��ͷ�Ҳ���0�����������ȡ�����ַ����ȴ���1���ʷ����������ȡ���Ƿ����ַ����������κε��ʵĹ��ɹ��򣩡����������ڱ������桢0ǰ���з��š�
�ַ�ǰ���з��š��������߳�������ʱ������Ͳ�ƥ�䡢�޷���ֵ����ȴ�з���ֵ��������ʱ�������±겻Ϊ�޷��������������СΪ0��
���Ӳ��Ϸ�������ĳ����Ӧ��Ϊ��ֵ���ߺ���������䣬ȴ�����ǵȣ�����䴦��ʱ�������������������±�Խ�硢���Ӵ���ʱ�����������������Ͳ�Ϊ���顢
�������ʱ���ʽ�����Ϊ���͡�forѭ���ж�������ѭ�������޹ء�forѭ������������ѭ�������޹ء�����Ϊ0����Ϊ������switch���ʽ������case�жϵ����Ͳ�ƥ�䡢
�������ָ�ֵ������Ϊ0

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
//�洢����������Լ������Ľṹ��
typedef struct {
	int lineNum;
	int ErrorType;
}ErrorItem;
//������Ĳ���
enum errorOperation {
	Quit, DoNothing, IntCharVoid, ConstIntChar, CommaSemicolon, LSmallBrack, RBigBrack,
	IfForLBigBrackICVISPSRS, PlusMinusTimesDivSemicolon, SemiColon, RSmallBrack, CaseDefaultRBigBrack
};
void error(int num, int dealType = -1);
void dealWithError(int operationType);
bool isNotInTable(ErrorItem erroritem, ErrorItem errortable[]);


//AssignTypeNotMatch ��ȥ�����ʹ�����51������
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
//������ṹ
typedef struct {
	int beginIndex;
	int endIndex;
}BasicBlockItem;
typedef struct {
	vector<BasicBlockItem> beforeBlock;
	BasicBlockItem Block;
	vector<BasicBlockItem> afterBlock;
}BasicBlock;
//dagͼ�ڵ�ṹ��
struct Dagitem {
	string name;
	int num;
	vector<string> varItems;
	vector<Dagitem*> parent;
	Dagitem* lchild;
	Dagitem* rchild;
	//�Ƿ��Ѿ���ӵ�������
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
extern VariableItem variableTable[];  //������
extern FunctionItem functionTable[]; //������

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
//���ü�������
//��Ӧ�����ڱ������еı���Լ���������
typedef struct {
	string name;
	int number;
	int address;//��¼���ƫ����
	int count; //�������ֵĸ���
}CountStruct;
//��Ӧ�����ڱ������еı���Լ�Ҫ����ļĴ���
typedef struct {
	string name;
	int number;
	int address;//��¼���ƫ����
	int regNum; //����ļĴ������
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
	defaultsy, forsy, voidsy, mainsy, returnsy, printfsy, scanfsy	//�����±�Ϊ37
};

#endif // MAIN