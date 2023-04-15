#include "meta/MetaMatcher.h"
#include <unordered_set>
#include "meta/MetaUtils.h"

namespace MetaTrans {

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

MetaBB* CraphBasedBBMatcher::next(MetaBB* bb) {
    if (bb->getNextBB().size() == 0) return nullptr;
    if (isPartOfLoop(bb)) {
        auto sucs = bb->getNextBB();
        for (MetaBB* next : bb->getNextBB()) {
            if (next == bb) continue;
            if (visitedSet.find(next) == visitedSet.end()) return next;
        }

        if (sucs.size() > 1)
            printf("INFO: self = %d, suc0 = %d, suc1 = %d\n", bb->getID(), sucs[0]->getID(), sucs[1]->getID());
        printf("WANR: Child of current Node all has been visited, skip and continue.\n");
        
        // 后代都访问过了，别重复访问，返回空指针
        return nullptr;
    }
    return bb->getNextBB()[0];
}

// 0 - black
// 1 - red
int getColor(MetaBB* bb) {
    if (bb->getNumMemOp() == 0 && bb->isLinearInCFG()) return 1;
    return 0;
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
    int ret = 0;
    ret = x.num_bb    - y.num_bb   ; if (ret) return ret;
    ret = x.num_inst  - y.num_inst ; if (ret) return ret;
    ret = x.num_memop - y.num_memop; if (ret) return ret;
    return y.order - x.order;
}


bool checkSwap(MetaBB* p, MetaBB* l, MetaBB* r, MetaBB* joint) {
    // joint为空是边界情况
    if (!joint) return false;

    std::list<MetaBB*>                   que;
    std::unordered_set<MetaBB*>          visited;
    std::unordered_map<MetaBB*, MetaBB*> occupied;
     std::unordered_map<MetaBB*, weight> weights;
    
    // 以防有指向父亲的边
    visited.insert(p);

    que.push_back(l); occupied[l] = l;
    que.push_back(r); occupied[r] = r;
    weights[l] = { 0, 0, 0, l->getID() };
    weights[r] = { 0, 0, 0, r->getID() };
    
    while (!que.empty()) {
        MetaBB* cur = que.front(); que.pop_front();

        visited.insert(cur);

        if (cur == joint) continue;

        // 更新这一控制流的权重
        MetaBB* anc = occupied[cur];
        weights[anc].num_bb     += 1;
        weights[anc].num_inst   += cur->getNumInst();
        weights[anc].num_memop  += cur->getNumMemOp();

        std::vector<MetaBB*> successors = cur->getNextBB();

        for (MetaBB* suc : successors) {
            if (visited.find(suc) != visited.end()) continue;
            occupied[suc] = occupied[cur];
            que.push_back(suc);
        }
        
    }

    if (compare(weights[l], weights[r]) < 0) {
        printf("INFO: Swapped %d and %d.\n", l->getID(), r->getID());
        printf("INFO: Weight l, %d, %d, %d, %d\n", weights[l].num_bb, weights[l].num_inst, weights[l].num_memop, weights[l].order);
        printf("INFO: Weight r, %d, %d, %d, %d\n", weights[r].num_bb, weights[r].num_inst, weights[r].num_memop, weights[r].order);
        p->swapSuccessors();
        return true;
    }

    return false;

}

// 寻找Joint节点
// 如有必要会交换 l 和 r 以保持左倾性
MetaBB* bfs(MetaBB* p, MetaBB* l, MetaBB* r, MetaBB* end) {

    std::list<MetaBB*>                   que;
    std::unordered_set<MetaBB*>          visited;
    std::unordered_map<MetaBB*, MetaBB*> occupied;
    
    // 以防有指向父亲的边
    visited.insert(p);

    que.push_back(l); occupied[l] = l;
    que.push_back(r); occupied[r] = r;

    MetaBB* ret = nullptr;
    
    while (!que.empty()) {
        MetaBB* cur = que.front(); que.pop_front();

        if (cur == ret) return cur;

        visited.insert(cur);

        std::vector<MetaBB*> successors = cur->getNextBB();

        for (MetaBB* suc : successors) {
            if (cur == end) break;

            // 汇合点
            if (occupied[suc]) {
                if (occupied[suc] != occupied[cur] && !ret) 
                    ret = suc;
            }
            else occupied[suc] = occupied[cur];

            if (visited.find(suc) != visited.end()) continue;

            que.push_back(suc);
        }
        
    }

    return ret;
}


bool CraphBasedBBMatcher::isPartOfLoop(MetaBB* bb) {
    return isStartOfLoop(bb) || isEndOfLoop(bb);
}

bool CraphBasedBBMatcher::isStartOfLoop(MetaBB* bb) {
    std::vector<int> features = bb->getFeature();
    return features[2] >= 2 && features[3] == 1;
}

bool CraphBasedBBMatcher::isEndOfLoop(MetaBB* bb) {
    for (MetaBB* suc : bb->getNextBB()) {
        if (visitedSet.find(suc) != visitedSet.end()) return true;
    }
    return false;
}


// Find the FIRST merge point of Control Flow.
// Using finger search to limit the complexity to O(n).
// Keep the left-leaning property before return.
MetaBB* CraphBasedBBMatcher::findJoint(MetaBB* split, MetaBB* merge) {
    
    if (split->getNextBB().size() < 2) return merge;

    MetaBB* l     = leftChild(split);
    MetaBB* r     = rightChild(split);
    MetaBB* joint = bfs(split, l, r, merge);

    checkSwap(split, l, r, joint);

    return joint;
}



// u -...-> x (ASM side)
// v -...-> y (IR  side)
void CraphBasedBBMatcher::match(MetaBB* u, MetaBB* v, MetaBB* x, MetaBB* y) {
    // 非法情况直接返回
    if (u == nullptr || v == nullptr || x == nullptr || y == nullptr) return;
    if (u == x || v == y) return;
    
    assert(u); assert(x);
    assert(v); assert(y);

    visitedSet.insert(u); visitedSet.insert(v);
    visitedSet.insert(x); visitedSet.insert(y);

    printf(
        "INFO: Start match for ASM(start) %d, ASM(end) %d, IR(start) %d, IR(end) %d.\n",
        u->getID(), x->getID(),
        v->getID(), y->getID()
    );

    // 一黑一红的情况
    if      (getColor(u) && !getColor(v)) {
        match(next(u), v, x, y);
    }
    else if (!getColor(u) && getColor(v)) {
        match(u, next(v), x, y);
    }
    // 都为红色，可以直接匹配
    else if (getColor(u) && getColor(v)) {
        bbMap[u] = v;
        matchedSet.insert(u);
        matchedSet.insert(v);
        match(next(u), next(v), x, y);
    }
    // 全为黑的情况
    else if (!getColor(u) && !getColor(v)) {
        // 访存操作完全一致则匹配成功
        if (u->memOpSimilarity(v) > 0.999) {
            bbMap[u] = v;
            matchedSet.insert(u);
            matchedSet.insert(v);
        }
        // 根据当前节点的性质决定如何继续往后匹配
        if (u->isLinearInCFG() && v->isLinearInCFG() || isPartOfLoop(u) && isPartOfLoop(v)) {
            // printf("INFO: Next Match ASM BB %d AND IR BB %d\n", next(u)->getID(), next(v)->getID());
            match(next(u), next(v), x, y);
        }
        else if (u->isLinearInCFG() && !(v->isLinearInCFG())) {
            printf("WARN: Skip ASM BB %d AND IR BB %d\n", u->getID(), v->getID());
            match(next(u), v, x, y);
            
        }
        else if (!(u->isLinearInCFG()) && v->isLinearInCFG()) {
            printf("WARN: Skip ASM BB %d AND IR BB %d\n", u->getID(), v->getID());
            match(u, next(v), x, y);
        }
        else {
            MetaBB* u_joint = findJoint(u, x);
            MetaBB* v_joint = findJoint(v, y);
            
            if (u_joint && v_joint)
                printf("INFO: Find Joint point, ASM %d --> %d, IR %d --> %d\n", u->getID(), u_joint->getID(), v->getID(), v_joint->getID());
            
            match(leftChild(u), leftChild(v), u_joint, v_joint);
            match(rightChild(u), rightChild(v), u_joint, v_joint);
            match(u_joint, v_joint, x, y);
        }

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


void findUntilLui(std::vector<std::vector<MetaInst*>>& res, std::vector<MetaInst*>& codes, MetaInst* cur) {
    assert(cur);
    printf("Cur ASM Inst: %s, id = %d, color = %d.\n", cur->getOriginInst().c_str(), cur->getID(), cur->getColor()->type);
    // 遇到phi和load / store直接Return
    if (cur->isMetaPhi() || (codes.size() && cur->isMemOp())) return;
    // TODO: 暂时硬编码，之后考虑加一个标志位来判断递归基
    if (cur->getOriginInst() == "LUI" || cur->getOriginInst() == "lui") {
        codes.push_back(cur);
        res.push_back(codes);
        codes.pop_back();
        return;
    }
    
    if (cur->isAddressing() || cur->isMemOp()) codes.push_back(cur);
    for (MetaOperand* operand : cur->getOperandList()) {
        if (operand->isMetaConstant()) continue;
        if (operand->isMetaArgument()) continue;
        findUntilLui(res, codes, (MetaInst*)operand);
    }
    if (cur->isAddressing() || cur->isMemOp()) codes.pop_back();

}


void findUntilGep(std::vector<std::vector<MetaInst*>>& res, std::vector<MetaInst*>& codes, MetaInst* cur) {
    assert(cur);
    printf("Cur IR Inst: %s, id = %d, color = %d.\n", cur->getOriginInst().c_str(), cur->getID(), cur->getColor()->type);
    // 遇到phi和load / store直接Return
    if (cur->isMetaPhi() || (codes.size() && cur->isMemOp())) return;
    // TODO: 暂时硬编码，之后考虑加一个标志位来判断递归基
    if (cur->getOriginInst() == "getelementptr") {
        codes.push_back(cur);
        res.push_back(codes);
        codes.pop_back();
        return;
    }

    for (MetaOperand* operand : cur->getOperandList()) {
        if (operand->isMetaConstant()) continue;
        if (operand->isMetaArgument()) continue;
        findUntilGep(res, codes, (MetaInst*)operand);
    }
    
    return;
}

MetaAddressMatcher& MetaAddressMatcher::match() {
    MetaInst* curASM = asb    ;
    MetaInst* curIR  = nullptr;

    std::vector<MetaInst*> matchedLoad = asb->findTheSameInst(irbb);

    if (matchedLoad.size() == 0) {
        printf("ERRO: Did't find matched load instruction.\n");
        return *this;
    }
    if (matchedLoad.size() > 1) {
        printf("WARN: Find multiple load when match addressing.\n");
    }

    curIR = matchedLoad[0];

    // TODO 这个assert过不去
    // assert(curASM->getParentScope() == curIR->getParentScope());

    std::string type = asb->isLoad() ? "load" : "store";
    printf("matched %s: %s\n", type.c_str(), curIR->getOriginInst().c_str());

    // 实际上IR那侧可以直接就翻为GEP，所以结构就是一个Code Piece的Set
    std::vector<std::vector<MetaInst*>> resIR;
    std::vector<std::vector<MetaInst*>> resASM;
    std::vector<MetaInst*>  addrIR ;
    std::vector<MetaInst*>  addrASM;
    printf("INFO: Start finding GEP\n");
    findUntilGep(resIR , addrIR , curIR );
    printf("INFO: Start finding LUI\n");
    findUntilLui(resASM, addrASM, curASM);

    printf("INFO: Result of lui found %d pieces.\n", resASM.size());
    for (auto codes : resASM) {
        printf("INFO: Length of cur piece is %d\n", codes.size());
        for (MetaInst* inst : codes) {
            printf("%s(%d) ", inst->getOriginInst().c_str(), inst->getID());
        }
        printf("\n");
    }

    printf("INFO: Result of gep found %d pieces.\n", resIR.size());
    for (auto codes : resIR) {
        printf("INFO: Length of cur piece is %d\n", codes.size());
        for (MetaInst* inst : codes) {
            printf("%s(%d) ", inst->getOriginInst().c_str(), inst->getID());
        }
        printf("\n");
    }

    // 竟然没匹配找到GEP？非法情况，返回
    if (resIR.size() == 0) return *this;
        
    for (int i = 0; i < resASM.size(); ++i) {
        CodePiece asmCodes, irCodes;
        addrIR = resIR[0];
        addrASM = resASM[i];
        for (int j = 0; j < addrIR.size(); ++j) {
            irCodes.addInst(addrIR[j]->getOriginInst());
        }
        // codes[0]是load指令，所以如果不把load加到pattern里面的话，要从1开始拷贝
        for (int j = 0; j < addrASM.size(); ++j) {
            asmCodes.addInst(addrASM[j]->getOriginInst());
        }
        codeMap[asmCodes.hashCode()] = {asmCodes, irCodes};
        printf("INFO: Create Addressing Mapping: (%s) -->> (%s)\n", asmCodes.toString().c_str(), irCodes.toString().c_str());
    }

    return *this;
}

std::vector<CodePiecePair> MetaAddressMatcher::getResult() {
    std::vector<CodePiecePair> ret;
    for (auto it = codeMap.begin(); it != codeMap.end(); ++it) {
        ret.push_back(it->second);
    }
    return ret;
}


std::unordered_map<uint64_t, CodePiecePair>& MetaAddressMatcher::getResultMap() {
    return codeMap;
}


bool MetaAddressMatcher::matched(MetaInst* inst) {
    return matchedSet.find(inst) != matchedSet.end();
}

} // namespace MetaTrans
