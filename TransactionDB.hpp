#include <stdio.h>
#include <vector>
#include <sstream>
#include <map>
#include <boost/dynamic_bitset.hpp>
#include <boost/bimap.hpp>

using namespace std;
using namespace boost;

enum DB_type {VECTOR, BITSET, SUB};
enum SORT_type {ASC, DSC, DICT, NONE};

union Transaction
{
    vector<int>* tids = NULL;
    dynamic_bitset<>* bt;
    Transaction(vector<int>* _tids)
    {
        tids = _tids;
    }
    Transaction(dynamic_bitset<>* _bt)
    {
        bt = _bt;
    }
    Transaction() {}
};

struct Itemset
{
    Transaction itemset;
    int support;
    Itemset(Transaction _itemset, int _support)
    {
        itemset = _itemset;
        support = _support;
    }
    Itemset(Itemset const &a, DB_type type)
    {
        switch (type)
        {
            case VECTOR: 
                support = a.support;
                itemset = Transaction(new vector<int>(*(a.itemset.tids)));
                break;
            default:
                break;
        }
    }
};

class TransactionDB
{
private:
    SORT_type sort_type;
    //unsigned long items_num = 0;
    bimap<int,int> item_bitpos;
    
public:
    DB_type db_type;
    map<int,int> items;
    vector<pair<int,int>> sorted_items;
    map<int, int> rank;
    vector<Transaction> db;
    
