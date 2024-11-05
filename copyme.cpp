
#include <stdio.h>
#include <stdlib.h>
#include "vpi_user.h"
#include "sv_vpi_user.h"
#include <vector>
#include <string>


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

    string      _moduelName;
    GateInstVec _instVec;
};

NetlistModule _glbNetlistModule;

void printModInfo()
{
    printf("module name is %s\n", _glbNetlistModule._moduelName.c_str());
    _glbNetlistModule.dumpInst();
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






  

