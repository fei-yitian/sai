#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>

using namespace std;

void findDuplicates(const vector<vector<string>>& a) {
    // 哈希表，键为字符串，值为数组索引和元素索引的对
    unordered_map<string, vector<pair<int, int>>> stringMap;

    // 遍历数组 a
    for (int i = 0; i < a.size(); ++i) {
        for (int j = 0; j < a[i].size(); ++j) {
            const auto& str = a[i][j];
            // 存储 (数组索引, 元素索引)
            stringMap[str].emplace_back(i, j);
        }
    }

    // 输出重复的字符串及其索引
    cout << "重复的字符串及其索引:" << endl;
    for (const auto& entry : stringMap) {
        if (entry.second.size() > 1) { // 如果该字符串出现超过一次
            cout << "字符串: " << entry.first << " 出现在数组: ";
            for (const auto& index : entry.second) {
                cout << "数组 " << index.first << " 的第 " << index.second << " 个元素; ";
            }
            cout << endl;
        }
    }
}

int main() {
    // 示例数组
    vector<vector<string>> a = {
        {"apple", "banana"},
        {"orange", "banana"},
        {"grape", "apple"},
        {"kiwi"}
    };

    findDuplicates(a);

    return 0;
}
