#include <iostream>
#include <vector>
#include <unordered_map>
#include <list>
#include <string>
#include <unordered_set>
#include <algorithm>

using namespace std;

// 图的邻接表表示，使用 unordered_map<string, list<string>> 表示图
unordered_map<string, list<string>> graph;

// 记录每个节点的访问状态
unordered_map<string, bool> visited;
unordered_map<string, bool> recStack;

// 存储所有检测到的环路
vector<vector<string>> allCycles;

// 当前路径
vector<string> path;

// 检查是否有环的辅助函数（递归），并记录环路
bool isCyclicUtil(const string& cur) {
    // 标记当前节点为已访问，并加入递归栈和当前路径
    visited[cur] = true;
    recStack[cur] = true;
    path.push_back(cur);

    // 遍历当前节点的所有相邻节点
    for (const string& nei : graph[cur]) {
        if (!visited[nei]) {
            if (isCyclicUtil(nei))
                ; // 继续查找其他环路
        }
        else if (recStack[nei]) {
            // 找到一个环路
            vector<string> cycle;
            // 从路径中找到环路的起始位置
            auto it = find(path.begin(), path.end(), nei);
            if (it != path.end()) {
                for (; it != path.end(); ++it) {
                    cycle.push_back(*it);
                }
                cycle.push_back(nei); // 完成环路
                allCycles.push_back(cycle);
            }
        }
    }

    // 移除当前节点从递归栈和路径
    recStack[cur] = false;
    path.pop_back();
    return false;
}

// 检测图中有多少个环，并记录所有环路
int countCyclesInDirectedGraph() {
    // 初始化访问状态
    for (const auto& node : graph) {
        visited[node.first] = false;
        recStack[node.first] = false;
    }

    // 遍历所有节点，检查每个节点是否存在环
    for (const auto& node : graph) {
        const string& nodeName = node.first;
        if (!visited[nodeName]) {
            isCyclicUtil(nodeName);
        }
    }

    return allCycles.size();  // 返回环的总数
}

int main() {
    // 定义图的邻接表，节点使用字符串
    graph["w0"] = {"I1"};
    graph["I1"] = {"W1"};
    graph["W1"] = {"I1"};

    // 统计图中的环数
    int cycles = countCyclesInDirectedGraph();

    cout << "图中有 " << cycles << " 个环路" << endl;

    // 打印所有环路
    for (size_t i = 0; i < allCycles.size(); ++i) {
        cout << "环路 " << i + 1 << ": ";
        for (size_t j = 0; j < allCycles[i].size(); ++j) {
            cout << allCycles[i][j];
            if (j != allCycles[i].size() - 1)
                cout << " -> ";
        }
        cout << endl;
    }

    return 0;
}