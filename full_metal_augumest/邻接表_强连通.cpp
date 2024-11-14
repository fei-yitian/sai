#include <bits/stdc++.h>
using namespace std;

// Tarjan算法用于寻找强连通分量(SCC)
void tarjan_dfs(int x, vector<int>& dfn, vector<int>& low, stack<int>& s, vector<bool>& in_stack, const vector<vector<int>>& g) {
    static int time = 1;
    dfn[x] = low[x] = time++;  // 记录发现时间
    s.push(x);  // 将当前节点入栈
    in_stack[x] = true;

    // 遍历当前节点x的所有邻接节点
    for (int y : g[x]) {
        if (dfn[y] == 0) {  // 如果y没有被访问过，进行DFS
            tarjan_dfs(y, dfn, low, s, in_stack, g);
            low[x] = min(low[x], low[y]);  // 更新当前节点的低链接值
        } else if (in_stack[y]) {  // 如果y在栈中，说明是当前强连通分量的一部分
            low[x] = min(low[x], dfn[y]);
        }
    }

    // 如果dfn[x] == low[x]，说明找到了一个强连通分量
    if (dfn[x] == low[x]) {
        int tmp;
        // 输出一个强连通分量
        do {
            tmp = s.top();
            s.pop();
            in_stack[tmp] = false;
            cout << tmp << " ";  // 输出SCC中的节点
        } while (tmp != x);
        cout << endl;
    }
}

void scc_tarjan(int V, const vector<vector<int>>& g) {
    vector<int> dfn(V, 0), low(V, 0);  // 存储每个节点的发现时间和低链接值
    vector<bool> in_stack(V, false);  // 标记节点是否在栈中
    stack<int> s;

    // 对所有未访问的节点执行DFS
    for (int i = 0; i < V; i++) {
        if (dfn[i] == 0) {
            tarjan_dfs(i, dfn, low, s, in_stack, g);
        }
    }
}

int main() {

    int V = 8;  // 顶点数
    vector<vector<int>> g(V);  // 邻接表
    // 初始化邻接表
    g[0] = {1};
    g[1] = {2};
    g[2] = {0};
    g[3] = {4, 7};
    g[4] = {5};
    g[5] = {0, 6};
    g[6] = {2, 4};
    g[7] = {3, 5};

    scc_tarjan(V,g);

    return 0;
}
