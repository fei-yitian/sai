#include <iostream> 
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <unordered_map>
#include "vpi_user.h"
#include "sv_vpi_user.h"
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

class shixian
{
public:
    vector<vector<string>> allCycles;//用于存储所有环路

    vector<string> path;//？？？

    vector<vector<string>> sorted_Gates;//门结点和线结点分开并排序
    vector<vector<string>> sorted_Signals;

    unordered_map<string, string> _gateTypeMap;//门结点的类型映射

    vector<vector<string>> can_oscillateCycles_Gates;//可震荡环路
    vector<vector<string>> can_oscillateCycles_Signals;
    //vector<int> can_oscillateCycles_num;//存放在allcycle中的可震荡环路标号
    vector<string> currentPath; // 当前DFS路径

    // 用来存储强连通分量的容器
    unordered_map<int, vector<string>> sccContainer;
    // 用来存储每个强连通分量内的环
    unordered_map<int, vector<vector<string>>> cycleContainer;
    vector<vector<string>> cycles; // 当前DFS路径
    // 定义一个容器来存储每个强连通分量中的每个环的反相器数量
    unordered_map<int, vector<int>> sccInvertersCount;
    int os=0;
    int nos=0;
void tarjansSCC(unordered_map<string, list<string>>& graph) 
{
        int index = 0;
        std::unordered_map<string, int> indices;
        std::unordered_map<string, int> lowlink;
        std::set<std::string> onStack;
        std::stack<std::string> s;
        int x = 1;
        for (const auto& pair : graph)
        {
            const std::string& v = pair.first;
            if (indices.find(v) == indices.end()) //indices 是一个哈希表，存储每个节点的访问顺序。如果 find(v) 返回 end()，则表示节点 v 尚未被访问。
            {
                strongconnect(v, index, indices, lowlink, onStack, s, allCycles, graph, x);
            }
        }
}
void findCycle(string start, int x) {
    // 当前路径包含了节点 v 到当前栈中节点之间的所有节点
    auto it = find(currentPath.begin(), currentPath.end(), start);
    vector<string> temp;
    if (it != currentPath.end()) {
        // 输出从start到尾部的所有元素
        for (auto i = it; i != currentPath.end(); ++i) {
            string s=*i;
            if(s[0] == 'I')
            {temp.push_back(*i);}
            //cout << *i << "     ";
        }
        cycles.push_back(temp);
    }  
}
void strongconnect(const std::string& v, int& index, std::unordered_map<std::string, int>& indices,
                      std::unordered_map<std::string, int>& lowlink, std::set<std::string>& onStack,
                      std::stack<std::string>& s, vector<vector<string>> & allCycles,unordered_map<string, list<string>>& graph, int &x)
{
        indices[v] = lowlink[v] = index++;
        s.push(v);
        onStack.insert(v);
        currentPath.push_back(v);
        for (const std::string& w : graph[v]) 
        {
            if (indices.find(w) == indices.end()) 
            {
                strongconnect(w, index, indices, lowlink, onStack, s, allCycles,graph,x);
                lowlink[v] = std::min(lowlink[v], lowlink[w]);
            } else if (onStack.find(w) != onStack.end()) 
            {
                lowlink[v] = std::min(lowlink[v], indices[w]);
                findCycle(w,x);
            }
        }
        currentPath.pop_back();
        if (lowlink[v] == indices[v]) 
        {
            std::vector<std::string> currentSCC;
            while (true) 
            {
                std::string w = s.top();
                s.pop();
                onStack.erase(w);
                currentSCC.push_back(w);
                if (w == v) break;
            }
            if (currentSCC.size() > 1) 
            { // 只添加包含多个节点的强连通分量
            
            allCycles.push_back(currentSCC);
            sccContainer[x]=currentSCC;
            //cycleContainer[x];  // 初始化该SCC的环容器
            x++;
            }
        }
}
int findSCCForCycle(const vector<string>& cycle) {
    if (cycle.empty()) return -1;
    const string& firstNode = cycle[0];

    // 遍历 sccContainer，找到环第一个节点所在的强连通分量
    for (const auto& entry : sccContainer) {
        int scc_id = entry.first;
        const vector<string>& sccNodes = entry.second;
        
        // 判断第一个节点是否在该强连通分量中
        if (find(sccNodes.begin(), sccNodes.end(), firstNode) != sccNodes.end()) {
            return scc_id;  // 找到该环所属的强连通分量索引
        }
    }

    return -1;  // 如果找不到所属的强连通分量，返回 -1
}

void assignCyclesToSCC() {
    for (const auto& cycle : cycles) {
        // 根据环的第一个节点，找到该环属于哪个强连通分量
        int scc_id = findSCCForCycle(cycle);

        if (scc_id != -1) {
            // 将环放到对应的 cyclesContainer 中
            cycleContainer[scc_id].push_back(cycle);
            // 输出调试信息，查看环是否成功分配
            //cout << "Cycle in SCC " << scc_id << ": ";
            for (const auto& node : cycle) {
                //cout << node << " ";
            }
            //cout << endl;
        }
    }
}

void detectCycles(unordered_map<string, list<string>>& graph,unordered_map<string, string>& gateTypeMap) 
{
    _gateTypeMap=gateTypeMap;
    tarjansSCC(graph);
    assignCyclesToSCC();
    // 整理环路信息
    for (int i=1;i<=sccContainer.size();i++) {
        vector<string> loopSignals, loopGates;
       const auto& cycle=sccContainer[i];
        for (const auto& signal : cycle) 
        {                
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
/* void show()
{
    x=x-1;
    cout<<x<<endl;
    cout<<cycleContainer[x-1].size()<<endl;
    for(int i=1;i<=cycleContainer.size() ;i++)
    {
        const auto& cycles=cycleContainer[i];
        cout << i << ")号强连通分量："<<endl;
        for(const auto& cycle : cycles)
        {
            for(const auto&signal : cycle )
            {
                cout<<signal<<"   ";
            }
            cout<<endl;
        }
        
    }
} */
void print_result1()
{

        cout << "******* result_1.txt *********"<<endl;
        //cout<<sccContainer.size()<<endl;
        for (int i = 0; i < sccContainer.size(); ++i){
            // 打印环路，按照你的期望格式
            cout << i+1 << ")" << endl;
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
bool isInvertingGate(const string& gateType) 
{
        return gateType.find("not") != string::npos || gateType.find("nand") != string::npos;
}

string findfirst(const vector<string> j, const vector<string> o) 
{
    for(const auto& gate1 : j)
    {
        for(const auto& gate2 : o)
        {
            if((gate1.substr(0, gate1.length() - 1)==gate2.substr(0, gate2.length() - 1))&&(gate1.back()!=gate2.back()))
            return gate2;
        }
    }
    return 0;
}

bool gateflag(const string& gateType)
{
    // 检查是否包含 "nand" 或 "and"
    if (gateType.find("nand") != string::npos || gateType.find("and") != string::npos) {
        return true; // 如果包含 "nand" 或 "and"，返回 true
    }
    // 检查是否包含 "or"
    if (gateType.find("or") != string::npos) {
        return false; // 如果包含 "or"，返回 false
    }
    // 如果没有包含 "nand"、"and" 或 "or"，返回 false
    return false;
}


bool findoscillate( bool &flag ,const vector<string>& o,const vector<string>& j,string firstgate)
{
    auto it = find(o.begin(), o.end(), firstgate);
    int firstgateIndex = distance(o.begin(), it);
    int index = firstgateIndex;
    do 
    {
        //cout << "gate: " << a[index] << endl;
        string type=_gateTypeMap[o[index]];
        //cout<<type<<endl;
        if(type.find("not") != string::npos)
        {
            flag=!flag;
        }
        else if(type.find("nand") != string::npos )
        {
            if(flag==1)
            {
                if (std::find(j.begin(), j.end(), o[index]) != j.end()) 
                {
                    return false;  // 如果找到了，返回 false
                }
                else
                {return true;}
            }
            else{flag=!flag;}
        }
        else if(type.find("and") != string::npos)
        {
            if(flag==0)
            {
                if (std::find(j.begin(), j.end(), o[index]) != j.end()) 
                {
                    return false;  // 如果找到了，返回 false
                }
                else
                {return true;}
            }
        }
        else
        {
            if(flag==1)
            {
                if (std::find(j.begin(), j.end(), o[index]) != j.end()) 
                {
                    return false;  // 如果找到了，返回 false
                }
                else
                {return true;}
            }
        }
        // 更新 index
        index = (index == 0) ? o.size() - 1 : index - 1; // 如果到头了，从最后一个开始

    } while (index != firstgateIndex); // 直到回到原来的位置
    
    return false;
}

void print_result2()
{

    //cout<<sccContainer.size()<<endl;

    cout << "******* result_2.txt *********"<<endl;
    
        // 判断环路是否可能产生震荡
    for (int i = 1; i <=sccContainer.size(); ++i)
    {
        cout<<cycleContainer[i].size()<<endl;
            //1个环
        if(cycleContainer[i].size()==1)
        {
            int invertingGateCount = 0;
            for(const auto& gate : cycleContainer[i][0])
            {
                string gateType = _gateTypeMap[gate];
                if (isInvertingGate(_gateTypeMap[gate])) 
                {
                    invertingGateCount++;
                }    
            }
            if(invertingGateCount % 2 == 1)//可震
            {
                cout<<"第"<<i<<"个强连通分量可震"<<endl;
                os++;
            }
            else if(invertingGateCount % 2 == 0)
            {
                cout<<"第"<<i<<"个强连通分量不可震"<<endl;
                nos++;
            }
        }
            //2个环
        if(cycleContainer[i].size()==2)
        {
            string firstgate;
            bool flag;
            int invertingGateCount[3] = {0,0,0};
            for (int j=0;j<cycleContainer[i].size();j++)
            {
                for(const auto& gate : cycleContainer[i][j])
                {
                    string gateType = _gateTypeMap[gate];
                    if (isInvertingGate(_gateTypeMap[gate])) 
                    {
                        invertingGateCount[j]++;
                    }    
                }
                //cout<<invertingGateCount[j]<<endl;
            }
            if(invertingGateCount[0] % 2 == 0&&invertingGateCount[1] % 2==0)
            {
                cout<<"第"<<i<<"个强连通分量不可震"<<endl;
                nos++;
            }
            else if(invertingGateCount[0] % 2 == 1&&invertingGateCount[1] % 2==1)
            {
                cout<<"第"<<i<<"个强连通分量可震"<<endl;
                os++;
            }
            else if(invertingGateCount[0] % 2 == 1&&invertingGateCount[1] % 2==0)
            {
                firstgate=findfirst(cycleContainer[i][0],cycleContainer[i][1]);
                //cout<<firstgate<<endl;
                flag=gateflag(firstgate);
                if(findoscillate(flag,cycleContainer[i][1],cycleContainer[i][0],firstgate))
                {
                    cout<<"第"<<i<<"个强连通分量可震"<<endl;
                    os++;
                }
                else{cout<<"第"<<i<<"个强连通分量不可震"<<endl;
                nos++;}
            }
           else if(invertingGateCount[0] % 2 == 0&&invertingGateCount[1] % 2==1)
            {
                //cout<<1<<endl;
                firstgate=findfirst(cycleContainer[i][1],cycleContainer[i][0]);
                //cout<<firstgate<<endl;
                flag=gateflag(firstgate);
                if(findoscillate(flag,cycleContainer[i][0],cycleContainer[i][1],firstgate))
                {
                    cout<<"第"<<i<<"个强连通分量可震"<<endl;
                    os++;
                
                }
                else{cout<<"第"<<i<<"个强连通分量不可震"<<endl;
                nos++;}
            }
        }
        /* if(cycleContainer[i].size()==3)
        {
            string firstgate;
            bool flag;
            int invertingGateCount[3] = {0,0,0};
            for (int j=0;j<cycleContainer[i].size();j++)
            {
                for(const auto& gate : cycleContainer[i][j])
                {
                    string gateType = _gateTypeMap[gate];
                    if (isInvertingGate(_gateTypeMap[gate])) 
                    {
                        invertingGateCount[j]++;
                    }    
                }
                cout<<invertingGateCount[j]<<endl;
            }
            if(invertingGateCount[0] % 2 == 0&&invertingGateCount[1] % 2==0&&invertingGateCount[2] % 2==0)//3个偶
            {
                cout<<"第"<<i<<"个强连通分量不可震"<<endl;
            }
            else if(invertingGateCount[0] % 2 == 1&&invertingGateCount[1] % 2==1&&invertingGateCount[2] % 2==1)//3个奇
            {
                cout<<"第"<<i<<"个强连通分量可震"<<endl;
            }
            else if((invertingGateCount[0] % 2)+ (invertingGateCount[1] % 2)+(invertingGateCount[2] % 2)== 1)//1个奇
            {
                
                firstgate=findfirst(cycleContainer[i][0],cycleContainer[i][1]);
                cout<<firstgate<<endl;
                flag=gateflag(firstgate);
                if(findoscillate(flag,cycleContainer[i][1],firstgate))
                {cout<<"第"<<i<<"个强连通分量可震"<<endl;}
                else{cout<<"第"<<i<<"个强连通分量不可震"<<endl;}
            }
            else if((invertingGateCount[0] % 2)+ (invertingGateCount[1] % 2)+(invertingGateCount[2] % 2)== 2)//2个奇
            {
                cout<<1<<endl;
                firstgate=findfirst(cycleContainer[i][1],cycleContainer[i][0]);
                cout<<firstgate<<endl;
                flag=gateflag(firstgate);
                if(findoscillate(flag,cycleContainer[i][0],firstgate))
                {cout<<"第"<<i<<"个强连通分量可震"<<endl;}
                else{cout<<"第"<<i<<"个强连通分量不可震"<<endl;}
            }
        } */
                     
            
                
    }
    cout<<os<<endl;
    cout<<nos<<endl;

            /* if (invertingGateCount % 2 == 0) {//不可能震荡震荡
            
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
                } */
            
        
}
    
/* void print_result3()
{
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

void findDuplicates(const vector<vector<string>>& a, vector<vector<pair<int, int>>>& indices) 
{

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
    
void print_result4()
{//我还没有考虑重复三次及以上的情况

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
 */
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

      /*   for (const auto& [key, _] : graph) {
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
    } */

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
    //A.show();
    A.print_result1();
    A.print_result2();
    /* A.print_result3();
    A.print_result4(); */
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
