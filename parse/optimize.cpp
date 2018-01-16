//
//  optimize.cpp
//  C0compiler
//
#include "globalData.h"
int midTablePtr = 0;  //��Ԫʽ���ָ�룬�� 0 ��ʼһֱ������ MidCodeTableTop
int tempmidTablePtr = 0;
fourExpression tempMidCodeTable[MAXMIDECODESIZE];
vector<BasicBlock> basicBlockVector;
//�ڵ����������Ϊ������Ӧ�Ľڵ����Ϊֵ
map<string, int> DagItemTable;
//�������洢ͼ��ÿ���ڵ�
map<int, DagItem*> dagIndexMap;
//�洢֮��Ҫ�õ�����ʱ����
vector<string> tempVarVector;
//ɨ�����е���Ԫ���ʽ
void optAllExpression() {
	//ɨ��
	//�Ȼ��ֻ�����
	while (midTablePtr < MidCodeTableTop)
	{
		//����func��λ��,������ӳ�ȥ��ʼ��ȫ�ֳ����Լ�����������ʼ
		if (MidCodeTable[midTablePtr].op == "func") {
			//������endΪֹ
			divideBasicBlock();
			midTablePtr++;
		}
		//ֱ����ȡ����һ��func��λ�ã���ȫ�ֳ����Լ������������뵽��ʱ��Ԫʽ����
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
	//���ݻ��ֵĻ�������й����ӱ��ʽ��ɾ��
	dag();
	//�����µ���Ԫʽ������MidCodeTable�У���ָ��ΪMidCodeTableTop
	MidCodeTableTop = 0;
	for (int i = 0; i < tempmidTablePtr; i++)
	{
		fourExpression tempFour = tempMidCodeTable[i];
		//outOptMidCode << "\t" << tempFour.op << ",\t" << tempFour.part_1 << ",\t" << tempFour.part_2 << ",\t" << tempFour.part_3 << endl;
		MidCodeTable[MidCodeTableTop] = tempFour;
		MidCodeTableTop++;
	}
	//�����ϲ�
	mergeConstant();
	//ɾ��������루�����ǩ��
	deleteLabel();
	//д���Ż������Ԫʽ���ļ�
	for (int i = 0; i < MidCodeTableTop; i++)
	{
		fourExpression tempFour = MidCodeTable[i];
		outOptMidCode << "\t" << tempFour.op << ",\t" << tempFour.part_1 << ",\t" << tempFour.part_2 << ",\t" << tempFour.part_3 << endl;
	}
}
//ɾ�������ӱ��ʽ
//�Ի�����Ϊ��λ��������ȡ��+ - * / = Ϊop����Ԫʽ�����й����ӱ��ʽ��ɾ��
void dag() {
	for (int index = 0; index < basicBlockVector.size(); index++)
	{
		//��ÿһ������д���dagͼ����
		BasicBlock basic = basicBlockVector[index];
		//����������+ - * / = ���
		int start = basic.Block.beginIndex;
		int end = basic.Block.endIndex;
		//��ʼ����������еı��ʽ(����)
		//�������õ�����ʱ��������������
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
					//����ÿһ�����ʽ
					int left_index, right_index = -1;
					// + x y z���� ע�� = x,,z  yΪ�գ���������
					if (start >= end) break;
					//���ڽڵ����Ѱ�� x ����û���ҵ�,���½�һ��Ҷ�ڵ㣬�������ΪitemCount�����Ϊx(��xΪ��������
					//���Ϊx$)���ڵ��������һ��(x$,i)
					part_1 = MidCodeTable[start].part_1;
					string varName = "";
					//û�ҵ�x
					if (DagItemTable.find(part_1) == DagItemTable.end()) {
						varName = part_1;
						if (!(varName[0] == '+' || varName[0] == '-' || isdigit(varName[0]))) {
							varName = varName + "$";
						}
						if (DagItemTable.find(varName) == DagItemTable.end()) {
							//����(x,i)/(x$,i)
							DagItemTable[varName] = itemCount;
							//����dagͼ�еĸýڵ�
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
							//�õ���Ӧ�Ľڵ�
							leftChild = dagIndexMap[left_index];
						}
					}
					//�ҵ�x
					else {
						left_index = DagItemTable[part_1];
						//cout << "find " << part_1 << left_index << endl;
						//�õ���Ӧ�Ľڵ�
						leftChild = dagIndexMap[left_index];
						//cout << "leftChild ========= " << leftChild << endl;
					}
					//�ж�y
					part_2 = MidCodeTable[start].part_2;
					varName = "";
					if (part_2 != "") {
						//û�ҵ�y
						if (DagItemTable.find(part_2) == DagItemTable.end()) {
							//�ж��ǲ��Ǳ�����
							varName = part_2;
							if (!(varName[0] == '+' || varName[0] == '-' || isdigit(varName[0]))) {
								varName = varName + "$";
							}
							if (DagItemTable.find(varName) == DagItemTable.end()) {
								//����(y$,i)
								DagItemTable[varName] = itemCount;
								//����dagͼ�еĸýڵ�
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
								//�õ���Ӧ�Ľڵ�
								rightChild = dagIndexMap[right_index];
							}
						}
						//�ҵ�y
						else {
							right_index = DagItemTable[part_2];
							//�õ���Ӧ�Ľڵ�
							/*cout << "leftChild: " << leftChild << endl;
							cout << "rightChild: " << rightChild << endl;
							cout << "right_index: " << right_index << endl;
							cout << "dagIndexMap[right_index]: " << dagIndexMap[right_index] << endl;*/
							rightChild = dagIndexMap[right_index];
							//cout<<rightChild<<endl;
						}
					}
					//�ж������ op
					op = MidCodeTable[start].op;
					int result;
					//op ��Ϊ = ��������ж����ҽڵ��Լ��м��������
					if (op != "=") {
						//����ҵ���
						if ((result = findInDag(op, left_index, right_index)) != -1) {
							//��ʱresult�Ѿ���¼�˽ڵ��,���ҵ��Ľڵ㸳ֵ��parent
							parent = dagIndexMap[result];
						}
						//δ�ҵ�
						else {
							//����dagͼ�е��м�ڵ㣬name�洢�����
							parent->name = op;
							//parent->name = op.c_str();
							//strcpy(parent->name, varName.c_str());
							parent->num = itemCount;
							parent->lchild = leftChild;
							parent->rchild = rightChild;
							parent->isInStack = false;
							//��ڵ㸸�ڵ����parent
							leftChild->parent.push_back(parent);
							//�ҽڵ㸸�ڵ����parent
							rightChild->parent.push_back(parent);
							result = itemCount;
							
							itemCount++;
						}
					}
					//op Ϊ = �����,�õ�x��Ӧ�Ľڵ��ż���
					else {
						result = left_index;
						parent = leftChild;
					}
					//�ж�z
					part_3 = MidCodeTable[start].part_3;
					//û�ҵ�z
					if (DagItemTable.find(part_3) == DagItemTable.end()) {
						//�½�һ��
						DagItemTable[part_3] = result;
					}
					else {
						//�޸�ԭ��z��Ӧ�Ľڵ�����
						DagItemTable[part_3] = result;
					}
					//ɾ������z�Ľڵ��vector�еĸ�Ԫ��
					deleteVarInVector(part_3);
					//���z���ڵ��Ӧ��vector��
					//strcpy(tempName, part_3.c_str());
					parent->varItems.push_back(part_3);
					//�������ڵ㱣�浽map��Ӧ��������
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
				//	cout << "  �ڵ����֣�" << it->second->name << "  �ڵ���ţ�" << it->second->num << "  " << endl;
				//	cout << "�ڵ��еı������֣�" << endl;
				//	//cout << "varItems��С �� "<<it->second.varItems.size() << endl;
				//	for (vector<string>::iterator  tempIter = it->second->varItems.begin(); tempIter != it->second->varItems.end(); tempIter++)
				//	{
				//		cout << " " << *tempIter ;
				//	}
				//	cout << endl;
				//}
				//for (map<string, int>::iterator it = DagItemTable.begin(); it != DagItemTable.end(); it++)
				//{
				//	cout << "  �ڵ����֣�" << it->first << "  �ڵ���ţ�" << it->second << "  " << endl;
				//}
				//�����ɵ�dagͼ������Ԫʽ����
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
//ɾ�������е�ָ��Ԫ��
void deleteVarInVector(string varName) {
	for (map<int, DagItem*>::iterator it = dagIndexMap.begin(); it != dagIndexMap.end(); it++)
	{
		for (vector<string>::iterator tempIt = (it->second->varItems).begin(); tempIt != (it->second->varItems).end(); tempIt++)
		{
			//�����н���һ�������֣�ɾ����break����
			if ((*tempIt) == varName) {
				(it->second->varItems).erase(tempIt);
				break;
			}
		}
	}
}
//����op��Ӧ�Ľڵ��Ƿ���dagͼ�У����ڣ����ض�Ӧ�Ľڵ��ţ������ڣ�����-1
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
//����dagͼ������Ԫʽ����
void exportDagToMidcode() {
	//�洢�ڵ����ţ�����������������Ԫʽ������
	stack<int> VarNumStack;
	int count = 0;
	//��ʱָ��
	DagItem* tempDagItem = NULL;
	//��dagͼ�еĽ�������ʱ��������������Ϊһ����ʱ������,ͬʱ���б����������е���ʱ����ȫ��ɾ�������豣������õ�����ʱ������
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
		//��dagͼ�еĽ�������ʱ��������������Ϊһ����ʱ������
		if (AllIsTemp && !((it->second)->varItems.empty())) {
			vector<string> tempVec;
			for (vector<string>::iterator tempIt = (it->second)->varItems.begin(); tempIt != (it->second)->varItems.end(); tempIt++)
			{
				string tempVarName = *tempIt;
				//Ѱ�Һ���Ҫ�õ�����ʱ����
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
			//���б����������е���ʱ����(֮�󲻻��õ���)ȫ��ɾ��
			for (vector<string>::iterator tempIt = (it->second)->varItems.begin(); tempIt != (it->second)->varItems.end(); tempIt++)
			{
				if ((*tempIt)[0] != '$') {
					tempVec.push_back((*tempIt));
					flag = true;
				}
				else {
					//Ѱ�Һ���Ҫ�õ�����ʱ����
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
	//�����Լ����ɵ���ʱ������Ӧ�ı���û��֮���޸ĵ����������dagͼ�в��ҽ������Լ����ɵ���ʱ�����Ľڵ㣨$��β�����ҽڵ���в�����ԭ�������ģ�ɾ��������ԭ����������
	for (map<int, DagItem*>::iterator it = dagIndexMap.begin(); it != dagIndexMap.end(); it++)
	{
		DagItem* tempDag = (it->second);
		//��ʱ����������
		string beforeName = tempDag->name;
		if (beforeName[beforeName.length()-1] == '$'){
			//��ʱ������Ӧԭ������������
			string originName = beforeName.substr(0, beforeName.length() - 1);
			if (DagItemTable.find(originName) == DagItemTable.end()) {
				//��ʱ��Ҫ�滻
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
	//���ڴ����ģ����ݴ����Լ����ɵ���ʱ������Ҫ����һ����Ԫʽ
	for (map<int, DagItem*>::iterator it = dagIndexMap.begin(); it != dagIndexMap.end(); it++)
	{
		DagItem* tempDag = (it->second);
		//��ʱ����������
		string beforeName = tempDag->name;
		if (beforeName[beforeName.length() - 1] == '$') {
			//��ʱ������Ӧԭ������������
			string originName = beforeName.substr(0, beforeName.length() - 1);
			//	//����һ����Ԫʽ , ��ʱ���� = ԭ���ı���;
			//cout << "=,\t" << originName << ",\t,\t" << ("$" + originName) << endl;
			//���Ƶ�tempMidCodeTable��
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
	//��ͳ��dagͼ�е��м�ڵ�ĸ���
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
		//��ѡȡû�и��ڵ���������и��ڵ�����뵽stack�е�һ���м����������
		for (map<int, DagItem*>::iterator it = dagIndexMap.begin(); it != dagIndexMap.end(); it++)
		{
			//��δ���뵽��������û�и��ڵ���������и��ڵ�����뵽stack��
			if ((it->second->lchild != NULL && it->second->rchild != NULL && (!it->second->isInStack)) && (it->second->parent.empty() || parentIsInStack((it->second)))) {
				//��ȡ���ýڵ��Ӧ�Ľṹ��
				tempDagItem = (it->second);
				//�洢�ڵ��ţ����ʱ���������
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
			//�������Ľڵ����ڵ�(��ڵ����ڵ��)
			while (parentIsInStack(tempDagItem->lchild))
			{
				VarNumStack.push(tempDagItem->lchild->num);
				//��������ջ�е�����Ϊtrue
				tempDagItem->lchild->isInStack = true;
				//�滻Ϊ��������ӽڵ㣬�����ж����ӽڵ���ӽڵ�
				tempDagItem = tempDagItem->lchild;
				//����������
				i++;
			}
		}
	}
	//����������е��м�ڵ�,ͬʱ������Ԫʽ
	while (!VarNumStack.empty())
	{
		int varNum = VarNumStack.top();
		//�õ���Ӧ���м�ڵ�
		DagItem *tempItem = dagIndexMap[varNum];
		string op = tempItem->name;
		//��ڵ�����ֻ�ȡ
		string leftVarName = realName(tempItem->lchild->name, tempItem->lchild);
		//�ҽڵ�����ֻ�ȡ
		string rightVarName = realName(tempItem->rchild->name, tempItem->rchild);
		//Ϊÿ��varItems�еı���������Ԫʽ
		for (vector<string>::iterator it = tempItem->varItems.begin(); it != tempItem->varItems.end(); it++) {
			//cout << op << ",\t" << leftVarName << ",\t" << rightVarName << ",\t" << (*it) << endl;
			//���Ƶ�tempMidCodeTable��
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
	//����Ҷ�ڵ㣬�������varItems������Ҷ�ڵ����ֲ�ͬ�ı���������Ҫ���øñ�����Ҷ�ڵ��Ӧ���������
	for (map<int, DagItem*>::iterator it = dagIndexMap.begin(); it != dagIndexMap.end(); it++)
	{
		if (it->second->lchild == NULL && it->second->rchild == NULL) {
			DagItem *tempItem = it->second;
			//�õ��ڵ������
			string name = tempItem->name;
			//��������������(������sizeΪ0�����)(Ҳ����һ���ڵ�����Ϊ��ʱ���������)
			if (tempItem->varItems.size() >= 1) {
				for (vector<string>::iterator it = tempItem->varItems.begin(); it != tempItem->varItems.end(); it++) {
					if ((*it) != name) {
						string leftName = realName(name, tempItem);
						//cout << "=,\t" << leftName << ",\t,\t" << (*it) << endl;
						//���Ƶ�tempMidCodeTable��
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
	//ÿ�λ����鴦��֮����Ҫ��սڵ���Լ�DAGͼ
	dagIndexMap.clear();
	DagItemTable.clear();
	//�����ʱ������
	tempVarVector.clear();
}
//�õ�Ҷ�ڵ�������������֣�����������
string realName(string str, DagItem *tempItem) {
	//Ϊ���������,ֱ�ӷ��س���
	if ((str != "+" && str != "-") && (str[0] == '+' || str[0] == '-' || isdigit(str[0]))) {
		return str;
	}
	//����� �Ӽ��˳� (�м�ڵ㣬���¼���ֵ�����������Ϊ��)���򷵻ظ�tempItem��varItems�еĵ�һ��Ԫ��
	else if (str == "+" || str == "-" || str == "*" || str == "/" || str == "getArr") {
		if (!tempItem->varItems.empty()) {
			/*for (vector<string>::iterator i = tempItem->varItems.begin(); i != tempItem->varItems.end(); i++)
			{
				cout << "asdasdsadsa" << *i << endl;
			}*/
			return tempItem->varItems[0];
		}
	}
	//��ʱ��������,��$ת����ǰ��
	else if (str.length() >= 1 && str[str.length()-1] == '$') {
		return "$" + str.substr(0, str.length() - 1);
	}
	//��������
	else {
		return str;
	}
	return str;
}
//��������ӽڵ�ĸ��ڵ����stack�У��򷵻�true�����򷵻�false
//��ΪҶ��㣬ֱ�ӷ���false
bool parentIsInStack(DagItem* tempDagItem) {
	//cout << tempDagItem << endl;
	if (tempDagItem->lchild == NULL && tempDagItem->rchild == NULL) {
		return false;
	}
	for (vector<DagItem *>::iterator it = tempDagItem->parent.begin(); it != tempDagItem->parent.end(); it++)
	{
		//����û�м���ջ�еĸ��ڵ㣬�򷵻�false
		if (!((*it)->isInStack)) {
			return false;
		}
	}
	return true;
}
//���ֻ�����
void divideBasicBlock() {
	string op;
	//������Ļ��֣���genLabel,jne,jump,ret ��Ϊ����������һ��
	while (MidCodeTable[midTablePtr].op != "end")
	{
		int begin = midTablePtr;
		op = MidCodeTable[midTablePtr].op;
		while (op != "genLabel" && op != "jne" && op != "jump"  && op != "ret") {
			//������end������ѭ��
			if (op == "end") break;
			midTablePtr++;
			op = MidCodeTable[midTablePtr].op;;
		}
		//���˻���������һ�䣬��������������(������ֱ�Ӹ���end��ֱ�����end���û����鼴��)
		int end = MidCodeTable[midTablePtr + 1].op == "end" ? midTablePtr + 1 : midTablePtr;
		BasicBlock basic;
		basic.Block.beginIndex = begin;
		basic.Block.endIndex = end;
		//��������ǰ�����̻�����
		basicBlockVector.push_back(basic);
		//����end����ʱ�Ѿ���end��ӵ�������Ļ������У�ֱ��return����
		if (MidCodeTable[midTablePtr].op == "end") return;
		midTablePtr++;
	}

}

//��+ - * / ָ���var1 var2��Ϊ������ָ��������㣬�ϲ�����
//+, r1, r2, r3�� �� r1 r2Ϊ��������ϲ�Ϊ = , (r1+r2), , r3
//�ϲ���������
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
				//cout << "���� " << part_1 << "\t" << part_2 << endl;
				num_1 = atoi(part_1.c_str());
				num_2 = atoi(part_2.c_str());
				result = op == "+" ? num_1 + num_2 : op == "-" ? num_1 - num_2 :
					op == "*" ? num_1 * num_2 : op == "/" ? num_1 / num_2 : 0;
				//������Ԫʽ
				MidCodeTable[i].op = "=";
				MidCodeTable[i].part_1 = convertNumToString(result);
				MidCodeTable[i].part_2 = "";
			}
		}
	}
}
//ɾ�������ǩ�������ڵ�genLabel��ǩ�������һ�����ɱ�ǩ�⣬�����ɾ�������ҽ�
//��ת����ɾ����ǩ��ָ���var3��ǩ��Ϊ���һ����ǩ
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
			//count��ʱ�洢Ҫ�滻��label�ĸ���������Ԫʽ���д� j ��ʼһֱ�� MidCodeTableTop - 1 �滻�� j - count��λ�ü���
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
				//����MidCodeTableTop��ָ��λ��
				MidCodeTableTop -= count;
				flag = false;
			}
			//�����滻����part_3Ϊɾ��������������е�ĳ����ǩԪ�ص�part_3�滻Ϊlabel
			//��jump��jne��Ҫ�滻
			for (int k = 0; k < MidCodeTableTop; k++)
			{
				if (MidCodeTable[k].op == "jump" || MidCodeTable[k].op == "jne") {
					string tempLabel = MidCodeTable[k].part_3;
					//vector<string>::iterator it = find(labelVector.begin(), labelVector.end(), tempLabel);
					//vector<string>::iterator it = findInVector(labelVector, tempLabel);
					//���ҵ����滻
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
//	//�ӵ�ǰ������ʼ��Ѱ��������������䣬�������ϲ��������ʽ
//	for ( i = midTablePtr; MidCodeTable[i].op == "=" || MidCodeTable[i].op == "+" || 
//		MidCodeTable[i].op == "-" || MidCodeTable[i].op == "*"|| MidCodeTable[i].op == "/"; i++)
//	{
//		if (i >= MidCodeTableTop) {
//			return;
//		}
//		//������ʱ�������޸�
//		if (MidCodeTable[i].part_3[0] == '$') {
//			for (j = i + 1; MidCodeTable[j].op == "=" || MidCodeTable[j].op == "+" ||
//				MidCodeTable[j].op == "-" || MidCodeTable[j].op == "*" || MidCodeTable[j].op == "/"; j++)
//			{
//				if (j >= MidCodeTableTop) {
//					return;
//				}
//				//op part_1 part_2 ����ͬ part_3 Ϊ��ʱ������������õ�part_3�ľ��޸�Ϊ֮ǰi��Ӧ����ʱ��������
//				if (MidCodeTable[i].op == MidCodeTable[j].op && MidCodeTable[i].part_1 == MidCodeTable[j].part_1 && 
//					MidCodeTable[i].part_2 == MidCodeTable[j].part_2 && MidCodeTable[j].part_3[0] == '$') {
//					//������,�滻�����õ�j��part_3Ϊ֮ǰ��i��Ӧ����ʱ����
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
//					//ɾ��j����ָ��
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

//�ж�������ַ����Ƿ�Ϊ����
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

