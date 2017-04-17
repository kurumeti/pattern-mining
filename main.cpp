#include <iostream>
#include <vector>
#include <ctime>
#include "TransactionDB.hpp"
#include "MINIT.hpp"
#include "MIWI.hpp"

void test_MINIT()
{
    TransactionDB D("/Users/jiyi/Documents/ut/lab/proj/data/mushroom.dat", DB_type::BITSET, SORT_type::ASC);
    Miner* miner = new MINIT(&D, 1000.0/8124.0, D.dim());
    miner->mine();
    if (miner->check())
    {
        printf("result is valid.\n");
        miner->print_result();
    }
    delete miner;
}

void test_MIWI()
{
    TransactionDB D("/Users/jiyi/Documents/ut/lab/proj/data/mushroom.dat", DB_type::VECTOR, SORT_type::DSC);
    Miner* miner = new MIWI(&D, 1000.0/8124.0);
    
    miner->mine();
    if (miner->check())
    {
        printf("result is valid.\n");
        miner->print_result();
    }

    delete miner;
}

int main(int argc, const char * argv[])
{
    test_MIWI();
    return 0;
}
