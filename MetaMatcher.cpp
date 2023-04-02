#include "meta/MetaMatcher.h"
#include <unordered_set>
#include "meta/MetaUtils.h"

namespace MetaTrans {

CodePiece::CodePiece() { }

CodePiece::CodePiece(std::vector<std::string> init) : instList(init) { }

CodePiece& CodePiece::addInst(std::string inst) {
    instList.push_back(inst);
    return *this;
}

uint64_t CodePiece::hashCode() {
    uint64_t factor = 1, hash = 0;
    for (int i = 0; i < instList.size(); ++i) {
        std::string inst = instList[i];
        for (int j = 0; j < inst.length(); ++j) {
            hash   += inst[j] * factor;
            factor *= 13331;
        }
    }
    return hash;
}


MetaBBMatcher::MetaBBMatcher() : x(nullptr), y(nullptr) {
    
}


LinearMetaBBMatcher::LinearMetaBBMatcher() {

}

MetaBBMatcher& MetaBBMatcher::setAsmMetaFunc(MetaFunction* x) {
    this->x = x;
    return *this;
}

MetaBBMatcher& MetaBBMatcher::setIrMetaFunc(MetaFunction* y) {
    this->y = y;
    return *this;
}

std::unordered_map<MetaBB*, MetaBB*>& MetaBBMatcher::getResult() {
    return bbMap;
}

MetaBBMatcher& LinearMetaBBMatcher::match() {
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

LinearMetaBBMatcher& LinearMetaBBMatcher::matchNextBB(int& i, int& j, std::vector<MetaBB*>& bbs_x, std::vector<MetaBB*>& bbs_y, std::unordered_set<MetaBB*>& visited_x, std::unordered_set<MetaBB*>& visited_y) {
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

LinearMetaBBMatcher& LinearMetaBBMatcher::matchInst() {
    for (auto pair = bbMap.begin(); pair != bbMap.end(); ++pair) {
        MetaBB& u = *(pair->first);
        MetaBB& v = *(pair->second);
        matchInstGraph(u, v);
    }

    return *this;
}

std::pair<MetaInst*, MetaInst*> LinearMetaBBMatcher::matchInstGraph(MetaBB& u, MetaBB& v) {
    for (auto inst = u.inst_begin(); inst != u.inst_end(); inst++) {
        MetaInst& instRef = **inst;
        if (instRef.isLoad() || instRef.isStore()) {
            std::vector<MetaInst*> matchedInsts = instRef.findTheSameInst(&v);
            std::cout <<"\nDEBUG:: matchInstGraph():: matchedInsts vector size = " << matchedInsts.size() << std::endl;
            if(matchedInsts.size()==1 && matchedInsts[0]!=NULL)
                instRef.buildMapping(matchedInsts[0]);
            MetaUtil::printVector(matchedInsts, "=========== Matched Inst For " + MetaUtil::toString(instRef.getInstType()) + " ===========");
        }
    }

}

CraphBasedBBMatcher::CraphBasedBBMatcher() { }


// 0 - black
// 1 - red
int getColor(MetaBB* bb) {
    if (bb->getNumMemOp() == 0 && bb->isLinearInCFG()) return 1;
    return 0;
}


MetaBBMatcher& CraphBasedBBMatcher::match() {
    MetaBB* asmRoot = x->getRoot();
    MetaBB* irRoot  = y->getRoot();

    MetaBB* asmExit = x->getExit();
    MetaBB* irExit  = y->getExit();

    // bbMap[asmRoot] = irRoot;
    bbMap[asmExit] = irExit;
    
    match(asmRoot, irRoot, asmExit, irExit);

    return *this;
}

MetaBB* next(MetaBB* bb) {
    if (bb->getNextBB().size() == 0) return nullptr;
    if (bb->isSelfLoop()) 
        for (MetaBB* next : bb->getNextBB()) 
            if (next != bb) return next;
    return bb->getNextBB()[0];
}


MetaBB* leftChild(MetaBB* bb) {
    return bb->getNextBB()[0];
}

MetaBB* rightChild(MetaBB* bb) {
    return bb->getNextBB()[1];
}

struct weight {
    int num_bb;
    int num_inst;
    int num_memop;
    int order;
};

// 比较权重大小
int compare(weight x, weight y) {
    int ret;
    ret = x.num_bb - y.num_bb;
    if (ret) return ret;
    ret = x.num_inst - y.num_inst;
    if (ret) return ret;
    ret = x.num_memop - y.num_memop;
    if (ret) return ret;

    return y.order - x.order;
}


// 寻找Joint节点
// 如有必要会交换 n 和 m 以保持左倾性
MetaBB* bfs(MetaBB* n, MetaBB* m, MetaBB* end, int& needSwap) {

    std::list<MetaBB*> que;
    std::unordered_set<MetaBB*> visited;
    std::unordered_map<MetaBB*, MetaBB*> occupied;
    std::unordered_map<MetaBB*, weight> weights;
    

    que.push_back(n); occupied[n] = n;
    que.push_back(m); occupied[m] = m;
    weights[n] = { 0, 0, 0, n->getID() };
    weights[m] = { 0, 0, 0, m->getID() };

    MetaBB* ret;
    
    while (!que.empty()) {
        MetaBB* cur = que.front(); que.pop_front();
        visited.insert(cur);

        // 更新这一控制流的权重
        MetaBB* anc = occupied[cur];
        weights[anc].num_bb     += 1;
        weights[anc].num_inst   += cur->getNumInst();
        weights[anc].num_memop  += cur->getNumMemOp();

        std::vector<MetaBB*> successors = cur->getNextBB();

        for (MetaBB* suc : successors) {
            // 汇合点
            if (suc == end || occupied[suc]) {
                if (compare(weights[n], weights[m]) < 0) {
                    needSwap = 1;
                }
                ret = suc;
            }

            if (visited.find(suc) != visited.end()) continue;

            occupied[suc] = occupied[cur];
            que.push_back(suc);
        }

        
    }

    return ret;

}


// Find the FIRST merge point of Control Flow.
// Using finger search to limit the complexity to O(n).
// Keep the left-leaning property before return.
MetaBB* findJoint(MetaBB* split, MetaBB* merge) {
    
    assert(split->getNextBB().size() >= 2);

    MetaBB* l = leftChild(split);
    MetaBB* r = rightChild(split);

    int needSwap = 0;
    MetaBB* joint = bfs(l, r, merge, needSwap);

    if (needSwap) {
        split->swapSuccessors();
    }

    return joint;
}



// u -...-> x (ASM side)
// v -...-> y (IR  side)
void CraphBasedBBMatcher::match(MetaBB* u, MetaBB* v, MetaBB* x, MetaBB* y) {

    if (u == x || v == y) return;

    assert(u);
    assert(v);

    // 都为红色，可以直接匹配
    if (getColor(u) && getColor(v)) {
        bbMap[u] = v;
        match(next(u), next(v), x, y);
    }
    // 全为黑的情况
    else if (!getColor(u) && !getColor(v)) {
        // 访存操作完全一致则匹配成功
        if (u->memOpSimilarity(v) > 0.999) {
            bbMap[u] = v;
        }
        // 继续往后匹配
        if (u->isLinearInCFG() && v->isLinearInCFG() || u->isSelfLoop() && v->isSelfLoop()) {
            match(next(u), next(v), x, y);
        }
        else if (u->isLinearInCFG() && !v->isLinearInCFG()) {
            printf("WARN: Skip ASM BB %d AND IR BB %d\n", u->getID(), v->getID());
            match(next(u), v, x, y);
            return;
        }
        else if (!u->isLinearInCFG() && v->isLinearInCFG()) {
            printf("WARN: Skip ASM BB %d AND IR BB %d\n", u->getID(), v->getID());
            match(u, next(v), x, y);
            return;
        }
        else {
            MetaBB* u_joint = findJoint(u, x);
            MetaBB* v_joint = findJoint(v, y);
            
            printf("INFO: Find Joint point, ASM %d IR %d\n", u_joint->getID(), v_joint->getID());
            match(leftChild(u), leftChild(v), u_joint, v_joint);
            match(rightChild(u), rightChild(v), u_joint, v_joint);
            match(u_joint, v_joint, x, y);
        }

    }
    // 一黑一红的情况
    else if (getColor(u) && !getColor(v)) {
        match(next(u), v, x, y);
    }
    else if (!getColor(u) && getColor(v)) {
        match(u, next(v), x, y);
    }
    
}



MetaAddressMatcher::MetaAddressMatcher() {

}

MetaAddressMatcher& MetaAddressMatcher::setAsmInst(MetaInst* i) {
    this->asb = i;
    return *this;
}

MetaAddressMatcher& MetaAddressMatcher::setIrInst(MetaInst* i) {
    this->ir = i;
    return *this;
}

MetaAddressMatcher& MetaAddressMatcher::setIrBB(MetaBB* bb) {
    this->irbb = bb;
    return *this;
}


std::vector<MetaInst*> findUntilLui(MetaInst* inst) {
    std::vector<MetaInst*> res;

    MetaInst* cur = inst;

    for (MetaOperand* operand : cur->getOperandList()) {
        
        if (operand->isMetaConstant()) continue;
        if (operand->isMetaArgument()) continue;
        cur = (MetaInst*) operand;

        res.push_back(cur);

        if (cur->getOriginInst() == "LUI") break;
    }
    printf("INFO: result of lui\n");
    for (MetaInst* inst : res) {
        printf("%s\n", inst->toString().c_str());
    }
    printf("\n");
    return res;
}

std::vector<MetaInst*> findUntilPti(MetaInst* inst) {
    std::vector<MetaInst*> res;
    MetaInst* cur = inst;

    return res;
}

MetaAddressMatcher& MetaAddressMatcher::match() {
    MetaInst* curASM = asb    ;
    MetaInst* curIR  = nullptr;

    std::vector<MetaInst*> matchedLoad = asb->findTheSameInst(irbb);

    if (matchedLoad.size() == 0) {
        printf("WARN: Did't find matched load instruction.\n");
        return *this;
    }
    if (matchedLoad.size() > 1) {
        printf("WARN: Find multiple load when match addressing.\n");
    }

    curIR = matchedLoad[0];

    std::string type = asb->isLoad() ? "load" : "store";
    printf("matched %s: %s\n", type.c_str(), curIR->getOriginInst().c_str());

    std::vector<MetaInst*>  addrIR  = findUntilPti(curIR);
    std::vector<MetaInst*>  addrASM = findUntilLui(curASM);
    
    CodePiece asmCodes, irCodes;
    for (int i = 0; i < addrIR.size(); ++i) {
        irCodes.addInst(addrIR[i]->getOriginInst());
    }
    for (int i = 0; i < addrASM.size(); ++i) {
        asmCodes.addInst(addrASM[i]->getOriginInst());
    }

    codeMap[asmCodes.hashCode()] = {asmCodes, irCodes};

    return *this;
}

std::vector<CodePiecePair> MetaAddressMatcher::getResult() {
    std::vector<CodePiecePair> ret;
    for (auto it = codeMap.begin(); it != codeMap.end(); ++it) {
        ret.push_back(it->second);
    }
    return ret;
}

bool MetaAddressMatcher::matched(MetaInst* inst) {
    return matchedSet.find(inst) != matchedSet.end();
}

} // namespace MetaTrans
