//
//  main.cpp
//  C0compiler
//

#include "globalData.h"
string inputFilePath, outputFilePath, outLexicalPath, outSymbolTablePath, outMidCodePath, OutInfixExpressionPath, outOptCodePath, 
outMipsCodePath, outOptMidCodePath; //�ļ�·��
ifstream infile;
ofstream outfile, outLexical, outSymbolTable, outMidCode, OutInfixExpression, outOptCode, outMipsCode, outOptMidCode;
void setup() {
	keyword[0] = "case";	//��ʼ���ؼ�������
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

	keywordsy[0] = casesy;	//��ʼ���ؼ���token����
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

	sps['+'] = plusSign; //���������ַ���Ӧ��token  (����)
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

	spsString['+'] = "plusSign"; //���������ַ���Ӧ��token  (�ַ���)  
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
	
	errorMsg[0] = "û�и��ļ�";
	errorMsg[1] = "ȱ��˫����";
	errorMsg[2] = "ȱ�ٵ�����";
	errorMsg[3] = "ȱ�ٷֺ�";
	errorMsg[4] = "ȱ�ٶ���";
	errorMsg[5] = "ȱ��ð��";
	errorMsg[6] = "ȱ����С����";
	errorMsg[7] = "ȱ����С����";
	errorMsg[8] = "ȱ����������";
	errorMsg[9] = "ȱ���������";
	errorMsg[10] = "ȱ���Ҵ�����";
	errorMsg[11] = "ȱ�ٱ�ʶ��";
	errorMsg[12] = "ȱ�ٷ���ֵ";
	errorMsg[13] = "ȱ�ټӼ���";
	errorMsg[14] = "ȱ�����ֵ�����";
	errorMsg[15] = "ȱ�� = ��";
	errorMsg[16] = "����int����char�����ͱ�ʶ��";
	errorMsg[17] = "main������������1";
	errorMsg[18] = "�ַ��ɷǷ��ַ����";
	errorMsg[19] = "�ַ����ɷǷ��ַ����";
	errorMsg[20] = "ʹ�õĺ���δ����";
	errorMsg[21] = "ʹ�õı���δ����";
	errorMsg[22] = "�ؼ��ִ���";
	errorMsg[23] = "�����ˣ����ֱ����ű��ֳ�����������Ԫʽ��ȣ�";
	errorMsg[24] = "���ű������������";
	errorMsg[25] = "main�������д���";
	errorMsg[26] = "�βθ�����ƥ��";
	errorMsg[27] = "�������Ͳ�ƥ��";
	errorMsg[28] = "�޷���ֵ������Ϊ������ֵ";
	errorMsg[29] = "��������ֵ";
	errorMsg[30] = "������Χ����";
	errorMsg[31] = "��ֵ����������Ͳ���";
	errorMsg[32] = "������0��ͷ�Ҳ���0�������";
	errorMsg[33] = "��ȡ�����ַ����ȴ���1";
	errorMsg[34] = "�ʷ����������ȡ���Ƿ����ַ����������κε��ʵĹ��ɹ���";
	errorMsg[35] = "���������ڱ�������";
	errorMsg[36] = "0ǰ���з���";
	errorMsg[37] = "�ַ�ǰ���з���";
	errorMsg[38] = "�������߳�������ʱ������Ͳ�ƥ��";
	errorMsg[39] = "�޷���ֵ����ȴ�з���ֵ";
	errorMsg[40] = "������ʱ�������±겻Ϊ�޷�������";
	errorMsg[41] = "�����СΪ0";
	errorMsg[42] = "���Ӳ��Ϸ�������ĳ����Ӧ��Ϊ��ֵ���ߺ���������䣬ȴ�����ǵȣ�";
	errorMsg[43] = "��䴦��ʱ������������";
	errorMsg[44] = "�����±�Խ��";
	errorMsg[45] = "���Ӵ���ʱ������������";
	errorMsg[46] = "���Ͳ�Ϊ����";
	errorMsg[47] = "�������ʱ���ʽ�����Ϊ����";
	errorMsg[48] = "forѭ���ж�������ѭ�������޹�";
	errorMsg[49] = "forѭ������������ѭ�������޹�";
	errorMsg[50] = "����Ϊ0";
	errorMsg[51] = "��Ϊ����";
	errorMsg[52] = "switch���ʽ������case�жϵ����Ͳ�ƥ��";
	errorMsg[53] = "Ϊ�������ָ�ֵ";
	errorMsg[54] = "����Ϊ0";

} 
void openFile(){
	cout << "input the path of codeFile:" << endl;
	cin >> inputFilePath;
	//inputFilePath = "in.txt";
	infile.open(inputFilePath.c_str());
	if (!infile.is_open()) {
		error(NoSuchFile);  //û�и��ļ�
		return;
	}
}
int main() {
	setup(); //��ʼ�����ֱ�
	openFile(); //��ȡ�ļ�
	outputFilePath = "outResult.txt";
	outLexicalPath = "Lexical.txt";
	outSymbolTablePath = "SymbolTable.txt";
	outMidCodePath = "15061097_����_�Ż�ǰ�м����.txt";
	OutInfixExpressionPath = "InfixExpression.txt";
	outOptCodePath = "15061097_����_�Ż���Ŀ�����.txt";
	outMipsCodePath = "15061097_����_�Ż�ǰĿ�����.txt";
	outOptMidCodePath = "15061097_����_�Ż����м����.txt";
	outfile.open(outputFilePath.c_str());
	outLexical.open(outLexicalPath.c_str());
	outSymbolTable.open(outSymbolTablePath.c_str());
	outMidCode.open(outMidCodePath.c_str());
	OutInfixExpression.open(OutInfixExpressionPath.c_str());
	//outOptCode.open(outOptCodePath.c_str());
	outMipsCode.open(outMipsCodePath.c_str());
	outOptMidCode.open(outOptMidCodePath.c_str());
	ch = ' '; // ��ʼ���ַ�
	outSymbolTable << "name" << "\t" << "value" << "\t" << "kind" << "\t" << "arraySize" << "\t" <<
		"paraNum" << "\t" << "address" << "\t" << endl;
	programme();
	if (mainNum != 1){ //programme����������������Ӧ��Ϊ1
		error(MainNumNotOne);  //������������Ϊ1��Ҫôû�У�Ҫô�ظ�����  
		//return -1;
	}
	//�������˴����Ѿ�����ĩβ�����ļ���û�������򱨴�
	if (!readOver) {
		error(CodeAfterMain);  //���� �����������滹�зǷ��ַ�
		//return -1;
	}
	//�ļ�����ʷ������Ľ��
	for (vector<INFO>::iterator it = infoVec.begin(); it != infoVec.end(); it++)
	{
		outLexical << *it;
	}
	//cout << "�ʷ�����������" << endl;
	if (errNum ==0) {
		midCode2Mips();
		cout << "�м����ת��ΪĿ�������ɣ�" << endl;
		optAllExpression();
		cout << "�Ż���Ԫʽ������ɣ�" << endl;
		outMipsCode.close();
		outMipsCode.open(outOptCodePath.c_str());
		//�����Ż�
		useCountRegOpt = true;
		midCode2Mips();
		cout << "�Ż�����м����ת��ΪĿ�������ɣ�" << endl;
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
