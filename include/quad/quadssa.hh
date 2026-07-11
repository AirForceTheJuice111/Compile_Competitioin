#ifndef __QUAD_SSA_HH
#define __QUAD_SSA_HH

#include <map>
#include <utility>
#include <set>
#include <string>
#include <vector>
#include "temp.hh"
#include "quad.hh"
#include "flowinfo.hh"

using namespace std;
using namespace quad;

class VersionedTemp {
public:

static void registerVersionedTemp(int versionedTempNum, int oldNum, int version) {
    registry()[versionedTempNum] = {oldNum, version};
}

static int versionedTempNum(int old_num, int version) {
    return old_num * 100 + version;
}

static int origTempNum(int versionedTempNum) {
    auto found = registry().find(versionedTempNum);
    if (found != registry().end()) {
        return found->second.first;
    }
    return versionedTempNum / 100;
}

static int versionNum(int versionedTempNum) {
    auto found = registry().find(versionedTempNum);
    if (found != registry().end()) {
        return found->second.second;
    }
    return versionedTempNum - origTempNum(versionedTempNum) * 100;
}

private:
static std::map<int, std::pair<int, int>>& registry() {
    static std::map<int, std::pair<int, int>> data;
    return data;
}
};

QuadProgram* quad2ssa(set<FuncFlowInfo*>* allFuncFlow);

#endif // __QUAD_SSA_HH
