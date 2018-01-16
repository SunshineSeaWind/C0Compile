//
//  lexical.cpp
//  C0compiler
//

#include "globalData.h"
const int keyWordsNum = 14, signMAXLen = 30, kmax = 10, nmax = 2147483647;/*keyWordsNumϵͳ����������, signMAXLenΪ���������ʶ������*/
string keyword[keyWordsNum];
int keywordsy[keyWordsNum];
map<char, int> sps;
map<char, string> spsString;
string lineContent, id;//id�洢ʶ������ı�ʶ��
unsigned char ch; //getch�õ��ĵ�ǰ�ַ�
int charCounter = 0, lineLength = 0;
int inum, token,  strlength, lineNum = 0 ; //inumΪ���������ַ�����ֵ��tokenΪʶ����ı�ʶ������
vector<INFO> infoVec; //���ڴʷ��������Խ׶�ʹ�ã��洢�ʷ����������Ϣ
int counter = 0;  //���ڴʷ��������Խ׶�ʹ�ã��洢�ʷ����������Ϣ��Ӧ���±�
int readOver = 0;  // �ļ��Ƿ����ĩβ
string MaxNumString = "2147483647";
string stringContent;
ostream &operator<< (ostream &os, const INFO & info) {
	os << info.num << " " << info.type << " " << info.name << endl;
	return os;
}
int charIsLegal(char ch) {
	return ch == '+' || ch == '-' || ch == '*' || ch == '/' ||
		isalnum(ch) || ch == '_';
}
int stringCharIsLegal(char ch) {
	return ch == 32 || ch == 33 || (ch >= 35 && ch <= 126);
}
void nextch() {
	//��������һ�У���������һ�У������ļ�ĩβ�������
	if (charCounter == lineLength) {
		if (infile.eof()) {
			ch = ' ';
			readOver = 1;
			//cout << "end!" << endl;
			return;
		}
		lineNum++;
		charCounter = 0;
		lineLength = 0;
		getline(infile, lineContent); //getline�����뻻�з�
		lineContent += "\n";//���ĩβ�Ŀհ��ַ������Էָ��������仰
		lineLength = lineContent.length();
		//cout << lineContent << endl;
	}
	ch = lineContent[charCounter];
	charCounter += 1;
}
int compareNum(string s1, string s2) { //���ڷ���1�� С�ڵ��ڷ��� 0
	int flag = 0;
	if (s1.length() > s2.length()) {
		return 1;
	}
	else if (s1.length() < s2.length()) {
		return 0;
	}
	else {
		for (int i = 0; i < s1.length(); i++) {
			if (s1[i]> s2[i]) {
				flag = 1;
				break;
			}
			else if(s1[i] == s2[i]){
				continue;
			}
			else {
				break;
			}
		}
	}
	return flag;
}
void insymbol() {
	int low, high, k;
	INFO tempInfo;
	string stringNum;
	stringstream ss;
	int errorFlag = 0;
	while (isspace(ch) && !readOver) 	nextch();
	if (readOver) { //��ȡ���ļ�ĩβ����insymbolֱ�ӷ���
		//����token = -1
		token = -1;
		return;
	}
	if (isalpha(ch) || ch == '_') {
		id.clear();
		k = 0;
		while (isalnum(ch) || ch == '_')
		{
			if (k < signMAXLen) {
				id += ch;
				k = k + 1;
			}
			/*else {
				cout << "warning: line " << lineNum << " ident name is longer than 15!" << endl;
			}*/
			nextch();
		}
		low = 0;
		high = keyWordsNum - 1;
		while (low <= high)
		{
			k = (low + high) / 2;
			if (keyword[k].compare(id) == 0) {	//�ҵ�
				break;
			}
			if (id < keyword[k]) {
				high = k - 1;
			}
			else if (id > keyword[k]) {
				low = k + 1;
			}
		}
		if (low <= high) {
			token = keywordsy[k];
			tempInfo.type = keyword[k] + "sy";
		}
		else {
			token = ident;
			tempInfo.type = "ident";
		}
		tempInfo.name = id;
	}
	else if (isdigit(ch)) { //�涨������󳤶�Ϊ10
		stringNum.clear();
		inum = 0;
		token = intconst;
		while (isdigit(ch))
		{
			stringNum += ch;
			nextch();
		}
		inum = atoi(stringNum.c_str());
		if (stringNum[0] == '0' && stringNum.length() != 1) {
			//��0��ͷ���Ҳ���0��������� �������
			error(BeginWithZeroButNotZero);
			errorFlag = 1;
			inum = 0;
		}
		else if (stringNum.length() > kmax || compareNum(stringNum,MaxNumString)) {
			//��ֵ���������
			error(NumOutOfRange);
			errorFlag = 1;
			inum = 0;
		}
		else {
			ss.clear();
			ss << inum;
			tempInfo.name = ss.str();
			tempInfo.type = "intconst";
		}
	}
	else if (ch == '\'') {
		k = 0;
		nextch();
		while (ch != '\'')
		{
			if (charIsLegal(ch)) //��Ϊ�涨���ַ�����
			{
				inum = ch;  //�����������ã�֮����ȥ�ж� k �Ƿ�Ϊ 1������Ϊ 1 ���˴���ʹ��ֵ����Ҳ�ᱨ��
				token = charconst;
				tempInfo.name = ch;
				tempInfo.type = "charconst";
				k = k + 1;
				nextch();
			}
			else {
				errorFlag = 1; //���зǷ��ַ�����
				error(CharIllegal);  //�ַ������Ƿ��ַ�
				while (ch != '\'' && ch != '\n') {
					nextch(); //ֱ��������һ�������Ż��߻���
				}
				break;
			}
		}
		if(ch != '\''){
			error(LackSingleQuote);  //ȱ�ٵ�����
		}
		if (k != 1) { //���ַ����ȴ���1
			error(CharLenBigThanOne);  //�����Ӧ����
			errorFlag = 1;
			inum = 0;
			token = charconst;
		}
		nextch();
	}
	else if (ch == '\"') {
		stringContent.clear();	//�ַ���ascii��Χ�ж�Ҫ���
		nextch();
		while (ch != '\"')
		{
			if (stringCharIsLegal(ch)) {
				stringContent += ch;
				nextch();
			}
			else {
				errorFlag = 1; //���зǷ��ַ�����
				error(StringIllegal);  //�ַ����ɷǷ��ַ����
				while (ch != '\"' && ch != '\n') {
					nextch(); //ֱ��������һ��˫���Ż��߻���
				}
				break;
			}
		}
		if(ch != '\"'){
			error(LackDoubleQuotes);  //ȱ��˫����
		}
		if (!errorFlag) {
			token = stringconst;
			/*cout << stringContent.size() << endl;
			for (int i = 0; i < stringContent.size(); i++)
			{
				cout << stringContent[i] << endl;
			}*/
			tempInfo.name = stringContent;
			tempInfo.type = "stringconst";
		}
		nextch();
	}
	else if (ch == '=') {
		nextch();
		if (ch == '=') {
			token = eql;
			nextch();
			tempInfo.name = "==";
			tempInfo.type = "eql";
		}
		else {
			token = becomes;
			tempInfo.name = "=";
			tempInfo.type = "becomes";
		}
	}
	else if (ch == '<') {
		nextch();
		if (ch == '=') {
			token = leq;
			tempInfo.name = "<=";
			tempInfo.type = "leq";
			nextch();
		}
		else {
			token = lss;
			tempInfo.name = "<";
			tempInfo.type = "lss";
		}
	}
	else if (ch == '>') {
		nextch();
		if (ch == '=') {
			token = geq;
			nextch();
			tempInfo.name = ">=";
			tempInfo.type = "geq";
		}
		else {
			token = gtr;
			tempInfo.name = ">";
			tempInfo.type = "gtr";
		}
	}
	else if (ch == '!') {
		nextch();
		if (ch != '=') {
			errorFlag = 1;
			error(LackBecome, 0); //Ӧ��Ϊ!=��ȱ�ٵ��ں�
		}
		else {
			token = neq;
			nextch();
			tempInfo.name = "!=";
			tempInfo.type = "neq";
		}
	}
	else if (ch == ':') {
		token = colon;
		tempInfo.name = ch;
		tempInfo.type = spsString[ch];
		nextch();
	}
	else if (ch == ';') {
		token = semicolon;
		tempInfo.name = ch;
		tempInfo.type = spsString[ch];
		nextch();
	}
	else if (ch == ',') {
		token = comma;
		tempInfo.name = ch;
		tempInfo.type = spsString[ch];
		nextch();
	}
	else if (ch == '{' || ch == '}' || ch == '[' || ch == ']' || ch == '(' || ch == ')' ||
		ch == '+' || ch == '-' || ch == '*' || ch == '/') {
		token = sps[ch];
		tempInfo.name = ch;
		tempInfo.type = spsString[ch];
		nextch();
	}
	else {
		error(LexicalIlegalCh); //�������ϴʷ���ɣ�����
		errorFlag = 1;
		//�����һ���ַ�
		nextch();
		//�����ݹ���ô���ֱ�����ֺϷ��ַ�Ϊֹ
		insymbol();
	}
	if (!errorFlag) {
		counter += 1;
		tempInfo.num = counter;
		infoVec.push_back(tempInfo);
	}
	else {
		errorFlag = 0;
	}
}
