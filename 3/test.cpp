#include <iostream>
#include <vector>
#include <string>

using namespace std;

class Gate {
public:
    string type; // "AND", "OR", "NOT", "NAND"
    vector<int> inputs; // 输入端口的状态 (0或1)

    Gate(string gateType) : type(gateType) {}

    int evaluate() {
        if (type == "AND") {
            return inputs[0] & inputs[1];
        } else if (type == "OR") {
            return inputs[0] | inputs[1];
        } else if (type == "NOT") {
            return !inputs[0];
        } else if (type == "NAND") {
            return !(inputs[0] & inputs[1]);
        }
        return 0; // 默认返回值
    }
};

bool checkOscillationCondition(vector<Gate>& circuit) {
    int inverterCount = 0;
    
    for (Gate& gate : circuit) {
        if (gate.type == "NOT" || gate.type == "NAND") {
            inverterCount++;
        }
    }

    // 检查反相器的数量是否为奇数
    return (inverterCount % 2 == 1);
}

void findOscillationInputCombination(vector<Gate>& circuit) {
    // 遍历输入组合 
    
    for (int i = 0; i < 2; ++i) { // 2^numInputs
        // 设置每个门的输入状态
        for (size_t j = 0; j < circuit.size(); ++j) {
            // 通过位掩码设置输入状态
            circuit[j].inputs = { (i >> 0) & 1, (i >> 1) & 1 };
        }

        // 计算输出
        for (Gate& gate : circuit) {
            int output = gate.evaluate();
            // 输出可以用于后续负反馈的检测逻辑
        }

        // 检查震荡条件
        if (checkOscillationCondition(circuit)) {
            cout << "Found oscillation condition with inputs: ";
            for (const auto& gate : circuit) {
                for (int input : gate.inputs) {
                    cout << input << " ";
                }
            }
            cout << endl;
            return; // 找到条件后返回
        }
    }

    cout << "No oscillation condition found." << endl;
}

int main() {
    vector<Gate> circuit;
    
    // 示例：创建一个电路
    circuit.push_back(Gate("NOT"));   // 反相器
    circuit.push_back(Gate("AND"));    // 与门
    circuit.push_back(Gate("OR"));     // 或门
    circuit.push_back(Gate("NAND"));   // 与非门

    // 查找可能的震荡输入组合
    findOscillationInputCombination(circuit);

    return 0;
}
