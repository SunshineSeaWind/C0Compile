//
//  syntax.cpp
//  C0compiler
//

#include "globalData.h"
int funcTotalNum = 0; //������ű��зֳ����������ȫ�ֱ���
int symbolTableTop = 0; //��ǰ���ű��ջ��λ��(��Ҫ��ű����λ��)
signTable symbolTable[maxTableSize]; //���ű�
int displayTable[maxTableSize];		//�ֳ�����������
int mainNum = 0;//���շ���������mainNumӦΪ1

#define INT 1
#define CHAR 2
string name = "";   //��ʶ������
int  kind = -1;      //��ʶ�������
int  value = -1;     //��ֵ�洢����Ϊ���������з���ֵ�洢1���޷���ֵ�洢0
int arraySize = -1; //���������Ļ����洢�����size��С���������sizeΪ0��
int  address = -1;   //�����ĵ�ַ����������ֵ�ĵ�ַ
int  paraNum = -1;   //��Ϊ�����������������������Ϊ0��
int factorType = -1;  //�������ͣ��Լ���¼�����Ƿ��з���ֵ
string OperationResult = "";  //���ʽ���������Ĵ洢�Ĵ���������ֵ
int callNoReturnFunc = 0;	 //�������������з���ֵ�����޷���ֵ���� //���ú�����ʱ����
bool hasReturnValue = false;  //�����Ƿ��з�����ֵ����䣬�����ؿ��������Ϊfalse�� //������ű��ʱ����
//string switchResult = ""/*, default_label_final = ""*/;  //�洢switch������ʽ�ļ������Լ�default����֮�н�����ı�ǩ
//int SwitchVarType = -1;
string forVarName = "";  //�洢forѭ��������name
bool hasForVarName= false, needToJudge = false; //���������ı��ʽ�г�����forѭ�����������ݣ�������Ϊtrue;����Ҫ�ڱ��ʽ���жϣ�������needTojudgeΪtrue
stack<int> ParaType;

//���﷨��������������ĳɷ�������ļ���
void writeToFile(string writeString) {
	//cout << "This is a " << writeString << " !" << endl;
	outfile << "This is a " << writeString << " !" << endl;
}

//������ת��Ϊ�ַ���
string convertNumToString(int num) {
	stringstream numStream;
	numStream << num;
	return numStream.str();
}

//��ӡ���ű��ļ�
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
//����׺���ʽд���ļ�
void writeToInfixExpressionFile(string str) {
	OutInfixExpression << str << endl;
}
//������ű��У���Ҫ����Ƿ�����
int insertIntoSymTable(string name, int kind, int value, int arraySize,int address,int paraNum) {
	int index;
	//��Ҫ�жϷ��ű����Ƿ���ڸò�������ڱ���
	if (symbolTableTop >= maxTableSize) { //���ű��Ѿ�����
		error(TableFull);  //���ű��Ѿ����˵Ĵ���
		//exit(-1);  //ֱ�Ӵ����������˳�������Ϊ���������Ҫ��ֹ����
		return -1;
	}
	//���������������������ݲ�ͬ����Ϊ�����ж��Ƿ��Ѿ�������Ҫ�ڷֳ�����������
	//���������������ڵ�ǰ�ֳ�����ű��в�ѯ����
	if (kind == FUNCTION) {
		//�������������漰�����棺һ���Ǻ���������ȫ�ֱ�����ͬ������һ���Ǻ���������ĺ���������ͬ����
		//����������ȫ�ֱ�����ͬ����
		for (index = 0; index < displayTable[1] - 1; index++)
		{
			if (symbolTable[index].name == name) {
				error(SymbolNameConflict);  //����������ȫ�ֱ�����ͬ������,���ͻ
				return -1;
			}
		}
		//�ӵ�һ���ֳ���洢�ĺ�����������ʼ���ң�������ĺ���������ͬ����
		for (index = 1; index <= funcTotalNum; index++)
		{
			if (symbolTable[displayTable[index]].name == name) {
				error(SymbolNameConflict);  //�ظ����庯��������
				return -1;
			}
		}
		//δ�ض��壬����ű�
		//��0���ֳ���洢��ȫ�ֳ������������һ����ʶ���洢λ�õ�����,
		//funcTotalNum��ֵΪ0����Ҫ����++funcTotalNum
		displayTable[++funcTotalNum] = symbolTableTop;
	}
	//�������������ڵ�ǰ�ֳ�����ű��в�ѯ
	else {
		//���˺�����������ͣ���Ϊȫ�ֱ��������ڵ�0���ֳ���洢����Ϊ�ֲ������������뺯�����ظ�
		if (funcTotalNum == 0) {  //ȫ�ֱ���
			for (index = displayTable[0]; index < symbolTableTop; index++)
			{
				if (symbolTable[index].name == name) {
					error(SymbolNameConflict);  //�ظ��������ݣ�����
					return -1;
				}
			}
		}
		else {  //�ֲ������������뺯�����ظ�,��inde = displayTable[funcTotalNum]+1
			for (index = displayTable[funcTotalNum]+1; index < symbolTableTop; index++)
			{
				if (symbolTable[index].name == name) {
					error(SymbolNameConflict);  //�ظ��������ݣ�����
					return -1;
				}
			}
		}
	}
	symbolTable[symbolTableTop].name = name;
	symbolTable[symbolTableTop].kind = kind;
	symbolTable[symbolTableTop].value = value;
	symbolTable[symbolTableTop].arraySize = arraySize;
	symbolTable[symbolTableTop].address = symbolTableTop;  //address����Ϊ��ǰ���ű�ջ��ָ��
	symbolTable[symbolTableTop].paraNum = paraNum;
	symbolTableTop++;
	return 1;
}

