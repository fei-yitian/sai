#include <iostream> 
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <unordered_map>
// #include "vpi_user.h"
// #include "sv_vpi_user.h"
#include <list>
#include <unordered_set>
#include <algorithm>
#include<stack>
#include <set>
using namespace std;

typedef vector<string>   StrVec;
class GateInst
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

    string  _gateType;
    string  _instName;
    StrVec  _instNodes;
};

typedef vector<GateInst*>  GateInstVec;

class NetlistModule
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


    void build() {//定义大图，位于netlistmodule类内

        for (int i = 0; i < _instVec.size(); ++i) {

            GateInst* pInst = _instVec[i];//遍历所有门类
            string node_str = pInst->_instName;
            string type_str = pInst->_gateType;
            string output = pInst->_instNodes[0];//输出的线结点

            mapstrg_num[node_str] = i;//将门结点与标号双向映射
            mapstrnum_g[i] = node_str;
            mapstrg_t[node_str] = type_str;//通过门结点知道门类型
            mapstrg_w[node_str].push_back(output);//门能找对应输出的线

            for (int j = 1; j < pInst->_instNodes.size(); ++j)
            {
                string intput = pInst->_instNodes[j];
                string node_str1 = node_str + ".port" + std::to_string(j);//端口编号
                mapstrw_p[intput].push_back(node_str1);//输入线结点能找对应端口
            }
        }

        for (int i = 0; i < _instVec.size(); ++i) {
            GateInst* pInst = _instVec[i];
            string node_str = pInst->_instName;
            string output = pInst->_instNodes[0];

            if ((mapstrw_p.find(output)) != mapstrw_p.end()) {
                for (string next_port : mapstrw_p[output]) {
                    graph[node_str].push_back((next_port.substr(0, next_port.size() - 6)));
                }
            }
        }
    }

    void find_scc() {
        int graphsize=_instVec.size();//顶点数
        vector<int> dfn(graphsize,0);
        vector<int> low(graphsize,0);
        vector<bool> instack(graphsize,false);
        stack<int> s;
        static int time = 1;
        for (int i = 0; i < graphsize; i++) {
            if(dfn[i]==0)
            tarjan(dfn, low, i,instack,time,s);
        }
    }

    void tarjan(vector<int>&dfn, vector<int>&low, int x, vector<bool> &instack,int &time,stack<int> &s) {
        dfn[x] = low[x] = time++;//初始化两个标记数组
        s.push(x);//表头入栈
        instack[x] = true;
        for (const string &nei : graph[mapstrnum_g[x]]) {///访问邻结点

            int y = mapstrg_num[nei];
            if (dfn[y] == 0) {
                tarjan(dfn, low, y,instack,time,s);
                low[x] = min(low[x], low[y]);
            }
            else if (instack[y]) {
                low[x] = min(low[x], dfn[y]);
            }
        }
        if (dfn[x] == low[x]) {
            int temp;
            vector<string> scc;
            do {
                if(!s.empty()){
                    temp=s.top();
                    s.pop();
                    instack[temp] = false;
                    scc.push_back(mapstrnum_g[temp]);
                }
            } while (temp != x);
                if(scc.size()>1){
                    all_scc.push_back(scc);

                    unordered_map<string, bool> visited;  // 记录当前SCC内部访问过的节点
                    unordered_map<string, bool> in_recursion;
                    vector<string> path;  // 当前路径
                    vector<vector<string>> cycles;  // 存储当前SCC中的所有环路
                    int zhen = 0;

                    for (const string& start_gate : scc) {
                        dfs_find_cycles(start_gate, visited, in_recursion, path, cycles, scc);
                    }

                    //其实在这里我有很多想法，比如把下面这个循环体放入dfs中，又比如我把cycles再放入一个vector中，定义为类成员，用于存各个scc的cycles
                    //cout<<"scc:"<<endl;

                    for (const vector<string>& cycle : cycles) {
                        int invertingGateCount = 0;

                        //cout<<"damn:"<<endl;

                        for (const string& gate : cycle) {

                            //cout<<gate<<", ";

                            if(isInvertingGate(mapstrg_t[gate])){
                                invertingGateCount++;
                            }
                        }

                        //cout<<endl;

                        if (invertingGateCount % 2 == 1) {//可能震荡
                            zhen++;
                        }
                    }
                    if(zhen == 0){
                        never.push_back(scc);
                    }
                    else {
                        if(cycles.size() == 1||zhen == 2){
                            could.push_back(scc);
                        }
                    }

                    //cout<<endl;

                }
        }
    }

    void find_point(const vector<string>& scc){
        vector<string> scc_w;
        vector<string> scc_g;
        for(const string gate:scc){
            for(const string wire:mapstrg_w[gate]){
                scc_w.push_back(wire);
                for(const string port:mapstrw_p[wire]){
                    auto it=find(scc.begin(),scc.end(),port.substr(0,port.size() - 6));
                    if(it!=scc.end()){
                        scc_g.push_back(port);
                    }
                }
            }
        }
    }

    void dfs_find_cycles(const string& node, unordered_map<string, bool>& visited, 
                          unordered_map<string, bool>& in_recursion,
                          vector<string>& path, vector<vector<string>>& cycles,
                          const vector<string>& scc) {
        visited[node] = true;
        in_recursion[node] = true;
        path.push_back(node);

        // 遍历当前节点的邻居节点
        for (const string& neighbor : graph[node]) {
            // 只考虑邻居在当前SCC内
            if (find(scc.begin(), scc.end(), neighbor) != scc.end()) {
                // 如果邻居已经在路径上，说明我们找到了一个环路
                auto it = find(path.begin(), path.end(), neighbor);
                if (it != path.end()) {
                    vector<string> cycle(it, path.end());  // 找到一个环路
                    cycles.push_back(cycle);
                } else if (!visited[neighbor]) {
                    // 如果邻居没有被访问过，继续递归DFS
                    dfs_find_cycles(neighbor, visited, in_recursion, path, cycles, scc);
                }
            }
        }

        // 回溯
        path.pop_back();
        in_recursion[node] = false;  // 当前节点不在递归栈中
    }

    // 辅助函数：判断一个门是否是反相器
    bool isInvertingGate(const string& gateType) {
        return gateType.find("not") != string::npos || gateType.find("nand") != string::npos;
    }

    void show_scc(const vector<string> &scc,const int &i,bool q3){
        vector<string> scc_w;
        vector<string> scc_g;
        for(const string gate:scc){
            for(const string wire:mapstrg_w[gate]){
                scc_w.push_back(wire);
                for(const string port:mapstrw_p[wire]){
                    auto it=find(scc.begin(),scc.end(),port.substr(0,port.size() - 6));
                    if(it!=scc.end()){
                        scc_g.push_back(port);
                    }
                }
            }
        }

        sort(scc_w.begin(), scc_w.end());
        sort(scc_g.begin(),scc_g.end());

        cout<<i<<")"<<endl<<"  Loop Signals: ";
        for(const string wire:scc_w)
            cout<<wire<<", ";
        cout<<endl<<"  Loop Gates: ";
        for(const string gate:scc_g)
            cout<<gate<<", ";
        cout<<endl;

        if(q3){

            cout<<"  Loop Conditions: ";
            for (string port:scc_g) {

                if(port.back()=='1')
                    port.back()='2';
                else
                    port.back()='1';

                if(mapstrg_t[port.substr(0,port.size() - 6)]=="and2"||mapstrg_t[port.substr(0,port.size() - 6)]=="nand2")
                    cout << port << "=1, " ;
                else if(mapstrg_t[port.substr(0,port.size() - 6)]=="or2")
                    cout << port << "=0, " ;
            }
            cout<<endl;
        }
        
        cout<<endl;
    }

    void result1(){
        cout << "******* result_1.txt *********"<<endl;
        int i=1;
        for(vector<string> scc:all_scc){
            show_scc(scc,i,false);
            i++;
        }
    }

    void result2(){
        cout << "******* result_2.txt *********"<<endl;
        int i=1;
        for(vector<string> scc:never){
            show_scc(scc,i,false);
            i++;
        }
    }

    void result3(){
        cout << "******* result_3.txt *********"<<endl;
        int i=1;
        for(vector<string> scc:could){
            show_scc(scc,i,true);
            i++;
        }
    }

    vector<vector<string>> all_scc;//存放所有scc
    vector<vector<string>> never;//存放所有不可能震荡的scc
    vector<vector<string>> could;//存放可能震荡的scc
    std::unordered_map<std::string, std::list<std::string>> graph;

    std::unordered_map<std::string, std::vector<std::string>> mapstrw_p;//wire find port
    std::unordered_map<std::string, std::vector<std::string>> mapstrg_w;//node find wire
    std::unordered_map<std::string, std::string> mapstrg_t;//node find type
    std::unordered_map<std::string, int> mapstrg_num;//node to num
    std::unordered_map<int, std::string> mapstrnum_g;//num to node

    string      _moduelName;
    GateInstVec _instVec;
};

NetlistModule _glbNetlistModule;

void printModInfo()
{
    //printf("module name is %s\n", _glbNetlistModule._moduelName.c_str());
    //_glbNetlistModule.dumpInst();
    _glbNetlistModule.build();
    _glbNetlistModule.find_scc();
    _glbNetlistModule.result1();
    _glbNetlistModule.result2();
    _glbNetlistModule.result3();

}



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


