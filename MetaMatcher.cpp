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
        else if (!bb_x->getModular()) {
            printf("skip bb %d", bb_x->getID());
            i++;
        }
        else if (!bb_y->getModular()) j++;
        else {
            int k = j;
            for (; k < bbs_y.size(); k++) {
                MetaBB* bb_k = bbs_y[k];
                if (visited_y.find(bb_k) != visited_y.end()) continue;
                if (bb_x->similarity(*bb_k) > 0.99) {
                    bbMap[bb_x] = bb_k;
                    visited_x.insert(bb_x);
                    visited_y.insert(bb_k);
                    i++; j += (k == j);
                    break;
                }
            }
            // if didn't find a matched bb for bb_x
            if (k == bbs_y.size()) {
                visited_x.insert(bb_x);
                i++;
            }
        }
    }

    return *this;
}

std::unordered_map<MetaBB*, MetaBB*>& MetaMatcher::getBBMatchResult() {
    return bbMap;
}

} // namespace MetaTrans
