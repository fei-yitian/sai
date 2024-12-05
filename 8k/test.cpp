#include <iostream> 
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <unordered_map>

#include <list>
#include <unordered_set>
#include <algorithm>
#include<stack>
#include <set>
#include <fstream>
using namespace std;

typedef vector<string>   StrVec;
class GateInst
{
public:
    GateInst(char* pName)
    { 
        _instName = pName ? pName : "Unknown";
    }

    void setGateType(char* pName)
    {
        _gateType = pName ? pName : "Unknown";
    }

    void addNodeName(char* pName)
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
            mapstrg_w[node_str] = output;//门能找对应输出的线

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
        all_scc.reserve(1000);
        never.reserve(1000);
        could.reserve(1000);
        points.reserve(1000);
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
                    //存放scc
                    all_scc.push_back(scc);

                    //scc内找环
                    unordered_map<string, bool> visited;  // 记录当前SCC内部访问过的节点
                    unordered_map<string, bool> in_recursion;
                    vector<string> path;  // 当前路径
                    vector<vector<string>> cycles;  // 存储当前SCC中的所有环路
                    for (const string& start_gate : scc) {
                        dfs_find_cycles(start_gate, visited, in_recursion, path, cycles, scc);
                    }

                    //找奇偶数的环
                    int zhen = 0;
                    vector<bool> ji;//判断环路内反相器是否是奇数
                    is_zhen(zhen,ji,cycles);

