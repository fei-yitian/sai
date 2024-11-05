#include <bits/stdc++.h>

using namespace std;

#define V (8) // 顶点数

vector<vector<int>> g(V); // 邻接表

/**********************************************
    强连通分量 strongly connected component
**********************************************/

void tarjan_dfs(int x, int dfn[], int low[], stack<int>& s, bool in_stack[]) {
    static int time = 1;
    dfn[x] = low[x] = time++;
    s.push(x);
    in_stack[x] = true;

    for (int y : g[x]) {
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
            cout << tmp << "-";
        } while (tmp != x);
        cout << endl;
    }
}

void scc_tarjan() {
    int dfn[V] = {0}, low[V] = {0};
    bool in_stack[V] = {false};
    stack<int> s;

    for (int i = 0; i < V; i++) {
        if (dfn[i] == 0) {
            tarjan_dfs(i, dfn, low, s, in_stack);
        }
    }
}

int main() {
    // 初始化邻接表
    g[0] = {1};
    g[1] = {2};
    g[2] = {0};
    g[3] = {4, 7};
    g[4] = {5};
    g[5] = {0, 6};
    g[6] = {2, 4};
    g[7] = {3, 5};

    scc_tarjan();

    return 0;
}
