#include <bits/stdc++.h>

using namespace std;

int V = 0; // 顶点数
map<string, vector<string>> g; // 邻接表存储为 map，键为字符串类型的顶点
map<string, int> vertex_map;   // 字符串顶点到整数的映射
vector<string> reverse_map;    // 整数到字符串顶点的反向映射

void tarjan_dfs(int x, int dfn[], int low[], stack<int>& s, bool in_stack[]) {
    static int time = 1;
    dfn[x] = low[x] = time++;
    s.push(x);
    in_stack[x] = true;

    // 遍历与当前顶点 x 连接的所有顶点
    for (const string& neighbor : g[reverse_map[x]]) {
        int y = vertex_map[neighbor];
        if (dfn[y] == 0) {
            tarjan_dfs(y, dfn, low, s, in_stack);
            low[x] = min(low[x], low[y]);
        } else if (in_stack[y]) {
            low[x] = min(low[x], dfn[y]);
        }
    }

    if (dfn[x] == low[x]) {
        int tmp;
        do {
            tmp = s.top();
            s.pop();
            in_stack[tmp] = false;
            cout << reverse_map[tmp] << "-";  // 使用反向映射输出原始字符串顶点
        } while (tmp != x);
        cout << endl;
    }
}

void scc_tarjan() {
    int dfn[V],low[V];
    bool in_stack[V] ;
    stack<int> s;


    fill(dfn, dfn + V, 0);
    fill(low, low + V, 0);
    fill(in_stack, in_stack + V, false);

    for (int i = 0; i < V; i++) {
        if (dfn[i] == 0) {
            tarjan_dfs(i, dfn, low, s, in_stack);
        }
    }
}

int main() {
    // 初始化图的邻接表，顶点为字符串类型
    g["A"] = {"B"};
    g["B"] = {"C"};
    g["C"] = {"A"};
    g["D"] = {"E", "H"};
    g["E"] = {"F"};
    g["F"] = {"A", "G"};
    g["G"] = {"C", "E"};
    g["H"] = {"D", "F"};

    // 将字符串顶点映射为整数
    for (const auto& [key, _] : g) {
        if (vertex_map.find(key) == vertex_map.end()) {
            vertex_map[key] = V++;
            reverse_map.push_back(key);  // 反向映射
        }
        for (const auto& neighbor : g[key]) {
            if (vertex_map.find(neighbor) == vertex_map.end()) {
                vertex_map[neighbor] = V++;
                reverse_map.push_back(neighbor);  // 反向映射
            }
        }
    }

    scc_tarjan(); // 执行 Tarjan 算法

    return 0;
}
