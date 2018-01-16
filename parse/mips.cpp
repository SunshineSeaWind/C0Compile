//
//  mips.cpp
//  C0compiler
//
#include "globalData.h"
//����������Լ�������Ĵ洢���ݵ�����
#define VOID 0
#define INT 1
#define CHAR 2
//���ݵ����ͣ������������
#define GLOBAL 0
#define LOCALCONST 1
#define LOCALVAR 2
#define PARA 3

VariableItem variableTable[MAXVARNUM];  //������
FunctionItem functionTable[MAXFUNCNUM]; //������
int sp, fp, gp;  //ջָ��(��Ե�ַ)��ָ֡�룬ȫ�ֱ���ָ��
int variableTableTop = 0;  //�������ջ��ָ��
int functionTableTop = 0;  //�������ջ��ָ��
int midCodeTablePtr = 0;  //��Ԫʽ���ָ�룬�� 0 ��ʼһֱ������ MidCodeTableTop
int constEndAddress = -1;  //��¼ȫ�������ڱ������еĽ���λ��
bool isGlobal = false;  //�ڲ���ʱ����¼��ǰ�����Ƿ�Ϊȫ�ֱ���
bool isMain = false;  //��¼��ǰ����ĺ����Ƿ�Ϊmain������
int CompareLabel = 0;	//�Ƚ�������ɵı�ǩ�ı��
int functionNum = 0; //�������ɺ�������ʱ��ı�ǩ������return���֮�����ת
bool useCountRegOpt = false;  //�Ƿ�ʹ�ü��������Ż��Ĵ���
ofstream outFile;   //����ı�������Ϣ�Ĵ�ӡ�ļ�
vector<CountStruct> CountVector;  //���CountStruct�ṹ����������洢�������ֵĴ���
vector<RegStruct> RegVector;  //���RegStruct�ṹ����������洢��������ļĴ���
map<string, vector<RegStruct>> FunctionRegMap; //ÿ����������Ϊ������Ӧ�ļĴ�����������Ϊֵ
string NowFuncName = ""; //���ڴ���ĺ�������
//bool hasCounted = false;
//���м�ָ��ת��Ϊmipsָ��
void midCode2Mips() {
	//�ٴγ�ʼ��,��Ϊ���������Σ��Ż�ǰ���Ż���
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
	//�Ƚ��г�ʼ��������ָ֡�롢ȫ�ֱ�����ŵ�ַ�Լ�����$s0-$s7�Ĵ������ݵĵ�ַ����$t9�Ĵ������ȫ�ֱ�������ʼ��ַ��
	outMipsCode << ".text" << endl;
	outMipsCode << "addi\t$fp\t$sp\t0\t# fp = sp" << endl; //addi $fp $sp 0
	//outMipsCode << "li\t$t9\t0x7fffeffc\t#global stack bottom" << endl;  //ȫ�ֱ�����ŵ�ַ
	outMipsCode << "add\t$t9\t$gp\t$0\t#global stack bottom" << endl;  //ȫ�ֱ�����ŵ�ַ����$gp��ʼ  add $t9 $gp $0 
	//outMipsCode << "li\t$t9\t0x10007ffc\t#global stack bottom" << endl;  //ȫ�ֱ�����ŵ�ַ����$gp��ʼ  li $t9 0x10007ffc
	//outMipsCode << "li\t$t8\t0x10010000\t#$s0-$s7 save address" << endl;  //����$s0-$s7�Ĵ������ݵĵ�ַ
	sp = gp = variableTableTop = midCodeTablePtr = 0;  //��ʼ������ָ��
	while (midCodeTablePtr < MidCodeTableTop)  //һֱ��������Ԫʽ�Ľ���
	{
		//����ȫ�ֳ�������Ԫʽ
		while (MidCodeTable[midCodeTablePtr].op == "const") {
			globalConst2Mips();
		}
		//����ȫ�ֱ�����������������һ����
		while (MidCodeTable[midCodeTablePtr].op == "int" || MidCodeTable[midCodeTablePtr].op == "char" ||
			MidCodeTable[midCodeTablePtr].op == "intArr" || MidCodeTable[midCodeTablePtr].op == "charArr")
		{
			//����ȫ�ֱ����������飩
			if (MidCodeTable[midCodeTablePtr].op == "int" || MidCodeTable[midCodeTablePtr].op == "char") {
				globalIntOrChar2Mips();
			}
			//����ȫ�ֱ��������飩
			else {
				globalIntOrCharArray2Mips();
			}
		}
		//������ȫ�����ݺ����ת��_main��ǩ����ʼ������ִ��
		outMipsCode << "j\t_main" << endl;  // j _main
		//��¼��ʱȫ�����ݵĽ���֮���ڱ������е�λ�ã������Ժ�ȷ�����ҵı����ĳ�ʼ��ַ������ں���fp����ȫ�ֵ�ַ$t9
		constEndAddress = variableTableTop;
		//������,������뺯�����У����ں������洢��ֵ���������������ջ
		//func, int / char / void, , ��������
		while (MidCodeTable[midCodeTablePtr].op == "func") {
			functionNum++;   //���Ӻ���������
			funcExplain2Mips();
			//�������е����
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
//�����������ѹ��ջ��,ѹ��sp����gpָ��λ��
void pushIntoStack(string value, int size, bool isGlobalData) {
	//�������sizeΪ1����Ӧ��sw������ֵ��ջ
	if (size == 1) {  //���洢һ����������� ��ֵ�洢�ڴ����value��
		//ȫ�ִ���gp
		if (isGlobalData) {
			outMipsCode << "li\t$t0\t" << value << "\t#" << MidCodeTable[midCodeTablePtr].part_3 << endl; // li $t0 value ���ϣ������������ֵ�ע��
			outMipsCode << "sw\t$t0\t0($gp)" << endl;  // sw $t0 0($gp)
		}
		//�������sp
		else {
			outMipsCode << "li\t$t0\t" << value << "\t#" << MidCodeTable[midCodeTablePtr].part_3 << endl; // li $t0 value ���ϣ������������ֵ�ע��
			outMipsCode << "sw\t$t0\t0($sp)" << endl;  // sw $t0 0($sp)
		}
	}
	//ȫ�������gp
	if (isGlobalData) {
		outMipsCode << "subi\t$gp\t$gp\t" << 4 * size << endl; //subi $gp $gp 4 * size
		gp -= 4 * size;
	}
	//�������sp
	else {
		outMipsCode << "subi\t$sp\t$sp\t" << 4 * size << endl; //subi $sp $sp 4 * size
		sp -= 4 * size;
	}
}

//������ı������뵽�������У�����֮���Ѱַ����
void insertIntoVariableTable(int kind, int DataType, int address, bool isGlobalData) {
	//����������ִ�ʱ�洢�ĵ�ַ�����鿪ʼλ�õĵ�ַ��֮��������Ҫ�任һ��
	variableTable[variableTableTop].kind = kind;
	//�洢��ַ����Ե�ַ��
	if (address == -1) {  
		//ȫ�ִ��������gp��ƫ����
		if (isGlobalData) {
			variableTable[variableTableTop].address = gp;
		}
		//����洢�����spƫ����
		else {
			variableTable[variableTableTop].address = sp;
		}
	}
	else {
		variableTable[variableTableTop].address = address;
	}
	//�洢����	ʹ�õ�ǰ��Ԫʽ����
	variableTable[variableTableTop].name = MidCodeTable[midCodeTablePtr].part_3;
	variableTable[variableTableTop].DataType = DataType;
	variableTableTop++;
}

//������ĺ������뺯������
void insertIntoFuncTable(int kind, string name) {
	functionTable[functionTableTop].kind = kind;
	functionTable[functionTableTop].name = name;
	functionTableTop++;
}
//��������ı��������Ƿ��ڱ������У�û�ҵ�����-1���ҵ����ص�ַ��-4�ı�������
int findVarInVariableTable(string name) {
	isGlobal = false;  //��ʼ��
					   //��Ϊ�������֣���ֱ�ӷ��� -1
	if (isdigit(name[0]) || name[0] == '+' || name[0] == '-') {
		return -1;
	}
	//һ��Ҫ�Ӷ������²��ң���֤ͬ���ı�����ȡ���������һ������������
	for (int start = variableTableTop - 1; start >= 0; start--)
	{
		if (variableTable[start].name == name) {
			if (start < constEndAddress) {
				isGlobal = true;  //�ҵ�������Ϊȫ������
			}
			return variableTable[start].address;
		}
	}
	return -1;
}
/*�ֲ�������Ԫʽת��Ϊmipsָ��
���� �� const, int/char, ��ֵ, ������ʶ������
*/
void const2Mips() {
	int kind = MidCodeTable[midCodeTablePtr].part_1 == "int" ? INT : CHAR;
	insertIntoVariableTable(kind, LOCALCONST);
	pushIntoStack(MidCodeTable[midCodeTablePtr].part_2);
	midCodeTablePtr++;
}
//ȫ�ֳ�������ת��Ϊmipsָ��
void globalConst2Mips() {
	int kind = MidCodeTable[midCodeTablePtr].part_1 == "int" ? INT : CHAR;
	insertIntoVariableTable(kind, GLOBAL, -1, true);
	pushIntoStack(MidCodeTable[midCodeTablePtr].part_2, 1, true);
	midCodeTablePtr++;
}
/*�ֲ������������飩��Ԫʽת��Ϊmipsָ��
���� �� int/char,  ,  , ��������
*/
void intOrChar2Mips() {
	int kind = MidCodeTable[midCodeTablePtr].op == "int" ? INT : CHAR;
	insertIntoVariableTable(kind, LOCALVAR);
	pushIntoStack("0");  //�����˴��洢��ֵ 0 ���ɣ���Ϊ��ʼ��
	midCodeTablePtr++;
}
//ȫ�ֱ����������飩ת��Ϊmipsָ��
void globalIntOrChar2Mips() {
	int kind = MidCodeTable[midCodeTablePtr].op == "int" ? INT : CHAR;
	insertIntoVariableTable(kind, GLOBAL, -1, true);
	pushIntoStack("0", 1, true);  //�����˴��洢��ֵ 0 ���ɣ���Ϊ��ʼ��
	midCodeTablePtr++;
}
/*�ֲ����������飩��Ԫʽת��Ϊmipsָ��
���� �� intArr/charArr, 0, �����С, ��������
*/
void intOrCharArray2Mips() {
	int kind = MidCodeTable[midCodeTablePtr].op == "intArr" ? INT : CHAR;
	insertIntoVariableTable(kind, LOCALVAR);
	pushIntoStack("0", atoi(MidCodeTable[midCodeTablePtr].part_2.c_str()));  //�����˴��洢��ֵ 0 ���ɣ���Ϊ��ʼ��,��������Ĵ�С
	midCodeTablePtr++;
}
//ȫ�ֱ��������飩ת��Ϊmipsָ��
void globalIntOrCharArray2Mips() {
	int kind = MidCodeTable[midCodeTablePtr].op == "intArr" ? INT : CHAR;
	insertIntoVariableTable(kind, GLOBAL, -1, true);
	pushIntoStack("0", atoi(MidCodeTable[midCodeTablePtr].part_2.c_str()), true);  //�����˴��洢��ֵ 0 ���ɣ���Ϊ��ʼ��,��������Ĵ�С
	midCodeTablePtr++;
}
/*����������Ԫʽת��Ϊmipsָ��
���� �� func,  int/char/void,  , ��������
*/
void funcExplain2Mips() {
	int kind = MidCodeTable[midCodeTablePtr].part_1 == "int" ? INT :
		MidCodeTable[midCodeTablePtr].part_1 == "char" ? CHAR : VOID;
	string name = MidCodeTable[midCodeTablePtr].part_3;
	insertIntoFuncTable(kind, name);
	NowFuncName = name;
	//������main����,����_main��ǩ
	if (name == "main") {
		isMain = true;
		outMipsCode << "_main:" << endl;
	}
	//�������ɺ������ֵı�ǩ
	else {
		outMipsCode << name << ":" << endl;
	}
	midCodeTablePtr++;
}
/*�������еĲ��֣�ת��Ϊmipsָ��
���ں����Ľ�������Ϊ end, 	, 	, �������� �� �ʴ���end���ɽ����ú���
��������Ҫ�����ֳ�����������һ��������fp�����淵�ص�ַ�������ٿ�ʼ����������*/
void funcContent2Mips() {
	int tempPtr = midCodeTablePtr;  //��ʱ����������ɨ��ú����е������м����
	sp = 0;  //���� sp����Ϊ���ݲ�����Ե�ַ����ȡ
			 //����$s0-$s7�Ĵ�������ֵ����һ��������fp�����淵�ص�ַ
	//outMipsCode << "#save $s0-$s7 Registers" << endl;
	//saveSRegisters();
	outMipsCode << "#save last Function $fp" << endl;
	outMipsCode << "sw\t$fp\t0($sp)" << endl;  // sw fp 0($sp)
	outMipsCode << "addi\t$fp\t$sp\t0\t# fp = sp" << endl; //addi $fp $sp 0 �� fp = sp
	outMipsCode << "subi\t$sp\t$sp\t" << 4 << endl; //subi $sp $sp 4  sp = sp - 4
	sp -= 4;
	outMipsCode << "#save last Function $ra" << endl;
	outMipsCode << "sw\t$ra\t0($sp)" << endl;  // sw ra 0($sp)
	outMipsCode << "subi\t$sp\t$sp\t" << 4 << endl; //subi $sp $sp 4  sp = sp - 4
	sp -= 4;
	outMipsCode << "#save last Function $fp and $ra Finished" << endl;
	//��ɨ��һ�麯��������Ԫʽ����������ֵ�������ʱ��������������У���Ԫʽ���ɵ���ʱ������$_X��ʾ��
	while (MidCodeTable[midCodeTablePtr].op != "end") {
		string part_1 = "", part_2 = "", part_3 = "";
		part_1 = MidCodeTable[midCodeTablePtr].part_1;
		part_2 = MidCodeTable[midCodeTablePtr].part_2;
		part_3 = MidCodeTable[midCodeTablePtr].part_3;
		//���е���ʱ��������Ԫʽ�����ģ����������
		if (part_1.length() > 0 && part_1[0] == '$' && findVarInVariableTable(part_1) == -1) { //���ڱ������У�����������
			insertIntoVariableTable(INT, LOCALVAR);  //part_1����ʱ����������Ľ��������������ΪINT
			pushIntoStack("0");
		}
		if (part_2.length() > 0 && part_2[0] == '$' && findVarInVariableTable(part_2) == -1) { //���ڱ������У�����������
			insertIntoVariableTable(INT, LOCALVAR);  //part_2����ʱ����������Ľ��������������ΪINT
			pushIntoStack("0");
		}
		if (part_3.length() > 0 && part_3[0] == '$' && findVarInVariableTable(part_3) == -1) { //���ڱ������У�����������
			//part_3����ʱ��������������ͺ������ã���Ҫ�ж�����Ԫ�������Լ���������ֵ����
			if (MidCodeTable[midCodeTablePtr].op == "getArr") {  //��Ҫ�õ����������
				//getArr, �������֣� �����±꣬ ����洢�ļĴ���
				insertIntoVariableTable(0, LOCALVAR);
			}
			//���ú������ call, �������֣�	���Ĵ�������������Ϊ�գ�
			else if (MidCodeTable[midCodeTablePtr].op == "call") {
				//call, �������֣�	���Ĵ�������������Ϊ�գ�
				insertIntoVariableTable(0, LOCALVAR);
			}
			else {  //������ʱ����������ֱ������ΪINT����
				insertIntoVariableTable(INT, LOCALVAR);
			}
			pushIntoStack("0");  //�Ĵ��������Ĵ�СΪ4���ֽڣ��ȳ�ʼ������Ϊ 0 
		}
		midCodeTablePtr++;
	}
	midCodeTablePtr = tempPtr; //��ԭ��Ԫʽ����������������еı���
	//�ٴ�ɨ�裬�õ������ļĴ���������������ú����ļĴ������䷽������FunctionRegMap��
	while (MidCodeTable[midCodeTablePtr].op != "end") {
		//���������������������飩ȫ�������������
		while (MidCodeTable[midCodeTablePtr].op == "const" || MidCodeTable[midCodeTablePtr].op == "int" || MidCodeTable[midCodeTablePtr].op == "char"
			|| MidCodeTable[midCodeTablePtr].op == "intArr" || MidCodeTable[midCodeTablePtr].op == "charArr" || MidCodeTable[midCodeTablePtr].op == "para") {
			//�������������壩   para,  int/char,  ,��������
			if (MidCodeTable[midCodeTablePtr].op == "para") {
				para2Mips();
			}
			//��������
			else if (MidCodeTable[midCodeTablePtr].op == "const") {
				const2Mips();
			}
			//�������壨�����飩
			else if (MidCodeTable[midCodeTablePtr].op == "int" || MidCodeTable[midCodeTablePtr].op == "char") {
				intOrChar2Mips();
			}
			//�������壨���飩
			else if (MidCodeTable[midCodeTablePtr].op == "intArr" || MidCodeTable[midCodeTablePtr].op == "charArr") {
				intOrCharArray2Mips();
			}
			else {
				break;
			}
		}
		break;
	}

	//��Ҫ�����Ż�
	//�˴�ʹ�����ü�����������Ϊ�����Ѿ������˱������У����Դ˴�����ͳ�ƹ���
	if (useCountRegOpt) {
		//���б����ļĴ�������
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
		//���ú����ļĴ������䷽������FunctionRegMap��
		FunctionRegMap[NowFuncName] = RegVector;
		CountVector.clear();
		RegVector.clear();

	}
	//�ٴ�ɨ�躯��������Ԫʽ���������е���Ԫ���ʽ
	//midCodeTablePtr = tempPtr; //��ԭ��Ԫʽ������
	while (MidCodeTable[midCodeTablePtr].op != "end") {

		//�Ӽ��˳� + - * /
		if (MidCodeTable[midCodeTablePtr].op == "+" || MidCodeTable[midCodeTablePtr].op == "-" ||
			MidCodeTable[midCodeTablePtr].op == "*" || MidCodeTable[midCodeTablePtr].op == "/") {
			int type = MidCodeTable[midCodeTablePtr].op == "+" ? 0 : MidCodeTable[midCodeTablePtr].op == "-" ? 1 :
				MidCodeTable[midCodeTablePtr].op == "*" ? 2 : MidCodeTable[midCodeTablePtr].op == "/" ? 3 : -1;
			calculation2Mips(type);
		}
		//�Ƚ������	< <= > >= == != 
		if (MidCodeTable[midCodeTablePtr].op == "<" || MidCodeTable[midCodeTablePtr].op == "<=" ||
			MidCodeTable[midCodeTablePtr].op == ">" || MidCodeTable[midCodeTablePtr].op == ">=" ||
			MidCodeTable[midCodeTablePtr].op == "!=" || MidCodeTable[midCodeTablePtr].op == "==") {
			int type = MidCodeTable[midCodeTablePtr].op == "<" ? 0 : MidCodeTable[midCodeTablePtr].op == "<=" ? 1 :
				type = MidCodeTable[midCodeTablePtr].op == ">" ? 2 : MidCodeTable[midCodeTablePtr].op == ">=" ? 3 :
				type = MidCodeTable[midCodeTablePtr].op == "!=" ? 4 : MidCodeTable[midCodeTablePtr].op == "==" ? 5 : -1;
			compare2Mips(type);
		}
		//ʵ����� funcPa�� �� ��ʵ�δ洢�ļĴ���(����Ϊ����)
		if (MidCodeTable[midCodeTablePtr].op == "funcPa") {
			funcPa2Mips();
		}
		//�õ�����Ԫ�� getArr, �������֣� �����±꣬ ����洢�ļĴ���
		if (MidCodeTable[midCodeTablePtr].op == "getArr") {
			getArr2Mips();
		}
		//��ֵ��䣨�����飩 =�� Ҫ������ֵ�� �� ������ʶ������
		if (MidCodeTable[midCodeTablePtr].op == "=") {
			Assign2Mips();
		}
		//��ֵ��䣨���飩	[]=, �������֣� �����±꣬ Ҫ�洢����ֵ
		if (MidCodeTable[midCodeTablePtr].op == "[]=") {
			AssignArr2Mips();
		}
		//�����	scf�� int/char�� �� ������ʶ��
		if (MidCodeTable[midCodeTablePtr].op == "scf") {
			scf2Mips();
		}
		//д���		ptf���ַ���(����Ϊ��)�� ����Ĵ���(����Ϊ��)��char/int/����Ϊ��
		if (MidCodeTable[midCodeTablePtr].op == "ptf") {
			ptf2Mips();
		}
		//������� ret�� �� ���������ļĴ�����Ҳ����Ϊ������������Ϊ�գ�
		if (MidCodeTable[midCodeTablePtr].op == "ret") {
			ret2Mips();
		}
		//���ú������ call, �������֣�	���Ĵ�������������Ϊ�գ�
		if (MidCodeTable[midCodeTablePtr].op == "call") {
			call2Mips();
		}
		//��������ת���	jump�� �� ��label
		if (MidCodeTable[midCodeTablePtr].op == "jump") {
			jump2Mips();
		}
		//������������ת���	jne�� �� ��label
		if (MidCodeTable[midCodeTablePtr].op == "jne") {
			jne2Mips();
		}
		//��ǩ���	genLabel�� �� ��label
		if (MidCodeTable[midCodeTablePtr].op == "genLabel") {
			genLabel2Mips();
		}
	}
	//��ӡÿ����������ʱ��ı�����
	outFile << "=======================================" << endl;
	outFile << "name\tkind\taddress\tDataType" << endl;
	for (int i = 0; i < variableTableTop; i++)
	{
		VariableItem variableitem = variableTable[i];
		outFile << variableitem.name << "\t" << variableitem.kind << "\t" << variableitem.address << "\t" << variableitem.DataType << endl;
	}
	//outFile << "=======================================" << endl;
	variableTableTop = constEndAddress;
	//���ɺ�������ʱ��Ľ�����ǩ
	outMipsCode << "FuncEndLabel_" << functionNum << ":" << endl;  //FuncEndLabel_x:
	//�ָ��ֳ����˻ص����øú���ǰ��״̬
	outMipsCode << "lw\t$ra\t-4($fp)" << endl;   //lw $ra -4($fp),���ص�ַ����$ra
	outMipsCode << "addi\t$sp\t$fp\t0" << endl;//addi $sp $fp 0	 //�ָ����øú����ĺ���$sp	 $sp = $fp												  
	outMipsCode << "lw\t$fp\t0($fp)" << endl;//lw $fp 0($fp)//�ָ����øú����ĺ���$fp
	//loadSRegisters();
	if (isMain) {  //��������������������������ֱ�ӽ���
		outMipsCode << "li\t$v0\t10" << endl;   //li $v0 10
		outMipsCode << "syscall" << endl;  //syscall �������
	}
	else { // ������ת��$ra�洢�ĵ�ַ
		outMipsCode << "jr\t$ra" << endl;   //jr $ra
	}
	midCodeTablePtr++;
}

//���ü�������
void CountReg() {
	int tempPointer = midCodeTablePtr;
	//ֱ������һ��������ĩβendΪֹ
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
	//��������󣬶�CountStruct����count�ֶν������򣬴Ӵ�С��ѡ��ǰ���8�����粻��8������ȫ��ѡ�񣩷���Ĵ���
	if (!CountVector.empty()) {
		sort(CountVector.begin(), CountVector.end(), compareVector);
		//ֻ����$s0 - $s7�Ĵ���
		for (int i = 0; i < CountVector.size(); i++)
		{
			RegStruct regStruct;
			regStruct.number = CountVector[i].number;
			regStruct.address = CountVector[i].address;
			regStruct.name = CountVector[i].name;
			regStruct.regNum = i;
			RegVector.push_back(regStruct);
			//��û�мĴ������Է��䣬���������
			if (i == 7) {
				break;
			}
		}
	}
}
//�Զ���ȽϺ���(�Ӵ�С)
bool compareVector(const CountStruct &a, const CountStruct &b) {
	return a.count > b.count;
}
//Ϊ����ı�������
void countNumOfVar(string varName) {
	//������Ĳ��Ǳ�������ֱ�ӷ��ؼ���
	if (!isVarName(varName)) {
		return;
	}
	int temp = variableTableTop - 1;
	//constEndAddress��¼ȫ�����ݵĶ���λ�ã��˴������ֲ�������tempҪ >= constEndAddress�ſ���ѭ��
	//ֻ���Ǿֲ��������������DataTypeΪLOCALVAR
	while (temp >= constEndAddress)
	{
		VariableItem tempItem = variableTable[temp];
		//����ڱ������в��ҵ��ñ�����temp��ʱΪ�ı����ڱ������е�λ������
		if (tempItem.name == varName && tempItem.DataType == LOCALVAR) {
			bool NotInVec = true;
			//�ڼ�¼������CountVector�в��ұ���
			for (vector<CountStruct>::iterator iter = CountVector.begin(); iter != CountVector.end(); iter++)
			{
				//�������иñ�����¼��������+1 ����
				if ((*iter).number == temp) {
					(*iter).count += 1;
					NotInVec = false;
					break;
				}
			}
			//�����ڣ������
			if (NotInVec) {
				CountStruct countStruct;
				countStruct.number = temp;
				countStruct.count = 1;
				countStruct.address = tempItem.address;
				countStruct.name = tempItem.name;
				//��ӵ�������
				CountVector.push_back(countStruct);
			}
		}
		temp--;
	}
	
}
//�ж��Ƿ�Ϊ���������֣�����ǿյĻ��������֣��򷵻�false�������Ǳ�������,����true
bool isVarName(string name) {
	if (name == "") {
		return false;
	}
	if (name.length() > 0 && isNumber(name)) {
		return false;
	}
	//��ʱ����������
	if (name.length() > 0 && name[0] == '$') {
		return false;
	}
	return true;
}
//����$s0-$s7�Ĵ����е���ֵ
void saveSRegisters(string functionName) {
	//����ǰ��������Ҫʹ�üĴ�����ֱ�ӷ���
	for (int i = 0; i < FunctionRegMap[functionName].size() ; i++)
	{
		int address = FunctionRegMap[functionName][i].address;
		int regNum = FunctionRegMap[functionName][i].regNum;
		//���Ĵ����е���ֵ�洢����Ӧ�ĵ�ַ�ռ���
		outMipsCode << "sw\t$s" << regNum << "\t" << address << "($fp)" << endl;  // sw $s0 address($fp)
	}
}
//�ָ�$s0-$s7�Ĵ����е���ֵ
void loadSRegisters(string functionName) {
	//����Ҫ���еĺ�������Ҫʹ�üĴ�����ֱ�ӷ���
	for (int i = 0; i < FunctionRegMap[functionName].size(); i++)
	{
		int address = FunctionRegMap[functionName][i].address;
		int regNum = FunctionRegMap[functionName][i].regNum;
		//���Ĵ����е���ֵ�洢����Ӧ�ĵ�ַ�ռ���
		outMipsCode << "lw\t$s" << regNum << "\t" << address << "($fp)" << endl;  // lw $s0 address($fp)
	}
}
//�ں����ļĴ�������map�в��ң������ڸñ�������ʹ�÷���ļĴ������ɣ�������lw����sw
bool VarUseRegister(string FuncName, string varName, string tempRegister, bool needSw) {
	vector<RegStruct> tempVec = FunctionRegMap[FuncName];
	for (vector<RegStruct>::iterator iter = tempVec.begin(); iter != tempVec.end(); iter++)
	{
		//�ܹ��üĴ����������ַ����
		if (iter->name == varName) {
			//ȡֵ����
			if (!needSw) {
				outMipsCode << "addi\t" << tempRegister << "\t$s" << iter->regNum << "\t0" << endl;  //addi $t0 $s0 0
			}
			//��ֵ����
			else {
				outMipsCode << "addi\t$s" << iter->regNum << "\t" << tempRegister << "\t0" << endl;  //addi $s0 $t0 0
			}
			return true;
		}
	}
	return false;
}
/*�Ӽ��˳������ת��Ϊmipsָ��
���� + - * /
���� �� + a, b, c
*/
void calculation2Mips(int type) {
	string op = (type == 0) ? "add" : type == 1 ? "sub" : type == 2 ? "mul" : type == 3 ? "div" : "";
	int addr_1, addr_2, addr_3;
	string part_1 = "", part_2 = "", part_3 = "";
	part_1 = MidCodeTable[midCodeTablePtr].part_1;
	part_2 = MidCodeTable[midCodeTablePtr].part_2;
	part_3 = MidCodeTable[midCodeTablePtr].part_3;
	//�ȷ���part_1�ǲ�������
	if (part_1.length() > 0 && isdigit(part_1[0]) || part_1[0] == '+' || part_1[0] == '-') {
		outMipsCode << "li\t$t0\t" << part_1 << endl;  //li $t0 number
	}
	else {
	//���ں����ļĴ�������map�в��ң������ڸñ�������ʹ�÷���ļĴ������ɣ�������lw����sw
		addr_1 = findVarInVariableTable(part_1);  //�õ��ñ�����Ӧ����Ե�ַ
		if (isGlobal) {
			//�����ȫ�ֱ�����ʼλ�õ�ƫ��
			outMipsCode << "lw\t$t0\t" << addr_1 << "($t9)" << endl;  //lw $t0 addr_1($t9) 
		}
		else {
			//���ں����ļĴ�������map�в��ң������ڸñ�������ʹ�÷���ļĴ������ɣ�������lw����sw
			//���û���õ��������lw ����sw����
			if (useCountRegOpt && VarUseRegister(NowFuncName, part_1, "$t0")) {
				;
			}
			else {
				//����ں���fp��ƫ��
				outMipsCode << "lw\t$t0\t" << addr_1 << "($fp)" << endl;  //lw $t0 addr_1($fp) 
			}
		}
	}
	//�ȷ���part_2�ǲ�������
	if (part_2.length() > 0 && isdigit(part_2[0]) || part_2[0] == '+' || part_2[0] == '-') {
		outMipsCode << "li\t$t1\t" << part_2 << endl;  //li $t0 number
	}
	else {
		addr_2 = findVarInVariableTable(part_2);  //�õ��ñ�����Ӧ����Ե�ַ
		if (isGlobal) {
			//�����ȫ�ֱ�����ʼλ�õ�ƫ��
			outMipsCode << "lw\t$t1\t" << addr_2 << "($t9)" << endl;  //lw $t0 addr_2($t9) 
		}
		else {
			if (useCountRegOpt && VarUseRegister(NowFuncName, part_2, "$t1")) {
				;
			}
			else {
				//����ں���fp��ƫ��
				outMipsCode << "lw\t$t1\t" << addr_2 << "($fp)" << endl;  //lw $t0 addr_2($fp)
			}
		}
	}
	addr_3 = findVarInVariableTable(part_3);  //�õ��ñ�����Ӧ����Ե�ַ
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

/*�Ƚ������ת��Ϊmipsָ��
���� < <= > >= == !=
���� �� lss a, b,
*/
void compare2Mips(int type) {
	int addr_1, addr_2;
	string part_1 = "", part_2 = "";
	string label_1 = "", label_2 = "";
	part_1 = MidCodeTable[midCodeTablePtr].part_1;
	part_2 = MidCodeTable[midCodeTablePtr].part_2;
	//�ȷ���part_1�ǲ�������
	if (part_1.length() > 0 && isdigit(part_1[0]) || part_1[0] == '+' || part_1[0] == '-') {
		outMipsCode << "li\t$t0\t" << part_1 << endl;  //li $t0 number
	}
	else {
		addr_1 = findVarInVariableTable(part_1);  //�õ��ñ�����Ӧ����Ե�ַ
		if (isGlobal) {
			//�����ȫ�ֱ�����ʼλ�õ�ƫ��
			outMipsCode << "lw\t$t0\t" << addr_1 << "($t9)" << endl;  //lw $t0 addr_1($t9) 
		}
		else {
			if (useCountRegOpt && VarUseRegister(NowFuncName, part_1, "$t0")) {
				;
			}
			else {
				//����ں���fp��ƫ��
				outMipsCode << "lw\t$t0\t" << addr_1 << "($fp)" << endl;  //lw $t0 addr_1($fp) 
			}
		}
	}
	//�ȷ���part_2�ǲ�������
	if (part_2.length() > 0 && isdigit(part_2[0]) || part_2[0] == '+' || part_2[0] == '-') {
		outMipsCode << "li\t$t1\t" << part_2 << endl;  //li $t0 number
	}
	else {
		addr_2 = findVarInVariableTable(part_2);  //�õ��ñ�����Ӧ����Ե�ַ
		if (isGlobal) {
			//�����ȫ�ֱ�����ʼλ�õ�ƫ��
			outMipsCode << "lw\t$t1\t" << addr_2 << "($t9)" << endl;  //lw $t0 addr_2($t9) 
		}
		else {
			if (useCountRegOpt && VarUseRegister(NowFuncName, part_2, "$t1")) {
				;
			}
			else {
				//����ں���fp��ƫ��
				outMipsCode << "lw\t$t1\t" << addr_2 << "($fp)" << endl;  //lw $t0 addr_2($fp)
			}
			 
		}
	}
	switch (type)
	{  //���ձ�֤ $t0�洢�ȽϽ������Ϊ0��������������Ϊ1 ��������
	case 0:  // < a < b
		outMipsCode << "slt\t$t0\t$t0\t$t1" << endl;  //slt $t0 $t0 $t1 
		break;
	case 1:  // <=  if(a <= b)  ת���� a > b �� �� ��1 - ����� ������ $t0�洢�ȽϽ��
		outMipsCode << "slt\t$t0\t$t1\t$t0" << endl;  //slt $t0 $t1 $t0
		outMipsCode << "li\t$t1\t1" << endl;  //li $t1 1 
		outMipsCode << "sub\t$t0\t$t1\t$t0" << endl;  //sub $t0 $t1 $t0
		break;
	case 2:  // > a > b
		outMipsCode << "slt\t$t0\t$t1\t$t0" << endl;  //slt $t0 $t1 $t0 
		break;
	case 3:  // >=  if(a >= b)  ת���� a < b �� �� ��1 - ����� ������ $t0�洢�ȽϽ��
		outMipsCode << "slt\t$t0\t$t0\t$t1" << endl;  //slt $t0 $t0 $t1
		outMipsCode << "li\t$t1\t1" << endl;  //li $t1 1 
		outMipsCode << "sub\t$t0\t$t1\t$t0" << endl;  //sub $t0 $t1 $t0
		break;
	case 4:  // !=  if(a != b)  $t0 = 1 else  $t0 = 0
		label_1 = genCompareLabel();
		label_2 = genCompareLabel();
		outMipsCode << "beq\t$t0\t$t1\t" << label_1 << endl;  //���������תlabel_1����ֵ$t0 = 0
		outMipsCode << "li\t$t0\t1" << endl;  //��ֵ$t0 = 1
		outMipsCode << "j\t" << label_2 << endl;//��ת��ǩlabel_2
		outMipsCode << label_1 << ":" << endl;  //���ɱ�ǩ label_1
		outMipsCode << "li\t$t0\t0" << endl;  //��ֵ$t0 = 0
		outMipsCode << label_2 << ":" << endl;  //���ɱ�ǩ label_2
		break;
	case 5:  // ==  if(a == b)  $t0 = 1 else  $t0 = 0
		label_1 = genCompareLabel();
		label_2 = genCompareLabel();
		outMipsCode << "bne\t$t0\t$t1\t" << label_1 << endl;  //����������תlabel_1����ֵ$t0 = 0
		outMipsCode << "li\t$t0\t1" << endl;  //��ֵ$t0 = 1
		outMipsCode << "j\t" << label_2 << endl;//��ת��ǩlabel_2
		outMipsCode << label_1 << ":" << endl;  //���ɱ�ǩ label_1
		outMipsCode << "li\t$t0\t0" << endl;  //��ֵ$t0 = 0
		outMipsCode << label_2 << ":" << endl;  //���ɱ�ǩ label_2
		break;
	default:
		break;
	}
	midCodeTablePtr++;
}
//Ϊִ�е���ת���������ת��ǩ������
string genCompareLabel() {
	stringstream ss;
	ss << CompareLabel;
	CompareLabel++;
	return "CompareLabel_" + ss.str();
}

/*����������Ԫʽת��Ϊmipsָ��
���� �� para,  int/char,  ,��������
*/
void para2Mips() {
	//��Ҫ�����������������,��������ջ��ֻ������Ե�ַ��
	string part_1 = "";
	int kind;
	int paraNum = 0;
	//ͳ�Ʋ���������Ȼ��д���ַ���õ�ַ����øú��������ʵ�ε�ַ��Ӧ��
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
		insertIntoVariableTable(kind, PARA, 4 * (paraNum - i));//��Ϊ�ʼ��ʵ��λ��ջ����ײ�����Ӧ��4 * (paraNum - i)��������ƫ����
		midCodeTablePtr++;
	}
}

/*ʵ����Ԫʽת��Ϊmipsָ��
���� �� funcPa�� �� ��ʵ�δ洢�ļĴ���(����Ϊ����)
*/
void funcPa2Mips() {
	int addr;
	string part_3 = "";
	part_3 = MidCodeTable[midCodeTablePtr].part_3;
	//�ж��Ƿ�Ϊ����
	if (part_3.length() > 0 && isdigit(part_3[0]) || part_3[0] == '+' || part_3[0] == '-') {
		outMipsCode << "li\t$t0\t" << part_3 << endl;  //li $t0 number
	}
	else {
		addr = findVarInVariableTable(part_3);
		if (isGlobal) {
			//�����ȫ�ֱ�����ʼλ�õ�ƫ��
			outMipsCode << "lw\t$t0\t" << addr << "($t9)" << endl;  //lw $t0 addr($t9) 
		}
		else {
			if (useCountRegOpt && VarUseRegister(NowFuncName, part_3, "$t0")) {
				;
			}
			else {
				//����ں���fp��ƫ��
				outMipsCode << "lw\t$t0\t" << addr << "($fp)" << endl;  //lw $t0 addr($fp)
			}
			 
		}
	}
	//��ʵ��ѹջ
	outMipsCode << "sw\t$t0\t0($sp)" << endl;  //sw $t0 0($sp) 
	outMipsCode << "subi\t$sp\t$sp\t4" << endl;  //subi $sp $sp 4
	sp -= 4;
	midCodeTablePtr++;

}

/*��ȡ����Ԫ��ת��Ϊmipsָ��
���� �� getArr, �������֣� �����±꣬ ����洢�ļĴ���
*/
void getArr2Mips() {
	int addr_1, addr_2, addr_3;
	string part_1 = "", part_2 = "", part_3 = "";
	part_1 = MidCodeTable[midCodeTablePtr].part_1;
	part_2 = MidCodeTable[midCodeTablePtr].part_2;
	part_3 = MidCodeTable[midCodeTablePtr].part_3;
	//�õ�part_1�ĵ�ַ 
	//����ĵ�ַ�洢���λ�����漰�������ȡ��Ҫ���±���ֵȡ����������
	addr_1 = findVarInVariableTable(part_1);
	//��ʱ�洢�������Ƿ�Ϊȫ�ֱ���
	bool part_1_isGlobal = isGlobal;
	//�ж������±��Ƿ�Ϊ�������õ�ƫ��������$t0��
	if (part_2.length() > 0 && isdigit(part_2[0]) || part_2[0] == '+') {
		outMipsCode << "li\t$t0\t" << part_2 << endl;  //li $t0 number
	}
	else {
		addr_2 = findVarInVariableTable(part_2);  //�õ��ñ�����Ӧ����Ե�ַ
		if (isGlobal) {
			//�����ȫ�ֱ�����ʼλ�õ�ƫ��
			outMipsCode << "lw\t$t0\t" << addr_2 << "($t9)" << endl;  //lw $t0 addr_2($t9) 
		}
		else {
			if (useCountRegOpt && VarUseRegister(NowFuncName, part_2, "$t0")) {
				;
			}
			else {
				//����ں���fp��ƫ��
				outMipsCode << "lw\t$t0\t" << addr_2 << "($fp)" << endl;  //lw $t0 addr_2($fp) 
			}
		}
	}
	outMipsCode << "mul\t$t0\t$t0\t4" << endl;  //mul $t0 $t0 4 
												//����ĵ�ַ�洢���λ�����漰�������ȡ��Ҫ���±���ֵȡ����������
	outMipsCode << "sub\t$t0\t$0\t$t0" << endl;	 //sub $t0 $0 $t0 // $t0 = 0 - $t0
	outMipsCode << "addi\t$t0\t$t0\t" << addr_1 << endl;	 //addi $t0 $t0 addr_1 // $t0 = $t0 + addr_1����ʱ$t0�Ѿ�����Ե�ַ�ˣ�������ֵ����0����
															 //��ʱ$t0�洢����ƫ�������ֽڣ�
															 //�Ȱ�������Ԫ��lwȡ��������ȥsw��ָ���Ĵ�����
															 //�õ�Ҫ��ֵ�������λ��
	if (part_1_isGlobal) {
		outMipsCode << "add\t$t0\t$t0\t$t9" << endl;	 //add $t0 $t0 $t9 // $t0 = $t0 + $t9
	}
	else {
		outMipsCode << "add\t$t0\t$t0\t$fp" << endl;	 //add $t0 $t0 $fp // $t0 = $t0 + $fp
	}
	//�Ȱ�������Ԫ��lwȡ������$t1
	outMipsCode << "lw\t$t1\t0($t0)" << endl;  //lw $t1 0($t0)
											   //��ȥsw��ָ���Ĵ�����
											   //�õ�����洢�ļĴ�����ַ
	addr_3 = findVarInVariableTable(part_3);
	if (isGlobal) {
		//�����ȫ�ֱ�����ʼλ�õ�ƫ��
		outMipsCode << "sw\t$t1\t" << addr_3 << "($t9)" << endl;  //sw $t1 addr_3($t9)
	}
	else {
		if (useCountRegOpt && VarUseRegister(NowFuncName, part_3, "$t1", true)) {
			;
		}
		else {
			//����ں���fp��ƫ��			
			outMipsCode << "sw\t$t1\t" << addr_3 << "($fp)" << endl;  //sw $t1 addr_3($fp)
		}
	}
	midCodeTablePtr++;

}

/*������ֵת��Ϊmipsָ��
���� �� =�� Ҫ������ֵ�� �� ������ʶ������
*/
void Assign2Mips() {
	int addr_1, addr_3;
	string part_1 = "", part_3 = "";
	part_1 = MidCodeTable[midCodeTablePtr].part_1;
	part_3 = MidCodeTable[midCodeTablePtr].part_3;
	//�ȷ���part_1�ǲ�������
	if (part_1.length() > 0 && isdigit(part_1[0]) || part_1[0] == '+' || part_1[0] == '-') {
		outMipsCode << "li\t$t0\t" << part_1 << endl;  //li $t0 number
	}
	//д��������
	//////////
	else {
		addr_1 = findVarInVariableTable(part_1);  //�õ��ñ�����Ӧ����Ե�ַ
		if (isGlobal) {
			//�����ȫ�ֱ�����ʼλ�õ�ƫ��
			outMipsCode << "lw\t$t0\t" << addr_1 << "($t9)" << endl;  //lw $t0 addr_1($t9) 
		}
		else {
			if (useCountRegOpt && VarUseRegister(NowFuncName, part_1, "$t0")) {
				;
			}
			else {
				//����ں�����ƫ��
				outMipsCode << "lw\t$t0\t" << addr_1 << "($fp)" << endl;  //lw $t0 addr_1($fp) 
			}
		}
	}
	addr_3 = findVarInVariableTable(part_3);  //�õ��ñ�����Ӧ����Ե�ַ
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

/*���鸳ֵת��Ϊmipsָ��
���� �� []=, �������֣� �����±꣬ Ҫ�洢����ֵ
*/
void AssignArr2Mips() {
	int addr_1, addr_2, addr_3;
	string part_1 = "", part_2 = "", part_3 = "";
	part_1 = MidCodeTable[midCodeTablePtr].part_1;
	part_2 = MidCodeTable[midCodeTablePtr].part_2;
	part_3 = MidCodeTable[midCodeTablePtr].part_3;
	//�õ�part_1�ĵ�ַ
	addr_1 = findVarInVariableTable(part_1);
	bool part_1_isGlobal = isGlobal;
	//�ж������±��Ƿ�Ϊ�������õ�ƫ��������$t0��
	if (part_2.length() > 0 && isdigit(part_2[0]) || part_2[0] == '+') {
		outMipsCode << "li\t$t0\t" << part_2 << endl;  //li $t0 number
	}
	else {
		addr_2 = findVarInVariableTable(part_2);  //�õ��ñ�����Ӧ����Ե�ַ
		if (isGlobal) {
			//�����ȫ�ֱ�����ʼλ�õ�ƫ��
			outMipsCode << "lw\t$t0\t" << addr_2 << "($t9)" << endl;  //lw $t0 addr_2($t9) 
		}
		else {
			if (useCountRegOpt && VarUseRegister(NowFuncName, part_2, "$t0")) {
				;
			}
			else {
				//����ں���fp��ƫ��
				outMipsCode << "lw\t$t0\t" << addr_2 << "($fp)" << endl;  //lw $t0 addr_2($fp) 
			}
		}
	}
	outMipsCode << "mul\t$t0\t$t0\t4" << endl;  //mul $t0 $t0 4 
												//����ĵ�ַ�洢���λ�����漰�������ȡ��Ҫ���±���ֵȡ����������
	outMipsCode << "sub\t$t0\t$0\t$t0" << endl;	 //sub $t0 $0 $t0 // $t0 = 0 - $t0
	outMipsCode << "addi\t$t0\t$t0\t" << addr_1 << endl;	 //addi $t0 $t0 addr_1 // $t0 = $t0 + addr_1
	 //��ʱ$t0�洢����ƫ�������ֽڣ�
	//�õ�Ҫ��ֵ�������λ��,(���swƫ����Ӧ��Ϊ0)
	if (part_1_isGlobal) {
		outMipsCode << "add\t$t0\t$t0\t$t9" << endl;	 //add $t0 $t0 $t9 // $t0 = $t0 + $t9
	}
	else {
		outMipsCode << "add\t$t0\t$t0\t$fp" << endl;	 //add $t0 $t0 $fp // $t0 = $t0 + $fp
	}
	//�ж�Ҫ�洢�������Ƿ�Ϊ����
	if (part_3.length() > 0 && isdigit(part_3[0]) || part_3[0] == '+' || part_3[0] == '-') {
		outMipsCode << "li\t$t1\t" << part_3 << endl;  //li $t1 number
	}
	else {
		//�õ�Ҫ�洢����ֵ�ĵ�ַ
		addr_3 = findVarInVariableTable(part_3);
		if (isGlobal) {
			//�����ȫ�ֱ�����ʼλ�õ�ƫ��
			outMipsCode << "lw\t$t1\t" << addr_3 << "($t9)" << endl;  //lw $t1 addr_3($t9) 
		}
		else {
			if (useCountRegOpt && VarUseRegister(NowFuncName, part_3, "$t1")) {
				;
			}
			else {
				//����ں���fp��ƫ��
				outMipsCode << "lw\t$t1\t" << addr_3 << "($fp)" << endl;  //lw $t1 addr_3($fp) 
			}
		}
	}
	outMipsCode << "sw\t$t1\t0($t0)" << endl;  //sw $t1 0($t0)
	midCodeTablePtr++;
}

/*����� ת��Ϊmipsָ��
���� �� scf�� int/char�� �� ������ʶ��
*/
void scf2Mips() {
	int addr;
	string part_1 = "", part_3 = "";
	part_1 = MidCodeTable[midCodeTablePtr].part_1;
	part_3 = MidCodeTable[midCodeTablePtr].part_3;
	//�ҵ�������ַ
	addr = findVarInVariableTable(part_3);
	int kind = part_1 == "int" ? INT : CHAR;
	if (kind == INT) {
		outMipsCode << "li\t$v0\t5" << endl;  //li $v0 5 ��ȡ����
	}
	else {
		outMipsCode << "li\t$v0\t12" << endl;  //li $v0 12 ��ȡ�ַ�
	}
	outMipsCode << "syscall" << endl;  //syscall   //����洢��$v0��
	if (isGlobal) {
		outMipsCode << "sw\t$v0\t" << addr << "($t9)" << endl;  //sw $v0 addr($t9) �洢
	}
	else {
		if (useCountRegOpt && VarUseRegister(NowFuncName, part_3, "$v0", true)) {
			;
		}
		else {
			outMipsCode << "sw\t$v0\t" << addr << "($fp)" << endl;  //sw $v0 addr($fp) �洢
		}
	}
	midCodeTablePtr++;
}

/*д��� ת��Ϊmipsָ��
���� �� ptf���ַ���(����Ϊ��)�� ����Ĵ���(����Ϊ��)��char/int/����Ϊ��
*/
void ptf2Mips() {
	int addr;
	string part_1 = "", part_2 = "", part_3 = "";
	part_1 = MidCodeTable[midCodeTablePtr].part_1;
	part_2 = MidCodeTable[midCodeTablePtr].part_2;
	part_3 = MidCodeTable[midCodeTablePtr].part_3;
	//�ȿ��ַ����Ƿ�Ϊ��
	if (part_1 != "") {
		//��Ϊ������д�ӡ  (����ϵͳ����)
		for (int i = 0; i < part_1.length(); i++)
		{
			outMipsCode << "li\t$v0\t11" << endl;  //li $v0 11
			outMipsCode << "li\t$a0\t" << int(part_1[i]) << endl;  //li $a0 �ַ�ת��Ϊ����
			outMipsCode << "syscall" << endl;  //syscall
		}
	}
	//�����Ĵ���(��Ϊ�յ�����½��з���)
	if (part_2 != "") {
		if (isdigit(part_2[0]) || part_2[0] == '+' || part_2[0] == '-') {
			//�ַ��Ĵ�ӡ
			if (part_3 == "char") {
				outMipsCode << "li\t$v0\t11" << endl;  //li $v0 11
				outMipsCode << "li\t$a0\t" << part_2 << endl;  //li $a0 part_2(part_2Ϊ����)
				outMipsCode << "syscall" << endl;  //syscall
			}
			//���ֵĴ�ӡ
			else if (part_3 == "int") {
				outMipsCode << "li\t$v0\t1" << endl;  //li $v0 1
				outMipsCode << "li\t$a0\t" << part_2 << endl;  //li $a0 part_2(part_2Ϊ����)
				outMipsCode << "syscall" << endl;  //syscall
			}
		}
		//�Ĵ������߱����Ĵ�ӡ
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
	//��ӡ���з�
	//outMipsCode << "li\t$a0\t\'\\n\'" << endl;  //li $a0 '\n'
	//outMipsCode << "li\t$v0\t11" << endl;  //li $v0 11
	//outMipsCode << "syscall" << endl;  //syscall
	midCodeTablePtr++;
}

/*������� ת��Ϊmipsָ��
���� �� ret�� �� ���������ļĴ�����Ҳ����Ϊ������������Ϊ�գ�
*/
void ret2Mips() {
	int addr;
	string  part_3 = "";
	part_3 = MidCodeTable[midCodeTablePtr].part_3;
	//��Ϊ������з���
	if (part_3 != "") {
		//��part_3Ϊ����
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

/*���ú�����Ԫʽת��Ϊmipsָ��
���� �� call, �������֣�	���Ĵ�������������Ϊ�գ�
*/
void call2Mips() {  //���ú���
	//xieddasdaskdhaskdjasjdhsajdhsajk
	int addr;
	string preFuncName = NowFuncName;
	
	//�������Ż�
	if (useCountRegOpt) {
		//����Sϵ�мĴ����е���ֵ����ַ�ռ���
		saveSRegisters(preFuncName);
	}
	string  part_1 = "", part_3 = "";
	part_1 = MidCodeTable[midCodeTablePtr].part_1;
	part_3 = MidCodeTable[midCodeTablePtr].part_3;
	outMipsCode << "jal\t" << part_1 << endl;  //jal ��������
	outMipsCode << "nop" << endl;  //����nopָ��
	
	//�������Ż�
	if (useCountRegOpt) {
		//����֮ǰ�������֣�����ر���lw����
		loadSRegisters(preFuncName);
	}
	//��������ֵ�洢��$v0��,����洢���Ĵ�������
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

/*��������ת ת��Ϊmipsָ��
���� �� jump�� �� ��label
*/
void jump2Mips() {
	string  part_3 = MidCodeTable[midCodeTablePtr].part_3;
	outMipsCode << "j\t" << part_3 << endl;  //j part_3
	midCodeTablePtr++;
}

/*������������ת ת��Ϊmipsָ��
���� �� jne�� �� ��label
*/
void jne2Mips() {  //�ж�$t0 ����ֵ��Ϊ1 ����ת����Ϊ1 ��ת
	string  part_3 = MidCodeTable[midCodeTablePtr].part_3;
	outMipsCode << "bne\t$t0\t1\t" << part_3 << endl;  //bne $t0, 1, label
	midCodeTablePtr++;
}

/*���ɱ�ǩ ת��Ϊmipsָ��
���� �� genLabel�� �� ��label
*/
void genLabel2Mips() {
	string  part_3 = MidCodeTable[midCodeTablePtr].part_3;
	outMipsCode << part_3 << ":" << endl;  //label:
	midCodeTablePtr++;
}

