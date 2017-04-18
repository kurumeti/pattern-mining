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

int min(int a,int b)
{
    if (a < b) {return a;}
    return b;
}

struct Node
{
    friend bool operator < (Node a,Node b)
    {
        return a.pre > b.pre;
    }
    vector<Node*> children;
    int count_local;
    int item_index;
    int pre;
    int post;
}root;
//FP树节点的数据类型

struct Point
{
    int pre;
    int post;
    int support;
    Point(int _pre,int _post,int _support)
    {
        pre = _pre;
        post = _post;
        support = _support;
    }
};

struct Nlist
{
    /*friend bool operator < (Nlist a,Nlist b)
    {
        return a.total_support < b.total_support;
    }*/
    vector<int> index;
    vector<Point*> nodes;
    //priority_queue<Node*> nodes;
    //map<Node*,int> support;
    int total_support = 0;
};
vector<Nlist*> curNlist;
map<string,Nlist*> nextNlist;
vector<Nlist*> final_result;

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
queue<item> items;

struct transaction
{
    vector<int> items;
};
vector<transaction> info;
map<int,int> sort_use;

void build_tree(vector<int> &it)
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
            tmp->count_local = 1;
            tmp->item_index = *i;
            temp->children.push_back(tmp);
            temp = tmp;
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

int num = 0;
void visit_tree_pre(Node* temp) //前序遍历FP树，设置其pre的值
{
    temp->pre = num;
    num += 1;
    vector<Node*>::iterator i;
    for (i = temp->children.begin();i != temp->children.end();i++)
    {
        visit_tree_pre(*i);
    }
}

void visit_tree_post(Node* temp)  //后序遍历FP树，设置其post的值
{
    vector<Node*>::iterator i;
    for (i = temp->children.begin();i != temp->children.end();i++)
    {
        visit_tree_post(*i);
    }
    temp->post = num;
    num += 1;
}

map<int,Nlist*> origin;
void gather_tree_pre(Node* temp) //前序遍历FP树，转为第一批Node list
{
    vector<Node*>::iterator i;
    for (i = temp->children.begin();i != temp->children.end();i++)
    {
        if (origin.count((*i)->item_index) == 0)
        {
            origin[(*i)->item_index] = new Nlist;
            origin[(*i)->item_index]->total_support = 0;
        }
        origin[(*i)->item_index]->nodes.push_back(new Point((*i)->pre,(*i)->post,(*i)->count_local));
        origin[(*i)->item_index]->total_support += (*i)->count_local;
        gather_tree_pre(*i);
    }
}

void tree2nlist()
{
    gather_tree_pre(&root);
    map<int,Nlist*>::iterator it;
    for (it = origin.begin();it != origin.end();it++)
    {
        (*it).second->index.push_back((*it).first);
        if ((*it).second->total_support > min_support)
        {
            curNlist.push_back((*it).second);
        }
    }
    origin.clear();
}

Nlist* mergeNlist(Nlist* a,Nlist* b)
{
    vector<Point*>::iterator i = a->nodes.begin();
    vector<Point*>::iterator j = b->nodes.begin();
    Nlist* result = NULL;
    /*Node-list核心算法*/
    for (int n = 0;n < 2;n++)
    {
        while (i != a->nodes.end() && j != b->nodes.end())
        {
            while (j != b->nodes.end())
            {
                if ((*j)->pre < (*i)->pre) {j++;}
                else {break;}
            }
            if (j == b->nodes.end()) {break;}
            if ((*j)->post < (*i)->post)
            {
                if (result == NULL) {result = new Nlist;}
                //合并node-list中的重复node
                if (result->nodes.size() > 0 && result->nodes[result->nodes.size()-1]->pre == (*i)->pre)
                {
                    result->nodes[result->nodes.size()-1]->support += (*j)->support;
                }
                else
                {
                    result->nodes.push_back(new Point((*i)->pre,(*i)->post,(*j)->support));
                }
                j++;
            }
            else
            {
                i++;
            }
        }
        Nlist* tmp = a;a = b;b = tmp;
        i = a->nodes.begin();
        j = b->nodes.begin();
    }
    /*合并项集的index，保证按照支持度降序*/
    if (result == NULL) {return NULL;}
    vector<int>::iterator p = a->index.begin();
    vector<int>::iterator q = b->index.begin();
    while (p != a->index.end() && q != b->index.end())
    {
        if (sort_use[*p] > sort_use[*q])
        {
            if (result->index.empty() || result->index[result->index.size()-1] != *p)
            {
                result->index.push_back(*p);
            }
            p++;
        }
        else
        {
            if (result->index.empty() || result->index[result->index.size()-1] != *q)
            {
                result->index.push_back(*q);
            }
            q++;
        }
    }
    if (p != a->index.end())
    {
        for (;p != a->index.end();p++)
        {
            if (result->index.empty() || result->index[result->index.size()-1] != *p)
            {
                result->index.push_back(*p);
            }
        }
    }
    if (q != b->index.end())
    {
        for (;q != b->index.end();q++)
        {
            if (result->index.empty() || result->index[result->index.size()-1] != *q)
            {
                result->index.push_back(*q);
            }
        }
    }
    /*统计总的支持度*/
    result->total_support = 0;
    vector<Point*>::iterator it = result->nodes.begin();
    for (;it != result->nodes.end();it++)
    {
        result->total_support += (*it)->support;
    }
    return result;
}

