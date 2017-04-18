#include <iostream>
#include <string>
#include <cstring>
#include <algorithm>
#include <map>
#include <set>
#include <vector>
#include <queue>
#include <sstream>
#include <stack>
using namespace std;

const int min_support = 406;
//定义最小支持度和时间统计变量

struct Node
{
	vector<Node*> children;
	Node* father;
	int count_local;
	int item_index;
	stack<int> counts; //这个栈存放之前fp子树中的count_local值，返回上一层时弹栈恢复
	int note;          //这个标记值表明在当前fp树中的这个结点是否已经在下一层fp树中
}root;
//FP树节点的数据类型

struct transaction
{
	vector<int> items;
};
vector<transaction> info;
map<int,int> sort_use;
//事务的存储数据类型，sort_use被mycompare函数用于将每项事务中的项目按支持度排序，它是从item_name到support的映射

struct item
{
	/*friend bool operator < (item a,item b)
     {
     return a.count_support > b.count_support;
     }*/
	int item_index;
	int count_support;
	vector<Node*> link;
};
//item是表头中单个项目的存储数据类型，其中link存储该项目在FP树中的位置指针
queue<item> items;
map<int,item> total_form;
//total_form和items是最开始的总的表头，items是1.0版本中采用的数据类型，后来被优化为不按支持度排序的total_form，其中的int是item_index

struct frequent_item_set
{
	friend bool operator < (frequent_item_set a,frequent_item_set b)
	{
		return a.count_support < b.count_support;
	}
	vector<int> items;
	int count_support;
};
priority_queue<frequent_item_set> result;
frequent_item_set fis;
//frequent_item_set是频繁项集，result是最后输出的结果集合，其中按支持度降序排序，fis是当前频繁项集（挖掘FP树时的外部变量）

void print();

void dig_tree(int sign,map<int,item> &form)
{
	map<int,item>::iterator f;
	int count_support_copy = fis.count_support;
	for (f = form.begin();f != form.end();f++)
	{
		//循环表头，每次根据不同的项作为叶结点来挖掘
		if (f->second.count_support < min_support) {continue;}
		//当前项集的支持度若低于最小支持度则略过
		map<int,item> next_form;
		vector<Node*>::iterator it;
		for (it = f->second.link.begin();it != f->second.link.end();it++)
		{
			//循环这个项的link集合，挖出它在这一层fp树中的子树
			Node* temp = (*it)->father;
			int count_this_line = (*it)->count_local;
			//向上更新count_local是根据叶结点的count_local值
			while (temp != &root)
			{
				switch (temp->note - sign)
				{
                        //note等于sign说明这个结点没被其它上升的路径经历过
					case 0: temp->counts.push(temp->count_local);
						temp->count_local = count_this_line;
						if (next_form.count(temp->item_index) == 0)
						{
							next_form[temp->item_index].count_support = temp->count_local;
							//针对map中值不一定初始化为0的问题
							next_form[temp->item_index].item_index = temp->item_index;
						}
						else {next_form[temp->item_index].count_support += temp->count_local;}
						next_form[temp->item_index].link.push_back(temp);break;
					case 1: temp->count_local += count_this_line;next_form[temp->item_index].count_support += count_this_line;break;
                        //否则是经历过的，就不再需要加入link集合
				}
				temp->note = sign + 1;
				//标记为经历过
				temp = temp->father;
			}
		}
		fis.items.push_back(f->second.item_index);
		fis.count_support = min(count_support_copy,f->second.count_support);
		result.push(fis);
		//准备进入下一层挖掘的更新频繁项集，并放入结果集合
		//print();
		dig_tree(sign+1,next_form);
		//进行下一层挖掘，然后恢复这一层被修改的count_local和sign值，为循环表头下一项准备，并且恢复当前频繁项集
		fis.items.pop_back();
		for (it = f->second.link.begin();it != f->second.link.end();it++)
		{
			Node* temp = (*it)->father;
			while (temp != &root)
			{
				if (temp->note == sign) {break;} //如果这个点已经恢复了，那么其上必然也恢复了，不必再向上
				temp->note = sign;
				temp->count_local = temp->counts.top(); //每个结点中栈的作用就在此，用于在递归过程中按顺序恢复count_local
				temp->counts.pop();
				temp = temp->father;
			}
		}
	}
}

