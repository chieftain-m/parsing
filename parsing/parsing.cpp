#include<string>
#include<vector>
#include<set>
#include<map>
#include<algorithm>
#include<iostream>
#include<iomanip>
#include<fstream>
using namespace std;
const int maxn = 1005;

ifstream lex_file;
ifstream in_file;
ofstream out_file;

struct node {//每个文法
	char left;//左侧
	string right;//右侧
};

map <char, int>Vn;//记录非终结符
map <char, int>Vt; //记录终结符 
vector <node> input;//输入文法集合
vector <char> non_c;//非终结符集合
vector <char> ter_c;//终结符集合
vector <char> sta, vs;
set <char> first[maxn];
set <char> follow[maxn];
int hash_non[maxn]; //非终结符是否存在
int hash_ter[maxn];
int tableMap[maxn][maxn];
int n;
/*初始化*/
void Init() {
	in_file.open("D:\\1\\语法分析_文法.txt");
	lex_file.open("D:\\1\\词法分析_output.txt");
	out_file.open("D:\\1\\output.txt");
	memset(hash_non, 0, sizeof(hash_non));
	memset(hash_ter, 0, sizeof(hash_ter));
	memset(tableMap, -1, sizeof(tableMap));
}
/*读文法*/
void Read() {
	in_file >> n;
	for (int i = 0; i < n; i++) {
		string s;//定义输入文法
		node temp;//定义保存文法
		in_file >> s;//读入文法
		temp.left = s[0];//处理左侧
		if (!Vn.count(s[0])) {//Vn中有s[0]返回1，否则返回（左侧的非终结符是否是第一次出现）
			Vn[s[0]] = non_c.size();
			hash_non[non_c.size()] = hash_ter[non_c.size()] = 1;//若该非终结符出现则赋1
			non_c.push_back(s[0]);//将非终结符加入数组
		}
		for (int j = 3; j < s.length(); j++) {
			if (!isupper(s[j]) && !Vt.count(s[j])) {//Vt中有s[0]返回1，否则返回（右侧的终结符是否是第一次出现）
				Vt[s[j]] = ter_c.size();
				if (s[j] != '@')ter_c.push_back(s[j]);//将终结符加入数组，空除外
			}
			temp.right += s[j];//右侧文法
		}
		input.push_back(temp);//将处理后的文法temp加入input数组保存
	}
	ter_c.push_back('#');
	Vt['#'] = ter_c.size() - 1;
}
/*获取first集*/
void getFirst(char temp) {
	if (!hash_non[Vn[temp]]) return;//若该非终结符不存在
	for (int i = 0; i < n; i++) {//遍历文法
		if (input[i].left == temp) {//输入文法的左侧为该非终结符
			if (!Vn.count(input[i].right[0])) {//右侧第一个不是非终结符
				first[Vn[temp]].insert(input[i].right[0]);//first集中插入该终结符
			}
			else {
				int num = 0;
				for (int j = 0; j < input[i].right.length(); j++) {//遍历文法右侧
					if (!Vn.count(input[i].right[j])) {//右侧第j个不是非终结符
						first[Vn[temp]].insert(input[i].right[j]);//first集中插入该终结符
						break;
					}
					getFirst(input[i].right[j]);//若是非终结符则循环该非终结符
					bool flag = 0;
					for (set<char>::iterator it = first[Vn[input[i].right[j]]].begin(); it != first[Vn[input[i].right[j]]].end(); it++) {
						if (*it == '@')flag = 1;//能推导出空
						else first[Vn[temp]].insert(*it);//不能推导出空
					}
					if (!flag) break;//不能推导出空，结束
					else {//能推导出空，继续遍历
						num += flag; flag = 0;
					}
				}
				if (num == input[i].right.length()) {//全都为空
					first[Vn[temp]].insert('@');
				}
			}
		}
	}
	hash_non[Vn[temp]] = 0;//将非终结符表清空
}
void showFirst() {
	out_file << "first集\n";
	for (int i = 0; i < non_c.size(); i++) {
		out_file << non_c[i] << ":";
		for (set<char>::iterator it = first[Vn[non_c[i]]].begin(); it != first[Vn[non_c[i]]].end(); it++) {
			out_file << *it << " ";
		}
		out_file << endl;
	}
}
/*获取follow集*/
void getFollow(char temp) {
	if (!hash_ter[Vn[temp]])return;//若该非终结符不存在
	for (int i = 0; i < n; i++) {
		int index = -1;
		for (int j = 0; j < input[i].right.length(); j++) {//遍历整个右侧
			if (input[i].right[j] == temp) {//找到第一个与当前相同的非终结符
				index = j; break;//index为当前位置
			}
		}
		if (index > -1 && index < input[i].right.length() - 1) {//如果当前非终结符存在且不是文法末尾的非终结符
			char nxt = input[i].right[index + 1];//非终结符后一位
			if (!Vn.count(nxt)) {//若为终结符
				follow[Vn[temp]].insert(nxt);//直接将终结符加入follow集中
			}
			else {//若不为终结符
				int flag = 0, j;
				for (j = index + 1; j < input[i].right.length(); j++) {//遍历右侧当前非终结符之后的符号
					nxt = input[i].right[j];//非终结符后j位
					if (!Vn.count(nxt)) {//若为终结符
						follow[Vn[temp]].insert(nxt);//插入follow集中
						break;//跳出循环
					}
					//若不是非终结符，加入当前非终结符的first集-空
					for (set<char>::iterator it = first[Vn[nxt]].begin(); it != first[Vn[nxt]].end(); it++) {
						if (*it == '@')flag = 1;//若为不加入
						else follow[Vn[temp]].insert(*it);//加入follow集
					}
					if (!flag)break;//若为空继续遍历
				}
				if (j == input[i].right.length() && flag) {//如果当前非终结符是末尾
					getFollow(input[i].left);//获取文法左侧非终结符的follow集
					for (set<char>::iterator it = follow[Vn[input[i].left]].begin(); it != follow[Vn[input[i].left]].end(); it++) {
						follow[Vn[temp]].insert(*it);
					}
				}
			}
		}
		else if (index == input[i].right.length() - 1 && input[i].left != input[i].right[index]) {//如果当前非终结符是文法末尾的非终结符
			getFollow(input[i].left);
			for (set<char>::iterator it = follow[Vn[input[i].left]].begin(); it != follow[Vn[input[i].left]].end(); it++) {
				follow[Vn[temp]].insert(*it);
			}
		}
	}
	hash_ter[Vn[temp]] = 0;
}
void showFollow() {
	out_file << "follow集\n";
	for (int i = 0; i < non_c.size(); i++) {
		out_file << non_c[i] << ":";
		for (set<char>::iterator it = follow[Vn[non_c[i]]].begin(); it != follow[Vn[non_c[i]]].end(); it++) {
			out_file << *it << " ";
		}
		out_file << endl;
	}
}
/*构建预测分析表*/
void getTable() {
	for (int i = 0; i < n; i++) {
		char temp = input[i].right[0];//右侧第一个符号
		if (!Vn.count(temp)) {//若不为非终结符
			if (temp != '@')	tableMap[Vn[input[i].left]][Vt[temp]] = i;//若不为空，放入table表
			else {//若为空，则将follow左侧非终结符加入table
				for (set<char>::iterator it = follow[Vn[input[i].left]].begin(); it != follow[Vn[input[i].left]].end(); it++) {
					tableMap[Vn[input[i].left]][Vt[*it]] = i;
				}
			}
		}
		else {//若为非终结符
			int j;
			bool flag = 0;
			for (j = 0; j < input[i].right.length(); j++) {//遍历右侧
				temp = input[i].right[j];//temp为第j个字符
				if (Vt.count(temp)) {//若为终结符
					tableMap[Vn[input[i].left]][Vt[temp]] = i;
					break;
				}
				for (set<char>::iterator it = first[Vn[temp]].begin(); it != first[Vn[temp]].end(); it++) {//加入该非终结符的first集-空
					if (*it == '@')	flag = 1;
					else tableMap[Vn[input[i].left]][Vt[*it]] = i;
				}
				if (!flag)	break;//若该非终结符能推出空，则继续遍历下一个非终结符
			}
			if (j == input[i].right.length() && flag) {//若所有右侧非终结符都能推出空，加上左侧非终结符的follow集
				for (set<char>::iterator it = follow[Vn[input[i].left]].begin(); it != follow[Vn[input[i].left]].end(); it++) {
					tableMap[Vn[input[i].left]][Vt[*it]] = i;
				}
			}
		}
	}
}
void showTable() {
	for (int i = 0; i < ter_c.size(); i++) {
		out_file << setw(6) << ter_c[i];
	}
	out_file << endl;
	for (int i = 0; i < non_c.size(); i++) {
		out_file << non_c[i] << ":";
		for (int j = 0; j < ter_c.size(); j++) {
			if (tableMap[i][j] == -1) {
				out_file << setw(6) << "";
			}
			else out_file << setw(6) << input[tableMap[i][j]].right;
		}
		out_file << endl;
	}
}
/*预测分析串*/
void analyExp(string s) {
	cout << s << endl;
	for (int i = s.length() - 1; i >= 0; i--) {//剩余输入串
		vs.push_back(s[i]);
	}
	sta.push_back('#');//加入#
	sta.push_back(non_c[0]);//加入第一个非终结符
	while (vs.size() > 0) {
		string outs;//剩余字符串
		string outs1;//输出分析栈
		for (int i = 0; i < sta.size(); i++) {
			outs += sta[i];
		}
		for (int i = vs.size() - 1; i >= 0; i--) {
			outs1 += vs[i];
		}
		cout << setw(16) << outs << setw(16) << outs1;
		char c1 = sta[sta.size() - 1], c2 = vs[vs.size() - 1];
		if (c1 == c2) {
			if (c1 == '#') {
				cout << setw(16) << "Accepted!\n";
				break;
			}
			else {
				sta.pop_back();
				vs.pop_back();
				cout << setw(16) << c1 << "\n";
			}
		}
		else if (tableMap[Vn[c1]][Vt[c2]] != -1) {//预测分析表中有数据的栏目
			int t = tableMap[Vn[c1]][Vt[c2]];
			sta.pop_back();//弹出产生式左侧非终结符
			if (input[t].right != "@") {
				for (int i = input[t].right.length() - 1; i >= 0; i--) {
					sta.push_back(input[t].right[i]);//产生式右侧加入分析栈
				}
			}
			cout << setw(16) << input[t].right << endl;//输出产生式
		}
		else {
			cout << setw(16) << "error" << endl;
			break;
		}
	}
}
/*关闭文件*/
void Endf() {
	in_file.close();
	lex_file.close();
	out_file.close();
}
int main() {
	Init();
	Read();
	for (int i = 0; i < non_c.size(); i++) {//遍历非终结符集，求每个非终结符的first集
		getFirst(non_c[i]);
	}
	showFirst();
	for (int i = 0; i < non_c.size(); i++) {
		if (i == 0)	follow[i].insert('#');//第一个加入#
		getFollow(non_c[i]);
	}
	showFollow();
	getTable();
	showTable();
	string s;
	lex_file >> s;
	analyExp(s);
	return 0;
}