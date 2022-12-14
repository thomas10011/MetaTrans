#include "meta/MetaMatcher.h"
#include <unordered_set>


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

MetaMatcher& MetaMatcher::match() {
    MetaBB* root_x = x->getRoot();
    MetaBB* root_y = y->getRoot();
    std::vector<MetaBB*>& bbs_x = x->getBB();
    std::vector<MetaBB*>& bbs_y = y->getBB();
    std::unordered_set<MetaBB*> visited_x, visited_y;

    int tmp = 0;
    for (int i = 0; i < bbs_x.size(); ++i) {
        for (int j = tmp; j < bbs_y.size(); ++j) {
            MetaBB* bb_x = bbs_x[i];
            MetaBB* bb_y = bbs_x[j];
            if (visited_y.find(bb_y) != visited_y.end()) continue;
            // if similarity great than 99% ...
            if (bb_x->similarity(*bb_y) >= 0.99) {
                bbMap[bb_x] = bb_y;
                visited_y.insert(bb_y);
                tmp = j+1;
                break;
            }
        }
    }

    return *this;
}

std::unordered_map<MetaBB*, MetaBB*>& MetaMatcher::getBBMatchResult() {
    return bbMap;
}

}
