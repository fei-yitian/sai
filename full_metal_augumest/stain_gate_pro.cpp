#include <iostream> 
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <unordered_map>
// #include "vpi_user.h"
// #include "sv_vpi_user.h"
#include <bits/stdc++.h>
#include <list>
#include <unordered_set>
#include <algorithm>
#include <set>

using namespace std;

typedef vector<string> StrVec;

int V = 0; // 顶点数

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

    // void dumpInst() // 打印信息
    // {
    //     printf("%-5s  %s  ", _gateType.c_str(), _instName.c_str());
    //     for(unsigned ii = 0; ii != _instNodes.size(); ++ii)
    //     {
    //         printf("%s  ", _instNodes[ii].c_str());
    //     }
    //     printf("\n");
    // }

    string  _gateType; // 门的类型
    string  _instName; // 门的名字
    StrVec  _instNodes; // 节点名称
};

class shixian{
public:
    
    unordered_map<string, bool> visited;
    unordered_map<string, bool> recStack;//环路结点标识

    vector<vector<string>> allCycles;//用于存储所有环路

    vector<string> path;//？？？

    vector<vector<string>> sorted_Gates;//门结点和线结点分开并排序
    vector<vector<string>> sorted_Signals;

    unordered_map<string, string> _gateTypeMap;//门结点的类型映射

    vector<vector<string>> can_oscillateCycles_Gates;//可震荡环路
    vector<vector<string>> can_oscillateCycles_Signals;

    //vector<int> can_oscillateCycles_num;//存放在allcycle中的可震荡环路标号

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


void detectCycles(unordered_map<string, list<string>>& graph,unordered_map<string, string>& gateTypeMap) {
    visited.clear();
    recStack.clear();
    allCycles.clear();
    _gateTypeMap=gateTypeMap;
    
    for (const auto& node : graph) {
        if (!visited[node.first]) {
            isCyclicUtil(node.first, graph);
        }
    }

    // 整理环路信息
    for (const auto& cycle : allCycles) {
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

    void print_result1(){

        cout << "******* result_1.txt *********"<<endl;

        for (size_t i = 0; i < allCycles.size(); ++i){
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

    // 辅助函数：判断一个门是否是反相器
    bool isInvertingGate(const string& gateType) {
        return gateType.find("not") != string::npos || gateType.find("nand") != string::npos;
    }
    
    void print_result2(){

        int x=1;

        cout << "******* result_2.txt *********"<<endl;

        // 判断环路是否可能产生震荡
        for (size_t i = 0; i < allCycles.size(); ++i){

            int invertingGateCount = 0;
            // 统计反相门的数量          //感觉这个循环体可以放出去，，
            for (const auto& gate : sorted_Gates[i]) {
                //string gateType = _gateTypeMap[gate];
                if (isInvertingGate(_gateTypeMap[gate])) {
                    invertingGateCount++;
                }
            }

            if (invertingGateCount % 2 == 0) {//不可能震荡震荡
            
                cout << x << ")" << endl;
                x++;

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
            else{   //可能震荡
                    can_oscillateCycles_Gates.push_back(sorted_Gates[i]);
                    can_oscillateCycles_Signals.push_back(sorted_Signals[i]);

                    //can_oscillateCycles_num.push_back(i);//这行代码似乎只有在运转result2之后才能运行，也就是说这样似乎不能单独运作result4，这里可能存在隐患
                }
            
        }
    }
    
    void print_result3(){
        cout << "******* result_3.txt *********"<<endl;
        
        //判断门的类型，并给
        for (size_t i = 0; i < can_oscillateCycles_Signals.size(); ++i){
            cout << i + 1 << ")" << endl;    
            cout << "  Loop Signals: ";
            for (size_t j = 0; j < can_oscillateCycles_Signals[i].size(); ++j) {
                cout << can_oscillateCycles_Signals[i][j];
                cout << ", ";
            }
            cout << endl;

            cout << "  Loop Gates: ";
            for (size_t j = 0; j < can_oscillateCycles_Gates[i].size(); ++j) {
                cout << can_oscillateCycles_Gates[i][j];
                cout << ", ";
            }

            cout << endl;

            cout<<"  Loop Conditions: ";
            for ( auto& gate : can_oscillateCycles_Gates[i]) {
                //string gateType = _gateTypeMap[gate]; // 获取门类型
                if(gate.back()=='1')
                    gate.back()='2';
                else
                    gate.back()='1';

                if(_gateTypeMap[gate]=="and2"||_gateTypeMap[gate]=="nand2")
                    cout << gate << "=1, " ;
                else if(_gateTypeMap[gate]=="or2")
                    cout << gate << "=0, " ;
 
            }

            cout << endl << endl;
        
        }
    }

    void findDuplicates(const vector<vector<string>>& a, vector<vector<pair<int, int>>>& indices) {

        unordered_map<string, vector<pair<int, int>>> sp_loop;

        // 遍历数组 a
        for (size_t i = 0; i < a.size(); ++i) {
            for (size_t j = 0; j < a[i].size(); ++j) {
                const auto& line_node = a[i][j];
                // 存储 (数组索引, 元素索引)
                sp_loop[line_node].emplace_back(i, j);
            }
        }

        // 查找重复的字符串
        for (const auto& entry : sp_loop) {
            if (entry.second.size() > 1) { // 如果该字符串出现超过一次
                indices.push_back(entry.second);
            }
        }
    }
    
    void print_result4(){//我还没有考虑重复三次及以上的情况

        cout << "******* result_4.txt *********"<<endl;

        vector<vector<pair<int, int>>> indices;
        findDuplicates(can_oscillateCycles_Signals,indices);

        for(size_t i = 0; i < can_oscillateCycles_Signals.size(); ++i){

            cout << i + 1 << ")" << endl;
            cout << "  Loop Breaker: ";

            bool found = false;


            for (const auto& duplicate : indices) {
                for (const auto& index : duplicate) {
                    if (index.first == i) {
                        //cout << "重复！";
                        cout << can_oscillateCycles_Signals[i][index.second];
                        cout << endl;
                        found = true;
                    }
                }

                if(found == true)
                    break;
            }

            if (!found) {
                cout << can_oscillateCycles_Signals[i][0];
                cout << endl;
            }

            cout<<endl;
        }
    }

};

typedef vector<GateInst*> GateInstVec;
shixian A;

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

            for (unsigned jj = 1; jj < pInst->_instNodes.size(); ++jj) {
                string input = pInst->_instNodes[jj];  // 输入信号
                
                // 将输入信号连接到门结点
                if(jj==1)
                    graph[input].push_back(gate_port1);
                else
                    graph[input].push_back(gate_port2);

            }
        }
    }

    string _moduelName;
    GateInstVec _instVec;
};


// 全局的电路实例_glbNetlistModule
NetlistModule _glbNetlistModule;

void printModInfo() 
{
    //printf("module name is %s\n", _glbNetlistModule._moduelName.c_str());
    //_glbNetlistModule.dumpInst();
    _glbNetlistModule.buildGraphAndDetectCycles();
    A.print_result1();
    A.print_result2();
    A.print_result3();
    A.print_result4();
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