                    if(zhen == 0){//scc内所有的环都为偶数
                        never.push_back(scc);
                    }
                    else {
                        if(cycles.size() == 1){//只有一个奇数环一定能震
                            could.push_back(scc);
                            cyc_size.push_back(1);
                            points.push_back({""});
                        }
                        else if(cycles.size() == 2 && zhen == 2){//（只有两个环，且都为奇数）一定能震吗？
                            could.push_back(scc);
                            cyc_size.push_back(2);
                            points.push_back(find_point(scc));
                        }
                        else if(cycles.size()==2 && zhen == 1){//只有两个环，一奇一偶

                            bool stop = true;//stop表示能否抑制
                            vector<string> point = find_point(scc);//point为一个存放后缀为port的容器，我们暂时默认他只有一个

                            for (int i = 0; i < cycles.size(); i++){

                                if(ji[i] == false){//找到偶数环路
                                    vector<string>& cycle = cycles[i];
                                    //cout<<endl;
                                    // cout<<"找到偶数环路"<<endl;

                                    for(int j = 0; j < cycle.size(); j++){//找到起始
                                        string & gate = cycle[j];
                                        //cout<<gate<<", ";

                                        if(gate == point[0]){

                                            //cout<<"找到point"<<endl;

                                            string g_t=mapstrg_t[gate];
                                            int point_num = j;//point为第j号元素

                                            int x;//x为信号的value
                                            if(g_t == "and2"||g_t == "nand2")
                                                x = 1;
                                            else if(g_t == "or2")
                                                x = 0;
                                            
                                            for(int k = 1; k < cycle.size(); k++){//循环cycle.size()-1次
                                                string now_gate;//目前需要判断的门

                                                if(point_num == 0)
                                                    point_num = cycle.size();
                                                
                                                now_gate = cycle[point_num - 1];
                                                
                                                //cout<<now_gate<<", ";
                                                point_num--;
                                                
                                                x = panduan(x,now_gate);

                                                if(x == 3){
                                                    could.push_back(scc);
                                                    cyc_size.push_back(2);
                                                    points.push_back(point);

                                                    stop = false;
                                                    break;
                                                }
                                            }
                                            break;
                                        }
                                    }
                                }
                            }
                            if(stop)
                                never.push_back(scc);
                        }
                        else if(zhen == 3){//三个奇数环
                            could.push_back(scc);
                            cyc_size.push_back(3);
                            points.push_back(find_point(scc));
                        }
                        else if(cycles.size() == 3 && zhen == 1){
                            could.push_back(scc);
                            cyc_size.push_back(3);
                            points.push_back(find_point(scc));
                        }
                        else if(cycles.size() == 3 && zhen == 2){
                            could.push_back(scc);
                            cyc_size.push_back(3);
                            points.push_back(find_point(scc));
                        }
                    }

                }
        }
    }

    void is_zhen(int &zhen,vector<bool> &ji,const vector<vector<string>> &cycles){
        for (const vector<string>& cycle : cycles) {
            int invertingGateCount = 0;
            for (const string& gate : cycle) {
                if(isInvertingGate(mapstrg_t[gate])){
                    invertingGateCount++;
                }
            }
            if (invertingGateCount % 2 == 1) {//可能震荡
                zhen++;
                ji.push_back(true);
            }
            else{
                ji.push_back(false);
            }
        }
    }

    int panduan(int &x,string gate){//判断节点是否会改变信号或者锁定信号
        string type=mapstrg_t[gate];
        if(x == 1){
            if(type == "nand2"||type == "or2"){//锁定值为1的门
                black_sheep.push_back(gate);
                x = 3;//跳出循环
            }
            if(type=="not1")
                x = 0;        
        }
        else if(x == 0){
            if(type == "not1"||type == "nand2")
                x = 1;
            if(type == "and2"){//锁定值为0的门
                black_sheep.push_back(gate);
                x = 3;
            }
        }
        return x;
    }

    vector<string> find_point(const vector<string>& scc){//传入scc，返回重复point
        vector<string> scc_p;
        for(const string gate:scc){
            for(const string port:mapstrw_p[mapstrg_w[gate]]){
                auto it = find(scc.begin(),scc.end(),port.substr(0,port.size() - 6));//一定要是scc内的port
                if(it!=scc.end()){
                    scc_p.push_back(port.substr(0,port.size() - 1));
                }
            }
        }
        vector<string> point = find_duplicates(scc_p);
        vector<string> true_point;
        for(auto g:point){
            true_point.push_back(g.substr(0,g.size() - 5));
        }
        return vector<string>(true_point.begin(), true_point.end());
    }

    vector<string> find_duplicates(const vector<string>& vec) {
        unordered_set<string> seen;       // 用于存储已经遇到的元素
        set<string> duplicates;           // 用于存储重复元素，使用set去重

        for (const string& str : vec) {
            if (!seen.insert(str).second) {         // 插入失败表示该元素重复
                duplicates.insert(str);             // 将重复元素加入 duplicates
            }
        }

        // 将 set 转换成 vector 返回
        return vector<string>(duplicates.begin(), duplicates.end());
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

    void show_scc(const vector<string> &scc,const int &i,bool q3,ofstream & result){
        vector<string> scc_w;
        vector<string> scc_g;//scc内所有port排好序
        for(const string gate:scc){
                scc_w.push_back(mapstrg_w[gate]);
                for(const string port:mapstrw_p[mapstrg_w[gate]]){
                    auto it=find(scc.begin(),scc.end(),port.substr(0,port.size() - 6));
                    if(it!=scc.end()){
                        scc_g.push_back(port);
                    }
                }  
        }

        sort(scc_w.begin(), scc_w.end());
        sort(scc_g.begin(),scc_g.end());

        result<<i<<")"<<endl<<"  Loop Signals: ";
        for(const string wire:scc_w)
            result<<wire<<", ";
        result<<endl<<"  Loop Gates: ";
        for(const string gate:scc_g)
            result<<gate<<", ";
        result<<endl;

        if(q3){

            result<<"  Loop Condition: ";

            vector<string> scc_p;
            for (string port:scc_g) {
                scc_p.push_back(port.substr(0,port.size() - 1));
            }
            vector<string> the_points = find_duplicates(scc_p);//找到point，在Loop Conditions我们不要他有输出

            for (string port:scc_g) {
                bool out = true;
                for(string & the_point:the_points){
                    if(port.substr(0,port.size() - 1) == the_point)
                        out = false;
                }

                bool out_bs = false;
                for(string & the_bs:black_sheep){//使得震荡的门要反向输出
                    if(port.substr(0,port.size() - 6) == the_bs){
                        out_bs = true;
                        break;
                    }
                }

                if(out){
                    if(port.back()=='1')
                        port.back()='2';
                    else
                        port.back()='1';

                    if(out_bs){

                        if(mapstrg_t[port.substr(0,port.size() - 6)]=="and2"||mapstrg_t[port.substr(0,port.size() - 6)]=="nand2")
                            result << port << "=0, " ;
                        else if(mapstrg_t[port.substr(0,port.size() - 6)]=="or2")
                            result << port << "=1, " ;
                    }
                    else{
                        if(mapstrg_t[port.substr(0,port.size() - 6)]=="and2"||mapstrg_t[port.substr(0,port.size() - 6)]=="nand2")
                            result << port << "=1, " ;
                        else if(mapstrg_t[port.substr(0,port.size() - 6)]=="or2")
                            result << port << "=0, " ;
                    }
                }
            }
            result<<endl;
        }
        
        result<<endl;
    }

    void result1(){
        //cout << "******* result_1.txt *********"<<endl;
        int i=1;
        ofstream result_1("result_1.txt");
        for(vector<string> scc:all_scc){
            show_scc(scc,i,false,result_1);
            i++;
        }
        result_1.close();
    }

    void result2(){
        //cout << "******* result_2.txt *********"<<endl;
        int i=1;
        ofstream result_2("result_2.txt");
        for(vector<string> scc:never){
            show_scc(scc,i,false,result_2);
            i++;
        }
        result_2.close();
    }

    void result3(){
        //cout << "******* result_3.txt *********"<<endl;
        int i=1;
        ofstream result_3("result_3.txt");
        for(vector<string> scc:could){
            show_scc(scc,i,true,result_3);
            i++;
        }
        result_3.close();
    }

void result4() {
    //cout << "******* result_4.txt *********" << endl;
    ofstream result_4("result_4.txt");
    int i = 1;
    for (vector<string> scc : could) {

        result_4 << i << ")" << endl << "  Loop Breaker: ";

        if (cyc_size[i-1] == 1) {
            result_4 << mapstrg_w[scc[0]];
        }
        else if (cyc_size[i-1] == 2) {
            result_4 << mapstrg_w[points[i-1][0]];
        }
        else if (cyc_size[i-1] == 3) {
            unordered_map<string, bool> visited;  // 记录当前SCC内部访问过的节点
            unordered_map<string, bool> in_recursion;
            vector<string> path;  // 当前路径
            vector<vector<string>> cycles;  // 存储当前SCC中的所有环路
            for (const string& start_gate : scc) {
                dfs_find_cycles(start_gate, visited, in_recursion, path, cycles, scc);
            }

            unordered_set<string> set0(cycles[0].begin(),cycles[0].end());
            unordered_set<string> set1;
            vector<string> fin;
            for (const auto& str : cycles[1]) 
                if (set0.count(str)) 
                    set1.insert(str);
            for (const auto& str : cycles[2]) 
                if (set1.count(str)) 
                    fin.push_back(str);

            if(!fin.empty()){//三环有某个门重复了三次，只需插入一个reg
                    result_4 << mapstrg_w[fin[0]];
            }
            else{//三环没有门重复了三次
                int zhen = 0;
                vector<bool> ji;
                is_zhen(zhen,ji,cycles);
                if(zhen == 1){//只有一个奇数环，只在奇数环上插一个reg
                    for(int j = 0 ; j < 3 ; j++){
                        if(ji[j] == true){
                            result_4 << mapstrg_w[cycles[j][0]];
                        }
                    }
                }
                else if(zhen == 2){//两个奇数环
                    bool is_ji[2][2] ={{false,false},{false,false}};
                    for(int z = 0 ; z < 2 ; z++){//第z个point
                        int a = 0;//记录第z个point找到的次数
                        for(int x = 0 ; x < 3 ; x++){//x为第x个环
                            for(int y = 0 ; y < cycles[x].size() ; y++){//遍历第x个环中所有gate
                                if((points[i-1][z]) == cycles[x][y]){//在第x环中找到了point
                                    int invertingGateCount = 0;
                                    for (const string& gate : cycles[x]) {
                                        if(isInvertingGate(mapstrg_t[gate])){
                                            invertingGateCount++;
                                        }
                                    }
                                    if (invertingGateCount % 2 == 1) {//奇数环
                                        is_ji[z][a] = true;
                                    }
                                    a++;
                                    break;
                                }
                            }
                        }
                    }
                    bool is_one = false;
                    int only_point;
                    for(int z = 0 ; z < 2 ; z++){
                        if(is_ji[z][0] == is_ji[z][1]){
                            is_one = true;
                            only_point = z;
                        }
                    }
                    if(is_one){//两个奇数环相邻，只需插一个
                        result_4 << mapstrg_w[points[i-1][only_point]];
                    }
                    else{//奇偶奇的情况，需要插两个
                        result_4 << mapstrg_w[points[i-1][0]];
                        result_4 << ", ";
                        result_4 << mapstrg_w[points[i-1][1]];
                        result_4 << ", ";
                    }
                }
                else if(zhen == 3){//如果三个奇数环，在两个point后插
                    result_4 << mapstrg_w[points[i-1][0]];
                    result_4 << ", ";
                    result_4 << mapstrg_w[points[i-1][1]];
                    result_4 << ", ";
                }
            }
        }
        i++;
        result_4 << endl << endl << endl;
    }
    result_4.close();
}

    vector<vector<string>> all_scc;//存放所有scc
    vector<vector<string>> never;//存放所有不可能震荡的scc
    vector<vector<string>> could;//存放可能震荡的scc
    vector<vector<string>> points;//存放可能震荡的scc中重复的point
    vector<string> black_sheep;//存放可能震荡的scc中，使得可能震荡的那个门 
    vector<int> cyc_size;//记录可能震荡的scc有几个环

    unordered_map<string, list<string>> graph;

    unordered_map<string, vector<string>> mapstrw_p;//wire find port
    unordered_map<string, string> mapstrg_w;//node find wire
    unordered_map<string, string> mapstrg_t;//node find type
    unordered_map<string, int> mapstrg_num;//node to num
    unordered_map<int, string> mapstrnum_g;//num to node

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
    _glbNetlistModule.result4();
}

