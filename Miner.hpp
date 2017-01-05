#include <stdio.h>
#include <vector>
#include <map>

using namespace std;

class Miner
{
protected:
    clock_t miner_begin;
    TransactionDB* DB;
    int threshold = 1;
    vector<Itemset>* MII = NULL;
public:
    Miner(){}
    Miner(TransactionDB* _db, float _ratio) : DB(_db), threshold(_ratio * _db->size()) {}
    virtual ~Miner()
    {
        for (vector<Itemset>::iterator i = MII->begin(); i != MII->end(); i++)
        {
            switch (DB->db_type)
            {
                case VECTOR:
                    delete i->itemset.tids;
                    break;
                case BITSET:
                    delete i->itemset.bt;
                    break;
                default:
                    break;
            }
        }
        delete MII;
    }
    void print_result()
    {
        for (vector<Itemset>::iterator i = MII->begin(); i != MII->end(); i++)
        {
            DB->print_itemset(*i);
        }
    }
    virtual void mine()
    {
        if (MII) {delete MII;}
        miner_begin = clock();
    }
    bool check()
    {
        printf("Miner done. MIIs num: %ld\nEnter any key to validate result...\n", MII->size());
        cin.get();
        bool valid = true;
        vector<pair<Itemset,int>> itemsets;
        for (vector<Itemset>::iterator i = MII->begin(); i != MII->end(); i++)
        {
            itemsets.push_back(pair<Itemset,int>(*i,0));
        }
        DB->validate(&itemsets);
        for (vector<pair<Itemset,int>>::iterator i = itemsets.begin(); i != itemsets.end(); i++)
        {
            if (i->second != i->first.support)
            {
                valid = false;
                DB->print_itemset_fault(*i);
            }
            if (i->first.support >= threshold)
            {
                valid = false;
                printf("threshold %d:", threshold);
                DB->print_itemset(i->first);
            }
        }
        return (valid && DB->validate(MII));
    }
};