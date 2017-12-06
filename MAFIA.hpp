#include <queue>

#define DYNAMIC_REORDER 0

class MAFIA : public Miner
{
private:

    M_Node* max_tree;
    deque<pair<Itemset_VERTICAL_UNIT*,int>> initial_tail;
    
public:
    
    MAFIA(TransactionDB* _db, float _ratio) : Miner(_db, _ratio) {}
    MAFIA(TransactionDB* _db, int _threshold) : Miner(_db, _threshold) {}
    
    bool in_MFI(Itemset_VERTICAL & itemset) //too slow, deprecated
    {
        itemset.sort_self(DB->sort_method); //sorting actually should not be put here
        for (vector<Itemset*>::iterator i = result.begin(); i != result.end(); i++)
        {
            if (itemset.is_subset_of(*i))
            {
                return true;
            }
        }
        return false;
    }
    
    bool HUTMFI(const Itemset_VERTICAL & head, const deque<pair<Itemset_VERTICAL_UNIT*,int>> & tail)
    {
        Itemset_VERTICAL HUT(head);
        for (deque<pair<Itemset_VERTICAL_UNIT*,int>>::const_iterator i = tail.begin(); i != tail.end(); i++)
        {
            HUT.add_item(i->first->item);
        }
        return !max_tree->check_maximal(HUT.get_tids(), DB, true);
    }
    
    void print_state(const Itemset_VERTICAL & head, const deque<pair<Itemset_VERTICAL_UNIT*,int>> & tail, bool is_HUT)
    {
        cout << "is_HUT: " << is_HUT << ", head: ";
        head.print_self();
        cout << "tail: ";
        for (deque<pair<Itemset_VERTICAL_UNIT*,int>>::const_iterator i = tail.begin(); i != tail.end(); i++)
        {
            cout << "(" << i->first->item << ", " << i->second << ")";
        }
        cout << endl;
    }
    
    bool mafia(Itemset_VERTICAL head, deque<pair<Itemset_VERTICAL_UNIT*,int>> tail, bool is_HUT) // returning bool is for FHUT pruning
    {
        //print_state(head, tail, is_HUT);
        dynamic_bitset<> mask(DB->size());
        mask.set();
        for (deque<pair<Itemset_VERTICAL_UNIT*,int>>::iterator i = tail.begin(); i != tail.end(); i++)
        {
            mask &= i->first->transactions;
        }
        bool FHUT = is_HUT && ((mask & head.transactions).count() >= threshold);
        for (deque<pair<Itemset_VERTICAL_UNIT*,int>>::iterator i = tail.begin(); i != tail.end(); i++)
        {
            i->second = (int) (head.transactions & i->first->transactions).count();
        }
#if DYNAMIC_REORDER
        sort(tail.begin(), tail.end(), [](const pair<Itemset_VERTICAL_UNIT*,int> & a, const pair<Itemset_VERTICAL_UNIT*,int>  & b) -> bool
        {
            return /*a.second == b.second ? a.first->item < b.first->item :*/ a.second < b.second;
        });
        while (!tail.empty() && tail.back().second == head.support)
        {
            head.add_item(tail.back().first->item);
            tail.pop_back();
        }
        while (!tail.empty() && tail.front().second < threshold)
        {
            tail.pop_front();
        }
#else
        deque<pair<Itemset_VERTICAL_UNIT*,int>>::iterator i = tail.begin();
        while (i != tail.end())
        {
            if (i->second < threshold)
            {
                i = tail.erase(i);
                continue;
            }
            else if (i->second == head.support)
            {
                head.add_item(i->first->item);
                i = tail.erase(i);
                continue;
            }
            i++;
        }
#endif
        if (HUTMFI(head, tail))
        {
            return FHUT;
        }
        if (tail.empty())
        {
            if (max_tree->check_maximal(head.get_tids(), DB))
            {
                result.push_back(new Itemset_VERTICAL(head));
                result.back()->sort_self(DB->sort_method);
                if (verbose)
                {
                    printf("%ld %fs\n", result.size(), elapsed_time());
                }
                //head.print_self();
            }
            return FHUT;
        }
        dynamic_bitset<> transactions_copy = head.transactions;
        bool prune = false, next_is_HUT = true;
        while (!tail.empty() && !prune)
        {
            head.add_item(tail.back().first->item);
            head.transactions &= tail.back().first->transactions;
            head.support = tail.back().second;
            tail.pop_back();
            prune = mafia(head, tail, next_is_HUT);
            head.transactions = transactions_copy;
            head.delete_item();
            next_is_HUT = false;
        }
        return FHUT;
    }
    virtual bool check() const
    {
        return check_basic() && check_threshold(true) && DB->validate_max(result);
    }
    ~MAFIA()
    {
        for (deque<pair<Itemset_VERTICAL_UNIT*,int>>::iterator i = initial_tail.begin(); i != initial_tail.end(); i++)
        {
            delete i->first;
        }
        M_Node::delete_tree(max_tree);
    }
    virtual void mine()
    {
        Miner::mine();
        Itemset_VERTICAL head(DB->size());
        initial_tail = DB->get_item_units();
        max_tree = new M_Node();
        mafia(head, initial_tail, false);
    }
};
