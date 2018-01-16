//
//  optimize.cpp
//  C0compiler
//
#include "globalData.h"
int midTablePtr = 0;  //四元式表的指针，从 0 开始一直分析到 MidCodeTableTop
int tempmidTablePtr = 0;
fourExpression tempMidCodeTable[MAXMIDECODESIZE];
vector<BasicBlock> basicBlockVector;
//节点表，变量名字为键，对应的节点序号为值
map<string, int> DagItemTable;
//索引表，存储图的每个节点
map<int, DagItem*> dagIndexMap;
//存储之后要用到的临时变量
vector<string> tempVarVector;
//扫描所有的四元表达式
void optAllExpression() {
	//扫描
	//先划分基本块
	while (midTablePtr < MidCodeTableTop)
	{
		//处理到func的位置,基本块从除去开始的全局常量以及变量声明开始
		if (MidCodeTable[midTablePtr].op == "func") {
			//分析到end为止
			divideBasicBlock();
			midTablePtr++;
		}
		//直到读取到第一个func的位置，将全局常量以及变量声明加入到临时四元式表中
		else {
			tempMidCodeTable[tempmidTablePtr] = MidCodeTable[midTablePtr];
			tempmidTablePtr++;
			midTablePtr++;
		}
	}
	/*for (int i = 0; i < basicBlockVector.size(); i++)
	{
		BasicBlock basic = basicBlockVector[i];
		cout << basic.Block.beginIndex << "\t" << basic.Block.endIndex << endl;
	}*/
	//依据划分的基本块进行公共子表达式的删除
	dag();
	//生成新的四元式表，填入MidCodeTable中，表顶指针为MidCodeTableTop
	MidCodeTableTop = 0;
	for (int i = 0; i < tempmidTablePtr; i++)
	{
		fourExpression tempFour = tempMidCodeTable[i];
		//outOptMidCode << "\t" << tempFour.op << ",\t" << tempFour.part_1 << ",\t" << tempFour.part_2 << ",\t" << tempFour.part_3 << endl;
		MidCodeTable[MidCodeTableTop] = tempFour;
		MidCodeTableTop++;
	}
	//常数合并
	mergeConstant();
	//删除冗余代码（多余标签）
	deleteLabel();
	//写入优化后的四元式到文件
	for (int i = 0; i < MidCodeTableTop; i++)
	{
		fourExpression tempFour = MidCodeTable[i];
		outOptMidCode << "\t" << tempFour.op << ",\t" << tempFour.part_1 << ",\t" << tempFour.part_2 << ",\t" << tempFour.part_3 << endl;
	}
}
//删除公共子表达式
//以基本块为单位，连续读取以+ - * / = 为op的四元式，进行公共子表达式的删除
void dag() {
	for (int index = 0; index < basicBlockVector.size(); index++)
	{
		//对每一个块进行处理dag图处理
		BasicBlock basic = basicBlockVector[index];
		//处理连续的+ - * / = 语句
		int start = basic.Block.beginIndex;
		int end = basic.Block.endIndex;
		//开始处理基本块中的表达式(连续)
		//将后续用到的临时变量加入容器中
		string part_1 = "", part_2 = "", part_3 = "", op = "";
		/*while (start <= end) {
			if (MidCodeTable[start].op != "=" && MidCodeTable[start].op != "+" && MidCodeTable[start].op != "-" &&
				MidCodeTable[start].op != "*" && MidCodeTable[start].op != "/" && MidCodeTable[start].op != "getArr")
			{
				if (MidCodeTable[start].part_1.length() >0 && MidCodeTable[start].part_1[0] == '$') {
					bool notInVec = true;
					for (vector<string>::iterator tempIt = tempVarVector.begin(); tempIt != tempVarVector.end(); tempIt++)
					{
						if (*tempIt == MidCodeTable[start].part_1) {
							notInVec = false;
							break;
						}
					}
					if (notInVec) {
						tempVarVector.push_back(MidCodeTable[start].part_1);
					}
				}
				if (MidCodeTable[start].part_2.length() >0 && MidCodeTable[start].part_2[0] == '$') {
					bool notInVec = true;
					for (vector<string>::iterator tempIt = tempVarVector.begin(); tempIt != tempVarVector.end(); tempIt++)
					{
						if (*tempIt == MidCodeTable[start].part_2) {
							notInVec = false;
							break;
						}
					}
					if (notInVec) {
						tempVarVector.push_back(MidCodeTable[start].part_2);
					}
				}
				if (MidCodeTable[start].part_3.length() >0 && MidCodeTable[start].part_3[0] == '$') {
					bool notInVec = true;
					for (vector<string>::iterator tempIt = tempVarVector.begin(); tempIt != tempVarVector.end(); tempIt++)
					{
						if (*tempIt == MidCodeTable[start].part_3) {
							notInVec = false;
							break;
						}
					}
					if (notInVec) {
						tempVarVector.push_back(MidCodeTable[start].part_3);
					}
				}
			}
			start++;
		}*/
		while (start <= end)
		{
			if (MidCodeTable[start].op == "=" || MidCodeTable[start].op == "+" || MidCodeTable[start].op == "-" ||
				 MidCodeTable[start].op == "*" || MidCodeTable[start].op == "/" || MidCodeTable[start].op == "getArr")
			{
				int itemCount = 0;
				while (MidCodeTable[start].op == "=" || MidCodeTable[start].op == "+" || MidCodeTable[start].op == "-" ||
					 MidCodeTable[start].op == "*" || MidCodeTable[start].op == "/" || MidCodeTable[start].op == "getArr") {
					DagItem *leftChild = new DagItem();
					DagItem *rightChild = new DagItem();
					DagItem *parent = new DagItem();
					//处理每一个表达式
					int left_index, right_index = -1;
					// + x y z举例 注意 = x,,z  y为空，不处理即可
					if (start >= end) break;
					//先在节点表中寻找 x ，若没有找到,则新建一个叶节点，标号设置为itemCount，标记为x(若x为变量名，
					//标记为x$)，节点表中新添一项(x$,i)
					part_1 = MidCodeTable[start].part_1;
					string varName = "";
					//没找到x
					if (DagItemTable.find(part_1) == DagItemTable.end()) {
						varName = part_1;
						if (!(varName[0] == '+' || varName[0] == '-' || isdigit(varName[0]))) {
							varName = varName + "$";
						}
						if (DagItemTable.find(varName) == DagItemTable.end()) {
							//设置(x,i)/(x$,i)
							DagItemTable[varName] = itemCount;
							//生成dag图中的该节点
							leftChild->name = varName;
							leftChild->num = itemCount;
							left_index = itemCount;
							leftChild->varItems.push_back(varName);
							leftChild->lchild = NULL;
							leftChild->rchild = NULL;
							leftChild->isInStack = false;
							dagIndexMap[left_index] = leftChild;
							itemCount++;
						}
						else {
							left_index = DagItemTable[varName];
							//得到对应的节点
							leftChild = dagIndexMap[left_index];
						}
					}
					//找到x
					else {
						left_index = DagItemTable[part_1];
						//cout << "find " << part_1 << left_index << endl;
						//得到对应的节点
						leftChild = dagIndexMap[left_index];
						//cout << "leftChild ========= " << leftChild << endl;
					}
					//判断y
					part_2 = MidCodeTable[start].part_2;
					varName = "";
					if (part_2 != "") {
						//没找到y
						if (DagItemTable.find(part_2) == DagItemTable.end()) {
							//判断是不是变量名
							varName = part_2;
							if (!(varName[0] == '+' || varName[0] == '-' || isdigit(varName[0]))) {
								varName = varName + "$";
							}
							if (DagItemTable.find(varName) == DagItemTable.end()) {
								//设置(y$,i)
								DagItemTable[varName] = itemCount;
								//生成dag图中的该节点
								rightChild->name = varName;
								//rightChild->name = varName.c_str();
								//strcpy(rightChild->name, varName.c_str());
								rightChild->num = itemCount;
								right_index = itemCount;
								rightChild->varItems.push_back(varName);
								rightChild->lchild = NULL;
								rightChild->rchild = NULL;
								rightChild->isInStack = false;
								dagIndexMap[right_index] = rightChild;
								itemCount++;
							}
							else {
								right_index = DagItemTable[varName];
								//得到对应的节点
								rightChild = dagIndexMap[right_index];
							}
						}
						//找到y
						else {
							right_index = DagItemTable[part_2];
							//得到对应的节点
							/*cout << "leftChild: " << leftChild << endl;
							cout << "rightChild: " << rightChild << endl;
							cout << "right_index: " << right_index << endl;
							cout << "dagIndexMap[right_index]: " << dagIndexMap[right_index] << endl;*/
							rightChild = dagIndexMap[right_index];
							//cout<<rightChild<<endl;
						}
					}
					//判断运算符 op
					op = MidCodeTable[start].op;
					int result;
					//op 不为 = 的情况，判断左右节点以及中间运算符号
					if (op != "=") {
						//如果找到了
						if ((result = findInDag(op, left_index, right_index)) != -1) {
							//此时result已经记录了节点号,将找到的节点赋值给parent
							parent = dagIndexMap[result];
						}
						//未找到
						else {
							//生成dag图中的中间节点，name存储运算符
							parent->name = op;
							//parent->name = op.c_str();
							//strcpy(parent->name, varName.c_str());
							parent->num = itemCount;
							parent->lchild = leftChild;
							parent->rchild = rightChild;
							parent->isInStack = false;
							//左节点父节点添加parent
							leftChild->parent.push_back(parent);
							//右节点父节点添加parent
							rightChild->parent.push_back(parent);
							result = itemCount;
							
							itemCount++;
						}
					}
					//op 为 = 的情况,得到x对应的节点标号即可
					else {
						result = left_index;
						parent = leftChild;
					}
					//判断z
					part_3 = MidCodeTable[start].part_3;
					//没找到z
					if (DagItemTable.find(part_3) == DagItemTable.end()) {
						//新建一项
						DagItemTable[part_3] = result;
					}
					else {
						//修改原来z对应的节点数字
						DagItemTable[part_3] = result;
					}
					//删除包含z的节点的vector中的该元素
					deleteVarInVector(part_3);
					//添加z到节点对应的vector中
					//strcpy(tempName, part_3.c_str());
					parent->varItems.push_back(part_3);
					//将各个节点保存到map对应的索引中
					dagIndexMap[left_index] = leftChild;
					if (right_index != -1) {
						dagIndexMap[right_index] = rightChild;
					}
					dagIndexMap[result] = parent;
					start++;
					//cout << "start == " << start << endl;
				}
				int tempStart = start;
				while (tempStart <= end) {
					if (MidCodeTable[tempStart].part_1.length() >0 && MidCodeTable[tempStart].part_1[0] == '$') {
						bool notInVec = true;
						for (vector<string>::iterator tempIt = tempVarVector.begin(); tempIt != tempVarVector.end(); tempIt++)
						{
							if (*tempIt == MidCodeTable[tempStart].part_1) {
								notInVec = false;
								break;
							}
						}
						if (notInVec) {
							tempVarVector.push_back(MidCodeTable[tempStart].part_1);
						}
					}
					if (MidCodeTable[tempStart].part_2.length() >0 && MidCodeTable[tempStart].part_2[0] == '$') {
						bool notInVec = true;
						for (vector<string>::iterator tempIt = tempVarVector.begin(); tempIt != tempVarVector.end(); tempIt++)
						{
							if (*tempIt == MidCodeTable[tempStart].part_2) {
								notInVec = false;
								break;
							}
						}
						if (notInVec) {
							tempVarVector.push_back(MidCodeTable[start].part_2);
						}
					}
					if (MidCodeTable[start].part_3.length() >0 && MidCodeTable[start].part_3[0] == '$') {
						bool notInVec = true;
						for (vector<string>::iterator tempIt = tempVarVector.begin(); tempIt != tempVarVector.end(); tempIt++)
						{
							if (*tempIt == MidCodeTable[start].part_3) {
								notInVec = false;
								break;
							}
						}
						if (notInVec) {
							tempVarVector.push_back(MidCodeTable[start].part_3);
						}
					}
					tempStart++;
				}
				//cout << "size =  " << dagIndexMap.size() << endl;
				//for (map<int, DagItem*>::iterator it = dagIndexMap.begin(); it != dagIndexMap.end(); it++)
				//{
				//	cout << "  节点名字：" << it->second->name << "  节点序号：" << it->second->num << "  " << endl;
				//	cout << "节点中的变量名字：" << endl;
				//	//cout << "varItems大小 ： "<<it->second.varItems.size() << endl;
				//	for (vector<string>::iterator  tempIter = it->second->varItems.begin(); tempIter != it->second->varItems.end(); tempIter++)
				//	{
				//		cout << " " << *tempIter ;
				//	}
				//	cout << endl;
				//}
				//for (map<string, int>::iterator it = DagItemTable.begin(); it != DagItemTable.end(); it++)
				//{
				//	cout << "  节点名字：" << it->first << "  节点序号：" << it->second << "  " << endl;
				//}
				//将生成的dag图导出四元式代码
				exportDagToMidcode();
			}
			else {
				tempMidCodeTable[tempmidTablePtr] = MidCodeTable[start];
				start++;
				tempmidTablePtr++;
			}
		}
	}
}
//删除容器中的指定元素
void deleteVarInVector(string varName) {
	for (map<int, DagItem*>::iterator it = dagIndexMap.begin(); it != dagIndexMap.end(); it++)
	{
		for (vector<string>::iterator tempIt = (it->second->varItems).begin(); tempIt != (it->second->varItems).end(); tempIt++)
		{
			//容器中仅有一个该名字，删除后break即可
			if ((*tempIt) == varName) {
				(it->second->varItems).erase(tempIt);
				break;
			}
		}
	}
}
//查找op对应的节点是否在dag图中，若在，返回对应的节点标号；若不在，返回-1
int findInDag(string op, int i, int j) {
	
	for (map<int, DagItem*>::iterator it = dagIndexMap.begin(); it != dagIndexMap.end(); it++)
	{
		if ( (it->second)->name == op && ( (it->second)->lchild != NULL ) && ( (it->second)->rchild != NULL )
			&& (it->second)->lchild->num == i && (it->second)->rchild->num == j) {
			return (it->second)->num;
		}
	}
	return -1;
}
//依据dag图导出四元式代码
void exportDagToMidcode() {
	//存储节点的序号，随后逆序输出进行四元式的生成
	stack<int> VarNumStack;
	int count = 0;
	//临时指针
	DagItem* tempDagItem = NULL;
	//将dag图中的仅含有临时变量的容器设置为一个临时变量名,同时含有变量的容器中的临时变量全部删除（还需保存后面用到的临时变量）
	for (map<int, DagItem*>::iterator it = dagIndexMap.begin(); it != dagIndexMap.end(); it++)
	{
		bool AllIsTemp = true;
		for (vector<string>::iterator tempIt = (it->second)->varItems.begin(); tempIt != (it->second)->varItems.end(); tempIt++)
		{
			if ((*tempIt)[0] != '$') {
				AllIsTemp = false;
				break;
			}
		}
		//将dag图中的仅含有临时变量的容器设置为一个临时变量名
		if (AllIsTemp && !((it->second)->varItems.empty())) {
			vector<string> tempVec;
			for (vector<string>::iterator tempIt = (it->second)->varItems.begin(); tempIt != (it->second)->varItems.end(); tempIt++)
			{
				string tempVarName = *tempIt;
				//寻找后面要用到的临时变量
				for (vector<string>::iterator tempite = tempVarVector.begin(); tempite != tempVarVector.end(); tempite++)
				{
					if (tempVarName == (*tempite)) {
						tempVec.push_back(tempVarName);
					}
				}
			}
			if (!tempVec.empty()) {
				(it->second)->varItems.clear();
				(it->second)->varItems = tempVec;
			}
			else {
				string tempName = (it->second)->varItems[0];
				(it->second)->varItems.clear();
				(it->second)->varItems.push_back(tempName);
			}
		}
		else {
			vector<string> tempVec;
			bool flag = false;
			//含有变量的容器中的临时变量(之后不会用到的)全部删除
			for (vector<string>::iterator tempIt = (it->second)->varItems.begin(); tempIt != (it->second)->varItems.end(); tempIt++)
			{
				if ((*tempIt)[0] != '$') {
					tempVec.push_back((*tempIt));
					flag = true;
				}
				else {
					//寻找后面要用到的临时变量
					for (vector<string>::iterator tempite = tempVarVector.begin(); tempite != tempVarVector.end(); tempite++)
					{
						if ((*tempIt) == (*tempite)) {
							tempVec.push_back((*tempIt));
						}
					}
					flag = true;
				}
			}
			if (flag) {
				(it->second)->varItems = tempVec;
			}
		}
	}
	//处理自己生成的临时变量对应的变量没有之后被修改的情况，先在dag图中查找仅含有自己生成的临时变量的节点（$结尾），且节点表中不存在原来变量的，删除它，用原来变量代替
	for (map<int, DagItem*>::iterator it = dagIndexMap.begin(); it != dagIndexMap.end(); it++)
	{
		DagItem* tempDag = (it->second);
		//临时变量的名字
		string beforeName = tempDag->name;
		if (beforeName[beforeName.length()-1] == '$'){
			//临时变量对应原来变量的名字
			string originName = beforeName.substr(0, beforeName.length() - 1);
			if (DagItemTable.find(originName) == DagItemTable.end()) {
				//此时需要替换
				tempDag->name = originName;
				//cout << "name =========="  <<originName << endl;
				for (vector<string>::iterator tempIt = (tempDag->varItems).begin(); tempIt != (tempDag->varItems).end(); tempIt++)
				{
					if (*tempIt == beforeName) {
						tempDag->varItems.erase(tempIt);
						break;
					}
				}
				tempDag->varItems.push_back(originName);
				DagItemTable[originName] = DagItemTable[beforeName];
				DagItemTable.erase(beforeName);
			}
		}
	}
	//对于处理后的，依据存在自己生成的临时变量，要插入一句四元式
	for (map<int, DagItem*>::iterator it = dagIndexMap.begin(); it != dagIndexMap.end(); it++)
	{
		DagItem* tempDag = (it->second);
		//临时变量的名字
		string beforeName = tempDag->name;
		if (beforeName[beforeName.length() - 1] == '$') {
			//临时变量对应原来变量的名字
			string originName = beforeName.substr(0, beforeName.length() - 1);
			//	//插入一句四元式 , 临时变量 = 原来的变量;
			//cout << "=,\t" << originName << ",\t,\t" << ("$" + originName) << endl;
			//复制到tempMidCodeTable中
			fourExpression tempFour;
			//cout << "tempFour.op" << op << endl;
			tempFour.op = "=";
			tempFour.part_1 = originName;
			tempFour.part_2 = "";
			tempFour.part_3 = ("$" + originName);
			tempMidCodeTable[tempmidTablePtr] = tempFour;
			tempmidTablePtr++;
		}
	}
	//先统计dag图中的中间节点的个数
	for (map<int, DagItem*>::iterator it = dagIndexMap.begin(); it != dagIndexMap.end(); it++)
	{
		if (it->second->lchild == NULL && it->second->rchild == NULL) {
			continue;
		}
		else {
			count++;
		}
	}
	int i = 0;
	while (i < count)
	{
		//先选取没有父节点或者其所有父节点均加入到stack中的一个中间点加入队列中
		for (map<int, DagItem*>::iterator it = dagIndexMap.begin(); it != dagIndexMap.end(); it++)
		{
			//还未加入到队列中且没有父节点或者其所有父节点均加入到stack中
			if ((it->second->lchild != NULL && it->second->rchild != NULL && (!it->second->isInStack)) && (it->second->parent.empty() || parentIsInStack((it->second)))) {
				//获取到该节点对应的结构体
				tempDagItem = (it->second);
				//存储节点编号（输出时反向输出）
				VarNumStack.push(tempDagItem->num);
				tempDagItem->isInStack = true;
				i++;
				break;
			}
		}
		if (i >= count) {
			break;
		}
		if (tempDagItem != NULL) {
			//处理加入的节点的左节点(左节点的左节点等)
			while (parentIsInStack(tempDagItem->lchild))
			{
				VarNumStack.push(tempDagItem->lchild->num);
				//设置其在栈中的属性为true
				tempDagItem->lchild->isInStack = true;
				//替换为加入的左子节点，继续判断左子节点的子节点
				tempDagItem = tempDagItem->lchild;
				//增量的增加
				i++;
			}
		}
	}
	//逆序输出所有的中间节点,同时生成四元式
	while (!VarNumStack.empty())
	{
		int varNum = VarNumStack.top();
		//得到对应的中间节点
		DagItem *tempItem = dagIndexMap[varNum];
		string op = tempItem->name;
		//左节点的名字获取
		string leftVarName = realName(tempItem->lchild->name, tempItem->lchild);
		//右节点的名字获取
		string rightVarName = realName(tempItem->rchild->name, tempItem->rchild);
		//为每个varItems中的变量生成四元式
		for (vector<string>::iterator it = tempItem->varItems.begin(); it != tempItem->varItems.end(); it++) {
			//cout << op << ",\t" << leftVarName << ",\t" << rightVarName << ",\t" << (*it) << endl;
			//复制到tempMidCodeTable中
			fourExpression tempFour;
			//cout << "tempFour.op" << op << endl;
			tempFour.op = op;
			tempFour.part_1 = leftVarName;
			tempFour.part_2 = rightVarName;
			tempFour.part_3 = (*it);
			tempMidCodeTable[tempmidTablePtr] = tempFour;
			tempmidTablePtr++;
		}
		VarNumStack.pop();
	}
	//最后的叶节点，如果存在varItems中有与叶节点名字不同的变量，还需要设置该变量与叶节点对应的名字相等
	for (map<int, DagItem*>::iterator it = dagIndexMap.begin(); it != dagIndexMap.end(); it++)
	{
		if (it->second->lchild == NULL && it->second->rchild == NULL) {
			DagItem *tempItem = it->second;
			//得到节点的名字
			string name = tempItem->name;
			//包含其他的名字(考虑了size为0的情况)(也包含一个节点内容为临时变量的情况)
			if (tempItem->varItems.size() >= 1) {
				for (vector<string>::iterator it = tempItem->varItems.begin(); it != tempItem->varItems.end(); it++) {
					if ((*it) != name) {
						string leftName = realName(name, tempItem);
						//cout << "=,\t" << leftName << ",\t,\t" << (*it) << endl;
						//复制到tempMidCodeTable中
						fourExpression tempFour;
						tempFour.op = "=";
						tempFour.part_1 = leftName;
						tempFour.part_2 = "";
						tempFour.part_3 = (*it);
						tempMidCodeTable[tempmidTablePtr] = tempFour;
						tempmidTablePtr++;
					}
				}
			}
		}
	}
	//每次基本块处理之后，需要清空节点表以及DAG图
	dagIndexMap.clear();
	DagItemTable.clear();
	//清空临时变量表
	tempVarVector.clear();
}
//得到叶节点的真正变量名字（包含常数）
string realName(string str, DagItem *tempItem) {
	//为常数的情况,直接返回常数
	if ((str != "+" && str != "-") && (str[0] == '+' || str[0] == '-' || isdigit(str[0]))) {
		return str;
	}
	//运算符 加减乘除 (中间节点，其记录名字的容器不可能为空)，则返回该tempItem的varItems中的第一个元素
	else if (str == "+" || str == "-" || str == "*" || str == "/" || str == "getArr") {
		if (!tempItem->varItems.empty()) {
			/*for (vector<string>::iterator i = tempItem->varItems.begin(); i != tempItem->varItems.end(); i++)
			{
				cout << "asdasdsadsa" << *i << endl;
			}*/
			return tempItem->varItems[0];
		}
	}
	//临时变量名字,将$转换到前面
	else if (str.length() >= 1 && str[str.length()-1] == '$') {
		return "$" + str.substr(0, str.length() - 1);
	}
	//变量名字
	else {
		return str;
	}
	return str;
}
//若传入的子节点的父节点均在stack中，则返回true；否则返回false
//若为叶结点，直接返回false
bool parentIsInStack(DagItem* tempDagItem) {
	//cout << tempDagItem << endl;
	if (tempDagItem->lchild == NULL && tempDagItem->rchild == NULL) {
		return false;
	}
	for (vector<DagItem *>::iterator it = tempDagItem->parent.begin(); it != tempDagItem->parent.end(); it++)
	{
		//若有没有加入栈中的父节点，则返回false
		if (!((*it)->isInStack)) {
			return false;
		}
	}
	return true;
}
//划分基本块
void divideBasicBlock() {
	string op;
	//基本块的划分，将genLabel,jne,jump,ret 作为基本块的最后一句
	while (MidCodeTable[midTablePtr].op != "end")
	{
		int begin = midTablePtr;
		op = MidCodeTable[midTablePtr].op;
		while (op != "genLabel" && op != "jne" && op != "jump"  && op != "ret") {
			//若读到end则跳出循环
			if (op == "end") break;
			midTablePtr++;
			op = MidCodeTable[midTablePtr].op;;
		}
		//到了基本块的最后一句，基本块属性设置(若后面直接跟着end则直接添加end到该基本块即可)
		int end = MidCodeTable[midTablePtr + 1].op == "end" ? midTablePtr + 1 : midTablePtr;
		BasicBlock basic;
		basic.Block.beginIndex = begin;
		basic.Block.endIndex = end;
		//还需设置前驱与后继基本块
		basicBlockVector.push_back(basic);
		//处理到end（此时已经将end添加到了上面的基本块中）直接return即可
		if (MidCodeTable[midTablePtr].op == "end") return;
		midTablePtr++;
	}

}

