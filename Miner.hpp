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
    bool verbose = false;
    Miner(){}
    Miner(TransactionDB* _db, int _threshold) : DB(_db), threshold(_threshold) {}
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
    void write_result(ofstream & output)
    {
        for (vector<Itemset*>::iterator i = result.begin(); i != result.end(); i++)
        {
            (*i)->write_self(output);
        }
    }
    virtual void mine()
    {
        result.clear();
        miner_begin = clock();
    }
    int result_size()
    {
        return int(result.size());
    }
    float elapsed_time()
    {
        return float(clock() - miner_begin)/CLOCKS_PER_SEC;
    }
    bool check_threshold(bool frequent) const
    {
        bool valid = true;
        for (vector<Itemset*>::const_iterator i = result.begin(); i != result.end(); i++)
        {
            if ((frequent && (*i)->support < threshold) || (!frequent && (*i)->support >= threshold))
            {
                valid = false;
                printf("threshold %d | ", threshold);
                (*i)->print_self();
            }
        }
        return valid;
    }
    bool check_basic() const
    {
        printf("Miner done. result num: %ld\nEnter any key to validate result...\n", result.size());
        cin.get();
        return DB->validate(result);
    }
    virtual bool check() const {return false;}
};
