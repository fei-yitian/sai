#include <iostream> 
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <unordered_map>

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

            // 第一个节点通常是输出，剩下的节点是输入
            string output = pInst->_instNodes[0];  // 输出信号
            string gate = pInst->_instName;  //门结点
            graph[gate].push_back(output);  //将门结点连接到输出信号
            cout << "Adding edge from " << gate << " to " << output << endl;
            for (unsigned jj = 1; jj < pInst->_instNodes.size(); ++jj) {
                string input = pInst->_instNodes[jj];  // 输入信号
                
                // 将输入信号连接到门结点
                graph[input].push_back(gate);

                cout << "Adding edge from " << input << " to " << gate << endl;
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
        vector<string> loopSignals;  // 保存信号
        vector<string> loopGates;    // 保存门

        // 将环路信号分开
        for (const auto& signal : allCycles[i]) {
            if (signal[0] == 'I') {
                loopGates.push_back(signal);
            } else if (signal[0] == 'w') {
                loopSignals.push_back(signal);
            }
        }

        // 将信号去重并排序
        unordered_set<string> uniqueSignals(loopSignals.begin(), loopSignals.end());
        vector<string> sortedSignals(uniqueSignals.begin(), uniqueSignals.end());
        sort(sortedSignals.begin(), sortedSignals.end());

        // 将门去重并排序
        unordered_set<string> uniqueGates(loopGates.begin(), loopGates.end());
        vector<string> sortedGates(uniqueGates.begin(), uniqueGates.end());
        sort(sortedGates.begin(), sortedGates.end());

        // 打印环路，按照你的期望格式
        cout << i + 1 << ")" << endl;
        cout << "Loop Signals: ";
        for (size_t j = 0; j < sortedSignals.size(); ++j) {
            cout << sortedSignals[j];
            if (j != sortedSignals.size() - 1)
                cout << ", ";
        }
        cout << endl;

        cout << "Loop Gates: ";
        for (size_t j = 0; j < sortedGates.size(); ++j) {
            cout << sortedGates[j];
            if (j != sortedGates.size() - 1)
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

// 测试主函数
int main() {
    // 创建测试模块
    _glbNetlistModule.setModuleName("testModule");

    // 创建门实例并添加节点（模拟电路）
    GateInst* gate0 = new GateInst("I1_0");
    gate0->setGateType("not1");
    gate0->addNodeName("w_1_0"); // 输出
    gate0->addNodeName("w_0_0");  // 输入

    GateInst* gate1 = new GateInst("I1_2");
    gate1->setGateType("or2");
    gate1->addNodeName("w_1_1"); // 输出
    gate1->addNodeName("w_0_3");  // 输入
    gate1->addNodeName("w_0_1");  // 输入

    GateInst* gate2 = new GateInst("I2_0");
    gate2->setGateType("or2");
    gate2->addNodeName("w_2_0"); // 输出
    gate2->addNodeName("w_1_0");  // 输入
    gate2->addNodeName("w_0_1");  // 输入

    GateInst* gate3 = new GateInst("I2_1");
    gate3->setGateType("nand2");
    gate3->addNodeName("w_2_1"); // 输出
    gate3->addNodeName("w_0_2");  // 输入
    gate3->addNodeName("w_1_1");  // 输入

    GateInst* gate4 = new GateInst("I4_0");
    gate4->setGateType("nand2");
    gate4->addNodeName("w_4_0"); // 输出
    gate4->addNodeName("w_2_0");  // 输入
    gate4->addNodeName("w_0_1");  // 输入

    GateInst* gate5 = new GateInst("I4_1");
    gate5->setGateType("and2");
    gate5->addNodeName("w_0_3"); // 输出
    gate5->addNodeName("w_2_1");  // 输入
    gate5->addNodeName("w_0_2");  // 输入

    GateInst* gate6 = new GateInst("I5_0");
    gate6->setGateType("nand2");
    gate6->addNodeName("w_5_0"); // 输出
    gate6->addNodeName("w_4_0");  // 输入
    gate6->addNodeName("w_0_2");  // 输入

    GateInst* gate7 = new GateInst("I5_1");
    gate7->setGateType("nand2");
    gate7->addNodeName("w_5_1"); // 输出
    gate7->addNodeName("w_2_0");  // 输入
    gate7->addNodeName("w_4_0");  // 输入

    GateInst* gate8 = new GateInst("I5_2");
    gate8->setGateType("not1");
    gate8->addNodeName("w_5_2"); // 输出
    gate8->addNodeName("w_0_3");  // 输入

    GateInst* gate9 = new GateInst("I6_0");
    gate9->setGateType("not1");
    gate9->addNodeName("w_0_0"); // 输出
    gate9->addNodeName("w_5_0");  // 输入

    // 添加所有门实例到模块
    _glbNetlistModule.addInst(gate0);
    _glbNetlistModule.addInst(gate1);
    _glbNetlistModule.addInst(gate2);
    _glbNetlistModule.addInst(gate3);
    _glbNetlistModule.addInst(gate4);
    _glbNetlistModule.addInst(gate5);
    _glbNetlistModule.addInst(gate6);
    _glbNetlistModule.addInst(gate7);
    _glbNetlistModule.addInst(gate8);
    _glbNetlistModule.addInst(gate9);

    // 打印模块信息并检测环路
    printModInfo();

    return 0;
}
