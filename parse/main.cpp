//
//  main.cpp
//  C0compiler
//

#include "globalData.h"
string inputFilePath, outputFilePath, outLexicalPath, outSymbolTablePath, outMidCodePath, OutInfixExpressionPath, outOptCodePath, 
outMipsCodePath, outOptMidCodePath; //文件路径
ifstream infile;
ofstream outfile, outLexical, outSymbolTable, outMidCode, OutInfixExpression, outOptCode, outMipsCode, outOptMidCode;
void setup() {
	keyword[0] = "case";	//初始化关键词数组
	keyword[1] = "char";
	keyword[2] = "const";
	keyword[3] = "default";
	keyword[4] = "else";
	keyword[5] = "for";
	keyword[6] = "if";
	keyword[7] = "int";
	keyword[8] = "main";
	keyword[9] = "printf";
	keyword[10] = "return";
	keyword[11] = "scanf";
	keyword[12] = "switch";
	keyword[13] = "void";

	keywordsy[0] = casesy;	//初始化关键词token数组
	keywordsy[1] = charsy;
	keywordsy[2] = constsy;
	keywordsy[3] = defaultsy;
	keywordsy[4] = elsesy;
	keywordsy[5] = forsy;
	keywordsy[6] = ifsy;
	keywordsy[7] = intsy;
	keywordsy[8] = mainsy;
	keywordsy[9] = printfsy;
	keywordsy[10] = returnsy;
	keywordsy[11] = scanfsy;
	keywordsy[12] = switchsy;
	keywordsy[13] = voidsy;

	sps['+'] = plusSign; //定义特殊字符对应的token  (整数)
	sps['-'] = minusSign;
	sps['*'] = timesSign;
	sps['/'] = idivSign;
	sps['('] = lsmallbrack;
	sps[')'] = rsmallbrack;
	sps['['] = lmidbrack;
	sps[']'] = rmidbrack;
	sps['{'] = lbigbrack;
	sps['}'] = rbigbrack;
	sps['='] = becomes;
	sps[','] = comma;
	sps[';'] = semicolon;
	sps[':'] = colon;

	spsString['+'] = "plusSign"; //定义特殊字符对应的token  (字符串)  
	spsString['-'] = "minusSign";
	spsString['*'] = "timesSign";
	spsString['/'] = "idivSign";
	spsString['('] = "lsmallbrack";
	spsString[')'] = "rsmallbrack";
	spsString['['] = "lmidbrack";
	spsString[']'] = "rmidbrack";
	spsString['{'] = "lbigbrack";
	spsString['}'] = "rbigbrack";
	spsString['='] = "becomes";
	spsString[','] = "comma";
	spsString[';'] = "semicolon";
	spsString[':'] = "colon";
	
	errorMsg[0] = "没有该文件";
	errorMsg[1] = "缺少双引号";
	errorMsg[2] = "缺少单引号";
	errorMsg[3] = "缺少分号";
	errorMsg[4] = "缺少逗号";
	errorMsg[5] = "缺少冒号";
	errorMsg[6] = "缺少左小括号";
	errorMsg[7] = "缺少右小括号";
	errorMsg[8] = "缺少右中括号";
	errorMsg[9] = "缺少左大括号";
	errorMsg[10] = "缺少右大括号";
	errorMsg[11] = "缺少标识符";
	errorMsg[12] = "缺少返回值";
	errorMsg[13] = "缺少加减号";
	errorMsg[14] = "缺少数字的声明";
	errorMsg[15] = "缺少 = 号";
	errorMsg[16] = "不是int或者char的类型标识符";
	errorMsg[17] = "main函数个数不是1";
	errorMsg[18] = "字符由非法字符组成";
	errorMsg[19] = "字符串由非法字符组成";
	errorMsg[20] = "使用的函数未定义";
	errorMsg[21] = "使用的变量未定义";
	errorMsg[22] = "关键字错误";
	errorMsg[23] = "表满了（多种表，符号表，分程序索引表，四元式表等）";
	errorMsg[24] = "符号表插入数据重名";
	errorMsg[25] = "main函数后还有代码";
	errorMsg[26] = "形参个数不匹配";
	errorMsg[27] = "参数类型不匹配";
	errorMsg[28] = "无返回值函数作为当做右值";
	errorMsg[29] = "给常量赋值";
	errorMsg[30] = "整数范围过大";
	errorMsg[31] = "赋值语句两边类型不对";
	errorMsg[32] = "数字以0开头且不是0这个整数";
	errorMsg[33] = "读取到的字符长度大于1";
	errorMsg[34] = "词法分析程序读取到非法的字符（不符合任何单词的构成规则）";
	errorMsg[35] = "常量声明在变量后面";
	errorMsg[36] = "0前面有符号";
	errorMsg[37] = "字符前面有符号";
	errorMsg[38] = "变量或者常量声明时候的类型不匹配";
	errorMsg[39] = "无返回值函数却有返回值";
	errorMsg[40] = "（定义时）数组下标不为无符号整数";
	errorMsg[41] = "数组大小为0";
	errorMsg[42] = "句子不合法（例如某句子应该为赋值或者函数调用语句，却都不是等）";
	errorMsg[43] = "语句处理时均不符合条件";
	errorMsg[44] = "数组下标越界";
	errorMsg[45] = "因子处理时均不符合条件";
	errorMsg[46] = "类型不为数组";
	errorMsg[47] = "数组调用时表达式结果不为整型";
	errorMsg[48] = "for循环判断条件与循环变量无关";
	errorMsg[49] = "for循环更新条件与循环变量无关";
	errorMsg[50] = "步长为0";
	errorMsg[51] = "不为常量";
	errorMsg[52] = "switch表达式类型与case判断的类型不匹配";
	errorMsg[53] = "为数组名字赋值";
	errorMsg[54] = "除数为0";

} 
void openFile(){
	cout << "input the path of codeFile:" << endl;
	cin >> inputFilePath;
	//inputFilePath = "in.txt";
	infile.open(inputFilePath.c_str());
	if (!infile.is_open()) {
		error(NoSuchFile);  //没有该文件
		return;
	}
}
int main() {
	setup(); //初始化各种表
	openFile(); //读取文件
	outputFilePath = "outResult.txt";
	outLexicalPath = "Lexical.txt";
	outSymbolTablePath = "SymbolTable.txt";
	outMidCodePath = "15061097_于乐_优化前中间代码.txt";
	OutInfixExpressionPath = "InfixExpression.txt";
	outOptCodePath = "15061097_于乐_优化后目标代码.txt";
	outMipsCodePath = "15061097_于乐_优化前目标代码.txt";
	outOptMidCodePath = "15061097_于乐_优化后中间代码.txt";
	outfile.open(outputFilePath.c_str());
	outLexical.open(outLexicalPath.c_str());
	outSymbolTable.open(outSymbolTablePath.c_str());
	outMidCode.open(outMidCodePath.c_str());
	OutInfixExpression.open(OutInfixExpressionPath.c_str());
	//outOptCode.open(outOptCodePath.c_str());
	outMipsCode.open(outMipsCodePath.c_str());
	outOptMidCode.open(outOptMidCodePath.c_str());
	ch = ' '; // 初始化字符
	outSymbolTable << "name" << "\t" << "value" << "\t" << "kind" << "\t" << "arraySize" << "\t" <<
		"paraNum" << "\t" << "address" << "\t" << endl;
	programme();
	if (mainNum != 1){ //programme结束后，主函数个数应该为1
		error(MainNumNotOne);  //主函数个数不为1，要么没有，要么重复定义  
		//return -1;
	}
	//分析到此处，已经到了末尾，若文件还没结束，则报错
	if (!readOver) {
		error(CodeAfterMain);  //报错 ，主函数后面还有非法字符
		//return -1;
	}
	//文件输出词法分析的结果
	for (vector<INFO>::iterator it = infoVec.begin(); it != infoVec.end(); it++)
	{
		outLexical << *it;
	}
	//cout << "词法分析输出完成" << endl;
	if (errNum ==0) {
		midCode2Mips();
		cout << "中间代码转换为目标代码完成！" << endl;
		optAllExpression();
		cout << "优化四元式生成完成！" << endl;
		outMipsCode.close();
		outMipsCode.open(outOptCodePath.c_str());
		//启动优化
		useCountRegOpt = true;
		midCode2Mips();
		cout << "优化后的中间代码转换为目标代码完成！" << endl;
	}
	outfile.close();
	outLexical.close();
	outSymbolTable.close();
	outMidCode.close();
	OutInfixExpression.close();
	//outOptCode.close();
	outMipsCode.close();
	outOptMidCode.close();
	system("pause");
	return 0;
}
