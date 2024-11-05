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

class shixian{
public:
    // 环路检测相关的代码
    unordered_map<string, bool> visited;
    unordered_map<string, bool> recStack;
    vector<vector<string>> allCycles;
    vector<string> path;
    vector<vector<string>> sorted_Gates;
    vector<vector<string>> sorted_Signals;
    unordered_map<string, string> _gateTypeMap;
    vector<vector<string>> can_oscillateCycles_Gates;
    vector<vector<string>> can_oscillateCycles_Signals;
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

    // 检测环路函数
    int detectCycles(unordered_map<string, list<string>>& graph,unordered_map<string, string>& gateTypeMap) {
        visited.clear();
        recStack.clear();
        allCycles.clear();
        _gateTypeMap=gateTypeMap;

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
  
        // 整理环路信息
        for (size_t i = 0; i < allCycles.size(); ++i) {
            vector<string> loopSignals;  // 保存信号
            vector<string> loopGates;    // 保存门
            
            // 将环路信号分开
            for (const auto& signal : allCycles[i]) {
                if (signal[0] == 'I') {
                    loopGates.push_back(signal);
                } 
                else if (signal[0] == 'w') {
                    loopSignals.push_back(signal);
                }
            }

            // 将信号去重并排序
            unordered_set<string> uniqueSignals(loopSignals.begin(), loopSignals.end());
            vector<string> sortedSignals(uniqueSignals.begin(), uniqueSignals.end());
            sort(sortedSignals.begin(), sortedSignals.end());
            sorted_Signals.push_back(sortedSignals);

            // 将门去重并排序
            unordered_set<string> uniqueGates(loopGates.begin(), loopGates.end());
            vector<string> sortedGates(uniqueGates.begin(), uniqueGates.end());
            sort(sortedGates.begin(), sortedGates.end());
            sorted_Gates.push_back(sortedGates);

        }

        return allCycles.size();
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
            // 统计反相门的数量
            for (const auto& gate : sorted_Gates[i]) {
                string gateType = _gateTypeMap[gate];
                if (isInvertingGate(gateType)) {
                    invertingGateCount++;
                }
            }

            if (invertingGateCount % 2 == 0) {
            
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
            else{   
                    can_oscillateCycles_Gates.push_back(sorted_Gates[i]);
                    can_oscillateCycles_Signals.push_back(sorted_Signals[i]);
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
                string gateType = _gateTypeMap[gate]; // 获取门类型
                if(gate[13]=='1')
                    gate[13]='2';
                else
                    gate[13]='1';

                if(gateType=="and2"||gateType=="nand2")
                    cout << gate << "=1, " ;
                else if(gateType=="or2")
                    cout << gate << "=0, " ;
 
            }

            cout << endl << endl;
       
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

        // 调用环路检测函数
        A.detectCycles(graph,gateTypeMap);// 传递 gateTypeMap 用于后续环路检测时获取门的类型
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