//�ú�����ĳ���������ű����������е��ã����˺������ֺͺ����������ͣ���������ȫ��ɾ����
void flushTable() {  
	//ȫ�ֱ�����Ҫɾ��
	signTable sign;
	int count = 0, tempParaNum = symbolTable[displayTable[funcTotalNum]].paraNum;
	for (int  i = displayTable[funcTotalNum] + tempParaNum +1; i < symbolTableTop; i++)
	{
		sign = symbolTable[i]; //������е����ԣ���ʵҲ���Բ�������Ϊ���²�����ű��ʱ���������Զ��Ѿ���ʼ��
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

//�ڷ��ű��в��ұ�ʶ�����ߺ����Ƿ����
int findInSymbolTable(string name,int searchMode) {
	//searchModeΪ0������������ʶ�����֣��ڵ�ǰ�ֳ����Լ��ܵ�ȫ�ֱ���������������
	//searchModeΪ1�����������������֣��ڷֳ�����������������
	int index, paraType;
	if (searchMode == 1) { //��������������Ҫ�жϲ����ĸ���������
		vector<int> tempVector;
		for ( index = 1; index <= funcTotalNum; index++)
		{
			if (symbolTable[displayTable[index]].name == name) {
				//cout << "���ű��ҵ���������" << endl;
				break;
			}
		}
		if (index > funcTotalNum) { // δ�����ĺ���
			error(FunctionNotDefine);  //δ�����ĺ���������
			cout << "δ������������" << endl;
			return -1;
		}
		if (symbolTable[displayTable[index]].paraNum != paraNum) {  //��������������ƥ��
			error(FormalParaNumNotMatch); //����������ƥ�䣬����
			/*cout << "name" << name << endl;
			cout << symbolTable[displayTable[index]].paraNum << "   " << paraNum << endl;
			cout << "����������ƥ��" << endl;*/
			return -1;
		}
		//�������ʹ��뵽����tempVector��(��ʱΪ���򣬺����Ƚ���Ҫ������)
		for (int  i = 1; i <= symbolTable[displayTable[index]].paraNum; i++)//������Ҫ���
		{
			//cout << ParaType.size() << endl;
			//���������ʹ洢��value������
			//paraType = symbolTable[displayTable[index] + i].value == intsy ? INT : CHAR;
			tempVector.push_back(ParaType.top());
			ParaType.pop();
			//cout << "bbbbbbbbbbb" << ParaType.size() << endl;
			//if(paraType != ParaType[i-1]){
			//	error(ParaTypeNotMatch);  //�������Ͳ�����
			//	//cout << "gggggggggggggggggg" << endl;
			//	return -1;
			//}
		}
		//�������ͱȽ�
		int size = tempVector.size();
		for (int i = 1; i <= symbolTable[displayTable[index]].paraNum; i++)//������Ҫ���
		{
			paraType = symbolTable[displayTable[index] + i].value == intsy ? INT : CHAR;
			if(paraType != tempVector[size - i]){
				error(ParaTypeNotMatch);  //�������Ͳ�����
				//cout << "gggggggggggggggggg" << endl;
				return -1;
			}
		}
		if (symbolTable[displayTable[index]].value == 0) {  //�޷���ֵ��������
			callNoReturnFunc = 1;
		}
		return symbolTable[displayTable[index]].address;
	}
	else {  //��ʶ������
		//���ڵ�ǰ�ֳ�����ű�������
		for ( index = displayTable[funcTotalNum]; index < symbolTableTop; index++)
		{
			if (symbolTable[index].name == name) {
				break;
			}
		}
		if (index == symbolTableTop) {  //δ�ڵ�ǰ�ֳ�����ű����ҵ�����ȥȫ�ֱ���������
			for ( index = 0; index < displayTable[1]; index++)
			{
				if (symbolTable[index].name == name) {
					break;
				}
			}
			if (index == displayTable[1]) {  //δ��ȫ�ֱ������ҵ���������ʶ��δ����
				error(VariableNotDefine);  //������ʶ��δ����
				return -1;
			}
		}
		return symbolTable[index].address;
	}
}

//������    :: = �ۣ�����˵�����ݣۣ�����˵������{ ���з���ֵ�������壾 | ���޷���ֵ�������壾 }����������
void programme() {
	int tempToken, tempCharCounter;
	char tempCh;
	insymbol();
	//const��ͷ����Ϊ����˵��
	if (token == constsy) {
		constExplain();
	}
	//int ���� char��ͷ����Ҫ���Ԥ���������ж��Ǳ���˵�����Ǻ���
	while (token == intsy || token == charsy) {  //����˵������,����˵����Ҫѭ��ִ��
		//�����ʱ�����У�����֮��������������ָ�����
		tempToken = token;
		tempCh = ch;
		tempCharCounter = charCounter;
		insymbol();
		if (token == ident) {
			insymbol();
			if (token == comma || token == semicolon || token == lmidbrack) {  //�������������飩˵��������
				token = tempToken;
				ch = tempCh;
				charCounter = tempCharCounter;
				variableExplain();
			}
			else {  //���Ǳ���˵��������ָ�룬ѭ������
				token = tempToken;
				ch = tempCh;
				charCounter = tempCharCounter;
				break;
			}
		}
		else {
			error(LackIdent, 0);  //Ӧ��Ϊ��ʶ��������
			//return;
		}
	}
	if (token == constsy) {
		error(ConstDefAfterVarDef, 0); //����˵���ڱ���˵�����棬����
		//return;
	}
	//д���ļ���
	outSymbolTable << "���ű���ȫ�ֱ���:" << endl;
	printSymtableTable();
	while (token == intsy || token == charsy || token == voidsy) {
		RegisterCount = 0;
		//���������岿�֣��з���ֵ�������壬�޷���ֵ�������壬main����
		if (token == intsy || token == charsy) {  //�з���ֵ��������
			returnFuncDefine();
		}
		else {  //void���֣������޷���ֵ�������壬main���� ��ҪԤ��
			tempToken = token;
			tempCh = ch;
			tempCharCounter = charCounter;
			insymbol();
			if (token == ident) {		//�޷���ֵ��������
				token = tempToken;
				ch = tempCh;
				charCounter = tempCharCounter;
				noReturnFuncDefine();
			}
			else if (token == mainsy) {	//main���壬�ڴ�����������������֮���������򱨴���ֹ����
				token = tempToken;
				ch = tempCh;
				charCounter = tempCharCounter;
				MainDefine();
				//return; //���������������˳�programme
			}
			else {
				error(LackIdent, 0);  //Ӧ��Ϊ��ʶ��
				//return;
			}
		}
	}
	//writeToFile("programme");
}

//��������        :: = �ۣ������ݣ��޷�������������  //��֧��+ - 0
//������˵���� :: = const���������壾; { const���������壾; }
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
			if (token == semicolon) {	//����ֺ�,�����Ǳ���
				insymbol();
			}
			else {
				error(LackSemicolon, 0);  //Ӧ��Ϊ�ֺ�
				//return;
			}
		}
		else {
			error(NotIntOrChar, 0); //�Ȳ���int �ֲ���char�����ͱ�ʶ������
		}
	}
	writeToFile("constExplain");
}

//���������壾   :: = int����ʶ��������������{ ,����ʶ�������������� } | char����ʶ���������ַ���{ ,����ʶ���������ַ��� }
void constDefine(int type) { //typeҪôΪint��ҪôΪchar
	int Sign = -1;
	if (token == ident) {
		name = id; //��¼��ʶ������
		insymbol();
		if (token == becomes) { //�ж��Ƿ�Ϊ�Ⱥ�
			insymbol();
			//�з�������
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
					error(DataTypeNotMatch); //���Ͳ�ƥ�����token��Ϊintconst
					return;
				}
			}
			//�޷�������
			else if (token == intconst && type == intsy) {
				value = inum;
				//������ű�
				if (insertIntoSymTable(name, kind, value, arraySize, address, paraNum) > 0){
					insertMiddleCode("const", "int", convertNumToString(value), name);
					writeToInfixExpressionFile("const int " + name + " = " + convertNumToString(value));
				}
				insymbol();
			}
			//�ַ�
			else if (token == charconst && type == charsy) {
				value = inum;
				//������ű�
				if (insertIntoSymTable(name, kind, value, arraySize, address, paraNum) > 0) {
					insertMiddleCode("const", "char", convertNumToString(value), name); //char���ͣ�value�洢��ASCII��
					writeToInfixExpressionFile("const char " + name + " = " + convertNumToString(value));
				}
				insymbol();
			}
			else {
				error(DataTypeNotMatch); //���Ͳ�ƥ��
				return;
			}
		}
		else {
			error(LackBecome, 1); //Ӧ��Ϊ =
			return;
		}
	}
	else {
		error(LackIdent, 1);  //Ӧ��Ϊ��ʶ��,���������ʡ�ԣ��ڵ��øú���֮ǰû���ж�token
		return;
	}
}

//������ͷ����   :: = int����ʶ���� | char����ʶ����
void declareHead() {
	if (token == intsy || token == charsy) {
		insymbol();
		if (token == ident) {
			name = id;
			insymbol();
		}
		else {
			error(LackIdent, 3); //ӦΪ��ʶ��
			return;
		}
	}
	else {
		error(NotIntOrChar, 1); //ӦΪint ����char
		return;
	}
}

//���з���ֵ�������壾  :: = ������ͷ������(������������)�� �� { ����������䣾�� }��
void returnFuncDefine() {
	string funcReturnType, funcName;
	if (token == intsy || token == charsy) {
		value = token;	//�з���ֵ����,�洢valueΪ����ֵ����
		funcReturnType = token == intsy ? "int" : "char";
		declareHead();//����ͷ��,����name
		funcName = name;
		if (token == lsmallbrack) {
			insymbol();
		}
		else {
			error(LackLSmallBrack, 0);  //Ӧ��Ϊ��С����
			return;
		}
		kind = FUNCTION;	//��������
		arraySize = 0;
		paraNum = 0;
		address = 0;
		if(insertIntoSymTable(funcName, kind, value, arraySize, address, paraNum) > 0) {   //�����ڶ��������ź󼴼�����ű���
			insertMiddleCode("func", funcReturnType, "", funcName);
			writeToInfixExpressionFile(funcReturnType + " " + funcName + "()");
		}
		parameterTable();  //����д����ű������paraNum
		if (token == rsmallbrack) {
			insymbol();
		}
		else {
			error(LackRSmallBrack, 0);   //Ӧ��Ϊ��С����
			return;
		}
		if (token == lbigbrack) {
			insymbol();
		}
		else {
			error(LackLBigBrack, 0);   //Ӧ��Ϊ�������
			return;
		}
		complexStatement();
		if (token == rbigbrack) {
			insymbol();
		}
		else {
			error(LackRBigBrack, 0);   //Ӧ��Ϊ�Ҵ�����
			return;
		}
		if (hasReturnValue) {
			hasReturnValue = false;
		}
		else {
			error(LackReturnValue);  //�з���ֵ����û�з���ֵ���
			return;
		}
		insertMiddleCode("end", "", "", funcName); //������������
		outSymbolTable << "���ű�ɾ������ĳЩ����֮ǰ��" << endl;
		printSymtableTable();
		flushTable();  //ɾ�����ű��к�����������
		outSymbolTable << "���ű�ɾ������ĳЩ����֮��" << endl;
		printSymtableTable();
	}
	writeToFile("returnFuncDefine");
}

