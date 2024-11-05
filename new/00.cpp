#include <iostream>
#include <stdio.h>
#include <stdlib.h>
// #include "vpi_user.h"
// #include "sv_vpi_user.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <list>
#include <bits/stdc++.h>
#include <unordered_set>
#include <set>


using namespace std;

typedef vector<string>   StrVec;

int V = 0; // 顶点数

class GateInst  //具体某个门的信息
{
public:
    GateInst(const char* pName)
    { 
        _instName = pName ? pName : "Unknown";
    }

    void setGateType(const char* pName)
    {
        _gateType = pName ? pName : "Unknown";
    }

    void addNodeName(const char* pName)
    {
        _instNodes.push_back(pName ? pName : "Unknown");
    }

    // void dumpInst()
    // {
    //     printf("%-5s  %s  ", _gateType.c_str(), _instName.c_str());
    //     for(unsigned ii = 0; ii != _instNodes.size(); ++ii)
    //     {
    //         printf("%s  ", _instNodes[ii].c_str());
    //     }
    //     printf("\n");
    // }

    string  _gateType; // 门的类型
    string  _instName; // 门的名称
    StrVec  _instNodes; // 线的名称
};

class shixian{
public:
    vector<vector<string>> all_SCC;//用于存储所有强连通分量
    vector<vector<string>> sorted_Gates;//门结点和线结点分开并排序
    vector<vector<string>> sorted_Signals;
    unordered_map<string, string> _gateTypeMap;//门结点的类型映射

void tarjan_dfs(int x, int dfn[], int low[], stack<int>& s, bool in_stack[],map<string, int> &vertex_map,vector<string> &reverse_map,unordered_map<string, list<string>> &g) {
    static int time = 1;
    dfn[x] = low[x] = time++;
    s.push(x);
    in_stack[x] = true;

    // 遍历与当前顶点 x 连接的所有顶点
    for (const string& neighbor : g[reverse_map[x]]) {
        int y = vertex_map[neighbor];
        if (dfn[y] == 0) {
            tarjan_dfs(y, dfn, low, s, in_stack,vertex_map,reverse_map,g);
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

void scc_tarjan(map<string, int> &vertex_map,vector<string> &reverse_map,unordered_map<string, list<string>> &g) {
    int dfn[V],low[V];
    bool in_stack[V] ;
    stack<int> s;

    for (int i = 0; i < V; i++) {
        if (dfn[i] == 0) {
            tarjan_dfs(i, dfn, low, s, in_stack,vertex_map,reverse_map,g);
        }
    }
}
void detectCycles(unordered_map<string, list<string>>& graph,unordered_map<string, string>& gateTypeMap) 
{
    _gateTypeMap=gateTypeMap;
   

    // 整理环路信息
    for (const auto& cycle : all_SCC) {
        vector<string> loopSignals, loopGates;
        for (const auto& signal : cycle) {                //如果result4还是要原来的答案格式，可以从这里下手修改
            if (signal[0] == 'I') loopGates.push_back(signal);
            else if (signal[0] == 'w') loopSignals.push_back(signal);
        }

        // 使用 set 进行去重并排序
        set<string> uniqueSignals(loopSignals.begin(), loopSignals.end());
        vector<string> sortedSignals(uniqueSignals.begin(), uniqueSignals.end());
        sorted_Signals.push_back(sortedSignals);

        set<string> uniqueGates(loopGates.begin(), loopGates.end());
        vector<string> sortedGates(uniqueGates.begin(), uniqueGates.end());
        sorted_Gates.push_back(sortedGates);
    }
}

void print_result1()
{

        cout << "******* result_1.txt *********"<<endl;

        for (size_t i = 0; i < all_SCC.size(); ++i){
            // 打印环路，按照你的期望格式
            cout << i + 1 << ")" << endl;
            cout << "  Loop Signals: ";
            for (size_t j = 0; j < sorted_Signals[i].size(); ++j) {
                cout << sorted_Signals[i][j];
                cout << ", ";
            }
            cout << endl;

            cout << "  Loop Gates: ";
            for (size_t j = 0; j < sorted_Gates[i].size(); ++j) {
                cout << sorted_Gates[i][j];
                cout << ", ";
            }
            cout << endl << endl;
        }
}
};

typedef vector<GateInst*>  GateInstVec;
shixian A;

class NetlistModule //一个网表信息
{
public:
    ~NetlistModule()
    {
        for(unsigned ii = 0; ii != _instVec.size(); ++ii)
        {
            delete _instVec[ii];
        }
    }

    void setModuleName(const char* pName)
    {
        if(pName)
            _moduelName = pName;
    }

    void addInst(GateInst* pInst)
    {
        _instVec.push_back(pInst);
    }

    // void dumpInst()
    // {
    //     for(unsigned ii = 0; ii != _instVec.size(); ++ii)
    //     {
    //         GateInst* pInst = _instVec[ii];
    //         if(pInst)
    //         {
    //             printf("%-3d: ", ii);
    //             pInst->dumpInst();
    //         }
    //     }
    // }

    void buildGraphAndDetectCycles() {
        unordered_map<string, list<string>> graph;
        map<string, int> vertex_map;   // 字符串顶点到整数的映射
        vector<string> reverse_map;    // 整数到字符串顶点的反向映射
        unordered_map<string, string> gateTypeMap;// 新增，用于存储门名称和门类型的映射
        // 构建图结构（邻接表）
        for (unsigned ii = 0; ii != _instVec.size(); ++ii) {
            GateInst* pInst = _instVec[ii];
            
            // 第一个节点通常是输出，剩下的节点是输入
            string output = pInst->_instNodes[0];  // 输出信号
            string gate_port1 = pInst->_instName + ".port1";  //门结点
            string gate_port2 = pInst->_instName + ".port2";
            gateTypeMap[gate_port1] = pInst->_gateType;
            gateTypeMap[gate_port2] = pInst->_gateType;
           
            graph[gate_port1].push_back(output);  //将门结点连接到输出信号
            graph[gate_port2].push_back(output);
            //cout << "Adding edge from " << gate << " to " << output << endl;
            for (unsigned jj = 1; jj < pInst->_instNodes.size(); ++jj) {
                string input = pInst->_instNodes[jj];  // 输入信号
                
                // 将输入信号连接到门结点
                if(jj==1)
                    graph[input].push_back(gate_port1);
                else
                    graph[input].push_back(gate_port2);
                //cout << "Adding edge from " << input << " to " << gate << endl;
            }
        }
        for (const auto& [key, _] : graph) {
        if (vertex_map.find(key) == vertex_map.end()) {
            vertex_map[key] = V++;
            reverse_map.push_back(key);  // 反向映射
        }
        for (const auto& neighbor : graph[key]) {
            if (vertex_map.find(neighbor) == vertex_map.end()) {
                vertex_map[neighbor] = V++;
                reverse_map.push_back(neighbor);  // 反向映射
            }
        }
    }

        // 调用环路检测函数
        A.scc_tarjan(vertex_map,reverse_map,graph);
        A.detectCycles(graph,gateTypeMap);// 传递 gateTypeMap 用于后续环路检测时获取门的类型

    }

    string      _moduelName;
    GateInstVec _instVec;  //一个类，包含门的信息
};

NetlistModule _glbNetlistModule;

void printModInfo()
{
    // printf("module name is %s\n", _glbNetlistModule._moduelName.c_str());
    // _glbNetlistModule.dumpInst();
}

/*

#ifdef  __cplusplus
extern "C" {
#endif

int get_module_info( )
{
    vpiHandle systf_h = vpi_handle( vpiSysTfCall, NULL );
    if( systf_h == NULL )
        return 0;

    vpiHandle modHdl = vpi_handle( vpiScope,systf_h );
    modHdl = vpi_handle(vpiModule, modHdl);
    char *pModDefName = vpi_get_str(vpiDefName, modHdl);
    _glbNetlistModule.setModuleName(pModDefName);

    vpiHandle instIter = vpi_iterate(vpiInstance, modHdl);
    vpiHandle instHdl = NULL;
    while( ( instHdl = vpi_scan( instIter ) ) )
    {
        char* pInstName = vpi_get_str( vpiName, instHdl );
        GateInst* pGateInst = new GateInst(pInstName);

        char *pModDefName = vpi_get_str(vpiDefName, instHdl);
        pGateInst->setGateType(pModDefName);

        vpiHandle portIter = vpi_iterate(vpiPort, instHdl);
        vpiHandle portHdl = NULL;
        while ( (portHdl = vpi_scan(portIter)) )
        {
            vpiHandle highConnHdl = vpi_handle(vpiHighConn, portHdl);
            char* pPortName = vpi_get_str(vpiName, highConnHdl);
            pGateInst->addNodeName(pPortName);
        }

        _glbNetlistModule.addInst(pGateInst);
    }

    printModInfo();
    vpi_control(vpiFinish);
    return 0;
}

#ifdef  __cplusplus
}
#endif

*/
