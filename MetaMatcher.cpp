#include "meta/MetaMatcher.h"
#include <unordered_set>
#include "meta/MetaUtils.h"

namespace MetaTrans {

MetaMatcher::MetaMatcher() : x(nullptr), y(nullptr) { }

MetaMatcher& MetaMatcher::setX(MetaFunction* x) {
    this->x = x;
    return *this;
}

MetaMatcher& MetaMatcher::setY(MetaFunction* y) {
    this->y = y;
    return *this;
}

MetaMatcher& MetaMatcher::matchBB() {
    MetaBB* root_x = x->getRoot();
    MetaBB* root_y = y->getRoot();
    std::vector<MetaBB*>& bbs_x = x->getBB();
    std::vector<MetaBB*>& bbs_y = y->getBB();
    std::unordered_set<MetaBB*> visited_x, visited_y;

    int i = 0, j =0;
    while (i < bbs_x.size() && j < bbs_y.size()) {
        MetaBB* bb_x = bbs_x[i];
        MetaBB* bb_y = bbs_y[j];
        printf("x modular: %f, id:%d y modular: %f, id: %d\n", bb_x->getModular(), bb_x->getID(), bb_y->getModular(), bb_y->getID());
        // if x and y has zero modular
        // 只为开始的几个模长为0的BB进行Match，后面的就不管了
        if (!bb_x->getModular() && !bb_y->getModular()) {
            bbMap[bb_x] = bb_y;
            visited_x.insert(bb_x);
            visited_y.insert(bb_y);
            i++; j++;
        }
        else if (!bb_x->getModular()) i++;
        else if (!bb_y->getModular()) j++;
        else matchNextBB(i, j, bbs_x, bbs_y, visited_x, visited_y);
    }

    return *this;
}

MetaMatcher& MetaMatcher::matchNextBB(int& i, int& j, std::vector<MetaBB*>& bbs_x, std::vector<MetaBB*>& bbs_y, std::unordered_set<MetaBB*>& visited_x, std::unordered_set<MetaBB*>& visited_y) {
    MetaBB* bb_x = bbs_x[i]; int k = j;
    for (; k < bbs_y.size(); k++) {
        MetaBB* bb_z = bbs_y[k];
        if (visited_y.find(bb_z) != visited_y.end()) continue;
        if (bb_x->similarity(*bb_z) < 0.99) continue;

        visited_x.insert(bb_x);
        visited_y.insert(bb_z);
        bbMap[bb_x] = bb_z;
        i += 1; j += (k == j); // j 每次最多只移动一次，防止出现交错的情况

        break;
    }
    // if didn't find a matched bb for bb_x
    if (k == bbs_y.size()) {
        visited_x.insert(bb_x);
        i++;
    }
    return *this;
}

MetaMatcher& MetaMatcher::matchInst() {
    for (auto pair = bbMap.begin(); pair != bbMap.end(); ++pair) {
        MetaBB& u = *(pair->first);
        MetaBB& v = *(pair->second);
        matchInstGraph(u, v);
    }

    return *this;
}

std::pair<MetaInst*, MetaInst*> MetaMatcher::matchInstGraph(MetaBB& u, MetaBB& v) {
    for (auto inst = u.inst_begin(); inst != u.inst_end(); inst++) {
        MetaInst& instRef = **inst;
        if (instRef.isLoad() || instRef.isStore()) {
            std::vector<MetaInst*> matchedInsts = instRef.findTheSameInst(&v);
            MetaUtil::printVector(matchedInsts, "=========== Matched Inst For " + MetaUtil::toString(instRef.getInstType()) + " ===========");
        }
    }

}

std::unordered_map<MetaBB*, MetaBB*>& MetaMatcher::getBBMatchResult() {
    return bbMap;
}

} // namespace MetaTrans