//��������    :: = �����ͱ�ʶ��������ʶ����{ ,�����ͱ�ʶ��������ʶ���� } | ���գ�
void parameterTable() {  //�����������󽫲�����������������ű����������
	int tempParaNum = 0;
	string stringParaType, tempName;
	while (token == intsy || token == charsy) {  //�������������㣬�������Ϊ�յ������ֱ�ӽ�����������
		arraySize = 0;
		stringParaType = token == intsy ? "int" : "char"; //��¼�������ͣ�������Ԫʽʹ��
		value = token;  //value�洢��������
		address = 0;
		paraNum = 0;
		insymbol();
		if (token == ident) {
			name = id;
			tempName = id;
			kind = PARAMETER;
			//������ű�
			if (insertIntoSymTable(name, kind, value, arraySize, address, paraNum) > 0) {
				insertMiddleCode("para", stringParaType, "", tempName);
				writeToInfixExpressionFile("para " + stringParaType + " " + tempName);
			}
			insymbol();
		}
		else {
			error(LackIdent, 2); //Ӧ��Ϊ��ʶ��
			return;
		}
		tempParaNum++;
		if (token == comma) {
			insymbol();
			continue;
		}
	}
	paraNum = tempParaNum;
	symbolTable[displayTable[funcTotalNum]].paraNum = paraNum; //���뺯���Ĳ�������
}

//���޷���ֵ�������壾  :: = void����ʶ������(������������)���� { ����������䣾�� }��
void noReturnFuncDefine() {
	string funcReturnType, funcName;
	if (token == voidsy) {
		value = token;	//�޷���ֵ����
		funcReturnType = "void";
		insymbol();
		name = id;	//ǰ���Ѿ��жϹ��ˣ������Ǳ�ʶ������main����ǰ���Ѿ������˴���Ϊ�Ϸ����
		funcName = name;
		insymbol();
		if (token == lsmallbrack) {
			insymbol();
		}
		else {
			error(LackLSmallBrack, 0);  //Ӧ��Ϊ��С����
			return;
		}
		kind = FUNCTION;	//��������
		arraySize = 0;
		paraNum = 0;
		address = 0;
		if (insertIntoSymTable(funcName, kind, value, arraySize, address, paraNum) > 0) { //�����ڶ��������ź󼴼�����ű���
			insertMiddleCode("func", funcReturnType, "", funcName);
			writeToInfixExpressionFile(funcReturnType + " " + funcName + "()");

		}
		parameterTable();  //����д����ű������paraNum
		if (token == rsmallbrack) {
			insymbol();
		}
		else {
			error(LackRSmallBrack, 0);   //Ӧ��Ϊ��С����
			return;
		}
		if (token == lbigbrack) {
			insymbol();
		}
		else {
			error(LackLBigBrack, 0);   //Ӧ��Ϊ�������
			return;
		}
		complexStatement();
		if (token == rbigbrack) {
			insymbol();
		}
		else {
			error(LackRBigBrack, 0);   //Ӧ��Ϊ�Ҵ�����
			return;
		}
		if (hasReturnValue) {  //�޷���ֵ�����з���ֵ���
			hasReturnValue = false;
			error(NoReturnFuncWithReturn);   //�޷���ֵ�����з���ֵ��䣬����
			return;
		}
		insertMiddleCode("end", "", "", funcName);
		outSymbolTable << "���ű�ɾ������ĳЩ����֮ǰ��" << endl;
		printSymtableTable();
		flushTable();  //ɾ�����ű��к�����������
		outSymbolTable << "���ű�ɾ������ĳЩ����֮��" << endl;
		printSymtableTable();
	}
	writeToFile("noReturnFuncDefine");
}

//����������    :: = void main��(����)�� �� { ����������䣾�� }��
void MainDefine() {
	string funcReturnType, funcName;
	if (token == voidsy) {
		value = token;	//�޷���ֵ����
		funcReturnType = "void";
		insymbol();
		name = id;	//ǰ���Ѿ��жϹ��ˣ������Ǳ�ʶ������main����ǰ���Ѿ������˴���Ϊ�Ϸ����,�˴�name Ϊ main
		funcName = name;
		insymbol();
		if (token == lsmallbrack) {
			insymbol();
		}
		else {
			error(LackLSmallBrack, 0);  //Ӧ��Ϊ��С����
			return;
		}
		kind = FUNCTION;	//��������
		arraySize = 0;
		paraNum = 0;
		address = 0;
		if (insertIntoSymTable(funcName, kind, value, arraySize, address, paraNum) > 0) { //�������ڶ��������ź󼴼�����ű���
			insertMiddleCode("func", funcReturnType, "", funcName);
			writeToInfixExpressionFile(funcReturnType + " " + funcName + "()");
		}
		if (token == rsmallbrack) {
			insymbol();
		}
		else {
			error(LackRSmallBrack, 0);   //Ӧ��Ϊ��С����
			return;
		}
		if (token == lbigbrack) {
			insymbol();
		}
		else {
			error(LackLBigBrack, 0);   //Ӧ��Ϊ�������
			return;
		}
		complexStatement();
		if (token == rbigbrack) {
			insymbol();
		}
		else {
			error(LackRBigBrack, 0);   //Ӧ��Ϊ�Ҵ�����
			return;
		}
		if (hasReturnValue) {  //main�޷���ֵ�����з���ֵ���
			hasReturnValue = false;
			error(NoReturnFuncWithReturn);   //�޷���ֵ����main �з���ֵ��䣬����
			return;
		}
		mainNum++;
		insertMiddleCode("end", "", "", funcName);
	}
	outSymbolTable << "���ű������������ݣ�" << endl;
	printSymtableTable();
	writeToFile("main");
}

//������˵����  :: = ���������壾; {���������壾; } ���ڱ���˵�����з���ֵ������ǰ������ͬ�������ڵ���
//����˵�������з���ֵ��������֮ǰ�����жϣ��ʱ���˵��ÿ��ִֻ�е�һ���ֺż��������´��Ƿ����Ҫ����֮���ж��Ƿ�
//Ϊ�����������˴�variableExplain���ɵݹ飬Ҳ��������ѭ����(ֻҪ��int����char�Ϳ�ʼvariableExplain�Ǵ����)
void variableExplain() {
	if (token == intsy || token == charsy) {
		arraySize = 0; //ȫ����ʼ��
		value = 0;
		address = 0;
		paraNum = 0;
		kind = token; //��ΪkindΪȫ�ֱ����������贫��variableDefine
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
			error(LackSemicolon, 1);  //Ӧ��Ϊ�ֺ�
		}
	}
	writeToFile("variableExplain");
}

