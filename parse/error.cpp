//
//  error.cpp
//  C0compiler
//
#include "globalData.h"
vector<int> errors;
map<int, string> errorMsg;
int errorpos = 0;
//错误的数量，也是errorTable的顶部指针
int errNum = 0;
//记录错误的种类以及出现行数，防止重复报错
ErrorItem errorTable[maxErrorNum];

void error(int num, int dealType) {
	int operationType = -1;
	ErrorItem tempItem;
	tempItem.ErrorType = num;
	tempItem.lineNum = lineNum;
	if (isNotInTable(tempItem,errorTable)) {
		errorTable[errNum] = tempItem;
		errNum++;
		cout << "第 " << lineNum << " 行出现错误: " << errorMsg[num] << " ！" << endl;
	}
	switch (num)
	{
	case NoSuchFile:	operationType = Quit;	//退出程序
		break;
	case TableFull:		operationType = Quit;	//退出程序
		break;
	case SymbolNameConflict:	operationType = DoNothing;
		break;
	case FunctionNotDefine:		operationType = DoNothing;
		break;
	case FormalParaNumNotMatch:	operationType = DoNothing;
		break;
	case ParaTypeNotMatch:		operationType = DoNothing;
		break;
	case VariableNotDefine:		operationType = SemiColon;
		break;
	case LackIdent:  operationType = operationType = dealType == 0 ? IntCharVoid : dealType == 1 ?
		CommaSemicolon : dealType == 2 ? RSmallBrack : dealType == 3 ?
		LSmallBrack : dealType == 4 ? RBigBrack : dealType == 5 ?
		DoNothing : -1;
		break;
	case ConstDefAfterVarDef: operationType = dealType == 0 ? IntCharVoid : dealType == 1 ?
		RBigBrack : -1;
		break;
	case LackSemicolon:	 operationType = operationType = dealType == 0 ? ConstIntChar : dealType == 1 ?
		IntCharVoid : dealType == 2 ? RBigBrack : dealType == 3 ?
		IfForLBigBrackICVISPSRS : dealType == 4 ? DoNothing : -1;
		break;
	case NotIntOrChar:	operationType = dealType == 0 ? ConstIntChar : dealType == 1 ?
		LSmallBrack : -1;
		break;
	case SignBeforeZero:  operationType = dealType == 0 ? CommaSemicolon : dealType == 1 ?
		CaseDefaultRBigBrack : dealType == 0 ? PlusMinusTimesDivSemicolon : -1;
		break;
	case DataTypeNotMatch:	operationType = CommaSemicolon;
		break;
	case LackBecome:  operationType = dealType == 0 ? DoNothing : dealType == 1 ?
		CommaSemicolon : dealType == 2 ? SemiColon : -1;
		break;
	case LackLSmallBrack:  operationType = dealType == 0 ? IntCharVoid : dealType == 1 ?
		DoNothing : -1;
		break;
	case LackRSmallBrack:operationType = dealType == 0 ? IntCharVoid : dealType == 1 ?
		IfForLBigBrackICVISPSRS : dealType == 2 ? PlusMinusTimesDivSemicolon : dealType == 3 ?
		DoNothing : -1;
		break;
	case LackLBigBrack:  operationType = dealType == 0 ? IntCharVoid : dealType == 1 ?
		DoNothing : -1;
		break;
	case LackRBigBrack:  operationType = dealType == 0 ? IntCharVoid : dealType == 1 ?
		DoNothing : dealType == 2 ? IfForLBigBrackICVISPSRS : -1;
		break;
	case NoReturnFuncWithReturn:  operationType = IntCharVoid;
		break;
	case ArraySizeIsZero: operationType = CommaSemicolon;
		break;
	case LackRMidBrack: operationType = dealType == 0 ? CommaSemicolon : dealType == 1 ?
		PlusMinusTimesDivSemicolon : dealType == 2 ? SemiColon : -1;
		break;
	case ArrayIndexNotUnsignedNum:  operationType = CommaSemicolon;
		break;
	case SwitchTypeNotMatch: operationType = CaseDefaultRBigBrack;
		break;
	case StatementIsIllegal: operationType = IfForLBigBrackICVISPSRS;
		break;
	case AssignWithNoReturnFunc: operationType = PlusMinusTimesDivSemicolon;
		break;
	case TypeNotArray:  operationType = dealType == 0 ? PlusMinusTimesDivSemicolon : dealType == 1 ?
		SemiColon : -1;
		break;
	case ArrayIndexOfRange:  operationType = dealType == 0 ? PlusMinusTimesDivSemicolon : dealType == 1 ?
		SemiColon : -1;
		break;
	case LackNum:  operationType = dealType == 0 ? PlusMinusTimesDivSemicolon : dealType == 1 ?
		DoNothing : -1;
		break;
	case FactorIllegalSign:  operationType = PlusMinusTimesDivSemicolon;
		break;
	case ArrayIndexNotIntType:  operationType = SemiColon;
		break;
	case AssignConst:  operationType = dealType == 0 ? SemiColon : dealType == 1 ?
		DoNothing : -1;
		break;
	case forConditionNotContainVar:  operationType = RSmallBrack;
		break;
	case forUpdateNotContainVar:  operationType = DoNothing;
		break;
	case LackPlusOrMinus:  operationType = DoNothing;
		break;
	case StepIsZero:  operationType = DoNothing;
		break;
	case KeywordError:   operationType = DoNothing;
		break;
	case SignBeforeChar:  operationType = CaseDefaultRBigBrack;
		break;
	case notConst:  operationType = CaseDefaultRBigBrack;
		break;
	case LackColon:  operationType = dealType == 0 ? CaseDefaultRBigBrack : dealType == 1 ?
		RBigBrack : -1;
		break;
	case MainNumNotOne:		operationType = DoNothing;
		break;
	case CodeAfterMain:		operationType = DoNothing;
		break;
	case BeginWithZeroButNotZero: operationType = DoNothing;
		break;
	case NumOutOfRange:		operationType = DoNothing;
		break;
	case CharIllegal:		operationType = DoNothing;
		break;
	case LackSingleQuote:	operationType = DoNothing;//词法已经处理
		break;
	case CharLenBigThanOne:		operationType = DoNothing;
		break;
	case StringIllegal:		operationType = DoNothing;
		break;
	case LackDoubleQuotes:	operationType = DoNothing;//词法已经处理
		break;
	case LexicalIlegalCh:	operationType = DoNothing;
		break;
	case LackComma:		operationType = DoNothing;  //没有出现的错误？？？
		break;
	case StatementIllegalSign:  operationType = IfForLBigBrackICVISPSRS;
		break;
	case LackReturnValue:  operationType = IntCharVoid;
		break;
	case AssignArray: operationType = SemiColon;
		break;
	case DivideZero:  operationType = DoNothing;
		break;
	default:
		break;
	}
	dealWithError(operationType);
}
void dealWithError(int operationType) {
	switch (operationType)
	{
	case Quit:
		cout << "程序由于错误而终止分析！" << endl;
		system("pause");
		exit(-1);
		break;
	case DoNothing:
		break;
	case IntCharVoid:
		while (token != intsy && token != charsy && token != voidsy) {
			insymbol();
			if (readOver) {
				//system("pause");
				return;
			}
		}
		break;
	case ConstIntChar:
		while (token != intsy && token != charsy && token != constsy) {
			insymbol();
			if (readOver) {
				//system("pause");
				return;
			}
		}
		break; 
	case CommaSemicolon:
		while (token != comma && token != semicolon) {
			insymbol();
			if (readOver) {
				//system("pause");
				return;
			}
		}
		break;
	case LSmallBrack:
		while (token != lsmallbrack) {
			insymbol();
			if (readOver) {
				//system("pause");
				return;
			}
		}
		break;
	case RSmallBrack:
		while (token != rsmallbrack) {
			insymbol();
			if (readOver) {
				//system("pause");
				return;
			}
		}
		break;
	case RBigBrack:
		while (token != rbigbrack) {
			insymbol();
			if (readOver) {
				//system("pause");
				return;
			}
		}
		break;
	case IfForLBigBrackICVISPSRS: //处理语句时候发生的错误
		while (token != ifsy && token != forsy && token != lbigbrack &&  token != intsy
			&& token != charsy && token != voidsy && token == semicolon && token != returnsy &&
			token != scanfsy && token != printfsy  && token != ident && token != switchsy) {
			insymbol();
			if (readOver) {
				//system("pause");
				return;
			}
		}
		break;
	case PlusMinusTimesDivSemicolon:
		while (token != plusSign && token != minusSign && token != timesSign && token != idivSign
			&& token != semicolon) {
			insymbol();
			if (readOver) {
				//system("pause");
				return;
			}
		}
		break;
	case SemiColon:
		while (token != semicolon) {
			insymbol();
			if (readOver) {
				//system("pause");
				return;
			}
		}
		break;
	case CaseDefaultRBigBrack:
		while (token != casesy && token != defaultsy && token != rbigbrack) {
			insymbol();
			if (readOver) {
				//system("pause");
				return;
			}
		}
		break;
	default:
		if (readOver) {
			//system("pause");
			return;
		}
		break;
	}
}
bool isNotInTable(ErrorItem erroritem, ErrorItem errortable[]) {
	for (int i = 0; i < errNum; i++)
	{
		if (erroritem.ErrorType == errortable[i].ErrorType && erroritem.lineNum == errortable[i].lineNum) {
			return false;
		}
	}
	return true;
}