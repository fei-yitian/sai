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
                tarjan(dfn, low, y, instack, time, s);
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

                visited.clear();
                recStack.clear();
                //allCycles.clear();
                for (const auto& node : scc) {
                    if (!visited[node]) {
                        findcyc(node, scc);
                    }
                }
            }
        }
    }

    bool findcyc(const string& cur, vector<string>& scc) {
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

    void result1(){
        int i=1;
        for(vector<string> scc:all_scc){
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
        sort(scc_w.begin(),scc_w.end());
        sort(scc_g.begin(),scc_g.end());

        cout<<i++<<")"<<endl<<"  Loop Signals: ";
            for(const string wire:scc_w)
                cout<<wire<<", ";
            cout<<endl<<"  Loop Gates: ";
            for(const string gate:scc_g)
                cout<<gate<<", ";
            cout<<endl<<endl;
        }
    }

    unordered_map<string, bool> visited;
    unordered_map<string, bool> recStack;//环路结点标识
    vector<string> path;//dfs寻找路径

    // vector<vector<vector<string>>> allcycles;
    vector<vector<string>> all_scc;//存放所有scc

    // vector<vector<string>> wires;
    // vector<vector<string>> nodes;
    std::unordered_map<std::string, std::list<std::string>> graph;
    std::unordered_map<std::string, std::vector<std::string>> mapstrw_p;//声明

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
    printf("module name is %s\n", _glbNetlistModule._moduelName.c_str());
    //_glbNetlistModule.dumpInst();
    _glbNetlistModule.build();
    _glbNetlistModule.find_scc();
    _glbNetlistModule.result1();
    // _glbNetlistModule.cyclesfind();
    // _glbNetlistModule.showgraph();
}