//���������壾  :: = �����ͱ�ʶ����(����ʶ���� | ����ʶ������[�����޷�����������]��) { , ����ʶ���� | ����ʶ������[�����޷�����������]�� }
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
		if (token == lmidbrack) { //����
			insymbol();
			if (token == intconst) {
				if (inum != 0) {
					arraySize = inum; // �����С
					insymbol();
				}
				else {
					error(ArraySizeIsZero);  //�����СΪ0������
					//insymbol();
					return;
				}
				if (token == rmidbrack) {
					//������ű������ڶ����������ź�Ż����!
					if (insertIntoSymTable(name, kind, value, arraySize, address, paraNum) > 0) {
						insertMiddleCode(arrType, "0", convertNumToString(arraySize), name);
						writeToInfixExpressionFile("var " + dataType + +" " + name + "[" + convertNumToString(arraySize) + "]");
					}
					insymbol();
				}
				else {
					error(LackRMidBrack, 0);  //ӦΪ��������
					return;
				}
			}
			else {
				error(ArrayIndexNotUnsignedNum);  //�����С��Ϊ�޷�������������
				return;
			}
		}
		else { //Ϊ�������Ҳ�������
			//������ű�
			if(insertIntoSymTable(name, kind, value, arraySize, address, paraNum) > 0){
				insertMiddleCode(dataType, "", "", name);
				writeToInfixExpressionFile("var " + dataType + " " + name);
			}
			token = tempToken;  //��ȡ����һ�����ţ�ָ�����
			ch = tempCh;
			charCounter = tempCharCounter;
			insymbol();
		}
	}
	else {
		error(LackIdent, 1);  //Ӧ��Ϊ��ʶ��,���������ʡ�ԣ��ڵ��øú���֮ǰû���ж�token
		return;
	}
}

//��������䣾   :: = �ۣ�����˵�����ݣۣ�����˵�����ݣ�����У�
void complexStatement() {
	char tempCh; //��Ҫ�������������Ų����жϳ������Ǳ������������з���ֵ��������
	int tempToken, tempCharCounter;
	if (token == constsy) {  //����˵��
		constExplain();
	}
	while (token == intsy || token == charsy) { //��Ϊ��������ÿ��ֻ����һ�У���Ӧ��ѭ��ִ��
		tempToken = token;
		tempCh = ch;
		tempCharCounter = charCounter;
		insymbol();
		if (token == ident) {
			name = id;
			insymbol();
			if (token == comma || token == lmidbrack || token == semicolon) { //��������,��ָ�����
				token = tempToken;
				ch = tempCh;
				charCounter = tempCharCounter;
				variableExplain();
			}
			else {
				error(LackSemicolon, 2);  //�����ϱ��������ķ���Ӧ��Ϊ�ֺ�
				return;
			}
		}
		else {
			error(LackIdent, 4); //Ӧ��Ϊ��ʶ��
			return;
		}
	}
	if (token == constsy) {  //����˵���ڱ���˵�����棬����
		error(ConstDefAfterVarDef, 1);  //����˵���ڱ���˵�����棬����
		return;
	}
	statementList(); //���������
}

//������У�   :: = ������䣾��
void statementList() {
	//����п���Ϊ��
	statement();
	while (token == ifsy || token == forsy || token == lbigbrack || token == intsy || token == charsy
		|| token == voidsy || token == ident || token == scanfsy || token == printfsy ||
		token == switchsy || token == returnsy || token == semicolon) {
		statement();
	}
}

/*����䣾    :: = ��������䣾����ѭ����䣾 | ��{ ��������У��� }�������з���ֵ����������䣾;
| ���޷���ֵ����������䣾; ������ֵ��䣾; ��������䣾; ����д��䣾; �����գ�; | �������䣾����������䣾;*/
void statement() {
	int tempToken, tempCharCounter;
	char tempCh;
	int result;
	string tempName;
	if (token == ifsy) {  //�������
		ifStatement();
	}
	else if (token == forsy) {  //ѭ�����
		forStatement();
	}
	else if (token == lbigbrack) {  //�����
		insymbol();
		statementList();
		if (token == rbigbrack) {
			insymbol();
		}
		else {
			error(LackRBigBrack, 2);  //Ӧ��Ϊ�Ҵ�����
			return;
		}
	}
	else if (token == ident) {	//�з���ֵ�����������,�޷���ֵ�����������,��ֵ��䣬��ҪԤ��
		tempName = id;
		tempToken = token;
		tempCh = ch;
		tempCharCounter = charCounter;
		insymbol();
		if (token == becomes || token == lmidbrack) {		//��ֵ���
			token = tempToken;
			ch = tempCh;
			charCounter = tempCharCounter;
			assignStatement();
		}
		else if (token == lsmallbrack) {	//���з���ֵ����������䣾 :: = ����ʶ������(����ֵ��������)��
			insymbol();						//���޷���ֵ����������䣾 :: = ����ʶ������(����ֵ��������)�� 
			valueOfParameterTable(); //����ֵ�������Ѿ�������һ����¼ֵ�������͵�����
			if (token == rsmallbrack) {
				insymbol();
			}
			else {
				error(LackRSmallBrack, 1);  //Ӧ��Ϊ��С����
				return;
			}
			result = findInSymbolTable(tempName, 1); //���Ҹú���
			if (result >= 0) {  //�ҵ�
				if (callNoReturnFunc == 1) {  //�õ��ú��������޷���ֵ���ú���
					callNoReturnFunc = 0;
					//if (tempName == "main") { // ���ܵ���main�����ļ�飡  ��Ϊ��maindef�У�statementList������main()������ú�ͻ����maindef��֮���ж��Ҵ����ű�û���Ҵ����ŵĴ���
					//	error(callMain); 
					//	return;											
					//}
				}
				else {
					//�з���ֵ��������

				}
				insertMiddleCode("call", tempName, "", ""); //�������õ���Ԫʽ ����е��з���ֵ�������ò����ý�����ֵ����洢��һ���Ĵ����У�
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
				cout << "����е��õĺ���δ������߲������Ͳ�ƥ����߲���������ƥ��" << endl;
				//���õĺ���δ������߲������Ͳ�ƥ����߲���������ƥ�䣬findInSymbolTable�Ѿ�����
				return;
			}
		}
		else {
			//��û�����ֿ��ܣ�����
			//error(StatementIsIllegal);  //�Ȳ��Ǻ���������䣬�ֲ��Ǹ�ֵ���
			//return;
		}
		if (token == semicolon) {
			insymbol();
		}
		else {
			error(LackSemicolon, 3);  // Ӧ��Ϊ�ֺţ�����
			return;
		}
	}
	else if (token == scanfsy) {  //�����
		scanfStatement();
		if (token == semicolon) {
			insymbol();
		}
		else {
			error(LackSemicolon, 3);  // Ӧ��Ϊ�ֺţ�����
			return;
		}
	}
	else if (token == printfsy) {  //д���
		printfStatement();
		if (token == semicolon) {
			insymbol();
		}
		else {
			error(LackSemicolon, 3);  // Ӧ��Ϊ�ֺţ�����
			return;
		}
	}
	else if (token == semicolon) {  //�����
		insymbol();
		writeToFile("emptyStatement");
	}
	else if(token == switchsy) {   //������
		switchStatement();
	}
	else if (token == returnsy) {  //�������
		returnStatement();
		if (token == semicolon) {
			insymbol();
		}
		else {
			error(LackSemicolon, 3);  // Ӧ��Ϊ�ֺţ�����
			return;
		}
	}
	//else {
	//	error(StatementIllegalSign);  // �������������ķ�
	//	return;
	//}
}


//��ֵ������   :: = �����ʽ��{ ,�����ʽ�� }�����գ�
void valueOfParameterTable() {
	vector<string> paraVector;
	//��ֵ�������ÿ�����������ͷ���ParaType�У�ֵ������������ͼ���ڲ���ʱ�����
	if (token == rsmallbrack) { //ֵ������Ϊ�յ����
		paraNum = 0;
		return;
	}
	else {
		do
		{
			if (token == comma)		insymbol();
			expression();
			paraVector.push_back(OperationResult);
			ParaType.push(factorType); //��������
		} while (token == comma);
		paraNum = paraVector.size();
		//cout << "aaaaaaa" << paraNum << endl;
		for (int i = 0; i < paraNum; i++)  //ʵ�α�����funcPa��Ԫʽ
		{
			insertMiddleCode("funcPa", "", "", paraVector[i]);
			writeToInfixExpressionFile("push " + paraVector[i]);
		}
		paraVector.clear();
	}

}

