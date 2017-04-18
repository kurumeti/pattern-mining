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
using namespace std;

// first: 候选项集
// second: 事务索引
typedef pair<set<int>, set<int>> Tidset;

// 读取文件并返回为初始的一项集及事务数
pair<int, vector<vector<Tidset>>> singleItemTidsets(string fileName) {
    map<int, set<int>> singleItemIndices;
    vector<Tidset> tidsets;
    
    ifstream readStream(fileName);
    string line;
    int count = 0;
    while (getline(readStream, line)) { // 读取一行
        std::istringstream iss(line);
        int item;
        while (iss >> item) { // 读取数字
            singleItemIndices[item].insert(count);
        }
        count += 1;
    }
    for (auto s = singleItemIndices.begin(); s != singleItemIndices.end(); s++) {
        set<int> singleItemSet;
        singleItemSet.insert(s->first);
        tidsets.push_back(Tidset(singleItemSet, s->second));
    }
    readStream.close();
    
    vector<vector<Tidset>> eclatSingles;
    eclatSingles.push_back(tidsets);
    return make_pair(count, eclatSingles);
}

// 计算当前tidsets的相互交集,返回高于minSup的交集
vector<vector<Tidset>> eclatIntersect(vector<vector<Tidset>> tidsetsGroups, int minSup) {
    vector<vector<Tidset>> tidsetsIntersections;
    for_each(tidsetsGroups.begin(), tidsetsGroups.end(),
             [&tidsetsIntersections, minSup](vector<Tidset> const& tidsetGroup) {
                 if (tidsetGroup.size() == 0) return;
                 for (auto s = tidsetGroup.begin(); s != tidsetGroup.end() - 1; s++) {
                     vector<Tidset> tidsetNewGroup;
                     for (auto e = s + 1; e != tidsetGroup.end(); e++) {
                         set<int> intersectIndices;
                         set_intersection(s->second.begin(), s->second.end(),
                                          e->second.begin(), e->second.end(),
                                          inserter(intersectIndices, intersectIndices.begin()));
                         if (intersectIndices.size() >= minSup) {
                             set<int> itemSet;
                             set_union(s->first.begin(), s->first.end(),
                                       e->first.begin(), e->first.end(),
                                       inserter(itemSet, itemSet.begin()));
                             tidsetNewGroup.push_back(Tidset(itemSet, intersectIndices));
                         }
                     }
                     tidsetsIntersections.push_back(tidsetNewGroup);
                 }
             }
             );
    
    return tidsetsIntersections;
}

// 根据所有一项集进行迭代,将所有支持度大于minSup的项集由writeStream输出
void eclat(vector<vector<Tidset>> singleTidsets, int minSup, std::ofstream& writeStream) {
    vector<vector<Tidset>> freqTidsets(1);
    int freqCount = 0;
    // 获取频繁一项集
    for_each(singleTidsets[0].begin(), singleTidsets[0].end(),
             [&freqTidsets, minSup](Tidset const & tidset) {
                 if (tidset.second.size() >= minSup)
                     freqTidsets[0].push_back(tidset); });
    int stepCount = 1;
    while (!freqTidsets.empty()) {
        int countOfStep = 0;
        for_each(freqTidsets.begin(), freqTidsets.end(),
                 [&countOfStep](vector<Tidset> const& group) { countOfStep += group.size(); });
        //cout << "挖掘" << stepCount++ << "项集: 共" << countOfStep << endl;
        for (auto groupIter = freqTidsets.begin(); groupIter != freqTidsets.end(); groupIter++) {
            for (auto freqTidsetIter = groupIter->begin(); freqTidsetIter != groupIter->end(); freqTidsetIter++) {
                // 输出频繁项集
                for_each(freqTidsetIter->first.begin(), freqTidsetIter->first.end(),
                         [&writeStream](int const & item) { writeStream << item << " "; });
                // 输出频繁项计数
                //writeStream << " : " << freqTidsetIter->second.size() << endl;
            }
            
        }
        freqTidsets = eclatIntersect(freqTidsets, minSup);
    }
    //cout << "最小支持度为 " << minSup << " 的项集数目为 " <<  freqCount << endl;
}


int main() {
    
    
    string fileName = "mushroom.dat";
    //cin >> fileName;
    
    // 读入一项集
    auto beginTime = std::chrono::high_resolution_clock::now();
    pair<int, vector<vector<Tidset>>> countAndTidsets = singleItemTidsets(fileName);
    int transCount = countAndTidsets.first;
    vector<vector<Tidset>> singleTidsets = countAndTidsets.second;
    
    auto nowTime = std::chrono::high_resolution_clock::now();
    auto elapsed_usec = std::chrono::duration_cast<std::chrono::milliseconds>(nowTime - beginTime).count();
    cout << "读入完毕: " << elapsed_usec << "ms" << endl;
    
    double minSupPercent;
    std::ofstream writeStream("result.txt");
    while (cin >> minSupPercent) {
        
        beginTime = std::chrono::high_resolution_clock::now();
        
        int minSup = transCount * minSupPercent;
        
        cout << "开始eclat算法" << endl;
        eclat(singleTidsets, minSup, writeStream);
        
        nowTime = std::chrono::high_resolution_clock::now();
        elapsed_usec = std::chrono::duration_cast<std::chrono::milliseconds>(nowTime - beginTime).count();
        
        cout << "用时: " << elapsed_usec << "ms" << endl;
    }
    writeStream.close();
    return 0;
}

