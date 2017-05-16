#include <iostream>
#include <vector>
#include <ctime>
#include <fstream>
#include <string>
#include "TransactionDB.hpp"
#include "Miner.hpp"
#include "MINIT.hpp"
#include "MIWI.hpp"
#include "MAFIA.hpp"

void test_MINIT()
{
    TransactionDB D("/Users/jiyi/Documents/ut/lab/proj/data/mushroom.dat", DB_type::BITSET);
    Miner* miner = new MINIT(&D, 100.0f/8124.0f, D.dim());
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
    Miner* miner = new MIWI(&D, 100.0f/8124.0f);
    
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
    Miner* miner = new MAFIA(&D, 1000.0f/8124.0f);
    
    miner->mine();
    if (miner->check())
    {
        printf("result is valid.\n");
        miner->print_result();
    }
    
    delete miner;
}

void gen_dataset(string input_file, int lower_bound, int upper_bound)
{
    TransactionDB D(input_file, DB_type::VECTOR);
    lower_bound = lower_bound == -1 ? 1 : lower_bound;
    upper_bound = upper_bound == -1 ? D.size() : upper_bound;
    string output_file = input_file.substr(0, input_file.length()-4) + "_" + to_string(lower_bound) + "_" + to_string(upper_bound) + ".its";
    ofstream output;
    output.open(output_file);
    output << "item:" << D.dim() << "transaction:" << D.size() << endl;
    for (int threshold = lower_bound; threshold <= upper_bound; threshold++)
    {
        Miner* miner = new MAFIA(&D, threshold);
        miner->mine();
        miner->write_result(output);
        cout << "MFI: threshold=" << threshold << " result_num=" << miner->result_size() << " time=" << miner->elapsed_time() << "s" << endl;
        if (miner->result_size() == 0)
        {
            delete miner;
            break;
        }
        delete miner;
    }
    for (int threshold = lower_bound; threshold <= upper_bound; threshold++)
    {
        Miner* miner = new MIWI(&D, threshold);
        miner->mine();
        miner->write_result(output);
        cout << "MII: threshold=" << threshold << " result_num=" << miner->result_size() << " time=" << miner->elapsed_time() << "s" << endl;
        delete miner;
    }
    output.close();
}

int main(int argc, const char * argv[])
{
    //gen_dataset("/Users/jiyi/Documents/ut/lab/proj/data/mushroom.dat", -1, -1);
    gen_dataset(argv[1], atoi(argv[2]), atoi(argv[3]));
    return 0;
}