//�����ʽ��    :: = �ۣ������ݣ��{ ���ӷ����������� }
void expression() {
	string register_1, register_2, register_final, op;
	int Sign = -1;
	factorType = -1; //��ʼ����������
	if (token == plusSign || token == minusSign) {
		factorType = INT;
		Sign = token;
		insymbol();
	}
	if (Sign == -1) { // û�� �Ӽ�����������
		item();
		register_final = OperationResult;  //�ȴ洢����һ�����Ľ��
	}
	else if (Sign == plusSign) {
		item();
		//Sign + �� ��һ��item()����൱��item()�ķ��ؽ��δ�ı䣬�����������ָ�����
		register_final = OperationResult;
	}
	else if (Sign == minusSign) {
		item();
		//Sign - �� ��һ��item()���
		register_1 = OperationResult; //�洢��ʱ���
		register_final = genNewRegister(); //�����µļĴ����洢register_1�븺�Ž�ϵĽ��
		insertMiddleCode("-", "0", register_1, register_final);		//register_final = 0 - register_1 = -register_1
		writeToInfixExpressionFile(register_final + " = 0 - " + register_1);
	}
	//����������Ҫ����
	while (token == plusSign || token == minusSign)
	{
		factorType = INT;
		register_1 = register_final; //register_1�洢������ı��ʽ���ֵĽ��
		op = token == plusSign ? "+" : "-";
		insymbol();
		item();
		register_2 = OperationResult;
		register_final = genNewRegister(); //�����µļĴ����洢register_1��register_2��ϵĽ��
		insertMiddleCode(op, register_1, register_2, register_final);
		writeToInfixExpressionFile(register_final + " = " + register_1 + " " + op + " " + register_2);

	}
	//����������󣬱��ʽ�����ս���洢��register_final�У�ͨ��OperationResult����
	OperationResult = register_final;
}

//���     :: = �����ӣ�{ ���˷�������������ӣ� }
void item() {
	string register_1, register_2, register_final, op;  //ͬ���ڱ��ʽ�Ĵ���
	factor();
	register_final = OperationResult; //�洢��Ľ��
	while (token == timesSign || token == idivSign) {
		factorType = INT;
		register_1 = register_final;
		op = token == timesSign ? "*" : "/";
		insymbol();
		factor();
		register_2 = OperationResult;
		register_final = genNewRegister();
		if (op == "/" && register_2 == "0") {
			//�������Ϊ0�Ĵ��󣬼����ճ�����
			error(DivideZero);
			// return;
		}
		insertMiddleCode(op, register_1, register_2, register_final);
		writeToInfixExpressionFile(register_final + " = " + register_1 + " " + op + " " + register_2);
	}
	//�����������������ս���洢��register_final�У�ͨ��OperationResult����
	OperationResult = register_final;
}