/*判断是否应该merge两个Nodelist，通过判断项集的diff是否为1*/
bool shouldMerge(Nlist* a,Nlist* b)
{
    vector<int>::iterator i = a->index.begin();
    vector<int>::iterator j = b->index.begin();
    int flag = 0;
    for (;i != a->index.end();i++,j++)
    {
        if (*i != *j) {flag+=1;}
    }
    if (flag == 1) {return true;}
    return false;
}

string get_nlist_id(Nlist* t)
{
    string result = "";
    vector<int>::iterator i = t->index.begin();
    for (;i != t->index.end();i++)
    {
        stringstream ss;
        ss << *i;
        result += ","+ss.str();
    }
    return result;
}

bool nodecmp(Point* a,Point* b)
{
    return a->pre < b->pre;
}

int counter = 0;
void gen_next()
{
    counter += 1;
    printf("%s%d%s\n","第",counter,"次迭代！");
    nextNlist.clear();
    vector<Nlist*>::iterator i,j;
    int ctr = 0;
    for (i = curNlist.begin();i != curNlist.end();i++)
    {
        ctr += 1;
        //printf("%s%f\n","完成",float(ctr)/float(curNlist.size()+1));
        for (j = i+1;j != curNlist.end();j++)
        {
            if (!shouldMerge(*i,*j)) {continue;}
            Nlist* temp = mergeNlist(*i,*j);
            if (temp == NULL) {continue;}
            string key = get_nlist_id(temp);
            if (nextNlist.count(key) == 0)
            {
                nextNlist[key] = temp;
            }
            else
            {
                nextNlist[key]->total_support += temp->total_support;
                vector<Point*>::iterator p = temp->nodes.begin();
                for (;p != temp->nodes.end();p++)
                {
                    nextNlist[key]->nodes.push_back(*p);
                }
                delete temp;
            }
        }
    }
    vector<Nlist*>::iterator it = curNlist.begin();
    for (;it != curNlist.end();it++)
    {
        final_result.push_back(*it);
        vector<Point*>::iterator p = (*it)->nodes.begin();
        for (;p != (*it)->nodes.end();p++)
        {
            delete *p;
        }
    }
    curNlist.clear();
    map<string,Nlist*>::iterator ele = nextNlist.begin();
    for (;ele != nextNlist.end();ele++)
    {
        if ((*ele).second->total_support > min_support)
        {
            sort((*ele).second->nodes.begin(),(*ele).second->nodes.end(),nodecmp);
            curNlist.push_back((*ele).second);
        }
    }
    printf("%s%d%s\n","第",counter,"次迭代结束！");
}

bool mycompare(int a,int b)
{
    return sort_use[a] > sort_use[b];
}
//该函数利用sort_use定义每项事务中项目的排序方式（支持度降序），这样使生成的FP树大小尽可能压缩

bool cmp(Nlist* a,Nlist* b)
{
    return a->total_support > b->total_support;
}

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
//该函数在指定的数据类型中写入数据

void debug()
{
    vector<Nlist*>::iterator i = curNlist.begin();
    for (;i != curNlist.end();i++)
    {
        vector<int>::iterator it = (*i)->index.begin();
        for (;it != (*i)->index.end();it++)
        {
            printf("%d ",*it);
        }
        printf(": %d\n",(*i)->total_support);
    }
}

void print()
{
    vector<Nlist*>::iterator it = final_result.begin();
    for (;it != final_result.end();it++)
    {
        vector<int>::iterator i = (*it)->index.begin();
        for (;i != (*it)->index.end();i++)
        {
            printf("%d ",*i);
        }
        printf(": %d\n",(*it)->total_support);
    }
}

int main(int argc, const char * argv[])
{
    auto beginTime = std::chrono::high_resolution_clock::now();
    input("/Users/jiyi/Documents/pku/ML/FP/mushroom.dat");
    cout << "数据输入完毕" << endl;
    queue<item> items_copy = items;
    while (items_copy.empty() == false)
    {
        sort_use[items_copy.front().item_index] = items_copy.front().count_support;
        items_copy.pop();
    }
    //上面这个while循环生成事务内排序用的sort_use
    root.item_index = -1;
    root.count_local = 0;
    vector<transaction>::iterator it;
    for (it = info.begin();it != info.end();it++)
    {
        sort((*it).items.begin(),(*it).items.end(),mycompare);
        build_tree(it->items);
    }
    //循环事务集合，每次传给build_tree一个事务去建FP树
    cout << "建树过程完毕" << endl;
    num = 0;
    visit_tree_pre(&root);
    num = 0;
    visit_tree_post(&root);
    //输出结果
    tree2nlist();
    while (!curNlist.empty())
    {
        //debug();
        gen_next();
    }
    cout << "挖掘过程完毕！" << endl;
    sort(final_result.begin(),final_result.end(),cmp);
    //print();
    auto nowTime = std::chrono::high_resolution_clock::now();
    auto elapsed_usec = std::chrono::duration_cast<std::chrono::milliseconds>(nowTime - beginTime).count();
    cout << "用时: " << elapsed_usec << "ms" << endl;
    return 0;
}



