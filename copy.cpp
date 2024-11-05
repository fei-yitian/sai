#include<iostream>
#include <stdio.h>
#include <stdlib.h>
#include "vpi_user.h"
#include "sv_vpi_user.h"
#include <vector>
#include <string>
#include<set>

using namespace std;

typedef vector<string>   StrVec;


class GateInst//门信息
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

    void dumpInst()//打印信息
    {
        printf("%-5s  %s  ", _gateType.c_str(), _instName.c_str());
        for(unsigned ii = 0; ii != _instNodes.size(); ++ii)
        {
            printf("%s  ", _instNodes[ii].c_str());
        }
        printf("\n");
    }

    string  _gateType;//门的类型
    string  _instName;//门的名字
    StrVec  _instNodes;//节点名称
};

typedef vector<GateInst*>  GateInstVec;

class NetlistModule//电路模块，包含多个门信息
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
    void func( ){//
         for(unsigned ii = 0; ii != _instVec.size(); ++ii){
        GateInst* pInst = _instVec[ii];
        arry.push_back(pInst->_instName);
        for(unsigned ii = 0; ii !=pInst-> _instNodes.size(); ++ii)
            {
            arry.push_back(pInst->_instNodes[ii]);
         }
        }
        std::set<string> s(arry.begin(), arry.end()); // 清空原来的 vector 并将去重后的元素插回
        arry.assign(s.begin(), s.end());
     }




void show(){
     for (unsigned ii = 0; ii != arry.size(); ++ii) {
    std::cout << arry[ii] << " ";
 }
    std::cout<<"一共有这么多节点："<<arry.size();
 }
    StrVec arry;
    string      _moduelName;
    GateInstVec _instVec;//GateInst 指针的向量 GateInstVec 来存储这些逻辑门实例
};

NetlistModule _glbNetlistModule;//全局的电路实例_glbNetlistModule

void printModInfo()//打印模块信息
{
    printf("module name is %s\n", _glbNetlistModule._moduelName.c_str());//先打印模块的名称
    _glbNetlistModule.dumpInst();//然后调用dumpInst打印所有逻辑门实例的详细信息
    _glbNetlistModule.func();
    _glbNetlistModule.show();
}


#ifdef  __cplusplus
extern "C" {
#endif//预处理器指令，用于处理 C++ 和 C 语言之间的接口。

int get_module_info( )
{
    vpiHandle systf_h = vpi_handle( vpiSysTfCall, NULL );
    if( systf_h == NULL )
        return 0;

    vpiHandle modHdl = vpi_handle( vpiScope,systf_h );
    modHdl = vpi_handle(vpiModule, modHdl);
    char *pModDefName = vpi_get_str(vpiDefName, modHdl);
    _glbNetlistModule.setModuleName(pModDefName);//将获取到的模块名称设置到全局 NetlistModule 实例中

    vpiHandle instIter = vpi_iterate(vpiInstance, modHdl);//遍历模块内的所有实例
    vpiHandle instHdl = NULL;
    while( ( instHdl = vpi_scan( instIter ) ) )
    {
        char* pInstName = vpi_get_str( vpiName, instHdl );
        GateInst* pGateInst = new GateInst(pInstName);//获取当前实例的名称，并创建一个新的 GateInst 实例

        char *pModDefName = vpi_get_str(vpiDefName, instHdl);
        pGateInst->setGateType(pModDefName);//获取实例的逻辑门的类型，并设置到 GateInst 实例中

        vpiHandle portIter = vpi_iterate(vpiPort, instHdl);//遍历实例的所有端口
        vpiHandle portHdl = NULL;
        while ( (portHdl = vpi_scan(portIter)) )
        {
            vpiHandle highConnHdl = vpi_handle(vpiHighConn, portHdl);
            char* pPortName = vpi_get_str(vpiName, highConnHdl);
            pGateInst->addNodeName(pPortName);
        }

        _glbNetlistModule.addInst(pGateInst);//将 GateInst 实例添加到全局 NetlistModule 实例中
    }

    printModInfo();//打印模块信息

    

    vpi_control(vpiFinish);//通知仿真器在当前时间点结束仿真。


    return 0;
}

#ifdef  __cplusplus
}
#endif