//�����ӣ�    :: = ����ʶ����������ʶ������[�������ʽ����]������������ | ���ַ���
//�����з���ֵ����������䣾 | ��(�������ʽ����)��
void factor() {
	int result, Sign = -1, tempType;
	int tempToken, tempCharCounter;
	char tempCh;
	string tempName, op;
	string tempRegister;
	if (token == ident) {
		tempName = id;
		tempToken = token;  //��ҪԤ��
		tempCh = ch;
		tempCharCounter = charCounter;
		insymbol();
		if (token == lsmallbrack) {  //�з���ֵ��������
			insymbol();
			valueOfParameterTable();
			if (token == rsmallbrack) {
				result = findInSymbolTable(tempName,1);
				if (result >= 0) {  //�ڷ��ű��в��Ҹú����Ƿ����,������ֵ���ڵ���0�������ú������ڣ��Ҳ������������������ͬ
					if (callNoReturnFunc != 1) {  //�з���ֵ��������
						//cout << "���ӵ����з���ֵ����" << endl;
						tempRegister = genNewRegister(); //�洢����ֵ�ļĴ���
						insertMiddleCode("call", tempName, "", tempRegister);
						writeToInfixExpressionFile("call " + tempName);
						writeToInfixExpressionFile(tempRegister + " = RET");
						insymbol();
						OperationResult = tempRegister; //��Ľ���洢
						factorType = (symbolTable[result].value == intsy ? INT : CHAR);		//������value�洢��������
					}
					else {
						callNoReturnFunc = 0;  //����callNoReturnFunc��
						error(AssignWithNoReturnFunc);  //�������޷���ֵ����������
						//cout << "���ӵ����޷���ֵ����" << endl;
						return;
					}
				}
				else {   //findInSymbolTable�Ѿ�����
					//cout << "bbbbbbbbbbbbbbbbbbbbb" << endl;
					cout << "���ӵ��õĺ���δ�������ߺ�������������ƥ�䣬�������Ͳ�ƥ��" << endl;
					return;
				}
			}
			else {
				error(LackRSmallBrack, 2);  //Ӧ��Ϊ��С����
				return;
			}
		}
		else if (token == lmidbrack) { //�������
			insymbol();
			tempType = factorType;  //�洢֮ǰ�ı������ͣ���Ϊ�ڵ���������±���ʽ��factorType��ı䣬��ʱ��������֮��ԭ��������ʽȷ����factorType���ж��±�
			expression();//���ʽ����洢��OperationResult��
			factorType = tempType;  //��ԭfactorType
			result = findInSymbolTable(tempName,0);  // ������������
			if (result >= 0) {  //�ڷ��ű��в��Ҹ������Ƿ����
								//��Ӧ���ж������±��Ƿ�Խ�磬�ж������±����ͣ�
				if (symbolTable[result].arraySize == 0) {
					error(TypeNotArray, 0);   //�ҵ��ı�����Ϊ�������ͣ������Ҫ�ж���????
					return;
				}
				if (factorType == -1) {  //��ֻ��һ���������ӣ���Ӧ������factorTypeΪ������Ԫ�ص����
					factorType = symbolTable[result].kind == intsy ? INT : CHAR;
				}
				if (isdigit(OperationResult[0])) {
					if (atoi(OperationResult.c_str()) >= symbolTable[result].arraySize) {
						//�����±�Խ�磬����
						error(ArrayIndexOfRange, 0);
						//return;
					}
				}
				// else {
				// 	error();  //�����±����Ϊ���ʽ����Ϊ�޷������������ж��Ƿ�Խ�磬����ʱȷ�������ñ���
				// }
			}
			else {
				//δ���������,�ڲ��Һ������Ѿ�����
				return;
			}
			if (token == rmidbrack) {
				//�õ��������Ԫ�أ���Ԫ���ʽ����
				insymbol();
				tempRegister = genNewRegister(); //���ɴ洢�Ĵ���
				insertMiddleCode("getArr", tempName, OperationResult, tempRegister); //OperationResult��ʱ�洢�����±�
				writeToInfixExpressionFile(tempRegister + " = " + tempName + "[" + OperationResult + "]");
				OperationResult = tempRegister; //OperationResult֮�����
			}
			else {
				error(LackRMidBrack, 1);  // Ӧ��Ϊ��������
				return;
			}
		}
		else {  //������ʶ���ж� ,��Ҫָ����ƶ�
			token = tempToken;
			ch = tempCh;
			charCounter = tempCharCounter;
			result = findInSymbolTable(tempName,0);  // ���ұ�������
			if (result >= 0) {  //�ҵ�
				//cout << "�ҵ���" << name << " result = " << result << endl;
				OperationResult = tempName;
				if (symbolTable[result].kind == intconst || symbolTable[result].kind == charconst) {  //���������
					if (factorType != INT) {
						factorType = (symbolTable[result].kind == intconst ? INT : CHAR);
					}
				}
				else if (symbolTable[result].kind == PARAMETER) {  //�������������
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
				//�����ǽ���forѭ��������������Ƿ����ѭ��������ʶ�������
				if (needToJudge && (tempName == forVarName)) {  //����Ҫ���жϣ������������ʱ����hasForVarNameΪtrue
					hasForVarName = true;
				}
			}
			else {
				//û�ҵ���δ���壬�Ѿ��ڲ��Һ����б���
				return;
			}
		}
	}
	else if (token == intconst || token == plusSign || token == minusSign) {  //��������
		if (token == plusSign || token == minusSign) {
			Sign = token;
			insymbol();
		}
		if (token == intconst) {
			if (Sign != -1) { //ǰ�溬�з���
				if (inum == 0) {
					error(SignBeforeZero, 2);  //0ǰ���� + ���� - ����
					return;
				}
				else {
					//�����������
					op = Sign == plusSign ? "" : "-";
					OperationResult = op + convertNumToString(inum);
				}
			}
			else {	
				//ǰ�治���з��ŵ�����
				OperationResult = convertNumToString(inum);
			}
			factorType = INT;  //����Ϊint����
			insymbol();
		}
		else {
			error(LackNum, 0);  //Ӧ��Ϊ����,����ȱʧ������
			return;
		}
	}
	else if (token == charconst) {
		//�����ַ������
		//����һ���ַ������������Ϊchar�����в����� + - * / �ȣ���ñ��ʽ���;�Ϊint��������Ϊintʱ�Ͳ���ת����char��
		if (factorType != INT) {
			factorType = CHAR;  //����һ���ַ������������Ϊchar����
		}
		OperationResult = convertNumToString(inum);  //��ʱ inum �洢�ַ���ASCII��
		insymbol();
	}
	else if (token == lsmallbrack) {  //���� ��(�����ʽ��)�� �����
		insymbol();
		expression();
		//tempRegister = OperationResult;
		if (token == rsmallbrack) {
			//OperationResult = tempRegister;
			insymbol();
		}
		else {
			error(LackRSmallBrack, 2);  //Ӧ��Ϊ��С����
			return;
		}
	}
	else {
		error(FactorIllegalSign); //���������ӵĹ����ķ�
		return;
	}
}

//����ֵ��䣾   :: = ����ʶ�����������ʽ�� | ����ʶ������[�������ʽ����]�� = �����ʽ��
void assignStatement() {
	int result;
	string tempName;
	string register_final;
	int leftType = -1, rightType = -1;
	if (token == ident) {
		tempName = id;
		insymbol();
		if (token == lmidbrack) { //����
			result = findInSymbolTable(tempName, 0);
			if (result >= 0) {
				if (symbolTable[result].arraySize != 0) {  //ȷ��������
					;
				}
				else {
					error(TypeNotArray, 1);   //�ҵ��ı�����Ϊ�������ͣ������Ҫ�ж�
					return;
				}
				leftType = symbolTable[result].kind == intsy ? INT : CHAR;
			}
			else {
				//error(VariableNotDefine);  //����δ���壬�Ѿ��ڲ��Һ����б���
				return;
			}
			insymbol();
			expression();
			register_final = OperationResult; //�洢���ʽ���
			if (isdigit(OperationResult[0])) {
				if (atoi(OperationResult.c_str()) >= symbolTable[result].arraySize) {
					//�����±�Խ�磬����
					error(ArrayIndexOfRange, 1);
					//cout << "��" << lineNum << "�и�ֵ�������Խ�磡" << endl;
					return;
				}
			}
			//else {
			//	//error();  //�����±����Ϊ���ʽ����Ϊ�޷������������ж��Ƿ�Խ��,����ʱȷ�������ñ���
			//}
			if (factorType != INT) {  //����±�����
				error(ArrayIndexNotIntType);  //���ʽ���Ͳ�Ϊ���ͣ��������±����Ͳ���Ӧ
				//cout << factorType << endl;
				cout << "���ʽ���Ͳ�Ϊ���ͣ��������±����Ͳ���Ӧ" << endl;
				return;
			}
			if (token == rmidbrack) {
				insymbol();
			}
			else {
				error(LackRMidBrack, 2);  //Ӧ��Ϊ��������
				return;
			}
			if (token == becomes) {
				insymbol();
			}
			else {
				error(LackBecome, 2);  //Ӧ��Ϊ�ȺŸ�ֵ����
				return;
			}
			expression();
			rightType = factorType;
			if (leftType != rightType) {
				//error();  //��ֵ��䣨���飩������ұߵ����Ͳ�ƥ��???
				//cout << leftType << " " << rightType << endl;
				cout << "warning:��" << lineNum << "�и�ֵ��䣨���飩������ұߵ����Ͳ�ƥ��!��֮���ת�����ܻ����" << endl;
				//return;
			}
			insertMiddleCode("[]=", tempName, register_final, OperationResult);
			writeToInfixExpressionFile(tempName + "[" + register_final + "] = " + OperationResult);
		}
		else if (token == becomes) {
			result = findInSymbolTable(tempName, 0);
			if (result >= 0) {
				if (symbolTable[result].kind == intconst || symbolTable[result].kind == charconst) {
					error(AssignConst, 0);  //Ϊ������ֵ������
					//cout << "��" << lineNum << "��Ϊ������ֵ��" << endl;
					return;
				}
				//Ϊ���鸳ֵ������
				else if (symbolTable[result].arraySize != 0) {
					error(AssignArray);
					return;
				}
			}
			else {
				//error(VariableNotDefine);  //����δ���壬�Ѿ��ڲ��Һ����б���
				return;
			}
			leftType = (symbolTable[result].kind == intsy || (symbolTable[result].kind == PARAMETER && symbolTable[result].value == intsy))? INT : CHAR;
			insymbol();
			expression();
			rightType = factorType;
			if (leftType != rightType) {
				//error();  //��ֵ��䣨�����������������ұߵ����Ͳ�ƥ�� ? �ַ�ת���ķ�Χ����жϣ�������
				//cout << leftType << " " << rightType << endl;
				cout << "warning:��" << lineNum << "�и�ֵ���������ұߵ����Ͳ�ƥ��!��֮���ת�����ܻ����" << endl;
				//return;
			}
			insertMiddleCode("=", OperationResult, "", tempName);
			writeToInfixExpressionFile(tempName + " = " + OperationResult);
		}
	}
	writeToFile("assignStatement");
}

//��������䣾  :: = if ��(������������)������䣾��else����䣾��
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
			error(LackLSmallBrack, 1);  //Ӧ��Ϊ��С����
			return;
		}
		if (token == rsmallbrack) {
			insertMiddleCode("jne", "", "", label_1); //��Ϊ�ٵģ�����ת��label_1��ǩ������˳��ִ��
			writeToInfixExpressionFile("BZ " + label_1);
			insymbol();
			statement();
			insertMiddleCode("jump", "", "", label_2); //��������ת��label_2,��elseִ�н����ĵط�����û��else������ת��if������֮��ĵط�
			writeToInfixExpressionFile("GOTO " + label_2);

		    /*��û��else��䲿�֣�Ҳ����������ǩ������ת
			 ����������Ϊ if ������ jump label_1
						......������ifҪִ�е����
						jump label_2
						label_1:
						label_2:������if������������		*/
			//���ɵ�һ����ǩ����Ϊif������������ת�ı�ǩ
			insertMiddleCode("genLabel", "", "", label_1);
			writeToInfixExpressionFile(label_1 + ":");
			if (token == elsesy) { //����else
				insymbol();
				statement();
			}
			insertMiddleCode("genLabel", "", "", label_2);
			writeToInfixExpressionFile(label_2 + ":");
		}
		else {
			error(LackRSmallBrack, 3);  //Ӧ��Ϊ��С����
			return;
		}
		writeToFile("ifStatement");
	}
}

//��������    :: = �����ʽ������ϵ������������ʽ���������ʽ�� //���ʽΪ0����Ϊ�٣�����Ϊ��,��Ϊforѭ�����ã�����Ҫ�ж������Ƿ���ѭ������
void condition(int conditionType) {
	string register_1, register_2;
	string CompareOp;
	expression();
	//�����ʽ�Ľ���洢��register_1
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
		//���ʽ����洢��register_2
		register_2 = OperationResult;
		insertMiddleCode(CompareOp, register_1, register_2, ""); //ת����mipsָ��Ӧ�ý��������Ĵ�����
		writeToInfixExpressionFile(register_1 + " " + CompareOp + " " + register_2);
		if (conditionType == forsy) {  //ֻ��for��if����condition����Ϊfor�����ж��������Ƿ����identName
			if (hasForVarName)   {	//�������а���ѭ������������hasForVarName
				hasForVarName = false;
			}
			else {	//��������identName���򱨴�;
				cout << "���������� "<< lineNum << " ѭ������ѭ������������ѭ��������" << endl;
				error(forConditionNotContainVar);
				return;
			}
			
		}
		return;
	}
	//����ֻ��һ�����ʽ�����!
	insertMiddleCode("!=", register_1, "0", "");
	writeToInfixExpressionFile(register_1 + " != 0 ");
	return;
}