    long position(int item)
    {
        return item_bitpos.left.at(item);
    }
    dynamic_bitset<>* item2bt(int item)
    {
        dynamic_bitset<>* bt = new dynamic_bitset<>(items.size());
        bt->set(position(item), 1);
        return bt;
    }
    void print_itemset_fault(pair<Itemset,int> i)
    {
        printf("true %d ", i.second);
        print_itemset(i.first);
    }
    void print_itemset(Itemset& i)
    {
        printf("support %d :", i.support);
        vector<int> vi;
        switch (db_type)
        {
            case BITSET:
                vi = bt2vt(i.itemset.bt);
                break;
            case VECTOR:
                vi = *(i.itemset.tids);
                break;
            default:
                break;
        }
        for (vector<int>::iterator j = vi.begin(); j != vi.end(); j++)
        {
            printf(" %d", *j);
        }
        printf("\n");
    }
    bool validate(vector<Itemset>* MII)
    {
        bool valid = true;
        for (vector<Itemset>::iterator i = MII->begin(); i != MII->end(); i++)
        {
            for (vector<Itemset>::iterator j = MII->begin(); j != MII->end(); j++)
            {
                if (i == j) {continue;}
                bool minimal = true;
                switch (db_type)
                {
                    case VECTOR:
                    {
                        vector<int>::iterator ii = i->itemset.tids->begin(), jj = j->itemset.tids->begin();
                        for (; ii != i->itemset.tids->end() && jj != j->itemset.tids->end(); ii++)
                        {
                            if (*ii == *jj)
                            {
                                jj++;
                            }
                        }
                        if (jj == j->itemset.tids->end())
                        {
                            minimal = false;
                        }
                        break;
                    }
                    case BITSET:
                        if (j->itemset.bt->is_subset_of(*(i->itemset.bt)))
                        {
                            minimal = false;
                        }
                        break;
                    default:
                        break;
                }
                if (!minimal)
                {
                    valid = false;
                    printf("not minimal: ");
                    print_itemset(*i);
                    printf("covered: ");
                    print_itemset(*j);
                }
            }
        }
        return valid;
    }
    void validate(vector<pair<Itemset,int>>* itemsets)
    {
        if (db_type == VECTOR)
        {
            for (vector<pair<Itemset,int>>::iterator j = itemsets->begin(); j != itemsets->end(); j++)
            {
                reverse(j->first.itemset.tids->begin(), j->first.itemset.tids->end());
            }
        }
        for (vector<Transaction>::iterator i = db.begin(); i != db.end(); i++)
        {
            for (vector<pair<Itemset,int>>::iterator j = itemsets->begin(); j != itemsets->end(); j++)
            {
                switch (db_type)
                {
                    case BITSET:
                        if (j->first.itemset.bt->is_subset_of(*(i->bt)))
                        {
                            j->second += 1;
                        }
                        break;
                    case VECTOR:
                    {
                        vector<int>::iterator p1 = j->first.itemset.tids->begin(), p2 = i->tids->begin();
                        for (; p1 != j->first.itemset.tids->end() && p2 != i->tids->end(); p2++)
                        {
                            if (j->first.itemset.tids->end() - p1 > i->tids->end() - p2)
                            {
                                break;
                            }
                            if (*p1 == *p2)
                            {
                                p1++;
                            }
                        }
                        if (p1 == j->first.itemset.tids->end())
                        {
                            j->second += 1;
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
        }
    }
    void count_items(vector<int>* t)
    {
        for (vector<int>::iterator i = t->begin(); i != t->end();i++)
        {
            int item = *i;
            if (items.find(item) == items.end())
            {
                items[item] = 1;
            }
            else
            {
                items[item] += 1;
            }
        }
        //items_num = items.size();
    }
    void input_vector(vector<int>* tids)
    {
        count_items(tids);
        Transaction t(tids);
        db.push_back(t);
    }
    dynamic_bitset<>* vt2bt(vector<int>* tids)
    {
        dynamic_bitset<>* bt = new dynamic_bitset<>(items.size());
        for (vector<int>::iterator i = tids->begin(); i != tids->end();i++)
        {
            bt->set(position(*i), 1);
        }
        return bt;
    }
    vector<int> bt2vt(dynamic_bitset<>* bt)
    {
        vector<int> tids;
        for (int i = 0; i < bt->size(); i++)
        {
            if ((*bt)[i] == 1)
            {
                tids.push_back(item_bitpos.right.at(i));
            }
        }
        return tids;
    }
    void input_bitset(vector<int>* tids)
    {
        Transaction t(vt2bt(tids));
        db.push_back(t);
        delete tids;
    }
    void read_file(const char* filepath, void (TransactionDB::*f)(vector<int>*))
    {
        if (freopen(filepath, "r", stdin) == nullptr)
        {
            printf("error when opening %s\n", filepath);
            exit(EXIT_FAILURE);
        }
        string line;
        while (true)
        {
            getline(cin, line);
            stringstream stream(line);
            int item;
            vector<int>* t = new vector<int>;
            while (stream >> item)
            {
                t->push_back(item);
            }
            if (t->empty()) {break;}
            (this->*f)(t);
        }
        cin.clear();
        fclose(stdin);
    }
    int dim()
    {
        return int(items.size());
    }
    long size()
    {
        return db.size();
    }
    void gen_sorted_items()
    {
        for (map<int,int>::iterator i = items.begin(); i != items.end(); i++)
        {
            sorted_items.push_back(*i);
        }
        if (sort_type == NONE) {return;}
        stable_sort(sorted_items.begin(), sorted_items.end(), [this](pair<int,int> const& a, pair<int,int> const& b)
        {
            switch (this->sort_type)
            {
                case ASC:
                    return a.second < b.second;
                    break;
                case DSC:
                    return a.second > b.second;
                    break;
                case DICT:
                    return a.first < b.first;
                    break;
                default:
                    return a.first < b.first;
                    break;
            }
        });
    }
    void gen_item_bitpos()
    {
        for (vector<pair<int,int>>::iterator i = sorted_items.begin(); i != sorted_items.end(); i++)
        {
            item_bitpos.insert(bimap<int,int>::value_type(i->first, int(i-sorted_items.begin())));
        }
    }
    void sort_each_transaction()
    {
        for (vector<Transaction>::iterator i = db.begin(); i != db.end(); i++)
        {
            stable_sort(i->tids->begin(), i->tids->end(), [this](const int& a, const int& b)
            {
                switch (this->sort_type)
                {
                    case ASC:
                        return this->items[a] < this->items[b];
                        break;
                    case DSC:
                        return this->items[a] > this->items[b];
                        break;
                    case DICT:
                        return a < b;
                        break;
                    default:
                        return a < b;
                }
            });
        }
    }
    void gen_rank()
    {
        for (vector<pair<int, int>>::iterator i = sorted_items.begin(); i != sorted_items.end(); i++)
        {
            rank[i->first] = int(i-sorted_items.begin());
        }
    }
    TransactionDB(const char* filepath, DB_type _db_type, SORT_type _sort_type)
    {
        db_type = _db_type;
        sort_type = _sort_type;
        switch (db_type)
        {
            case VECTOR:
                read_file(filepath, &TransactionDB::input_vector);
                gen_sorted_items();
                sort_each_transaction();
                gen_rank();
                break;
            case BITSET:
                read_file(filepath, &TransactionDB::count_items);
                gen_sorted_items();
                gen_item_bitpos();
                read_file(filepath, &TransactionDB::input_bitset);
                break;
            default:
                break;
        }
        printf("item:%lu transaction:%lu\n", items.size(), db.size());
    }
    TransactionDB(TransactionDB* root, TransactionDB* D, int item)
    {
        db_type = SUB;
        sort_type = root->sort_type;
        db.reserve(D->size()/D->dim());
        for (vector<Transaction>::iterator i = D->db.begin(); i != D->db.end(); i++)
        {
            if ((*(i->bt))[root->position(item)] == 1)
            {
                db.push_back(*i);
                vector<int> tids = root->bt2vt(i->bt);
                count_items(&tids);
            }
        }
        for (map<int,int>::iterator i = root->items.begin(); i != root->items.end(); i++) // generate 0-support items
        {
            if (items.find(i->first) == items.end())
            {
                items[i->first] = 0;
                sorted_items.push_back(pair<int,int>(i->first, 0));
            }
        }
        gen_sorted_items();
    }
    ~TransactionDB()
    {
        for (vector<Transaction>::iterator i = db.begin(); i != db.end();i++)
        {
            switch (db_type)
            {
                case VECTOR:
                    delete i->tids;
                    break;
                case BITSET:
                    delete i->bt;
                    break;
                case SUB:
                    break;
                default:
                    break;
            }
        }
    }
};