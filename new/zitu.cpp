    void build_z(vector<string> scc) {
        unordered_map<string, list<string>> graph_z;

        unordered_map<string, string> port_to_type;// 新增，用于存储门名称和门类型的映射
        // 构建图结构（邻接表）
        for (unsigned ii = 0; ii != scc.size(); ++ii) {
            GateInst* pInst = scc[ii];
            
            // 第一个节点通常是输出，剩下的节点是输入
            string output = pInst->_instNodes[0];  // 输出信号
            string gate_port1 = pInst->_instName + ".port1";  //门结点
            string gate_port2 = pInst->_instName + ".port2";
            port_to_type[gate_port1] = pInst->_gateType;
            port_to_type[gate_port2] = pInst->_gateType;
           
            graph_z[gate_port1].push_back(output);  //将门结点连接到输出信号
            graph_z[gate_port2].push_back(output);

            for (unsigned jj = 1; jj < pInst->_instNodes.size(); ++jj) {
                string input = pInst->_instNodes[jj];  // 输入信号
                
                // 将输入信号连接到门结点
                if(jj==1)
                    graph_z[input].push_back(gate_port1);
                else
                    graph_z[input].push_back(gate_port2);

            }
        }
    }



    visited.clear();
    recStack.clear();
    allCycles.clear();
    _gateTypeMap=gateTypeMap;
    
    for (const auto& node : graph) {
        if (!visited[node.first]) {
            isCyclicUtil(node.first, graph);
        }
    }

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