//��ѭ����䣾   :: = for��(������ʶ�����������ʽ��; ��������; ����ʶ����������ʶ����(+| -)����������)������䣾
//��������    :: = ���������֣��������֣���
void forStatement() {
	/*������ѭ�������й�
	������ʶ��������ѭ������еĵ�һ����ʶ����ѭ��������*/
	/*for�����ִ��һ��ѭ�����е�����ٽ���ѭ�������Ƿ�Խ��Ĳ���
	forѭ������ִ�к��ж���Ҫ3��label
	����ָ��˳��Ϊ��ִ����䣬������䣬�ж��������*/
	/*	��ʶ�� = ���ʽ;����һ��ָ��
	   jump label_1
	   label_2:
	   (����)���������㣬jne label_3
	   label_1:
	   ִ����䲿��
	   ����ѭ����������
	   jump label_2
	   label_3:
	   for����֮������
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
			error(LackLSmallBrack, 1);  //Ӧ��Ϊ��С����
			return;
		}
		if (token == ident) {   //ֻ��Ϊ��ʶ�����������������ĳ��Ԫ�أ�
			SymbolName_1 = id;
			forVarName = id;  //��¼��ѭ���������ֵ�ȫ�ֱ�����
			result = findInSymbolTable(SymbolName_1, 0);
			if (result >= 0) {
				//�ж��ǲ��ǳ���Ϊ��ֵ
				if (symbolTable[result].kind == intconst || symbolTable[result].kind == charconst) {
					error(AssignConst, 1);  //Ϊ������ֵ������
					cout << "��" << lineNum << "��Ϊ������ֵ��" << endl;
					return;
				}
				insymbol();
			}
			else {
				//error(VariableNotDefine); //δ�ҵ������ҳ����Ѿ�����ֱ�ӷ��ؼ���
				return;
			}
		}
		else {
			error(LackIdent, 5);  //Ӧ��Ϊ��ʶ��������
			return;
		}
		if (token == becomes) {
			insymbol();
		}
		else {
			error(LackBecome, 0);  //Ӧ��Ϊ�Ⱥţ�����
			return;
		}
		expression();
		insertMiddleCode("=", OperationResult, "", SymbolName_1);  // for(��ʶ�� = ���ʽ;...;...)
		writeToInfixExpressionFile(SymbolName_1 + " = " + OperationResult);
		if (token == semicolon) {
			insymbol();
		}
		else {
			error(LackSemicolon, 4);  //Ӧ��Ϊ�ֺţ�����
			//return;
		}
		insertMiddleCode("jump", "", "", label_1); //��������ת��label_1,��for��ִ����䲿��
		writeToInfixExpressionFile("GOTO " + label_1);
		insertMiddleCode("genLabel", "", "", label_2);  //�������ж�֮ǰ����һ����ǩ
		writeToInfixExpressionFile(label_2 + ":");
		needToJudge = true;//��ʼ��������������needToJudge����Ϊtrue
		condition(forsy); // for(...; ���� ;...)  
		needToJudge = false;  //������������������needToJudge����Ϊfalse
		if (token == semicolon) {
			insymbol();
		}
		else{
			error(LackSemicolon, 4);  //Ӧ��Ϊ�ֺţ�����
			//return;
		}
		insertMiddleCode("jne", "", "", label_3); //��Ϊ�ٵģ�����ת��label_3��ǩ������˳��ִ��
		writeToInfixExpressionFile("BZ " + label_3);
		if (token == ident) {
			SymbolName_2 = id;
			result = findInSymbolTable(SymbolName_2, 0); //�ñ�ʶ������Ϊ֮ǰ��������ʶ����
			if (result >= 0) {
				if (SymbolName_2 == SymbolName_1) {
					insymbol();
				}
				else {  //�����ñ�ʶ����������ʶ���޹�!
					error(forUpdateNotContainVar);  //����
					return;
				}
			}
			else {
				//δ�ҵ������ҳ����Ѿ�����ֱ�ӷ��ؼ���
				return;
			}
		}
		else {
			error(LackIdent, 5);  //Ӧ��Ϊ��ʶ��������
			return;
		}
		if (token == becomes) {
			insymbol();
		}
		else {
			error(LackBecome);  //Ӧ��Ϊ�Ⱥţ�����
			return;
		}
		if (token == ident) {
			SymbolName_3 = id;
			result = findInSymbolTable(SymbolName_3, 0); //�ñ�ʶ������Ϊ֮ǰ��������ʶ����
			if (result >= 0) {
				if (SymbolName_3 == SymbolName_1) {
					insymbol();
				}
				else {  //�����ñ�ʶ����������ʶ���޹�!
					error(forUpdateNotContainVar);  //����
					return;
				}
			}
			else {
				//δ�ҵ������ҳ����Ѿ�����ֱ�ӷ��ؼ���
				return;
			}
		}
		else {
			error(LackIdent, 5);  //Ӧ��Ϊ��ʶ��������
			return;
		}
		if (token == plusSign || token == minusSign) {
			Sign = token;
			insymbol();
		}
		else {
			error(LackPlusOrMinus);  //Ӧ��Ϊ�ӺŻ��߼���
			return;
		}
		//��������
		if (token == intconst) {
			if (inum != 0) {
				step = inum;
				insymbol();
			}
			else {
				error(StepIsZero);  //����Ϊ0 ������Ҫ�Է�0���ֿ�ͷ
				return;
			}
		}
		else {
			error(LackNum, 1);  //Ӧ��Ϊ��������
			return;
		}
		if (token == rsmallbrack) {
			insymbol();
			insertMiddleCode("genLabel", "", "", label_1);  //��ִ�����ǰ������һ����ǩ
			writeToInfixExpressionFile(label_1 + ":");
			statement();
		}
		else {
			error(LackRSmallBrack, 3);  //Ӧ��Ϊ��С����
			return;
		}
		op = Sign == plusSign ? "+" : "-";
		tempRegister = genNewRegister();
		insertMiddleCode(op, SymbolName_3, convertNumToString(step), tempRegister);
		writeToInfixExpressionFile(tempRegister + " = " + SymbolName_3 + " " + op + " " + convertNumToString(step));
		insertMiddleCode("=", tempRegister, "", SymbolName_2);  //���±������
		writeToInfixExpressionFile(SymbolName_2 + " = " + tempRegister);
		insertMiddleCode("jump", "", "", label_2);
		writeToInfixExpressionFile("GOTO " + label_2);
		insertMiddleCode("genLabel", "", "", label_3);  //��forѭ����������λ������һ����ǩ
		writeToInfixExpressionFile(label_3 + ":");
		writeToFile("forStatement");
	}
}

//�������䣾  :: = switch ��(�������ʽ����)�� �� { �����������ȱʡ���� }��
void switchStatement() {
	//�������У�switch����ı��ʽ��case����ĳ���ֻ�������int��char���ͣ�ÿ����������ִ����Ϻ󣬲�����ִ�к������������
	/*switch (���ʽ) ���ʽ����洢��switchResult��
	 case xxx:
		����switchResult �� xxx �ıȽ�ָ��
		jne , , ,labelָ����ȣ���������һ��case����defaultλ�ã�
		�������ָ��
		jump default_label_final ִ�и÷�֧��ֱ������default������λ��
		genLabel label  ������һ��case����default��ʼʱ��ı�ǩ
	case yyy: 
		.......
	default:
		�������ָ��
	genLabel default_label_final  //����default����λ�õ�label��ǩ
	*/
	//�������ɵ�default������λ�õı�ǩ
	string default_label_final = "";
	string switchResult = "";
	int SwitchVarType = -1;
	if (token == switchsy) {
		insymbol();
		if (token == lsmallbrack) {
			insymbol();
		}
		else {
			error(LackLSmallBrack, 1);  //Ӧ��Ϊ��С����
			return;
		}
		expression();
		switchResult = OperationResult;  //�洢���ʽ���
		SwitchVarType = factorType;
		if (token == rsmallbrack) {
			insymbol();
		}
		else {
			error(LackRSmallBrack, 3);  //Ӧ��Ϊ��С����
			return;
		}
		if (token == lbigbrack) {
			insymbol();
		}
		else {
			error(LackLBigBrack, 1);  //Ӧ��Ϊ�������
			return;
		}
		if (token == casesy) {
			//�������ɵ�default������λ�õı�ǩ
			default_label_final = genNewLabel();
			situationTable(default_label_final, switchResult, SwitchVarType);
		}
		else {
			error(KeywordError);  //Ӧ��Ϊcase�ؼ���
			return;
		}
		if (token == defaultsy) {
			defaultStatement();
		}
		else {
			error(KeywordError);  //Ӧ��Ϊdefault�ؼ���
			return;
		}
		if (token == rbigbrack) {
			insymbol();
		}
		else {
			error(LackRBigBrack, 1);  //Ӧ��Ϊ�Ҵ�����
			return;
		}
		insertMiddleCode("genLabel", "", "", default_label_final);  //����default����λ�õ�label��ǩ
		writeToInfixExpressionFile(default_label_final + ":");
		writeToFile("switchStatement");
	}
}

