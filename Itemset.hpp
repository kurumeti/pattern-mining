#include <stdio.h>
#include <vector>
#include <sstream>
#include <map>
#include <boost/dynamic_bitset.hpp>
#include <boost/bimap.hpp>

using namespace std;
using namespace boost;

class Itemset
{
    
public:
    int support = 1;
    virtual ~Itemset() {}
    virtual void print_self() const = 0;
    virtual bool is_subset_of(Itemset* b) const = 0;
    virtual void reverse_self() {}
    virtual void sort_self(function<bool(const int&,const int&)>) {}
    virtual void add_item(int) = 0;
    virtual bool has_item(int) const {return false;}
    virtual void count_items(map<int,int> & items) const {}
    virtual void delete_item() {}
    virtual void write_self(ofstream&) const {}
    virtual const vector<int> & get_tids() const {return get_tids();} // this is so ugly...
};

class Itemset_VECTOR : public Itemset
{
protected:
    vector<int> tids;
    
public:
    
    const vector<int> & get_tids() const
    {
        return tids;
    }
    
    Itemset_VECTOR() {}
    Itemset_VECTOR(Itemset* itemset)
    {
        tids = dynamic_cast<Itemset_VECTOR*>(itemset)->tids;
        support = itemset->support;
    }
    void print_self() const
    {
        printf("%d:", support);
        for (vector<const int>::iterator i = tids.begin(); i != tids.end(); i++)
        {
            printf(" %d", *i);
        }
        printf("\n");
    }
    void write_self(ofstream & output) const
    {
        output << support << ":";
        for (vector<const int>::iterator i = tids.begin(); i != tids.end(); i++)
        {
            output << " " << *i;
        }
        output << endl;
    }
    bool is_subset_of(Itemset* b) const
    {
        Itemset_VECTOR* that = dynamic_cast<Itemset_VECTOR*>(b);
        vector<const int>::iterator i = tids.begin(), j = that->tids.begin();
        for (; i != tids.end() && j != that->tids.end(); j++)
        {
            if (*i == *j)
            {
                i++;
            }
        }
        return i == tids.end();
    }
    void reverse_self()
    {
        reverse(tids.begin(), tids.end());
    }
    void add_item(int item)
    {
        tids.push_back(item);
    }
    void sort_self(function<bool(const int&,const int&)> sort_method)
    {
        stable_sort(tids.begin(), tids.end(), sort_method);
    }
    void delete_item()
    {
        tids.pop_back();
    }
};

class Itemset_BITSET : public Itemset
{
    dynamic_bitset<> bt;
    bimap<int,int>& item_bitpos;
    
public:
    Itemset_BITSET(bimap<int,int> & _item_bitpos) : item_bitpos(_item_bitpos)
    {
        bt = dynamic_bitset<>(item_bitpos.size());
    }
    void print_self() const
    {
        printf("support %d :", support);
        for (int i = 0; i < bt.size(); i++)
        {
            if (bt[i] == 1)
            {
                printf(" %d", item_bitpos.right.at(i));
            }
        }
        printf("\n");
    }
    bool is_subset_of(Itemset* b) const
    {
        Itemset_BITSET* that = dynamic_cast<Itemset_BITSET*>(b);
        return bt.is_subset_of(that->bt);
    }
    void add_item(int item)
    {
        bt.set(item_bitpos.left.at(item), true);
        //bt[item_bitpos.left.at(item)] = 1;
    }
    bool has_item(int item) const
    {
        return bt[item_bitpos.left.at(item)] == 1;
    }
    void count_items(map<int,int>& items) const
    {
        for (int i = 0; i < bt.size(); i++)
        {
            if (bt[i] == 1)
            {
                int item = item_bitpos.right.at(i);
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
    }
};

class Itemset_VERTICAL : public Itemset_VECTOR
{
public:
    Itemset_VERTICAL() {}
    dynamic_bitset<> transactions;
    Itemset_VERTICAL(int transaction_num)
    {
        transactions = dynamic_bitset<>(transaction_num);
        transactions.set();
        support = transaction_num;
    }
    Itemset_VERTICAL(Itemset_VERTICAL & itemset)
    {
        tids = itemset.tids;
        transactions = itemset.transactions;
        support = itemset.support;
    }
};

class Itemset_VERTICAL_UNIT : public Itemset_VERTICAL
{
public:
    int item;
    Itemset_VERTICAL_UNIT(int _item, dynamic_bitset<> _transactions)
    {
        item = _item;
        transactions = _transactions;
    }
};