//将+ - * / 指令的var1 var2均为常数的指令进行运算，合并常数
//+, r1, r2, r3， 若 r1 r2为常数，则合并为 = , (r1+r2), , r3
//合并常数运算
void mergeConstant() {
	int i, num_1, num_2, result;
	string op = "", part_1 = "", part_2 = "";
	for (i = 0; i < MidCodeTableTop; i++)
	{
		op = MidCodeTable[i].op;
		if (op == "+" || op == "-" || op == "*" || op == "/") {
			part_1 = MidCodeTable[i].part_1;
			part_2 = MidCodeTable[i].part_2;
			if (isNumber(part_1) && isNumber(part_2)) {
				//cout << "数字 " << part_1 << "\t" << part_2 << endl;
				num_1 = atoi(part_1.c_str());
				num_2 = atoi(part_2.c_str());
				result = op == "+" ? num_1 + num_2 : op == "-" ? num_1 - num_2 :
					op == "*" ? num_1 * num_2 : op == "/" ? num_1 / num_2 : 0;
				//更改四元式
				MidCodeTable[i].op = "=";
				MidCodeTable[i].part_1 = convertNumToString(result);
				MidCodeTable[i].part_2 = "";
			}
		}
	}
}
//删除冗余标签，将相邻的genLabel标签除了最后一个生成标签外，其余均删除，并且将
//跳转到被删除标签的指令的var3标签换为最后一个标签
void deleteLabel() { 
	int i = 0, j, count = 0;
	bool flag = false;
	vector<string> labelVector;
	string label = "";
	while (i < MidCodeTableTop)
	{
		if (MidCodeTable[i].op == "genLabel") {
			labelVector.clear();
			count = 0;
			j = i + 1;
			label = MidCodeTable[i].part_3;
			while (j < MidCodeTableTop &&  MidCodeTable[j].op == "genLabel")
			{
				flag = true;
				labelVector.push_back(MidCodeTable[j].part_3);
				count++;
				j++;
			}
			//count此时存储要替换的label的个数，将四元式表中从 j 开始一直到 MidCodeTableTop - 1 替换到 j - count的位置即可
			if (flag)
			{
				while (j < MidCodeTableTop)
				{
					MidCodeTable[j - count].op = MidCodeTable[j].op;
					MidCodeTable[j - count].part_1 = MidCodeTable[j].part_1;
					MidCodeTable[j - count].part_2 = MidCodeTable[j].part_2;
					MidCodeTable[j - count].part_3 = MidCodeTable[j].part_3;
					j++;
				}
				//更新MidCodeTableTop所指的位置
				MidCodeTableTop -= count;
				flag = false;
			}
			//进行替换，将part_3为删除后的容器集合中的某个标签元素的part_3替换为label
			//仅jump和jne需要替换
			for (int k = 0; k < MidCodeTableTop; k++)
			{
				if (MidCodeTable[k].op == "jump" || MidCodeTable[k].op == "jne") {
					string tempLabel = MidCodeTable[k].part_3;
					//vector<string>::iterator it = find(labelVector.begin(), labelVector.end(), tempLabel);
					//vector<string>::iterator it = findInVector(labelVector, tempLabel);
					//若找到则替换
					//if (it != labelVector.end()) {
						//MidCodeTable[k].part_3 = label;
					//}
					for (vector<string>::iterator it = labelVector.begin(); it != labelVector.end(); it++)
					{
						if ((*it) == tempLabel) {
							MidCodeTable[k].part_3 = label;
							break;
						}
					}
				}
			}
		}
		i++;
	}
}

