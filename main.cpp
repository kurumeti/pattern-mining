#include <iostream>
#include <vector>
#include <ctime>
#include "TransactionDB.hpp"
#include "MINIT.hpp"
#include "Growth.hpp"

int main(int argc, const char * argv[])
{
    //TransactionDB D("/Users/jiyi/Documents/ut/lab/proj/data/mushroom.dat", DB_type::BITSET, SORT_type::ASC);
    //Miner* miner = new MINIT(&D, 100.0/8124.0, D.dim());
    
    TransactionDB D("/Users/jiyi/Documents/ut/lab/proj/data/T10I4D100K.dat", DB_type::VECTOR, SORT_type::DSC);
//    for (int k = 0; k < 10; k++)
//    {
//    for (int i = 10; i <= 100; i += 1)
//    {
        Miner* miner = new Growth(&D, 0.8);
        
        miner->mine();
    //    if (miner->check())
    //    {
    //        printf("result is valid.\n");
    //        //miner->print_result();
    //    }
        
        delete miner;
        
        //fflush(stdout);
//    }
//    }
    return 0;
}