int main(){
    _glbNetlistModule.setModuleName("test");


    char* pInstName0 = "I0";
    GateInst* pGateInst0 = new GateInst(pInstName0);

    char *pModDefName0 = "and2";
    pGateInst0->setGateType(pModDefName0);

    char* pPortName00 = "w_0_0";
    pGateInst0->addNodeName(pPortName00);
    char* pPortName01 = "w_0_1";
    pGateInst0->addNodeName(pPortName01);
    char* pPortName02 = "w_0_2";
    pGateInst0->addNodeName(pPortName02);

    _glbNetlistModule.addInst(pGateInst0);



    char* pInstName1 = "I1";
    GateInst* pGateInst1 = new GateInst(pInstName1);

    char *pModDefName1 = "or2";
    pGateInst1->setGateType(pModDefName1);

    char* pPortName10 = "w_1_0";
    pGateInst1->addNodeName(pPortName10);
    char* pPortName11 = "w_0_0";
    pGateInst1->addNodeName(pPortName11);
    char* pPortName12 = "w_1_2";
    pGateInst1->addNodeName(pPortName12);

    _glbNetlistModule.addInst(pGateInst1);


    char* pInstName2 = "I2";
    GateInst* pGateInst2 = new GateInst(pInstName2);

    char *pModDefName2 = "or2";
    pGateInst2->setGateType(pModDefName2);

    char* pPortName20 = "w_0_1";
    pGateInst2->addNodeName(pPortName20);
    char* pPortName21 = "w_2_1";
    pGateInst2->addNodeName(pPortName21);
    char* pPortName22 = "w_1_0";
    pGateInst2->addNodeName(pPortName22);

    _glbNetlistModule.addInst(pGateInst2);




    char* pInstName3 = "I3";
    GateInst* pGateInst3 = new GateInst(pInstName3);
    char *pModDefName3 = "not1";
    pGateInst3->setGateType(pModDefName3);

    char* pPortName30 = "w_2_1";
    pGateInst3->addNodeName(pPortName30);
    char* pPortName31 = "w_0_1";
    pGateInst3->addNodeName(pPortName31);

    _glbNetlistModule.addInst(pGateInst3);



    char* pInstName4 = "I4";
    GateInst* pGateInst4 = new GateInst(pInstName4);

    char *pModDefName4 = "nand2";
    pGateInst4->setGateType(pModDefName4);

    char* pPortName40 = "w_4_0";
    pGateInst4->addNodeName(pPortName40);
    char* pPortName41 = "w_4_1";
    pGateInst4->addNodeName(pPortName41);
    char* pPortName42 = "w_4_2";
    pGateInst4->addNodeName(pPortName42);

    _glbNetlistModule.addInst(pGateInst4);


    char* pInstName5 = "I5";
    GateInst* pGateInst5 = new GateInst(pInstName5);

    char *pModDefName5 = "not1";
    pGateInst5->setGateType(pModDefName5);

    char* pPortName50 = "w_4_2";
    pGateInst5->addNodeName(pPortName50);
    char* pPortName51 = "w_4_0";
    pGateInst5->addNodeName(pPortName51);

    _glbNetlistModule.addInst(pGateInst5);



    printModInfo();

    return 0;
}