inline void build_tree(vector<int> &it)
{
	//建树函数的参数是一项事务中的项目集合，非递归，在main函数中被循环调用
	Node* temp = &root;
	//每次都从树根开始
	vector<int>::iterator i;
	for (i = it.begin();i != it.end();i++)
	{
		//扫描项集，每个项目要么插入temp的children集合中，要么在children集合中与已有的同样项目合并
		vector<Node*>::iterator t = temp->children.begin();
		while (t != temp->children.end() && *i != (*t)->item_index) {t++;}
		if (t != temp->children.end())
		{
			(*t)->count_local++;
			temp = *t;
		}
		//if内是合并的情况
		//else内是插入的情况
		else
		{
			Node* tmp = new Node;
			tmp->note = 0;
			tmp->count_local = 1;
			tmp->father = temp;
			tmp->item_index = *i;
			temp->children.push_back(tmp);
			temp = tmp;
			total_form[*i].link.push_back(temp); //这句之前放在else之外导致结果有误，因为已经存在的结点不应再放入link
		}
		//1.1中对表头的建立采用不排序的方式（实际上对挖掘时间没有显著影响），以下被注释的代码是1.0中的排序方法，事实上1.0速度极慢的主要原因就在此
		/*queue<item> items_copy;
         while(items.empty()==false)
         {
         if (items.top().item_index == *i)
         {
         items.top().link.push_back(temp);
         }
         items_copy.push(items.top());
         items.pop();
         }
         items = items_copy;*/
	}
}

bool mycompare(int a,int b)
{
	return sort_use[a] > sort_use[b];
}
//该函数利用sort_use定义每项事务中项目的排序方式（支持度降序），这样使生成的FP树大小尽可能压缩

void input(std::string directory){
    if(directory!=""){
        if(freopen(directory.c_str(), "r", stdin)==nullptr)
        {
            printf("error when opening %s\n",directory.c_str());
            exit(EXIT_FAILURE);
        }
    }
    std::string line;
    std::vector<item> vitems;
    while (!feof(stdin)) {
        transaction current_trans;
        std::getline(std::cin,line);
        std::stringstream stream(line);
        int current_int;
        while (stream>>current_int) {
            current_trans.items.push_back(current_int);
            auto current_item = std::find_if(vitems.begin(), vitems.end(), [current_int](item const& tempitem){
                return tempitem.item_index==current_int;
            });
            if (current_item==vitems.end()) {
                vitems.push_back({current_int,0});
                current_item = std::find_if(vitems.begin(), vitems.end(), [current_int](item const& tempitem){
                    return tempitem.item_index==current_int;
                });
            }
            current_item->count_support++;
        }
        info.push_back(current_trans);
    }
    std::for_each(vitems.begin(),vitems.end(),[](item const& tempitem){items.push(tempitem);});
}
//该函数由杨俊睿完成，实现在我指定的数据类型中写入数据

void print()
{
	while (result.empty() == false)
	{
		for (auto i = result.top().items.begin();i != result.top().items.end();i++)
		{
			//cout << *i << " ";
			printf("%d ",*i);
		}
		//cout << ": " << result.top().count_support << endl;
		printf(": %d\n",result.top().count_support);
		result.pop();
	}
}
//打印函数，输出最后的结果

int main(int argc, const char * argv[])
{
	auto beginTime = std::chrono::high_resolution_clock::now();
	//freopen("o.txt","w",stdout);
	input("/Users/jiyi/Documents/pku/ML/FP/mushroom.dat");
	cout << "数据输入完毕："<< endl;
	queue<item> items_copy = items;
	while (items_copy.empty() == false)
	{
		sort_use[items_copy.front().item_index] = items_copy.front().count_support;
		items_copy.pop();
	}
	//上面这个while循环生成事务内排序用的sort_use
	while (items.empty()==false)
	{
		total_form[items.front().item_index].count_support = items.front().count_support;
		total_form[items.front().item_index].item_index = items.front().item_index;
		items.pop();
	}
	//上面这个while循环存在的原因是数据结构的转化
	vector<transaction>::iterator it;
	for (it = info.begin();it != info.end();it++)
	{
		sort((*it).items.begin(),(*it).items.end(),mycompare);
		build_tree(it->items);
	}
	//循环事务集合，每次传给build_tree一个事务去建FP树
	fis.count_support = 0x7fffffff;
	//初始化频繁项集
	root.father = NULL;
	cout << "建树过程完毕："  << endl;
	dig_tree(0,total_form);
	//挖掘FP树
	//print();
	//输出结果
	cout <<"挖掘过程完毕！" << endl;
	auto nowTime = std::chrono::high_resolution_clock::now();
    auto elapsed_usec = std::chrono::duration_cast<std::chrono::milliseconds>(nowTime - beginTime).count();
    cout << "用时: " << elapsed_usec << "ms" << endl;
	return 0;
}