//�������   :: = ���������䣾{ ���������䣾 }
void situationTable(string default_label_final,string switchResult,int SwitchVarType) {
	while (token == casesy) {
		situationStatement(default_label_final, switchResult, SwitchVarType);
	}

}

//���������䣾  :: = case��������������䣾
void situationStatement(string default_label_final, string switchResult, int SwitchVarType) {  //����Ϊ���������ַ���(�������Դ�����)
	string label;
	int Sign = -1, tempNum;
	label = genNewLabel();
	if (token == casesy) {
		insymbol();
		if (token == plusSign || token == minusSign) {
			Sign = token;
			insymbol();
		}
		if (token == intconst || token == charconst) {	//ʶ��������inum��
			//���ͼ��
			if ((token == intconst && SwitchVarType == CHAR) || (token == charconst && SwitchVarType == INT)) {
				//cout << "warning:��" << lineNum << "�е�switch�����жϵ����Ͳ�ƥ�䣡" << endl;
				error(SwitchTypeNotMatch);
				return;
			}
			if (Sign != -1 && inum == 0) {
				error(SignBeforeZero, 1);  //0ǰ���з���
				return;
			}
			else if (Sign != -1 && token == charconst) {
				error(SignBeforeChar);  //�ַ�����ǰ���з���
				return;
			}
			if (Sign == minusSign) {
				tempNum = -inum;
			}
			else {
				tempNum = inum;
			}
			insertMiddleCode("==", switchResult, convertNumToString(tempNum), ""); //�����Ƿ���ȵ��ж�
			writeToInfixExpressionFile(switchResult + " == " + convertNumToString(tempNum));
			insertMiddleCode("jne", "", "", label);
			writeToInfixExpressionFile("BZ " + label);
			insymbol();
		}
		else {
			error(notConst);  //Ӧ��Ϊ����
			return;
		}
		if (token == colon) {
			insymbol();
		}
		else {
			error(LackColon, 0);  //Ӧ��Ϊð��
			return;
		}
		statement();
		insertMiddleCode("jump", "", "", default_label_final);  //ִ�н�����ת��default������λ��
		writeToInfixExpressionFile("GOTO " + default_label_final);
		insertMiddleCode("genLabel", "", "", label);  //������һ��case����default��label��ǩ
		writeToInfixExpressionFile(label + ":");
	}


}

//��ȱʡ��   :: = default: ����䣾
void defaultStatement() {
	if (token == defaultsy) {
		insymbol();
		if (token == colon) {
			insymbol();
		}
		else {
			error(LackColon, 1);  //Ӧ��Ϊð��
			return;
		}
		statement();
	}
}

//������䣾    :: = scanf ��(������ʶ����{ ,����ʶ���� }��)��
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
			error(LackLSmallBrack, 1);  //Ӧ��Ϊ��С����
			return;
		}
		do
		{
			if (token == comma) insymbol();
			if (token == ident) {
				tempName = id;
				result = findInSymbolTable(tempName, 0);  //�洢��������ж�result
			}
			else {
				error(LackIdent, 5); //Ӧ��Ϊ��ʶ��
				return;
			}
			if (result >= 0) { //�����жϲ�Ϊ����
				//�ж��ǲ��ǳ���Ϊ��ֵ
				if (symbolTable[result].kind == intconst || symbolTable[result].kind == charconst) {
					error(AssignConst, 1);  //Ϊ������ֵ������
					cout << "��" << lineNum << "��Ϊ������ֵ��" << endl;
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
			error(LackRSmallBrack, 3);  //Ӧ��Ϊ��С����
			return;
		}
		writeToFile("scanfStatement");
	}
}

//��д��䣾    :: = printf ��(�� ���ַ�����, �����ʽ�� ��)�� | printf ��(�����ַ����� ��)�� | printf ��(�������ʽ����)��
void printfStatement() {
	string tempString = "", tempResult= "", printType = "";
	if (token == printfsy) {
		insymbol();
		if (token == lsmallbrack) {
			insymbol();
		}
		else{
			error(LackLSmallBrack, 1);  //Ӧ��Ϊ��С����
			return;
		}
		if (token == stringconst) {
			tempString = stringContent;
			insymbol();
			if (token == comma) {
				insymbol();
				expression();
				tempResult = OperationResult;
				//cout << "printf����е�" << factorType << endl;
				printType = factorType == INT ? "int" : "char";
			}
		}
		else { //������ʽ
			expression();
			tempResult = OperationResult;
			printType = factorType == INT ? "int" : "char";  //���ݱ��ʽ���ѡ���ӡ����
		}
		if (token == rsmallbrack) {
			insymbol();
		}
		else {
			error(LackRSmallBrack, 3);  //Ӧ��Ϊ��С����
			return;
		}
		insertMiddleCode("ptf", tempString, tempResult, printType);
		writeToInfixExpressionFile("output " + tempString + " (string)");
		writeToInfixExpressionFile("output " + tempResult + " (" + printType + ")");
		writeToFile("printfStatement");
	}
}

//��������䣾   :: = return[��(�������ʽ����)��]
void returnStatement() {
	string tempResult="";
	if (token == returnsy) {
		insymbol();
		if (token == lsmallbrack) {
			insymbol();
			expression();  //���ʽ����洢
			tempResult = OperationResult;
			if (token == rsmallbrack) {
				hasReturnValue = true;
				insertMiddleCode("ret", "", "", tempResult);
				writeToInfixExpressionFile("ret " + tempResult);
				insymbol();
				return;
			}
			else {
				error(LackRSmallBrack, 3);  //Ӧ��Ϊ��С����
				return;
			}
		}
		//ֻ��һ��return�����ؼ���
		insertMiddleCode("ret", "", "", "");
		writeToInfixExpressionFile("ret ");
		writeToFile("returnStatement");
		return;
	}
}

//
//����˵����
//
//��1��char���͵ı��ʽ�����ַ���ASCII���Ӧ�������μ����㣬��д���������ַ�
//
//��2����ʶ�����ִ�Сд��ĸ
//
//��3��д����е��ַ���ԭ�����
//
//��4���������У�switch����ı��ʽ��case����ĳ���ֻ�������int��char���ͣ�ÿ����������ִ����Ϻ󣬲�����ִ�к������������
//
//��5��������±��0��ʼ
//
//��6��for�����ִ��һ��ѭ�����е�����ٽ���ѭ�������Ƿ�Խ��Ĳ���
//forѭ������ִ�к��ж���Ҫ3��label
//ֻ�ڸ�ֵ���������int��char����char��int������ת������Ҫ����������ʾ��
