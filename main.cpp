#include <iostream>
#include <vector>
#include <ctime>
#include "TransactionDB.hpp"
#include "Miner.hpp"
#include "MINIT.hpp"
#include "MIWI.hpp"
#include "MAFIA.hpp"

void test_MINIT()
{
    TransactionDB D("/Users/jiyi/Documents/ut/lab/proj/data/mushroom.dat", DB_type::BITSET);
    Miner* miner = new MINIT(&D, 100.0/8124.0, D.dim());
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
    TransactionDB D("/Users/jiyi/Documents/ut/lab/proj/data/mushroom.dat", DB_type::VECTOR);
    Miner* miner = new MIWI(&D, 100.0/8124.0);
    
    miner->mine();
    if (miner->check())
    {
        printf("result is valid.\n");
        miner->print_result();
    }

    delete miner;
}

void test_MAFIA()
{
    TransactionDB D("/Users/jiyi/Documents/ut/lab/proj/data/mushroom.dat", DB_type::VECTOR);
    Miner* miner = new MAFIA(&D, 100.0/8124.0);
    
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
    test_MAFIA();
    return 0;
}
