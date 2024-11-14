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

    void dumpInst()
    {
        printf("%-5s  %s  ", _gateType.c_str(), _instName.c_str());
        for(unsigned ii = 0; ii != _instNodes.size(); ++ii)
        {
            printf("%s  ", _instNodes[ii].c_str());
        }
        printf("\n");
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
 void buildhash() {//定义一个双向hash，位于netlistmodule类内
        // 填充哈希表
        for (int i = 0; i < _instVec.size(); ++i) {
            GateInst* pInst = _instVec[i];
            string node_str = pInst->_instName;
            string type_str = pInst->_gateType;
            mapstrn_num[node_str] = i;
            //cout<<v<<"---"<<node_str<<endl;
            mapstrnum_n[i]=node_str;
            //cout<<mapstrnum_n[v-1]<<"---"<<mapstrn_num[node_str]<<endl;
            mapstrn_t[node_str]=type_str;
            string wire_str = pInst->_instNodes[0];
            mapstrn_w[node_str].push_back(wire_str);
            for (int j = 1; j < pInst->_instNodes.size(); ++j)
            {
                string wire_str = pInst->_instNodes[j];
                string node_str1 = node_str + ".port" + std::to_string(j);//node find wire
                mapstrw_n[wire_str].push_back(node_str1);
            }
        }
    }
    void buildgraph() {
        buildhash();
        for (int i = 0; i < _instVec.size(); ++i) {
            GateInst* pInst = _instVec[i];
            string node1 = pInst->_instName;
            string wire = pInst->_instNodes[0];
            auto node2 = mapstrw_n.find(wire);//如果找到了，返回wire对应的node，未找到则返回mapstr2_n.end()
            if (node2 != mapstrw_n.end()) {//判断是否找到了
                for (const auto& node : mapstrw_n[wire]) {//wire对应多个节点，循环遍历
                    string str = node;
                    str = str.substr(0, str.size() - 6);//!字符数出错
                    if (node1 != str)
                    {
                        graph[node1].push_back(str);
                        //cout << node1 << "--->" << str << endl;
                    }
                }
            }
        }
    }
    void zhixin() {
        int graphsize=_instVec.size();
        vector<int> dfn(graphsize,0);
        vector<int> low(graphsize,0);
        vector<bool> instack(graphsize,false);
        static int time = 1;
        for (int i = 0; i < graphsize; i++) {
            if(dfn[i]==0)
            tarjan(dfn, low, i,instack,time);
    }
    }
    void tarjan(vector<int>&dfn, vector<int>&low, int x, vector<bool> &instack,int &time) {
        dfn[x] = low[x] = time++;//初始化两个标记数组
        s.push(x);//表头入栈
        instack[x] = true;
        //cout<<"doing :"<<mapstrnum_n[x];
        for (const string &nei : graph[mapstrnum_n[x]]) {///访问邻结点
            //cout<<"访问邻接点："<<nei<<endl;
            int y = mapstrn_num[nei];
            if (dfn[y] == 0) {
                //cout<<"y="<<y<<endl;
                tarjan(dfn, low, y,instack,time);
                low[x] = min(low[x], low[y]);
                //cout<<low[x]<<" if  "<<dfn[x]<<endl;
            }
            else if (instack[y]) {
                //cout<<"重复访问"<<endl;//test
                low[x] = min(low[x], dfn[y]);
                //cout<<low[x]<<"elseif "<<dfn[x]<<endl;
            }
        }
        if (dfn[x] == low[x]) {
            int temp;
            vector<string> cyc;
             do {
                if(!s.empty()){
                 temp=s.top();
                 s.pop();
                 instack[temp] = false;
                 cyc.push_back(mapstrnum_n[temp]);
                 }
             } while (temp != x);
                 if(cyc.size()>1)
                 cycles.push_back(cyc);
        }

    }
    void result1(){
        cout<<"------result1-------"<<endl;
        int i=1;
        for(auto stary:cycles){
            show(stary,i);  
            i++;
        }
    }
    void result2(){
        cout<<"------result2-------"<<endl;
        int i=1;
        for(auto stary:unstcycles){
            show(stary,i);  
            i++;
        }
    }
    void result3(){

    }
    void show(vector<string> &stary,int &i){
        vector<string> wire;
        vector<string> node;
            for(auto str:stary){
                 for(const string str1:mapstrn_w[str]){
                    //cout<<str1<<"  ";
                    wire.push_back(str1);
                    for(const string str2:mapstrw_n[str1]){
                     auto it=find(stary.begin(),stary.end(),str2.substr(0,str2.size() - 6));//判断找到的节点是否位于scc内
                    if(it!=stary.end()){
                    //cout<<str2<<"  ";
                    node.push_back(str2);
                    }
                    }
                 }
                 }
                sort(wire.begin(),wire.end());
                sort(node.begin(),node.end());
                //wires.push_back(wire);
                //nodes.push_back(node);
                cout<<i<<")"<<endl<<"Loop Signals：";
                for(const string str:wire)
                    cout<<str<<",";
                cout<<endl<<"Loop Gates：";
                for(const string str:node)
                    cout<<str<<",";
                cout<<endl;
    }
    void buildGraphz(vector<string>&stary,vector<vector<string>> &Cycles) {
        //cout<<"----------------------------"<<endl;
        unordered_map<string, list<string>> g;
        string node1;
        for(auto str:stary){
            int i=mapstrn_num[str];
            GateInst* pInst = _instVec[i];
            node1 = pInst->_instName;
            string wire = pInst->_instNodes[0];
            auto node2 = mapstrw_n.find(wire);//也是查找映射
            if (node2 != mapstrw_n.end()) {
                for (const auto& node : mapstrw_n[wire]) {
                    string str = node;
                    str = str.substr(0, str.size() - 6);//!字符数出错
                    if (node1 != str)
                    {
                        g[node1].push_back(str);
                        //cout << node1 << "--->" << str << endl;
                    }
                }
        }
        }
        unordered_set<string> onPath;
        dfs(node1,onPath,g,Cycles);

    }
void dfs(const string& node, unordered_set<string>& onPath, unordered_map<string, list<string>>& g, vector<vector<string>>& Cycles) {
    visited.insert(node);  // 标记当前节点已访问
    onPath.insert(node);   // 标记当前节点在路径上
    path.push_back(node);  // 将当前节点加入路径

    for (const string& neighbor : g[node]) {
        // 如果邻居在当前路径上，说明发现环路
        auto it = find(path.begin(), path.end(), neighbor);
        if (it != path.end()) {
            // 环路部分从找到的节点开始到路径结束
            vector<string> cycle(it, path.end());//此处dfs时如何存放节点的，未知
            if (cycle.front() == cycle.back()) { 
                cycle.pop_back();  // 去除重复的起始节点
            }
            // set<string> cycle1(cycle.begin(),cycle.end());//cycle中的节点是不可能重复的
            // vector<string> cycle2(cycle1.begin(),cycle1.end());
            Cycles.push_back(cycle);
        }
        // 如果邻居没有访问过，继续递归
        else if (visited.find(neighbor) == visited.end()) {
            dfs(neighbor, onPath, g, Cycles);
        }
    }

    // 回溯时，清除路径上的节点
    onPath.erase(node);
    path.pop_back();
}


void cyclesfind(){
    int i=0,j=0;
      for(auto stary:cycles){
        vector<vector<string>> Cycles;  //局部变量
        buildGraphz(stary,Cycles);
        int A=0;
        //cout<<"size:"<<Cycles.size()<<endl;
        for(auto cycle:Cycles){
            bool B=shockdetect(cycle);
            A+=int(B);
            }
            if(Cycles.size()==1){
                //cout<<"size:1   a:"<<A<<endl;
                if(A==1)
                unstcycles.push_back(stary);
                else if(A==0)
                    stcycles.push_back(stary);
            }
            else if(Cycles.size()==2)
            {
                   //cout<<"size:2   a:"<<A<<endl;
                if(A==0)
                    stcycles.push_back(stary);
                else if(A==1){
                    //主要讨论，当两环相套，一环仅为not且为偶数则锁
                cout<<"开始执行单偶环检测"<<endl;
                        std::unordered_set<std::string> set1(Cycles[0].begin(),Cycles[0].end());
                        vector<string> repeatcycle;
                        for(const auto &str:Cycles[1]){
                            if (set1.find(str) != set1.end()) {
                                repeatcycle.push_back(str);
                            }
                        }
                        for(auto cycle:Cycles){
                         bool B=shockdetect(cycle);//检测到偶数环
                        if(B){
                            sccskdetect(cycle,repeatcycle[0]);
                        }
                    }
                }
                }
                else if(A==2)
                   unstcycles.push_back(stary); 
            }
    //         else if(Cycles.size()==3){
    //             if(A>=2){
    //         unstcycles.push_back(stary);
    //     }
    //     else if(A==1){
    //         cout<<"test"<<endl;
    //     }
    //     else if(A==0){
    //         cout<<"buzheng"<<endl;
    //     }
             
}
int sccskdetect(vector<string>vec,string target){
        string nodetype=mapstrn_t[target];
        int i;
    if(nodetype=="and2"||"nand2")
         i=0;
    else if(nodetype=="or2")
         i=1;
        auto it = std::find(vec.begin(), vec.end(), target);
    if (it != vec.end()) {
        // 1. 先倒序输出目标元素后面的部分
        for (auto rit = vec.rbegin(); rit != std::make_reverse_iterator(it); ++rit) {
            string node=*rit;
            i=panduan(i,node);
            if(i==3)
            return i;
        }

        // 2. 输出目标元素
        string it=*it;
        i=panduan(i,it);
        if(i==3)
            return i;
        // 3. 然后倒序输出目标元素前面的所有元素
        for (auto it2 = it - 1; it2 != vec.begin() - 1; --it2) {
            string it2=*it2;
            i=panduan(i,it2);
            if(i==3)
            return i;
        }
    } 
    return i;
}
int panduan(int &i,string node){//判断节点是否会改变信号或者锁定信号
    string type=mapstrn_t[node];
    if(i==1){
            if(type=="nand2"||type=="or2")//锁定值为1的门
            i=3;//跳出循环
            if(type=="not1")
            i=0;        
    }
    else if(i==0){
        if(type=="not1"||"nand2")
        i=1;
        if(type=="and2")//锁定值为0的门
        i=3;
    }
    return i;
}
// bool invectdetect(vector<string>&cycle){//一次失败的尝试
//     int i=0,j=0;
//     bool ii;
//     for(auto node:cycle){
//         string str=mapstrn_t[node];
//         cout<<node<<"----"<<str<<endl;
//             if(str=="not1")
//             i++; 
//             else if(str=="nand2") //判断是否存在nand    
//             j++;     
//     }
//     if(j!=0)//判断是否存在nand
//     ii=0;
//     if(i%2==0)
//     ii=1;
//     return ii;
// }
bool shockdetect(vector<string>&cycle){//检测环路是否反向器为偶数，偶数返回true，奇数false
    int ii=0;
    for(auto node:cycle){
        string str=mapstrn_t[node];
        //cout<<node<<"----"<<str<<endl;
            if(str=="not1"||str=="nand2")
                ii++;
    }
    if(ii%2==0)
    return true;
    else
    return false;
}
// void showgraph(){
//     int i=1;
//     for(auto cycles:allcycles){
//         int j=1;
//         cout<<"连通分量"<<i++<<endl;
//         for(auto cycle:cycles){
//             cout<<"环"<<j++<<endl;
//             for(auto str:cycle){
//                 cout<<str<<"   ";
//             }
//             cout<<endl;
//         }
//     }
// }
    unordered_set<string> visited;  // 记录访问过的节点
    vector<string> path;  // 当前遍历路径
    vector<vector<vector<string>>> allcycles;
    vector<vector<string>> cycles;
    std::stack<int> s;
    vector<vector<string>> stcycles;
    vector<vector<string>> unstcycles;
    std::unordered_map<std::string, std::list<std::string>> graph;
    std::unordered_map<std::string, std::vector<std::string>> mapstrn_w;
    std::unordered_map<std::string, std::vector<std::string>> mapstrw_n;//声明
    std::unordered_map<std::string, std::string> mapstrn_t;
    std::unordered_map<std::string, int> mapstrn_num;
    std::unordered_map<int, std::string> mapstrnum_n;
    string      _moduelName;
    GateInstVec _instVec;
};

NetlistModule _glbNetlistModule;

void printModInfo()
{
    printf("module name is %s\n", _glbNetlistModule._moduelName.c_str());
    //_glbNetlistModule.dumpInst();
    _glbNetlistModule.buildgraph();
    _glbNetlistModule.zhixin();
    //_glbNetlistModule.show();
    //_glbNetlistModule.result1();
    _glbNetlistModule.cyclesfind();
    _glbNetlistModule.result2();
    //_glbNetlistModule.showgraph();
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


