
#include <iostream> 
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <unordered_map>
#include "vpi_user.h"
#include "sv_vpi_user.h"
#include <list>
#include <unordered_set>
#include <algorithm>

using namespace std;

typedef vector<string> StrVec;

// 函数声明
int detectCycles(unordered_map<string, list<string>>& graph);

class GateInst // 门信息
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

    void dumpInst() // 打印信息
    {
        printf("%-5s  %s  ", _gateType.c_str(), _instName.c_str());
        for(unsigned ii = 0; ii != _instNodes.size(); ++ii)
        {
            printf("%s  ", _instNodes[ii].c_str());
        }
        printf("\n");
    }

    string  _gateType; // 门的类型
    string  _instName; // 门的名字
    StrVec  _instNodes; // 节点名称
};

typedef vector<GateInst*> GateInstVec;

class NetlistModule // 电路模块，包含多个门信息
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

    void dumpInst()
    {
        for(unsigned ii = 0; ii != _instVec.size(); ++ii)
        {
            GateInst* pInst = _instVec[ii];
            if(pInst)
            {
                printf("%-3d: ", ii);
                pInst->dumpInst();
            }
        }
    }

    void buildGraphAndDetectCycles() {
    unordered_map<string, list<string>> graph;

    // 构建图结构（邻接表）
    for (unsigned ii = 0; ii != _instVec.size(); ++ii) {
        GateInst* pInst = _instVec[ii];

        if (pInst->_instNodes.size() < 2) {
            continue; // 每个门至少需要两个节点（输入和输出）
        }

        // 第一个节点通常是输出，剩下的节点是输入
        string output = pInst->_instNodes[0];  // 输出信号
        for (unsigned jj = 1; jj < pInst->_instNodes.size(); ++jj) {
            string input = pInst->_instNodes[jj];  // 输入信号

            // 将输入信号连接到输出信号
            graph[input].push_back(output);
            cout << "Adding edge from " << input << " to " << output << endl;
        }
    }

    // 调用环路检测函数
    detectCycles(graph);
}


    string _moduelName;
    GateInstVec _instVec;
};

// 环路检测相关的代码
unordered_map<string, bool> visited;
unordered_map<string, bool> recStack;
vector<vector<string>> allCycles;
vector<string> path;

// 辅助函数：检查是否有环
bool isCyclicUtil(const string& cur, unordered_map<string, list<string>>& graph) {
    visited[cur] = true;
    recStack[cur] = true;
    path.push_back(cur);

    for (const string& nei : graph[cur]) {
        if (!visited[nei]) {
            if (isCyclicUtil(nei, graph)) ;
        }
        else if (recStack[nei]) {
            vector<string> cycle;
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

    recStack[cur] = false;
    path.pop_back();
    return false;
}

// 主函数：检测环路
int detectCycles(unordered_map<string, list<string>>& graph) {
    visited.clear();
    recStack.clear();
    allCycles.clear();

    for (const auto& node : graph) {
        visited[node.first] = false;
        recStack[node.first] = false;
    }

    for (const auto& node : graph) {
        const string& nodeName = node.first;
        if (!visited[nodeName]) {
            isCyclicUtil(nodeName, graph);
        }
    }

    cout << "图中有 " << allCycles.size() << " 个环路" << endl;

    // 打印所有环路
    for (size_t i = 0; i < allCycles.size(); ++i) {
        // 将环路信号去重并排序
        unordered_set<string> uniqueSignals(allCycles[i].begin(), allCycles[i].end());
        vector<string> sortedSignals(uniqueSignals.begin(), uniqueSignals.end());
        sort(sortedSignals.begin(), sortedSignals.end());

        // 打印环路，按照你的期望格式
        cout << i + 1 << ")" << endl;
        cout << "Loop Signals: ";
        for (size_t j = 0; j < sortedSignals.size(); ++j) {
            cout << sortedSignals[j];
            if (j != sortedSignals.size() - 1)
                cout << ", ";
        }
        cout << endl << endl;
    }

    return allCycles.size();
}


// 全局的电路实例_glbNetlistModule
NetlistModule _glbNetlistModule;

void printModInfo() {
    printf("module name is %s\n", _glbNetlistModule._moduelName.c_str());
    _glbNetlistModule.dumpInst();
    _glbNetlistModule.buildGraphAndDetectCycles();
}

#ifdef __cplusplus
extern "C" {
#endif

int get_module_info() {
    vpiHandle systf_h = vpi_handle(vpiSysTfCall, NULL);
    if (systf_h == NULL)
        return 0;

    vpiHandle modHdl = vpi_handle(vpiScope, systf_h);
    modHdl = vpi_handle(vpiModule, modHdl);
    char *pModDefName = vpi_get_str(vpiDefName, modHdl);
    _glbNetlistModule.setModuleName(pModDefName);

    vpiHandle instIter = vpi_iterate(vpiInstance, modHdl);
    vpiHandle instHdl = NULL;
    while ((instHdl = vpi_scan(instIter))) {
        char* pInstName = vpi_get_str(vpiName, instHdl);
        GateInst* pGateInst = new GateInst(pInstName);

        char *pModDefName = vpi_get_str(vpiDefName, instHdl);
        pGateInst->setGateType(pModDefName);

        vpiHandle portIter = vpi_iterate(vpiPort, instHdl);
        vpiHandle portHdl = NULL;
        while ((portHdl = vpi_scan(portIter))) {
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

#ifdef __cplusplus
}
#endif
