#include <iostream>
#include <vector>
#include <string>
#include "vpi_user.h"
#include "sv_vpi_user.h"
#include <unordered_map>
#include <unordered_set>
#include <stack>
#include <list>
#include <algorithm>
#include <set>

using namespace std;

typedef vector<string> StrVec;

class GateInst {
public:
    GateInst(const char* pName) {
        _instName = pName ? pName : "Unknown";
    }

    void setGateType(const char* pName) {
        _gateType = pName ? pName : "Unknown";
    }

    void addNodeName(const char* pName) {
        _instNodes.push_back(pName ? pName : "Unknown");
    }

    string _gateType;
    string _instName;
    StrVec _instNodes;
};

class shixian {
public:
    vector<vector<string>> all_SCC;
    vector<vector<string>> sorted_Gates;
    vector<vector<string>> sorted_Signals;
    unordered_map<string, string> _gateTypeMap;

    void tarjan_dfs(int x, int dfn[], int low[], stack<int>& s, bool in_stack[], unordered_map<string, int>& vertex_map, vector<string>& reverse_map, unordered_map<string, list<string>>& g) {
        static int time = 1;
        dfn[x] = low[x] = time++;
        s.push(x);
        in_stack[x] = true;

        for (const string& neighbor : g[reverse_map[x]]) {
            int y = vertex_map[neighbor];
            if (dfn[y] == 0) {
                tarjan_dfs(y, dfn, low, s, in_stack, vertex_map, reverse_map, g);
                low[x] = min(low[x], low[y]);
            } else if (in_stack[y]) {
                low[x] = min(low[x], dfn[y]);
            }
        }

        if (dfn[x] == low[x]) {
            int tmp;
            vector<string> scc;
            do {
                tmp = s.top();
                s.pop();
                in_stack[tmp] = false;
                scc.push_back(reverse_map[tmp]);
            } while (tmp != x);

            if (scc.size() > 1) {  // Skip SCCs with a single node (without a self-loop)
                all_SCC.push_back(scc);
            }
        }
    }

    void scc_tarjan(const int V, unordered_map<string, int>& vertex_map, vector<string>& reverse_map, unordered_map<string, list<string>>& g) {
        int dfn[V], low[V];
        bool in_stack[V];
        stack<int> s;

        fill(dfn, dfn + V, 0);
        fill(low, low + V, 0);
        fill(in_stack, in_stack + V, false);

        for (int i = 0; i < V; i++) {
            if (dfn[i] == 0) {
                tarjan_dfs(i, dfn, low, s, in_stack, vertex_map, reverse_map, g);
            }
        }
    }

        // 辅助函数：检查是否有环
    bool isCyclicUtil(const string& cur, unordered_map<string, list<string>>& graph) {
        if (visited[cur]) return false; // 已访问，直接返回
        visited[cur] = true;
        recStack[cur] = true;
        path.push_back(cur);

        for (const string& nei : graph[cur]) {
            if (!visited[nei]) {
                if (isCyclicUtil(nei, graph)) return true; // 直接返回 true
            } else if (recStack[nei]) {
                vector<string> cycle;
                auto it = find(path.begin(), path.end(), nei);
                cycle.insert(cycle.end(), it, path.end()); // 使用插入
                cycle.push_back(nei);
                allCycles.push_back(cycle);
            }
        }

        recStack[cur] = false;
        path.pop_back();
        return false;
    }

    void detectCycles(unordered_map<string, string>& gateTypeMap) {
        _gateTypeMap = gateTypeMap;

        for (const auto& SCC : all_SCC) {
            vector<string> loopSignals, loopGates;
            for (const auto& node : SCC) {
                if (node[0] == 'I') loopGates.push_back(node);
                else if (node[0] == 'w') loopSignals.push_back(node);
            }

            set<string> uniqueSignals(loopSignals.begin(), loopSignals.end());
            sorted_Signals.push_back(vector<string>(uniqueSignals.begin(), uniqueSignals.end()));

            set<string> uniqueGates(loopGates.begin(), loopGates.end());
            //我直接在这里找环
                for (const auto& node : uniqueGates) {
                    if (!visited[node.first]) {
                        isCyclicUtil(node.first, uniqueGates);
                    }
                }
            sorted_Gates.push_back(vector<string>(uniqueGates.begin(), uniqueGates.end()));
        }
    }

    void print_result1() {
        cout << "******* result_1.txt *********\n";

        for (size_t i = 0; i < all_SCC.size(); ++i) {
            cout << i + 1 << ")\n";
            cout << "  Loop Signals: ";
            for (const auto& signal : sorted_Signals[i]) {
                cout << signal << ", ";
            }
            cout << "\n";

            cout << "  Loop Gates: ";
            for (const auto& gate : sorted_Gates[i]) {
                cout << gate << ", ";
            }
            cout << "\n\n";
        }
    }
};

typedef vector<GateInst*> GateInstVec;
shixian A;

class NetlistModule {
public:
    ~NetlistModule() {
        for (auto& inst : _instVec) {
            delete inst;
        }
    }

    void setModuleName(const char* pName) {
        if (pName) _moduelName = pName;
    }

    void addInst(GateInst* pInst) {
        _instVec.push_back(pInst);
    }

    void buildGraphAndDetectCycles() {
        unordered_map<string, list<string>> graph;
        unordered_map<string, int> vertex_map;
        vector<string> reverse_map;
        unordered_map<string, string> gateTypeMap;
        int vertexCount = 0;

        // Reserve space to avoid dynamic resizing
        vertex_map.reserve(_instVec.size() * 3);  // Reserve space for vertices
        reverse_map.reserve(_instVec.size() * 3); // Reserve space for reverse mapping
        graph.reserve(_instVec.size() * 3);       // Reserve space for edges

        for (auto& pInst : _instVec) {
            string output = pInst->_instNodes[0];
            string gate_port1 = pInst->_instName + ".port1";
            string gate_port2 = pInst->_instName + ".port2";
            gateTypeMap[gate_port1] = pInst->_gateType;
            gateTypeMap[gate_port2] = pInst->_gateType;

            if (vertex_map.find(gate_port1) == vertex_map.end()) {
                vertex_map[gate_port1] = vertexCount++;
                reverse_map.push_back(gate_port1);
            }
            if (vertex_map.find(gate_port2) == vertex_map.end()) {
                vertex_map[gate_port2] = vertexCount++;
                reverse_map.push_back(gate_port2);
            }
            if (vertex_map.find(output) == vertex_map.end()) {
                vertex_map[output] = vertexCount++;
                reverse_map.push_back(output);
            }

            graph[gate_port1].push_back(output);
            graph[gate_port2].push_back(output);

            for (size_t jj = 1; jj < pInst->_instNodes.size(); ++jj) {
                string input = pInst->_instNodes[jj];
                if (vertex_map.find(input) == vertex_map.end()) {
                    vertex_map[input] = vertexCount++;
                    reverse_map.push_back(input);
                }

                if (jj == 1)
                    graph[input].push_back(gate_port1);
                else
                    graph[input].push_back(gate_port2);
            }
        }

        A.scc_tarjan(vertexCount, vertex_map, reverse_map, graph);
        A.detectCycles(gateTypeMap);
    }

    string _moduelName;
    GateInstVec _instVec;
};

NetlistModule _glbNetlistModule;

void printModInfo() {
    _glbNetlistModule.buildGraphAndDetectCycles();
    A.print_result1();
}
