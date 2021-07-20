/*************************************************************************
    > File Name: split.cpp
    > Author: ian
    > Mail: 18896500132@163.com 
    > Created Time: Fri 14 Jun 2019 03:46:59 PM CST
 ************************************************************************/
#include <iostream>
#include <string>
#include <stdlib.h>
#include <unordered_set>

using namespace std;

static std::unordered_set<int> g_default_pfeed_cmd;

void _init(std::string list) {
    if (list.empty()) {
        cout << "[REDIS]:Need config default passive feed cmd list" << endl;
        return ;
    }
        
    std::string _list = list;
    if (list[list.size() - 1] != ',')
        _list += ',';
    cout << "total list is : " << _list << endl;
    size_t pos = _list.find(',');
    size_t len = _list.size();

    while (pos != std::string::npos) {
        std::string _scmd = _list.substr(0, pos);
        cout << "list:" << _list << "  pos:" << pos << "  sub:"<<_scmd << endl;
        int _icmd = atoi(_scmd.c_str());
        if (_icmd > 0) {
            g_default_pfeed_cmd.insert(_icmd + 15000);
        }
        _list = _list.substr(pos + 1, len - pos);
        pos = _list.find(',');
    }

    for (auto it: g_default_pfeed_cmd)
        cout << it << endl;
}

int main() {
    _init(std::string("7003,7004"));
    _init(std::string("7003,7004,"));
    return 0;
}