//void deletePublicExpression() {
//	int i, j, k;
//	//从当前索引开始，寻找连续的运算语句，遍历，合并公共表达式
//	for ( i = midTablePtr; MidCodeTable[i].op == "=" || MidCodeTable[i].op == "+" || 
//		MidCodeTable[i].op == "-" || MidCodeTable[i].op == "*"|| MidCodeTable[i].op == "/"; i++)
//	{
//		if (i >= MidCodeTableTop) {
//			return;
//		}
//		//对于临时变量的修改
//		if (MidCodeTable[i].part_3[0] == '$') {
//			for (j = i + 1; MidCodeTable[j].op == "=" || MidCodeTable[j].op == "+" ||
//				MidCodeTable[j].op == "-" || MidCodeTable[j].op == "*" || MidCodeTable[j].op == "/"; j++)
//			{
//				if (j >= MidCodeTableTop) {
//					return;
//				}
//				//op part_1 part_2 均相同 part_3 为临时变量，则后面用到part_3的均修改为之前i对应的临时变量即可
//				if (MidCodeTable[i].op == MidCodeTable[j].op && MidCodeTable[i].part_1 == MidCodeTable[j].part_1 && 
//					MidCodeTable[i].part_2 == MidCodeTable[j].part_2 && MidCodeTable[j].part_3[0] == '$') {
//					//向后遍历,替换后面用的j的part_3为之前的i对应的临时变量
//					for ( k = j + 1; MidCodeTable[k].op == "=" || MidCodeTable[k].op == "+" ||
//						MidCodeTable[k].op == "-" || MidCodeTable[k].op == "*" || MidCodeTable[k].op == "/"; k++)
//					{
//						if (k >= MidCodeTableTop) {
//							return;
//						}
//						if (MidCodeTable[k].part_1 == MidCodeTable[j].part_3) {
//							MidCodeTable[k].part_1 = MidCodeTable[i].part_3;
//						}
//						if (MidCodeTable[k].part_2 == MidCodeTable[j].part_3) {
//							MidCodeTable[k].part_2 = MidCodeTable[i].part_3;
//						}
//					}
//					//删除j那条指令
//					for (int index = j + 1; index < MidCodeTableTop; index++)
//					{
//						MidCodeTable[index - 1].op = MidCodeTable[index].op;
//						MidCodeTable[index - 1].part_1 = MidCodeTable[index].part_1;
//						MidCodeTable[index - 1].part_2 = MidCodeTable[index].part_2;
//						MidCodeTable[index - 1].part_3 = MidCodeTable[index].part_3;
//
//					}
//					MidCodeTableTop--;
//					j--;
//				}
//			}
//		}
//		
//	}
//}

//判断输入的字符串是否为数字
bool isNumber(string str) {
	int i = 0;
	if (str[i] == '+' || str[i] == '-') {
		i++;
	}
	for (; i < str.length(); i++)
	{
		if (!isdigit(str[i])) {
			return false;
		}
	}
	return true;
}

