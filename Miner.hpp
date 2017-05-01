#include <stdio.h>
#include <vector>
#include <map>
#include "MTree.hpp"

using namespace std;

class Miner
{
protected:
    clock_t miner_begin;
    TransactionDB* DB;
    int threshold = 1;
    vector<Itemset*> result;
public:
    Miner(){}
    Miner(TransactionDB* _db, float _ratio) : DB(_db), threshold(_ratio * _db->size()) {}
    virtual ~Miner()
    {
        for (vector<Itemset*>::iterator i = result.begin(); i != result.end(); i++)
        {
            delete *i;
        }
    }
    void print_result()
    {
        for (vector<Itemset*>::iterator i = result.begin(); i != result.end(); i++)
        {
            (*i)->print_self();
        }
    }
    virtual void mine()
    {
        result.clear();
        miner_begin = clock();
    }
    virtual bool check()
    {
        printf("Miner done. result num: %ld\nEnter any key to validate result...\n", result.size());
        cin.get();
        bool valid = true;
        vector<pair<Itemset*,int>> itemsets;
        for (vector<Itemset*>::iterator i = result.begin(); i != result.end(); i++)
        {
            itemsets.push_back(pair<Itemset*,int>(*i,0));
        }
        DB->validate(itemsets);
        for (vector<pair<Itemset*,int>>::iterator i = itemsets.begin(); i != itemsets.end(); i++)
        {
            if (i->second != i->first->support)
            {
                valid = false;
                printf("true %d ", i->second);
                i->first->print_self();
            }
            if (i->first->support >= threshold)
            {
                valid = false;
                printf("threshold %d:", threshold);
                i->first->print_self();
            }
        }
        return (valid && DB->validate_min(result));
    }
};
