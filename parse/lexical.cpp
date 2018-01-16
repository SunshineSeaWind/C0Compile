//
//  lexical.cpp
//  C0compiler
//

#include "globalData.h"
const int keyWordsNum = 14, signMAXLen = 30, kmax = 10, nmax = 2147483647;/*keyWordsNum系统保留字数量, signMAXLen为允许的最大标识符长度*/
string keyword[keyWordsNum];
int keywordsy[keyWordsNum];
map<char, int> sps;
map<char, string> spsString;
string lineContent, id;//id存储识别出来的标识符
unsigned char ch; //getch得到的当前字符
int charCounter = 0, lineLength = 0;
int inum, token,  strlength, lineNum = 0 ; //inum为整数或者字符的数值，token为识别出的标识符种类
vector<INFO> infoVec; //仅在词法分析测试阶段使用，存储词法分析结果信息
int counter = 0;  //仅在词法分析测试阶段使用，存储词法分析结果信息对应的下标
int readOver = 0;  // 文件是否读到末尾
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
	//处理完了一行，继续读下一行（到达文件末尾则结束）
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
		getline(infile, lineContent); //getline不读入换行符
		lineContent += "\n";//添加末尾的空白字符，用以分割上下两句话
		lineLength = lineContent.length();
		//cout << lineContent << endl;
	}
	ch = lineContent[charCounter];
	charCounter += 1;
}
int compareNum(string s1, string s2) { //大于返回1， 小于等于返回 0
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
	if (readOver) { //读取到文件末尾，则insymbol直接返回
		//设置token = -1
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
			if (keyword[k].compare(id) == 0) {	//找到
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
	else if (isdigit(ch)) { //规定数字最大长度为10
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
			//以0开头，且不是0这个整数， 传入错误
			error(BeginWithZeroButNotZero);
			errorFlag = 1;
			inum = 0;
		}
		else if (stringNum.length() > kmax || compareNum(stringNum,MaxNumString)) {
			//数值过大传入错误
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
			if (charIsLegal(ch)) //若为规定的字符内容
			{
				inum = ch;  //先在这里设置，之后再去判断 k 是否为 1，若不为 1 ，此处即使赋值后续也会报错
				token = charconst;
				tempInfo.name = ch;
				tempInfo.type = "charconst";
				k = k + 1;
				nextch();
			}
			else {
				errorFlag = 1; //含有非法字符内容
				error(CharIllegal);  //字符包含非法字符
				while (ch != '\'' && ch != '\n') {
					nextch(); //直到读到下一个单引号或者换行
				}
				break;
			}
		}
		if(ch != '\''){
			error(LackSingleQuote);  //缺少单引号
		}
		if (k != 1) { //若字符长度大于1
			error(CharLenBigThanOne);  //传入对应错误
			errorFlag = 1;
			inum = 0;
			token = charconst;
		}
		nextch();
	}
	else if (ch == '\"') {
		stringContent.clear();	//字符的ascii范围判断要添加
		nextch();
		while (ch != '\"')
		{
			if (stringCharIsLegal(ch)) {
				stringContent += ch;
				nextch();
			}
			else {
				errorFlag = 1; //含有非法字符内容
				error(StringIllegal);  //字符串由非法字符组成
				while (ch != '\"' && ch != '\n') {
					nextch(); //直到读到下一个双引号或者换行
				}
				break;
			}
		}
		if(ch != '\"'){
			error(LackDoubleQuotes);  //缺少双引号
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
			error(LackBecome, 0); //应该为!=，缺少等于号
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
		error(LexicalIlegalCh); //均不符合词法组成，报错
		errorFlag = 1;
		//获得下一个字符
		nextch();
		//继续递归调用处理，直到出现合法字符为止
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
