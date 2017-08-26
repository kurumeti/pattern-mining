#include <stdio.h>
#include <vector>
#include <sstream>
#include <map>
#include <boost/dynamic_bitset.hpp>
#include <boost/bimap.hpp>
#include "Itemset.hpp"

using namespace std;
using namespace boost;

enum DB_type {VECTOR, BITSET, SUB};
enum SORT_type {ASC, DSC, DICT, NONE};

class TransactionDB
{
private:
    //SORT_type sort_type;
    bimap<int,int> item_bitpos;
    
public:
    DB_type db_type;
    map<int,int> items;
    vector<pair<int,int>> sorted_items;
    map<int, int> rank;
    vector<Itemset*> db;
    function<bool(const int&,const int&)> sort_method;

    int position(int item)
    {
        return item_bitpos.left.at(item);
    }
    bool validate_min(vector<Itemset*> & MII) const
    {
        bool valid = true;
        for (vector<Itemset*>::iterator i = MII.begin(); i != MII.end(); i++)
        {
            for (vector<Itemset*>::iterator j = MII.begin(); j != MII.end(); j++)
            {
                if (i == j) {continue;}
                if ((*j)->is_subset_of(*i))
                {
                    valid = false;
                    printf("not minimal: ");
                    (*i)->print_self();
                    printf("covered: ");
                    (*j)->print_self();
                }
            }
        }
        return valid;
    }
    bool validate_max(vector<Itemset*> & MFI) const
    {
        bool valid = true;
        for (vector<Itemset*>::iterator i = MFI.begin(); i != MFI.end(); i++)
        {
            for (vector<Itemset*>::iterator j = MFI.begin(); j != MFI.end(); j++)
            {
                if (i == j) {continue;}
                if ((*i)->is_subset_of(*j))
                {
                    valid = false;
                    printf("not maximal: ");
                    (*i)->print_self();
                    printf("covered by: ");
                    (*j)->print_self();
                }
            }
        }
        return valid;
    }
    vector<Itemset*> filter_non_minimal(vector<Itemset*> & MII) const
    {
        vector<Itemset*> result;
        for (vector<Itemset*>::iterator i = MII.begin(); i != MII.end(); i++)
        {
            bool valid = true;
            for (vector<Itemset*>::iterator j = MII.begin(); j != MII.end(); j++)
            {
                if (i == j) {continue;}
                if ((*j)->is_subset_of(*i))
                {
                    valid = false;
                    break;
                }
            }
            if (valid)
            {
                result.push_back(*i);
            }
        }
        return result;
    }
    void validate(vector<pair<Itemset*,int>> & itemsets)
    {
        for (vector<Itemset*>::iterator i = db.begin(); i != db.end(); i++)
        {
            for (vector<pair<Itemset*,int>>::iterator j = itemsets.begin(); j != itemsets.end(); j++)
            {
                if (j->first->is_subset_of(*i))
                {
                    j->second += 1;
                }
            }
        }
    }
    void count_items(stringstream& ss)
    {
        int item;
        while (ss >> item)
        {
            if (items.find(item) == items.end())
            {
                items[item] = 1;
            }
            else
            {
                items[item] += 1;
            }
        }
    }
    Itemset* new_itemset(int support = 1, int item = -1) // no item can have id -1
    {
        Itemset* temp = NULL;
        switch (db_type)
        {
            case VECTOR:
                temp = new Itemset_VECTOR();
                break;
            case BITSET:
                temp = new Itemset_BITSET(item_bitpos);
                break;
            default:
                break;
        }
        if (item != -1)
        {
            temp->add_item(item);
        }
        temp->support = support;
        return temp;
    }
    void input_itemset(stringstream& ss)
    {
        Itemset* temp = new_itemset();
        int item;
        while (ss >> item)
        {
            temp->add_item(item);
        }
        db.push_back(temp);
    }
    void read_file(string filepath, void (TransactionDB::*f)(stringstream&))
    {
        if (freopen(filepath.c_str(), "r", stdin) == nullptr)
        {
            printf("error when opening %s\n", filepath.c_str());
            exit(EXIT_FAILURE);
        }
        string line;
        while (getline(cin, line))
        {
            stringstream ss(line);
            (this->*f)(ss);
        }
        cin.clear();
        fclose(stdin);
    }
    int dim()
    {
        return int(items.size());
    }
    int size()
    {
        return int(db.size());
    }
    void set_sort_method(SORT_type sort_type)
    {
        switch (sort_type)
        {
            case ASC:
                sort_method = [this](const int& a, const int& b) {return this->items[a] == this->items[b] ? a < b : this->items[a] < this->items[b];};
                break;
            case DSC:
                sort_method = [this](const int& a, const int& b) {return this->items[a] == this->items[b] ? a < b : this->items[a] > this->items[b];};
                break;
            case DICT:
                sort_method = [this](const int& a, const int& b) {return a < b;};
                break;
            default:
                break;
        }
    }
    void gen_sorted_items()
    {
        for (map<int,int>::iterator i = items.begin(); i != items.end(); i++)
        {
            sorted_items.push_back(*i);
        }
        //if (sort_type == NONE) {return;}
        stable_sort(sorted_items.begin(), sorted_items.end(), [this](pair<int,int> const& a, pair<int,int> const& b)
        {
            return this->sort_method(a.first, b.first);
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
        for (vector<Itemset*>::iterator i = db.begin(); i != db.end(); i++)
        {
            (*i)->sort_self(sort_method);
        }
    }
    void gen_rank()
    {
        for (vector<pair<int, int>>::iterator i = sorted_items.begin(); i != sorted_items.end(); i++)
        {
            rank[i->first] = int(i-sorted_items.begin());
        }
    }
    deque<pair<Itemset_VERTICAL_UNIT*,int>> get_item_units()
    {
        deque<pair<Itemset_VERTICAL_UNIT*,int>> tail;
        map<int, dynamic_bitset<>> item2transactions;
        for (map<int,int>::iterator i = items.begin(); i != items.end(); i++)
        {
            dynamic_bitset<> transactions(this->size());
            transactions.reset();
            item2transactions[i->first] = transactions;
        }
        for (vector<Itemset*>::iterator i = db.begin(); i != db.end(); i++)
        {
            const vector<int> & tids = (*i)->get_tids();
            for (vector<const int>::iterator j = tids.begin(); j != tids.end(); j++)
            {
                item2transactions[*j].set(i-db.begin());
            }
        }
        for (map<int, dynamic_bitset<>>::iterator i = item2transactions.begin(); i != item2transactions.end(); i++)
        {
            tail.push_back(pair<Itemset_VERTICAL_UNIT*,int>(new Itemset_VERTICAL_UNIT(i->first, i->second),0));
        }
        return tail;
    }
    TransactionDB(string filepath, DB_type _db_type)
    {
        db_type = _db_type;
        read_file(filepath, &TransactionDB::count_items);
        switch (db_type)
        {
            case VECTOR:
                set_sort_method(SORT_type::DSC);
                read_file(filepath, &TransactionDB::input_itemset);
                gen_sorted_items();
                sort_each_transaction();
                gen_rank();
                break;
            case BITSET:
                set_sort_method(SORT_type::ASC);
                gen_sorted_items();
                gen_item_bitpos();
                read_file(filepath, &TransactionDB::input_itemset);
                break;
            default:
                break;
        }
        printf("item:%lu transaction:%lu\n", items.size(), db.size());
    }
    TransactionDB(TransactionDB* root, TransactionDB* D, int item)
    {
        db_type = SUB;
        sort_method = root->sort_method;
        db.reserve(D->size()/D->dim());
        for (vector<Itemset*>::iterator i = D->db.begin(); i != D->db.end(); i++)
        {
            if ((*i)->has_item(item))
            {
                db.push_back(*i);
                (*i)->count_items(items);
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
        if (db_type != SUB)
        {
            for (vector<Itemset*>::iterator i = db.begin(); i != db.end();i++)
            {
                delete *i;
            }
        }
    }
};
