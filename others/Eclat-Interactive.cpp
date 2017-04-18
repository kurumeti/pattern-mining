//
//  main.cpp
//  test
//
//  Created by 郝有峰 on 15/6/12.
//  Copyright (c) 2015年 郝有峰. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <algorithm>
#include <map>
#include <set>
#include <vector>
#include <queue>
#include <sstream>
#include <stack>
#include <chrono>
#include <bitset>
using namespace std;

struct Node;

typedef priority_queue<pair<int, Node*>> NegaTable;
const int itemsMax = 870;
const int transMax = 100000;

// bitset类
// 因为bitset::count()是O(n)时间,因此直接设置变量trueCount存储非空值方便读取
template<size_t N>
class MyBitset {
private:
    size_t trueCount;
    bitset<N> data;
public:
    const size_t count() {
        return trueCount;
    };
    const size_t size() {
        return data.size();
    }
    MyBitset operator & (const MyBitset & bs) {
        MyBitset newBs;
        newBs.data = this->data & bs.data;
        newBs.trueCount = newBs.data.count();
        return newBs;
    };
    MyBitset operator | (const MyBitset & bs) {
        MyBitset newBs;
        newBs.data = this->data | bs.data;
        newBs.trueCount = newBs.data.count();
        return newBs;
    };
    void set(int i) {
        if (!this->data[i])
            trueCount++;
        this->data.set(i);
    }
    bool operator [](const int i) {
        return this->data[i];
    };
    MyBitset<N>() {
        trueCount = 0;
        data.reset();
    };
};

struct Node {
    MyBitset<itemsMax> items;
    MyBitset<transMax> transIndices;
    Node* parent;
    vector<Node*> children;
    const Node operator & (const Node & node) { // 重载操作符
        Node addNode;
        addNode.parent = this;
        addNode.items = this->items | node.items;
        addNode.transIndices = this->transIndices & node.transIndices;
        return addNode;
    }
    Node(): parent(nullptr) {};
    ~Node() {
        for (auto child: children) {
            delete child;
        }
        children.clear();
    }
    void addChildren(Node *child) {
        this->children.push_back(child);
        child->parent = this;
    }
    void intersectWithLeftSiblings(NegaTable & negaTable, int minSup) {
        if (transIndices.count() < minSup) return;
        if (parent == nullptr) return;
        for (auto sibling: this->parent->children) {
            if (sibling == this) {
                break;
            } else {
                if (sibling->transIndices.count() < minSup) {
                    continue;
                }
                Node *newNode = new Node();
                *newNode = *sibling & *this;
                negaTable.push(make_pair(newNode->transIndices.count(), newNode));
            }
        }
    }
};


struct EclatTree {
    
public:
    int getResult(ofstream & writeStream) {
        int count = 0;
        writeStream << "Min sup: " << minSup << endl;
        queue<Node*> result;
        for (auto node: root->children) {
            if (node->transIndices.count() >= minSup) {
                result.push(node);
            }
        }
        while (!result.empty()) {
            Node* node = result.front();
            result.pop();
            for (int i = 0; i < node->items.size(); i++) {
                if (node->items[i]) {
                    writeStream << index2Item[i] << " ";
                }
            }
            writeStream << ": " << node->transIndices.count() << endl;
            count++;
            for (auto child: node->children) {
                if (child->transIndices.count() >= minSup) {
                    result.push(child);
                }
            }
        }
        return count;
    }
    
    void setMinSupPercent(double newMinSupPercent) {
        int newMinSup = totalCount * newMinSupPercent;
        if (minSup > newMinSup) {
            minSup = newMinSup;
            run();
        }
        minSup = newMinSup;
    }
    
    void run() {
        while (negaTable.top().first >= minSup) {
            Node *node = negaTable.top().second;
            negaTable.pop();
            node->parent->addChildren(node);
            node->intersectWithLeftSiblings(negaTable, minSup);
        }
    }
    
    EclatTree(string fileName) {
        root = new Node();
        totalCount = 0;
        minSup = INT_MAX;
        map<int, set<int>> singleItemIndices;
        
        ifstream readStream(fileName);
        string line;
        int itemCount = 0;
        while (getline(readStream, line)) { // 读取一行
            std::istringstream iss(line);
            int item;
            while (iss >> item) { // 读取数字
                if (item2Index.find(item) == item2Index.end()) {
                    item2Index[item] = itemCount;
                    index2Item[itemCount] = item;
                    itemCount++;
                }
                singleItemIndices[item2Index[item]].insert(totalCount);
            }
            totalCount += 1;
        }
        readStream.close();
        cout << "项的bitset设置空间为" << itemsMax
        << ", 共" << itemCount << "." <<  endl;
        cout << "事务的bitset设置空间为" << transMax
        << ", 共" << totalCount << "." << endl;
        if (itemCount > itemsMax || totalCount > transMax) {
            cout << "Error: bitset空间不足, 请在代码中重新设置bitset." <<  endl;
            exit(1);
        }
        for (auto it: singleItemIndices) {
            set<int> singleItemSet;
            Node *node = new Node();
            node->items.set(it.first);
            for_each(it.second.begin(), it.second.end(),
                     [&node](const int &transIndex){ node->transIndices.set(transIndex); });
            node->parent = root;
            negaTable.push(make_pair(node->transIndices.count(), node));
        }
    }
    
    ~EclatTree() {
        delete root;
    }
    
    int getTotalCount() {
        return totalCount;
    }
private:
    Node* root;
    NegaTable negaTable;
    int minSup;
    int totalCount;
    map<int, int> index2Item;
    map<int, int> item2Index;
};

int main() {
    
    string fileName;
    int minSup;
    double minSupPercent;
    
    cin >> fileName;
    
    // 读入一项集
    auto beginTime = std::chrono::high_resolution_clock::now();
    EclatTree eclatTree(fileName);
    auto nowTime = std::chrono::high_resolution_clock::now();
    auto elapsed_usec = std::chrono::duration_cast<std::chrono::milliseconds>(nowTime - beginTime).count();
    cout << "读入完毕: " << elapsed_usec << "ms" << endl;
    
    
    std::ofstream writeStream("result.txt"); //输出至result.txt
    while (cin >> minSupPercent) {
        minSup = eclatTree.getTotalCount() * minSupPercent;
        beginTime = std::chrono::high_resolution_clock::now();
        
        cout << "开始eclat算法" << endl;
        eclatTree.setMinSupPercent(minSupPercent);
        int count = eclatTree.getResult(writeStream);
        
        nowTime = std::chrono::high_resolution_clock::now();
        elapsed_usec = std::chrono::duration_cast<std::chrono::milliseconds>(nowTime - beginTime).count();
        
        cout << "支持度大于 " << minSupPercent << " 共 " << count << endl;
        cout << "用时: " << elapsed_usec << "ms" << endl;
    }
    writeStream.close();
    return 0;
}

