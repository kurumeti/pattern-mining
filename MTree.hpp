class M_Node
{
private:
    vector<M_Node*> children;
    M_Node* father = NULL;
    vector<int> path;
    
public:
    M_Node(){}
    M_Node(vector<int>::const_iterator l, vector<int>::const_iterator r)
    {
        while (l != r)
        {
            path.push_back(*l);
            l++;
        }
    }
    static void delete_tree(M_Node* temp)
    {
        if (temp == NULL) {return;}
        for (vector<M_Node*>::iterator i = temp->children.begin(); i != temp->children.end(); i++)
        {
            delete_tree(*i);
        }
        delete temp;
    }
    bool insert_tree(M_Node* temp, const vector<int> & tids, vector<int>::iterator v)
    {
        bool children_covered = false;
        vector<int>::iterator i = temp->path.begin();
        while (i != temp->path.end() && v != tids.end() && *i == *v)
        {
            v++;i++;
        }
        if (i == temp->path.begin() && temp != this)
        {
            return false;
        }
        if (i == temp->path.end())
        {
            for (vector<M_Node*>::iterator j = temp->children.begin(); j != temp->children.end(); j++)
            {
                if (insert_tree(*j, tids, v))
                {
                    children_covered = true;
                    break;
                }
            }
        }
        if (i != temp->path.end() || !children_covered)
        {
            if (i != temp->path.end())
            {
                M_Node* t2 = new M_Node(i, temp->path.end());
                for (vector<M_Node*>::iterator j = temp->children.begin(); j != temp->children.end(); j++)
                {
                    t2->children.push_back(*j);
                }
                temp->children.clear();
                temp->children.push_back(t2);
                i--;
                while (temp->path.back() != *i)
                {
                    temp->path.pop_back();
                }
            }
            M_Node* t1 = new M_Node(v, tids.end());
            temp->children.push_back(t1);
        }
        return true;
    }
    bool check_minimal(const vector<int> & tids, TransactionDB* DB)
    {
        vector<int> ranks;
        for (vector<const int>::iterator i = tids.begin(); i != tids.end(); i++)
        {
            ranks.push_back(DB->rank[*i]);
        }
        reverse(ranks.begin(), ranks.end()); // dual is ok. if not reverse here then just reverse the corresponding inequation in check_min_tree
        if (!check_min_tree(this, ranks, ranks.begin()))
        {
            insert_tree(this, ranks, ranks.begin());
            return true;
        }
        return false;
    }
    bool check_min_tree(M_Node* temp, const vector<int> & tids, vector<int>::iterator v)
    {
        vector<int>::iterator i = temp->path.begin();
        while (i != temp->path.end() && v != tids.end())
        {
            if (*i < *v)
            {
                return false;
            }
            if (*i == *v)
            {
                i++;
            }
            v++;
        }
        if (i == temp->path.end())
        {
            if (temp->children.empty() && temp != this) {return true;}
            for (vector<M_Node*>::iterator j = temp->children.begin(); j != temp->children.end(); j++)
            {
                if (check_min_tree(*j, tids, v)) {return true;}
            }
        }
        return false;
    }
    bool check_max_tree(M_Node* temp, const vector<int> & tids, vector<int>::iterator v)
    {
        vector<int>::iterator i = temp->path.begin();
        while (i != temp->path.end() && v != tids.end())
        {
            if (*i > *v)
            {
                return false;
            }
            if (*i == *v)
            {
                v++;
            }
            i++;
        }
        if (v == tids.end())
        {
            return true;
        }
        else
        {
            for (vector<M_Node*>::iterator j = temp->children.begin(); j != temp->children.end(); j++)
            {
                if (check_max_tree(*j, tids, v)) {return true;}
            }
        }
        return false;
    }
    bool check_maximal(const vector<int> & tids, TransactionDB* DB, bool no_insert = false)
    {
        vector<int> ranks;
        for (vector<const int>::iterator i = tids.begin(); i != tids.end(); i++)
        {
            ranks.push_back(DB->rank[*i]); //put rank in here
        }
        stable_sort(ranks.begin(), ranks.end());
        if (!check_max_tree(this, ranks, ranks.begin()))
        {
            if(!no_insert)
            {
                insert_tree(this, ranks, ranks.begin());
            }
            return true;
        }
        return false;
    }
};
