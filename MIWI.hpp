#include <stack>

using namespace std;

#define AMEND 1
#define PRUNING 1

class MIWI : public Miner
{
protected:
    struct Node
    {
        map<int, Node*> children;
        Node* father;
        int count;
        int item;
        stack<int> counts;
        int depth;
        Node(Node* _father, int _item, int _count)
        {
            count = _count;
            item = _item;
            father = _father;
            depth = 0;
        }
    };
    struct Item
    {
        int support;
        vector<Node*> link;
        Item() {support = 0;}
        Item(int _support)
        {
            support = _support;
        }
    };
    map<int, Item> items;
    vector<Item> sorted_items;
    Node* tree;
    M_Node* min_tree;
    Itemset* pattern = DB->new_itemset(0);
    map<int, bool> pruned;
    
public:
    int raw_num = 0;
    float filter_time = 0.0;
    MIWI(TransactionDB* _db, float _ratio) : Miner(_db, _ratio) {}
    MIWI(TransactionDB* _db, int _threshold) : Miner(_db, _threshold) {}
    ~MIWI()
    {
        delete_tree(tree);
        M_Node::delete_tree(min_tree);
    }
    Node* build_tree()
    {
        Node* root = new Node(NULL, -1, -1);
        for (vector<Itemset*>::iterator i = DB->db.begin(); i != DB->db.end(); i++)
        {
#if AMEND
            insert_tree(root, sorted_items, (*i)->get_tids(), 1);
#else
            insert_tree(root, &items, (*i)->get_tids(), 1);
#endif
        }
        return root;
    }
    void print_form(const map<int, Item>& form)
    {
        for (map<int, Item>::const_iterator i = form.begin(); i != form.end(); i++)
        {
            printf("%d %d\n", i->first, i->second.support);
        }
    }
    void insert_tree(Node* temp, map<int, Item>* form, const vector<int> & tids, int count)
    {
        for (vector<const int>::iterator j = tids.begin(); j != tids.end(); j++)
        {
            map<int, Node*>::iterator k = temp->children.find(*j);
            if (k == temp->children.end())
            {
                Node* node = new Node(temp, *j, count);
                temp->children[*j] = node;
                temp = node;
                form->find(*j)->second.link.push_back(node);
            }
            else
            {
                k->second->count += count;
                temp = k->second;
            }
        }
    }
    void insert_tree(Node* temp, vector<Item> & sorted_form, const vector<int> & tids, int count)
    {
        for (vector<const int>::iterator j = tids.begin(); j != tids.end(); j++)
        {
            map<int, Node*>::iterator k = temp->children.find(*j);
            if (k == temp->children.end())
            {
                Node* node = new Node(temp, *j, count);
                temp->children[*j] = node;
                temp = node;
                sorted_form[DB->rank[*j]].link.push_back(node);
            }
            else
            {
                k->second->count += count;
                temp = k->second;
            }
        }
    }
    void delete_tree(Node* temp)
    {
        if (temp == NULL) {return;}
        for (map<int, Node*>::iterator i = temp->children.begin(); i != temp->children.end(); i++)
        {
            delete_tree(i->second);
        }
        delete temp;
    }
    void dig_tree(Node* root, const map<int, Item> & form)
    {
        for (map<int, Item>::const_iterator i = form.begin(); i != form.end(); i++)
        {
            if (verbose)
            {
                if (form.size() == DB->dim()) {printf("%ld/%ld %lus\n", distance(form.begin(), i), form.size(), (clock()-miner_begin)/CLOCKS_PER_SEC);}
            }
            pattern->add_item(i->first);
            pattern->support = i->second.support;
            if (pattern->support < threshold)
            {
                result.push_back(new Itemset_VECTOR(pattern));
                pattern->delete_item();
                continue;
            }
            map<int, Item> next_form;
            Node* next_root = new Node(NULL, -1, -1);
            for (vector<pair<int, int>>::iterator j = DB->sorted_items.begin(); j != DB->sorted_items.end(); j++)
            {
                if (j->first == i->first) {break;}
                next_form.insert(pair<int, Item>(j->first, Item(0)));
            }
            for (vector<Node*>::const_iterator j = i->second.link.begin(); j != i->second.link.end(); j++)
            {
                Node* temp = (*j)->father;
                vector<int> tids;
                while (temp->father != NULL)
                {
                    tids.push_back(temp->item);
                    next_form[temp->item].support += (*j)->count;
                    temp = temp->father;
                }
                reverse(tids.begin(), tids.end());
                insert_tree(next_root, &next_form, (const vector<int>) tids, (*j)->count);
            }
            dig_tree(next_root, next_form);
            pattern->delete_item();
            delete_tree(next_root);
        }
    }
    void identify_leafs(int depth, Node* temp, vector<Node*> & leafs)
    {
        if (temp->depth != depth)
        {
            return;
        }
        if (temp->children.empty() && temp->count < threshold)
        {
            leafs.push_back(temp);
        }
        for (map<int, Node*>::iterator i = temp->children.begin(); i != temp->children.end(); i++)
        {
            identify_leafs(depth, i->second, leafs);
        }
    }
    void identify_pruned(int depth, const vector<Item> & form, vector<int> & pruned_items)
    {
        vector<bool> temp_form(form.size(), true);
        vector<Node*> leafs;
        identify_leafs(depth, tree, leafs);
        for (vector<Node*>::iterator i = leafs.begin(); i != leafs.end(); i++)
        {
            Node* temp = *i;
            int pos = DB->rank[temp->item];
            while (temp != tree && temp_form[pos])
            {
                temp_form[pos] = false;
                temp = temp->father;
            }
        }
        for (vector<Item>::const_iterator i = form.begin(); i != form.end(); i++)
        {
            if (!temp_form[i-form.begin()])
            {
                pruned_items.push_back(DB->sorted_items[i-form.begin()].first);
            }
        }
    }
    void dig_tree_stack(int depth, const vector<Item> & form)
    {
        //DB->print_itemset(pattern);
        vector<Item> sorted_form;
        for (vector<Item>::const_iterator i = form.begin(); i != form.end(); i++)
        {
            int item = DB->sorted_items[int(i-form.begin())].first;
#if PRUNING
            if (pruned[item]) {continue;}
#endif
            if (verbose)
            {
                if (depth == 0) {printf("%ld/%ld %fs\n", distance(form.begin(), i), form.size(), elapsed_time());}
            }
            pattern->add_item(item);
            pattern->support = i->support;
            if (pattern->support < threshold)
            {
                raw_num += 1;
                unsigned long _begin_time = clock();
                if (min_tree->check_minimal(pattern->get_tids(), DB))
                {
                    result.push_back(new Itemset_VECTOR(pattern));
                    //DB->print_itemset(pattern);
                }
                filter_time += float(clock()-_begin_time)/CLOCKS_PER_SEC;
//                else
//                {
//                    printf("non-minimal: ");
//                    DB->print_itemset(pattern);
//                }
                pattern->delete_item();
                continue;
            }
            vector<Item> next_form;
            for (vector<pair<int, int>>::iterator j = DB->sorted_items.begin(); j != DB->sorted_items.end(); j++)
            {
                //printf("[%d %d] [%d %d]\n", j->first, j->second, i->first, i->second.support);
                if (j->first == item) {break;}
                next_form.push_back(Item(0));
            }
            for (vector<Node*>::const_iterator j = i->link.begin(); j != i->link.end(); j++)
            {
                Node* temp = (*j)->father;
                while (temp != tree)
                {
                    if (temp->depth > depth)
                    {
                        temp->count += (*j)->count;
                    }
                    else
                    {
                        temp->counts.push(temp->count);
                        temp->count = (*j)->count;
                        next_form[DB->rank[temp->item]].link.push_back(temp);
                        temp->depth = depth + 1;
                    }
                    next_form[DB->rank[temp->item]].support += (*j)->count;
                    temp = temp->father;
                }
            }
#if PRUNING
            vector<int> pruned_items;
            identify_pruned(depth+1, next_form, pruned_items);
            for (vector<int>::iterator j = pruned_items.begin(); j != pruned_items.end(); j++)
            {
                pruned[*j] = true;
            }
#endif
            //print_form(&next_form);
            dig_tree_stack(depth+1, next_form);
            pattern->delete_item();
#if PRUNING
            for (vector<int>::iterator j = pruned_items.begin(); j != pruned_items.end(); j++)
            {
                pruned[*j] = false;
            }
#endif
            for (vector<Node*>::const_iterator j = i->link.begin(); j != i->link.end(); j++)
            {
                Node* temp = (*j)->father;
                while (temp != tree)
                {
                    if (temp->depth == depth) {break;}
                    temp->depth = depth;
                    temp->count = temp->counts.top();
                    temp->counts.pop();
                    temp = temp->father;
                }
            }
        }
    }
    void antichain()// not working correctly
    {
        clock_t t_begin = clock();
        long average_len = 0, minimal_num = 0, set_num = result.size();
        vector<Itemset*> filtered_result;
        for (vector<Itemset*>::iterator i = result.begin(); i != result.end(); i++)
        {
            if (min_tree->check_minimal((*i)->get_tids(), DB))
            {
                minimal_num += 1;
                filtered_result.push_back(*i);
            }
            average_len += (*i)->get_tids().size();
        }
        result = filtered_result;
        printf("set_num: %ld minimal_num: %ld average_len: %.4f time: %.4f s\n", set_num, minimal_num, float(average_len)/float(set_num), float(clock()-t_begin)/CLOCKS_PER_SEC);
    }
    virtual void mine()
    {
        Miner::mine();
        for (vector<pair<int, int>>::iterator i = DB->sorted_items.begin(); i != DB->sorted_items.end(); i++)
        {
            items.insert(pair<int, Item>(i->first, Item(i->second)));
            sorted_items.push_back(Item(i->second));
            pruned.insert(pair<int, bool>(i->first, false));
        }
        tree = build_tree();
        if (verbose) printf("build tree done.\n");
        min_tree = new M_Node();
#if AMEND
        dig_tree_stack(0, sorted_items);
#else
        dig_tree(tree, items);
        //antichain();
#endif
        for (vector<Itemset*>::iterator i = result.begin(); i != result.end(); i++)
        {
            (*i)->reverse_self();
        }
    }
    virtual bool check() const
    {
        return check_basic() && check_threshold(false) && DB->validate_max(result);
    }
};
