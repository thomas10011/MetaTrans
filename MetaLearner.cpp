#include "meta/MetaLearner.h"
#include "meta/MetaUtils.h"



namespace MetaTrans {

const std::string RED("\033[31m");
const std::string GRN("\033[32m");
const std::string YEL("\033[33m");
const std::string CYN("\033[36m");
const std::string MAG("\033[35m");
const std::string RST("\033[0m");
const std::string BOLD("\033[1m");

void MetaLearner::setMapTable(MetaTrans::MappingTable* table) {
    this->MapTable = table;
}

MetaBB* MetaLearner::trainLoad(MetaBB* cur, MetaBB* irbb) {
    std::cout <<"DEBUG:: Entering trainLoad()......\n";
    MetaInst* irinst = NULL;

    for (auto inst = cur->begin(); inst != cur->end(); inst++) {

        if ((*inst)->getInstType()[0] == InstType::LOAD || (*inst)->getInstType()[0] == InstType::STORE ) {
            std::vector<MetaInst *> matchvec;
            // TODO:: CHECK If implict LOAD/STORE operations within an
            // instruction can be traversed correctly
            if (ifMatched(*inst)) {
                matchvec = getMatchedInst(*inst);
                std::cout << "DEBUG:: In trainLoad():: ASM instruction: "
                            << (*inst)->getOriginInst()
                            << " has been mathced to IR instruction: "
                            << matchvec[0]->getOriginInst() << std::endl;
            } else {
                matchvec = findTheSameInst(*inst, irbb);
                if (matchvec.size() != 0) {
                    std::cout << "DEBUG:: In trainLoad():: ASM LOAD: "
                                << (*inst)->getOriginInst()
                                << " Find a new match in IR: "
                                << matchvec[0]->getOriginInst() << std::endl;
                    for (auto i = matchvec.begin(); i != matchvec.end(); i++)
                        (*i)->toString();
                    // Build Mapping for new matched load instructions
                    if (matchvec.size() == 1)
                        buildMapping(*inst, matchvec[0]);
                }
            }
            // Skip unmatched or ambiguous instructions
            // Can be optimized to address ambiguity if needed
            if (matchvec.size() != 1) {
                if (matchvec.size() > 1) {
                    std::cout << "DEBUG:: Matched more than one load "
                                    "instruction!\n";
                    for (auto i = matchvec.begin(); i != matchvec.end();
                            i++)
                        std::cout << (*i)->toString() << std::endl;
                } else
                    std::cout
                        << "DEBUG:: No load instruction is matched!\n";
                continue;
            }

            // irinst = matchvec[0];
            // std::cout
            //     << "DEBUG:: Calling function trainEquivClass().....\n";
            // (*inst)->trainEquivClass(irinst);
            // std::cout << "\nDEBUG:: TrainLoad Completes the "
            //              "trainEquivClass().....\n";

        }
    }
    std::cout <<"DEBUG:: Leaving trainLoad()......\n";

}

MetaBB* MetaLearner::trainBB(MetaBB* cur, MetaBB* irbb) {
    std::cout << "\nDEBUG:: Entering function trainBB.....\n";
    MetaInst* irinst = NULL;
    std::vector<MetaInst*> asmvec;
    std::vector<MetaInst*> irvec;
    std::vector<MetaInst*> retvec;

    trainLoad(cur, irbb);

    for(auto inst= cur->begin(); inst != cur->end(); inst++){
        // Visit all the load and store instructions
        //if((*inst)->getInstType()[0] !=InstType::LOAD && (*inst)->getInstType()[0] !=InstType::STORE)
        std::cout << "DEBUG:: Checking Inst: "<< (*inst)->getOriginInst()<< std::endl;

        if(ifMatched(*inst) && (*inst)->getInstType()[0] != InstType::LOAD) {
            
            std::cout << "DEBUG:: Calling function QualifiedForMatch().....\n";
            auto res = QualifiedForMatch(*inst);
            if(res){
                std::cout << "DEBUG:: MatchedInst() size = "<< getMatchedInst(*inst).size() << std::endl;
                if(getMatchedInst(*inst).size() == 0)
                    continue;

                auto it = getMatchedInst(*inst)[getMatchedInst(*inst).size()-1];
                std::cout << "DEBUG:: The end of MatchedInst ID is = "<< it->getID()
                            << ", Type is "
                            <<  it->getInstType()[0] 
                            << std::endl;
                if(!it)
                    std::cout << "DEBUG:: getMatchedInst() returns NULL!\n";
                if(it)
                    trainEquivClass(*inst, it);
            }
        }
            
        if((*inst)->isType(InstType::BRANCH)) {
            auto matchvec = findTheSameInst(*inst, irbb);
            if(matchvec.size() != 1 ){
                if(matchvec.size() > 1)
                    std::cout << "DEBUG:: Matched more than one branch instruction!\n";
                else
                    std::cout << "DEBUG:: No branch instruction is matched!\n";
                continue;
            }
            irinst = matchvec[0]; 
            std::cout << "DEBUG:: Calling function trainControlFlow().....\n";
            trainControlFlow(*inst, irinst);
        }
        
        if((*inst)->getInstType().size() == 0){
            std::cout << "DEBUUG:: trainBB meets instruction " << (*inst)->getOriginInst() 
                        << ", which has no instruction type recorded\n";
            continue;
        }

        if((*inst)->getInstType()[0] == InstType::LOAD) {
            std::vector<MetaInst*> matchvec;
            // TODO:: CHECK If implict LOAD/STORE operations within an instruction can be traversed correctly
            if(ifMatched(*inst)){
                matchvec = getMatchedInst(*inst);
                std::cout << "DEBUG:: In trainBB():: ASM instruction: " << (*inst)->getOriginInst()
                            << " has been mathced to IR instruction: " <<  matchvec[0]->getOriginInst()
                            << std::endl;
            }                 
            else{
                matchvec = findTheSameInst(*inst, irbb);
                if(matchvec.size() != 0 ){
                    std::cout   << "DEBUG:: In trainBB():: ASM LOAD: " << (*inst)->getOriginInst()
                                << " Find a new match in IR: " <<  matchvec[0]->getOriginInst()
                                << std::endl;
                    for(auto i = matchvec.begin();i!=matchvec.end();i++)
                        (*i)->toString();
                    // Build Mapping for new matched load instructions
                    if(matchvec.size()==1)
                        buildMapping(*inst, matchvec[0]);
                }
            }
            // Skip unmatched or ambiguous instructions 
            // Can be optimized to address ambiguity if needed
            if(matchvec.size() != 1 ){
                if(matchvec.size() > 1){
                    std::cout << "DEBUG:: Matched more than one load instruction!\n";
                    for(auto i = matchvec.begin();i!=matchvec.end();i++)
                        std::cout << (*i)->toString() << std::endl;
                }
                else
                    std::cout << "DEBUG:: No load instruction is matched!\n";
                continue;
            }

            irinst = matchvec[0]; 
            std::cout << "DEBUG:: Calling function trainEquivClass().....\n";
            trainEquivClass(*inst, irinst);
            std::cout << "\nDEBUG:: TrainBB Completes the trainEquivClass().....\n";


            // asmvec = (*inst)->getUsers();
            // irvec  = irinst->getUsers();
        
        }

    

        

        

        
        // SKIP STORE INSTRUCTIONS
        // else{
        //     // TODO: RISC-V places val in rs2 for Store instruction
        //     // This may change for ARM or X86. We may need a unified
        //     // load store protocols for ASM TIR of different ISAs
        //     asmvec = (*inst)->getOperandList()[1]->getUsers();
        //     irvec  = irinst->getOperandList()[0]->getUsers();
        // }

        // if(asmvec.size() == 0 || irvec.size()== 0)
        //     continue;   
        
        // for(auto it = asmvec.begin(); it != asmvec.end(); it++){
        //     retvec = (*it)->findMatchedInst(irvec);

        //     // TODO: Ambiguous cases can be addressed if adding further hash check
        //     if(retvec.size() == 1)
        //         for(auto itt = retvec.begin();itt!=retvec.end();itt++)
        //             (*inst)->trainInst(*itt);
        // }

    }

    return cur;
}


// According to <Learning-Phase Algorithm> - Match Load/Store
std::vector<MetaInst *> MetaLearner::findTheSameInst(MetaInst* source, MetaBB *bb) {
    std::cout << "Enter findTheSameInst for asmInst: " << std::hex << source << std::dec << std::endl;
    std::vector<MetaInst *> ans;

    // 1. if only one load/store/branch, directly match
    // Each bb only have 1 branch or jump and it's in the end instruction
    if(source->isType(InstType::BRANCH)) {
        if(bb->inst_last()->isType(InstType::BRANCH)) {
            ans.push_back(bb->inst_last());
            return ans;
        }
    }

    // 2. TODO: Compare with dataRoot: if global->match name;
    if(source->getDataRoot() == "RISCV_GLOBAL") {
    //if(dataRoot != "") {

        std::string asmGlobalName = source->getGlobalSymbolName();
        std::cout << "DEBUG:: globalSymbolName =  " << asmGlobalName << std::endl;
            
        for (auto it = bb->inst_begin(); it != bb->inst_end(); it++) {
            MetaInst *IRinst = *it;
            // std::string rootAns = MetaUtil::findDataRoot(IRinst);
            if(source->hasRelaxedSameType(IRinst) && IRinst->getDataRoot() == "TIR_GLOBAL") {
                std::string rootAns = IRinst->getGlobalSymbolName();
                if(rootAns == asmGlobalName) {
                    std::cout << "findTheSameGlobalVariable " << rootAns << ": " << std::hex << IRinst << std::dec << std::endl;
                    ans.push_back(IRinst);
                    return ans;
                }
            }
        }
    }

    // 3. Compare with hashCode
    if(source->getHashcode() != 0) {
        for (auto it = bb->inst_begin(); it != bb->inst_end(); it++) {
            MetaInst *inst = *it;
            if(inst->getHashcode() == source->getHashcode()) {
                std::cout << "findTheSameHash: " << source->getHashcode() << std::endl;
                ans.push_back(inst);
            }
        }
    }

    // 4. (Optional) Find the instruction has same path: each data compute/addressing/control flow path has the same numLoad, numStore, numPHI, numGEP be seen as the same Path
    for (auto it = bb->inst_begin(); it != bb->inst_end(); it++) {
        MetaInst *inst = *it;
        std::vector<Path *> paths = source->getAllPath();
        std::vector<Path *> anotherPath = inst->getAllPath();

        // std::cout << inst->toString() << std::endl;
        if (inst->isType(source->getInstType()[0])) { // TODO: fix if is the same type, not to use index 0 in case of AMO instr.
            if(source->isType(InstType::BRANCH)) {
                if(paths[2] != nullptr && anotherPath[2] != nullptr) {
                inst->dumpPath(2);
                if (*(paths[2]) == *(anotherPath[2])) { // control flow
                    std::cout << "findTheSamePath" << std::endl;
                    ans.push_back(inst);
                }
                }
            }else {
                if(paths[1] != nullptr && anotherPath[1] != nullptr){
                    inst->dumpPath(1);
                    if(*(paths[1]) == *(anotherPath[1])) { // addressing
                        if(source->isType(InstType::STORE)) {
                            if(paths[0] != nullptr && anotherPath[0] != nullptr){
                                inst->dumpPath(0);
                                if(*(paths[0]) == *(anotherPath[0])) { // data compute
                                    std::cout << "findTheSamePath" << std::endl;
                                    ans.push_back(inst);
                                }
                            }
                        }else {
                            std::cout << "findTheSamePath" << std::endl;
                            ans.push_back(inst);
                        }
                    }
                }
            }
        }
    }
    std::cout << "Exit findTheSameInst" << std::endl;
    if(ans.size() != 0) {
        std::cout << "Ans = " << std::endl;
        for(auto a : ans) {
            std::cout << a->toString() << std::endl;
        }
        std::cout << "End Ans " << std::endl;
    }
    return ans;
}

void MetaLearner::buildEquivClass(MetaInst* inst) {
    auto vec = inst->getOperandList();
    auto bb  = (MetaBB*)(inst->getParentScope());
    equivClassTagMap[inst] = 1;
    // inst->EquivClassTag = 1;
    for(auto i = vec.begin(); i!=vec.end();i++) {
            if((*i)->isMetaInst() && (MetaBB*)(dynamic_cast<MetaInst*>(*i)->getParentScope()) == bb)
                buildEquivClass(dynamic_cast<MetaInst*>(*i)); 
    }
}

void MetaLearner::resetEquivClass(MetaInst* inst) {
    auto vec = inst->getOperandList();
    auto bb  = (MetaBB*)(inst->getParentScope());
    // inst->EquivClassTag = 0;
    equivClassTagMap[inst] = 0;
    for(auto i = vec.begin(); i!=vec.end();i++){
            if((*i)->isMetaInst() && (MetaBB*)(dynamic_cast<MetaInst*>(*i)->getParentScope()) == bb)
                resetEquivClass(dynamic_cast<MetaInst*>(*i));     
    }
}

bool MetaLearner::ifMatched(MetaInst* inst) {
    return matchedMap[inst];
}


void MetaLearner::setTypeSrc(MetaInst* inst, std::vector<std::vector<int>> src) {
    inst->setTypeSrc(src);
}

MetaInst* MetaLearner::fuseInst(std::vector<InstType> vec, MetaInst* inst, std::vector<MetaInst*> &fused) {
    auto        OpVec    = inst->getInstType();
    int         OpCnt    = OpVec.size();
    int         cnt      = vec.size();
    int         range    = abs(cnt-OpCnt);
    MetaInst*   ret      = inst;
    int         mismatch = 0;

    std::cout << "DEBUG:: Entering function fuseInst(), inst = " << inst->getOriginInst() << ", vec.size = " << cnt <<", inst.type.size = " << OpCnt << ".....\n";

    
    //TODO: Currently skip the bidirectional fusion
    if(OpCnt > cnt)
        return NULL;
    
    //int max = cnt >= OpCnt? cnt:OpCnt;

    // Map Function call if only if this is a 1-1 operation Mapping
    // Can be optimized with more constraints and equivclass refinement
    if(vec.size()== 1 && OpVec.size()==1 ){
        if(vec[0] == InstType::CALL || OpVec[0] == InstType::CALL){
            fused.push_back(inst);
            return inst;
        }
    }

    // Since Function Call has been checked
    // Strict Type Matching can be conducted
    for(int i = 0; i < OpCnt; i++){
        if(vec[i]!=OpVec[i])
            return NULL;
    }




    // Comply with the instruction ordering
    fused.push_back(inst);
    std::cout << "DEBUG:: fused.push_back(" << inst->getOriginInst() << "), fused.size = " << fused.size() << ".....\n";

    // Recursive Instruction Fusion
    if(range){
        std::vector<InstType> v(vec.begin()+OpCnt, vec.end());
        if(v.size())
            std::cout << "DEBUG:: fuseInst() now moves to Op type " << MetaUtil::toString(v[0]) << std::endl;
        else
            std::cout << "DEBUG:: fuseInst() No Op in the OpVec!\n " ;

            
        auto childVec = inst->getUsers();
        // Fused Instruction implies intermediate its result (rd) which can ONLY be reused by 1 instruction!
        if(childVec.size() != 1)
            return NULL;
        else
            ret = fuseInst(v,childVec[0],fused);                  
    }

    if(ret)
        return inst;
    else
        return NULL;

}


int MetaLearner::ifFind(MetaInst* inst, std::vector<MetaInst*> vec) {
    if(inst == NULL){
        std::cout << "Warning:: NULL Metainst in ifFind()!\n";
        return -1;
    }
    for(int i = 0; i < vec.size(); i++){
        if(inst == vec[i])
            return i;
    }  
    return -1;
}

int MetaLearner::ifFind(MetaOperand* inst, std::vector<MetaOperand*> vec) {
    if(inst == NULL){
        std::cout << "Warning:: NULL MetaOperand in ifFind()!\n";
        return -1;
    }
    for(int i = 0; i < vec.size(); i++){
        if(inst == vec[i])
            return i;
    }  
    return -1;
}

int MetaLearner::ifFind(MetaOperand* inst, std::vector<MetaInst*> vec) {
    if(inst == NULL){
        std::cout << "Warning:: NULL MetaOperand in ifFind()!\n";
        return -1;
    }else if(!inst->isMetaInst()) {
        std::cout << "Warning:: MetaOperand is NOT MetaInst in ifFind()!\n";
        return -1;
    }
    for(int i = 0; i < vec.size(); i++){
        if(inst == vec[i])
            return i;
    }  
    return -1;
}

void MetaLearner::dumpMapping(std::string mapping) {

    std::cout <<  "\nDEBUG:: Create Instruction Mapping: " << BOLD << GRN << mapping << RST <<std::endl;
    std::cout << std::endl;
}

// used to traverse MT Meta
MetaInst* MetaLearner::updateMappingTable(MetaInst* cur, std::string mapstr, std::string asmInst, std::string irInst,  int index, std::string firstASM) {

    std::cout <<  "\nDEBUG:: Entering updateMappingTable()....\n" <<std::endl;

    //Update MTable of 1-N mapping
    auto it = MapTable->MTable[index].find(MetaUtil::lower(asmInst));
    // std:: cout << "DEBUG::buildMapping() checked the MTable\n";
    // std:: cout << "DEBUG::buildOperandMapping returns string = " << str << std::endl;
    if(it != MapTable->MTable[index].end()){
        std:: cout << "DEBUG::MTable contains the mapping for " << cur->getOriginInst() 
                    << " : "<<it->second << "; current mapstr: " << mapstr << std::endl;
        return cur;
    }

    if(mapstr != "" && it == MapTable->MTable[index].end()){
        std:: cout << "DEBUG::buildMapping() Writing to MTable " << MapTable->getTableName(index) << std::endl;
        MapTable->MTable[index][MetaUtil::lower(asmInst)] = irInst;
        MetaUtil::writeMapping(mapstr, MapTable->getTableName(index));
    }
    

    std::cout <<  "\nDEBUG:: Updating TableMeta....\n" <<std::endl;

    int bitmap = (1<<(index-1));
    if(MapTable->TableMeta.find(firstASM) != MapTable->TableMeta.end()){
        // The bitmap records which mapping table contains such instruction
        // 0001 -> 1-N
        // 0010 -> 2-N
        // 0100 -> 3-N
        // O110 -> 2-N & 3-N
        // 0011 -> 1-N & 2-N
        if (MapTable->TableMeta[firstASM] & bitmap == 0x0)
            MapTable->TableMeta[firstASM] |= (1<<(index-1));
        
    }
    else  // Add a new mapping entry to the TableMeta
        MapTable->TableMeta.insert(std::make_pair(firstASM, bitmap));
    

    std::cout <<  "\nDEBUG:: Leaving updateMappingTable()....\n" <<std::endl;

    return cur;
}


MetaInst& MetaLearner::buildMapping(MetaInst* cur, std::vector<MetaInst*> fused, std::string ASMorIR) {
    std::cout <<"DEBUG:: Enterng function buildMapping(fused,\"" << ASMorIR << "\")....." << std::endl;
    if(!fused.size()){
        std::cout << "\n\nERROR:: In MetaLearner::buildMapping(fused,\"" << ASMorIR << "\"), Empty Fused Instruction Vector!\n\n";
        return *cur;
    }
    std::cout << "DEBUG:: fused vec = [";
    for(auto it = fused.begin(); it != fused.end(); it++) {
        std::cout << (*it)->getOriginInst() << ", ";
    }
    std::cout << "]" << std::endl;

    int fuseID      = fused[0]->getID();
    matchedMap[cur] = true;
    std::string str;
    std::string ret;

    std::cout << "DEBUG:: MatchedInst Connecting... this and fused::" << std::endl;
    std::cout << cur->toString() << std::endl;
    for(auto it = fused.begin(); it!= fused.end(); it++){
        std::cout << (*it)->toString() << std::endl;
        matchedMap[*it] = true;
        fusedMap[*it]  = fuseID;
        pushMatchedInst(cur, *it);
        pushMatchedInst(*it, cur);
        // cur->MatchedInst.push_back((*it));
        // (*it)->MatchedInst.push_back(cur);
        str +=  (*it)->getOriginInst() + " ";
    }

    ret = buildOperandMapping(cur, fused, ASMorIR);
    // 1-N Mapping
    if(ASMorIR == "IR"){
        updateMappingTable(cur, ret, cur->getOriginInst(), str, 1, cur->getOriginInst());
    }
    else{
        int id = fused.size();
        if(id > MapTable->max) {
            std::cout << BOLD << RED << "ERROR:: ASM inst fusion involves " << id << " instructions (More than MapTable->max)\n" << RST;
            std::cout << "DEBUG:: No new mapping recorded & Skip the Mapping Table Updates!\n";
            return *cur;
        }
        // When ASMorIR = ASM, then this points to an IR TIR inst
        updateMappingTable(cur, ret, str, cur->getOriginInst(), id, fused[0]->getOriginInst());
    }


    std::cout <<"DEBUG:: Leaving function buildMapping(fused,\"" << ASMorIR << "\")....." << std::endl;;

    return *cur;
}

MetaInst& MetaLearner::buildMapping(MetaInst* cur, MetaInst* inst) {
    

    std::cout <<"DEBUG:: Enterng function buildMapping(inst)....." << std::endl;;
    std::string str = "";

    // Skip the matched nodes
    // if(this->MatchedInst.size() || inst->MatchedInst.size())
    //     return *this;


    matchedMap[cur]    = true;
    matchedMap[inst]   = true;


    std::cout << "DEBUG:: MatchedInst Connecting... this and inst::" << std::endl;
    std::cout << cur->toString() << std::endl;
    std::cout << inst->toString() << std::endl;

    pushMatchedInst(cur, inst);
    pushMatchedInst(inst, cur);


    std::cout <<"DEBUG:: Calling function buildOperandMapping().....\n";

    if(cur->getOriginInst()!="PHI")
        str = buildOperandMapping(cur, inst);
    else{ 
        std::cout << "DEBUG:: Encounter PHI Node, Skip Operand Mapping\n";
        dumpMapping(cur->getOriginInst() + " : " + inst->getOriginInst());
    }
    

    // //Update MTable of 1-N mapping
    updateMappingTable(cur, str, cur->getOriginInst(), inst->getOriginInst(), 1, cur->getOriginInst());

    // auto it = MapTable->MTable[1].find(this->getOriginInst());
    // // std:: cout << "DEBUG::buildMapping() checked the MTable\n";
    // // std:: cout << "DEBUG::buildOperandMapping returns string = " << str << std::endl;
    // if(it != MapTable->MTable[1].end())
    //     std:: cout << "DEBUG::MTable contains the mapping for " << this->getOriginInst() 
    //                << " : "<<it->second << std::endl;

    // if(str != "" && it == MapTable->MTable[1].end()){
    //     std:: cout << "DEBUG::buildMapping() Writing to MTable " << MapTable->getTableName(1) << std::endl;
    //     MapTable->MTable[1][this->getOriginInst()] = inst->getOriginInst();
    //     MetaUtil::writeMapping(str, MapTable->getTableName(1));
    // }


    std::cout <<"DEBUG:: Leaving function buildMapping(inst)....." << std::endl;;

    return *cur;

}

// Find duplicate operands and build constraint string for mapping rules
std::string MetaLearner::findDuplicateOperands(std::vector<MetaOperand*> vec) {
    std::string ret = " ";
    for(int i = 0; i < vec.size(); i++ )
        for(int j = i+1; j < vec.size(); j++)
            if(vec[i] == vec[j])
                ret += "rs" + std::to_string(i) + "=rs" + std::to_string(j)+ " ";
    return ret;
}

//std::pair<std::string, std::string> 
std::string MetaLearner::buildOperandMapping(MetaInst* cur, std::vector<MetaInst*> Fuse, std::string ASMorIR){

    std::cout <<"DEBUG:: Entering function buildOperandMapping().....\n";

    int                         opNum      = cur->getOperandNum();
    MetaInst*                   match      = NULL;
    int                         find       = -1;
    std::vector<MetaOperand*>   asmOpVec;
    std::vector<MetaOperand*>   irOpVec;
    std::string                 str;
    std::string                 tmp;
    std::vector<MetaInst*>      fused;

    // Mapping dump instruction information
    if(ASMorIR == "IR"){
        // Currently for ASM only!!!
        if(cur->getInstType()[0] == InstType::COMPLEX){
            auto dup = findDuplicateOperands(cur->getOperandList());
            if(dup != " ")
                str     =  cur->getOriginInst() + dup + " : ";
        }
        else
            str         =  cur->getOriginInst() + " : ";

        asmOpVec    =  cur->getOperandList();
    }
    else{
        irOpVec     =  cur->getOperandList();
        for(auto it =  Fuse.begin(); it != Fuse.end(); it++)
            str     += (*it)->getOriginInst() + "  ";
        str         += " : " + cur->getOriginInst() + "  ";
    }

    // Fused Insts are LLVM IR
    // 1 ASM inst <-> N IR Insts
    if(ASMorIR == "IR"){
        fused = Fuse;
    }
    // Fused Insts are ASM
    // N ASM insts <-> 1 IR int
    else
        fused.push_back(cur);

        for(int i = 0; i < fused.size(); i++){
            str         += fused[i]->getOriginInst() + "  ";
            if(fused[i]->getInstType()[0] == InstType::CALL){
                //std::cout << "DEBUG:: build operand mapping for function " << dynamic_cast<MetaCall*>(inst)->getFuncName() << std::endl;
                    str +=  dynamic_cast<MetaCall*>(fused[i])->getFuncName()  + "  ";
            }

            // irOpVec  =  fused[i]->getOperandList();
            auto vec    =  dynamic_cast<MetaInst*>(fused[i])->getOperandList();
            auto tmpvec =  asmOpVec;


            // judge if contains auto match case
            int numOfOp = 0, numOfMatchedOp = 0, emptyElementIndex = -1;
            std::vector<std::vector<MetaOperand*>> AsmMatchVec(vec.size());
            for( int id = 0; id < vec.size(); id++ ){
                if(vec[id]->isMetaConstant()) { 
                    // if metaConst, find metaConst in `asmOpVec` as its matched inst 
                    //TODO: only when ASmorIR=="IR" and only for train control flow
                    std::vector<MetaOperand*> AsmMatchImm;
                    for(auto it = asmOpVec.begin(); it != asmOpVec.end(); it++) {
                        if(cur->isType(InstType::BRANCH) && it == asmOpVec.begin()) continue; // `asmOpVec`[0] is target imm
                        if((*it)->isMetaConstant()) {
                            AsmMatchImm.push_back(*it); 
                            numOfMatchedOp++;
                            break; // TODO: maybe more than 1 imm
                        }
                    }
                    AsmMatchVec[id] = AsmMatchImm;
                }else if(vec[id]->isMetaInst()){
                    numOfOp++;
                    std::vector<MetaInst*> AsmMatch = getMatchedInst(dynamic_cast<MetaInst*>(vec[id]));
                    std::vector<MetaOperand*> AsmMatchOperand;
                    AsmMatchOperand.assign(AsmMatch.begin(), AsmMatch.end()); // dynamic cast
                    if(AsmMatch.size() != 0) numOfMatchedOp++; else emptyElementIndex = id;
                    AsmMatchVec[id] = AsmMatchOperand;
                }
            }
            if(numOfOp == numOfMatchedOp + 1) { // auto match at `emptyElementIndex`
                // find unmatched inst in `tmpvec` and `vec`, auto match them
                // only there is only 1 unmatched both in `tempvec` and `vec` then we auto match them
                int mark = -1;
                for(int idd = 0; idd < tmpvec.size(); idd++){
                    if(tmpvec[idd]->isMetaConstant()) continue;
                    if(getMatchedInst(dynamic_cast<MetaInst*>(tmpvec[idd])).size() == 0) {
                        if(mark != -1) { // the second time found asmOpVec has unmatched inst
                            mark = -1;
                        }else {
                            mark = idd;
                        }
                    }
                }
                if(mark != -1) {
                    std::cout << "DEBUG:: Auto match between " << fused[i]->getOriginInst() << " and " << dynamic_cast<MetaInst*>(tmpvec[mark])->getOriginInst() << std::endl;
                    AsmMatchVec[emptyElementIndex].push_back(tmpvec[mark]);
                    numOfMatchedOp++;
                }
            }
            std::cout << "DEBUG:: Matched op Analysis for " << fused[i]->getOriginInst() << ": numOfOp: " << numOfOp << ", numOfMatchedOp: " << numOfMatchedOp << std::endl;
            // if icmp fcmp: must have 2 operands
            if(fused[i]->getOriginInst().find("icmp") != std::string::npos) {
            // Now: icmp->operand[0] = icmp.rs2, icmp->operand[1] = icmp.rs1; bge/lt/...->operand[0] = target_imm, [1] = rs1, [2] = rs2; need to reverse icmp->operand
                std::reverse(std::begin(tmpvec), std::end(tmpvec));
                if(numOfOp < 2) {
                    std::cout << BOLD << RED << "ERROR:: # of operands of `compare` (" << fused[i]->getOriginInst() << ") = " << numOfOp << " < 2, cannot build mapping!\n" << RST;
                    return "";
                }
            }
            if(numOfOp != numOfMatchedOp) {
                std::cout << BOLD << RED << "ERROR:: numOfOp != numOfMatchedOp (after auto match), cannot build mapping!\n" << RST;
                return "";
            }

            for( int id = 0; id < vec.size(); id++ ){
                if(!vec[id]->isMetaInst() && fused[i]->getOriginInst().find("icmp") != std::string::npos) { // if icmp, allow metaConst
                    continue;
                }
                // Check if RS points to any RD in the fused instructions
                int ret = ifFind((vec[id]),fused);
                if( ret != -1 )
                    str += std::to_string(ret+1)+ "." + "0 ";  // Using 0 represents RD
                else if (ret > i)
                    std::cout << "ERROR:: Incorrect Parent Edge detected in buildOperandMapping()\n";

                // Find matched ASM instruction of such "operand" 
                auto AsmMatch  = AsmMatchVec.at(id);
                // std::cout << "DEBUG:: fused["<<i<<"] " << fused[i]->getOriginInst() <<" .operands["<<id<<"] " << dynamic_cast<MetaInst*>(vec[id])->getOriginInst() << " ->getMatchedInst().size() = " << AsmMatch.size() << std::endl;

                // ASM 1-N Mapping
                // Current logic follows ASM operand ordering and directly apply it to IR's operand sequence
                if(ASMorIR == "IR"){
                    int skip = 0;
                    // Check RS matching between ASM and LLVM IR
                    for(int idd = 0; idd < tmpvec.size(); idd++){
                        if(tmpvec[idd] == NULL) {
                            continue;
                        }else if(tmpvec[idd]->isMetaConstant()) {
                            // Meta Constant should be at the end of the OP list
                            skip++;
                            continue;
                        }
                        find = ifFind ((tmpvec[idd]), AsmMatch);
                        std::cout << "DEBUG:: tmpvec[" << idd << "] is " << tmpvec[idd]->toString() << std::endl;
                        if (find != -1){
                            str += std::to_string(idd+1-skip) + " ";
                            //str += std::to_string(idd+1-skip) + " ";
                            
                            // Handle the Case like add a1, a0, a0, wherein a0 has been used twice
                            // We assume the reg mapping of the same inst will only hit once
                            tmpvec[idd] = NULL;
                            break;
                        }
                    }
                }
                else{
                    // tmpvec.clear();
                    for( int idd = 0; idd < Fuse.size(); idd++ ){
                        asmOpVec =  Fuse[i]->getOperandList();
                        
                        for( int opid = 0; opid < asmOpVec.size(); opid++ ){
                            
                            if(asmOpVec[opid] == NULL || asmOpVec[opid]->isMetaConstant())
                                continue;

                            find  = ifFind(dynamic_cast<MetaInst*>(asmOpVec[opid]), AsmMatch);

                            if ( find != -1 ){
                                // Skip the mapped operands of the same instruction
                                // to address the case of like add a1, a0, a0, 
                                // wherein a0 has been used twice
                                if(ifFind( asmOpVec[opid],tmpvec)){
                                    asmOpVec[opid] = NULL;
                                    continue;
                                }
                                // Instruction Id + "." + Op ID.
                                // E.g. 1.1, 2.1, 2.2, etc.
                                str += std::to_string(idd+1) + "." + std::to_string(opid+1);
                                str += " ";
                                tmpvec.push_back(asmOpVec[find]);
                                break;
                            }
                        }
                        if(find != -1)
                            break;
                    }
                    asmOpVec.clear();
                }       
            }
            str += " ; ";
        }

    dumpMapping(str);
    std::cout <<"DEBUG:: Leaving function buildOperandMapping().....\n";
    return str;

}


//std::pair<std::string, std::string> 
std::string MetaLearner::buildOperandMapping(MetaInst* cur, MetaInst* inst){
    
    std::cout <<"DEBUG:: Entering function buildOperandMapping()..this and inst is...\n";
    std::cout << (cur)->toString() << std::endl;
    std::cout << (inst)->toString() << std::endl;

    int         opNum      = cur->getOperandNum();
    int         opNumIr    = inst->getOperandNum();
    auto        asmOpVec   = cur->getOperandList();
    auto        irOpVec    = inst->getOperandList();
    auto        asmVec     = getMatchedInst(cur);
    auto        irVec      = getMatchedInst(inst);
    std::string str        = cur->getOriginInst();
    MetaInst*   match      = NULL;
    int         find       = 0;
    int         hasConst   = 0;

        if(cur->getInstType()[0] == InstType::COMPLEX){
        auto dup = findDuplicateOperands(cur->getOperandList());
        if(dup != " ")
            str     +=  dup;
    }

    // Mapping dump instruction information
    str += " : ";
    str += inst->getOriginInst() + "  ";
    if(inst->getInstType()[0] == InstType::CALL){
        //std::cout << "DEBUG:: build operand mapping for function " << dynamic_cast<MetaCall*>(inst)->getFuncName() << std::endl;
        str +=  dynamic_cast<MetaCall*>(inst)->getFuncName()  + "  ";
    }


    // <IR OP ID, ASM OP ID>
    std::map<int, int> mapping;
    std::cout << "DEBUG:: buildOperandMapping() ASM->getOperandNum = " << opNum << std::endl;
    std::cout << "DEBUG:: buildOperandMapping() IR->getOperandNum = " << opNumIr << std::endl;
    std::cout << "DEBUG:: buildOperandMapping() ASM->MatchedInst Size = " << asmVec.size() << std::endl;
    std::cout << "\nDEBUG:: buildOperandMapping() IR->MatchedInst Size = " << irVec.size() << std::endl;

    for(auto it = irVec.begin(); it != irVec.end();it++){
        std::cout << (*it)->toString() << std::endl;
    }

    std::cout << std::endl;

    // std::cout << "DEBUG:: buildOperandMapping() ASM->getOperandNum = " << opNum << std::endl;
    // std::cout << "DEBUG:: buildOperandMapping() IR->getOperandNum = " << opNumIr << std::endl;
    // std::cout << "DEBUG:: buildOperandMapping() ASM->MatchedInst Size = " << asmVec.size() << std::endl;

    for(auto it = asmVec.begin(); it != asmVec.end();it++){
        std::cout << (*it)->getOriginInst() << " ";
    }

    std::cout << "\nDEBUG:: buildOperandMapping() IR->MatchedInst Size = " << irVec.size() << std::endl;

    for(auto it = irVec.begin(); it != irVec.end();it++){
        std::cout << (*it)->toString() << std::endl;
    }

    std::cout << std::endl;

    // Clean up ASM MetaConstant if exists
    // for(auto it = asmOpVec.begin(); it < asmOpVec.end(); it++)
    //         if((*it)->isMetaConstant())
    //             asmOpVec.erase(it);
    // opNum = asmOpVec.size();

    opNum = asmOpVec.size();

    // Currently, we don't evict IR constant operands,
    // since IR inst operand can be either reg or imm without limitation.
    // We thus focus more on the ASM side, in which some large imm have to
    // be buffered in reg instead of a 12-bit imm field in ASM

    // for(auto it = irOpVec.begin(); it < irOpVec.end(); it++)
    //         if((*it)->isMetaConstant())
    //             irOpVec.erase(it);            
    // opNumIr = irOpVec.size();

    // if(opNum != opNumIr){
    //     std::cout << BOLD << RED << "ERROR:: Operand Number mismatched between IR & ASM! STOP buildOperandMapping()!\n" << RST;
    //     return "";
    // }
        
    

    if(cur->getInstType().size() == 1 && inst->getInstType().size() == 1)
        // Unordered Operations can directly dump sequence
        if(!ifOrderMatters(cur->getInstType()[0])){    
            std::cout << "DEBUG:: Operand Ordering Unnecessary according to InstType check\n";
            for(int i = 0; i < opNumIr; i++)
                str +=  std::to_string(i+1) + " ";
            dumpMapping(str);
            return str;
        }  


    // 1-1 Mapping
    if(asmVec.size() == 1 && irVec.size() == 1){
        std::cout << "DEBUG::buildOperandMapping() encounters 1-1 Mapping\n"; 
        for(int id = 0; id < asmOpVec.size(); id++){
            // if(asmOpVec[id]->isMetaConstant()){
            //     hasConst = 1;
            //     std::cout << "DEBUG::buildOperandMapping() ASM Operand "<< id+1 << " is MetaConstant\n"; 
            //     //continue;
            // }

            // Filter MetaConstant operand & add IMM operand mapping
            if(asmOpVec[id]->isMetaConstant()){
                std::cout << "DEBUG::buildOperandMapping() ASM Operand "<< id+1 << " is MetaConstant\n"; 
                // Find respective MetaConstant operand in the IR OpVec and match them
                for(auto i = 0; i < irOpVec.size(); i++){
                        if(irOpVec[i]->isMetaConstant()){
                            mapping.insert(std::make_pair(i,id));
                            std::cout << "DEBUG:: buildOperandMapping() builds operand pair <IR.id, ASM.id>: < "
                                        << i   << ", " << id << " >" <<std::endl;
                            break;
                        }
                }
                continue;
            }

            auto vec = getMatchedInst(dynamic_cast<MetaInst*>(asmOpVec[id]));
            std::cout << "DEBUG:: getMatchedInst() " << dynamic_cast<MetaInst*>(asmOpVec[id])->getOriginInst()<<" returns the vector of size " << vec.size() << "\n";
            for(int ir = 0; ir < irOpVec.size(); ir++){
                if(irOpVec[ir]->isMetaConstant()){
                    continue;
                    // TODO:: Need to handle the case of multiple IMM operands for ARM
                    // for(auto i = 0; i < asmOpVec.size(); i++){
                    //     if(asmOpVec[i]->isMetaConstant()){
                    //         mapping.insert(std::make_pair(ir,i));
                    //         std::cout << "DEBUG:: buildOperandMapping() builds operand pair <IR.id, ASM.id>: < "
                    //                   << ir   << ", " << i << " >" <<std::endl;
                    //         break;
                    //     }
                    // }
                    // continue;
                }
                if (ifFind (dynamic_cast<MetaInst*>(irOpVec[ir]), vec) != -1){
                    mapping.insert(std::make_pair(ir,id));
                    std::cout << "DEBUG:: buildOperandMapping() builds operand pair <IR.id, ASM.id>: < "
                    << ir << ", " << id << " >"<<std::endl;
                    find++;
                    break;
                }
            }
        }
    }

    // COMMENTED: We need to get perfect matching to avoid ambiguity
    // Such as the case:
    // ASM:: LOAD A                     IR:  1% = LOAD A
    //       LOAD B                          2% = LOAD B
    //       Inst A B                        3% = Inst1 %2
    //                                       4% = Inst2 %3, %1  
    //
    // In such case, Traversing from LOAD A will trigger the mapping between ASM Inst and IR inst 2
    // But the real case should be ASM Inst mapps to IR Inst1 and IR Inst2
    
    // Handle the case that only one LOAD parent instruction is matched
    // And two RS cannot be naturally matched
    // if(find == 1 && opNum == 2){
    //     int i  = opNum - mapping.begin()->first - 1;
    //     int ii = opNum - mapping.begin()->second - 1 ;
    //     mapping.insert(std::make_pair(i, ii));
    // }

    for(int i = 0; i < opNumIr; i++){
        auto it = mapping.find(i);
        if(it!=mapping.end())
            str += std::to_string(it->second+1) + "  ";

        else if( cur->isStore() ){
            str += std::to_string(i+1) + "  ";
        }

        // If parent inst is the fake instruction linking zero registers
        else if(dynamic_cast<MetaInst*>(cur->getOperand(i))){
            if(dynamic_cast<MetaInst*>(cur->getOperand(i))->isFake()){
                std::cout << "DEBUG:: Enter the case of Fake Inst\n";
                str += std::to_string(i+1) + "  ";
            }
        }
        else{
            std::cout << BOLD << RED << "\nERROR!! Unmapped Operand!! Invalid Mapping of "
                        << cur->getOriginInst() << " : " << inst->getOriginInst() << RST << std::endl;
            return "";
        }
    }

    // IMM should always be the last Operand in ASM
    // Temporal Fix for RV and TBD for ARM
    // Adding imm to the rule 
    // if(hasConst)
    //     str += std::to_string(opNumIr) + " ";

    dumpMapping(str);
    std::cout <<"DEBUG:: Leaving function buildOperandMapping().....\n";

    return str;
}



MetaInst& MetaLearner::trainInst(MetaInst* cur, MetaInst* irinst){
    
    std::cout << "DEBUG:: Entering function trainInst()....." << std::endl;

    int  flag       = 0;
    auto asmOpVec   = cur->getInstType();
    auto irOpVec    = irinst->getInstType();
    int  asmOpCnt   = asmOpVec.size();
    int  irOpCnt    = irOpVec.size();
    // std::vector<MetaInst*> childVec;
    // std::vector<MetaInst*> tmp;
    std::vector<MetaInst*> fused;

    // cur->Trained = true;
    // irinst->Trained = true;
    trainedMap[cur]     = true;
    trainedMap[irinst]  = true;


    //Perfect Match Case: 1-1 or N-N operation mapping
    // CASE: (asmOpCnt  ==  irOpCnt)
    if(cur->hasStrictSameType(irinst)){
        std::cout << "DEBUG:: trainInst() found the Same Type between IR and ASM \n";
        buildMapping(cur, irinst);
    }
    else if (asmOpCnt == irOpCnt){
        std::cout << "DEBUG:: trainInst() Unmatched instructions and skip training \n";
        return *cur;
    }

    // Fuse IR TIR instructions
    else if(asmOpCnt > irOpCnt){
        std::cout << "DEBUG:: asmOpCnt = " << asmOpCnt
                    << ", irOpCnt = " << irOpCnt
                    << "\n        asminst = " << cur->getOriginInst()
                    << ", asm->type[0] = " << cur->getInstType()[0]
                    << "\n        irinst = " << irinst->getOriginInst()
                    << ", ir->type[0] = " << irinst->getInstType()[0]
                    << std::endl;
        if (fuseInst(asmOpVec, irinst, fused)) {
            buildMapping(cur, fused,"IR");
            std::cout << "DEBUG:: fuseInst() return TRUE! fused.size = " << fused.size() << std::endl;
        } else {
            std::cout << "ERROR:: fuseInst() return FALSE! fused.size = " << fused.size() << std::endl;
        }
    }
    // Fuse ASM TIR instructions
    else{
        std::cout << "DEBUG:: asmOpCnt = " << asmOpCnt
                    << ", irOpCnt = " << irOpCnt
                    << "\n        asminst = " << cur->getOriginInst()
                    << ", asm->type[0] = " << cur->getInstType()[0]
                    << "\n        irinst = " << irinst->getOriginInst()
                    << ", ir->type[0] = " << irinst->getInstType()[0]
                    << std::endl;

        if (fuseInst(irOpVec, cur ,fused)) {
            buildMapping(irinst, fused, "ASM");
            std::cout << "DEBUG:: fuseInst() return TRUE! fused.size = " << fused.size() << std::endl;
        } else {
            std::cout << "ERROR:: fuseInst() return FALSE! fused.size = " << fused.size() << std::endl;
        }
    }
    // else if(asmOpCnt > irOpCnt){

    //     childVec = irinst->getUsers();
    //     std::vector<InstType>vec(asmOpVec.begin()+irOpCnt, asmOpVec.end()-1);

    //     for(int i = irOpCnt; i < asmOpCnt; i++){
    //         for(auto it = childVec.begin(); it!=childVec.end();it++)
    //             if(fuseInst( vec, *it, fused ))
    //                 tmp.push_back(fuseInst( vec, *it,fused));
    //     }
        

    // }

    std::cout << "DEBUG:: Leaving function trainInst()....." << std::endl;

    return (*cur);
}

std::vector<MetaInst*> MetaLearner::findMatchedInst(MetaInst* cur, std::vector<MetaInst*> irvec){
    
    auto asmOpVec   = cur->getInstType();
    int  asmOpCnt   = asmOpVec.size();
    int  irOpCnt    = 0;

    std::cout << "DEBUG:: Enter function findMatchedInst(), ASM OpCnt = " << asmOpCnt;
    for (int i = 0; i < asmOpCnt; i++){
        std::cout << ", " << MetaUtil::toString(asmOpVec[i]);
    }
    std::cout << "\n";
    
    std::vector<MetaInst*> tmp;

    // Perfect 1-1 or N-N mapping 
    std::cout << "this: " << MetaUtil::toString(cur->getInstType()) << std::endl;

    for( auto it = irvec.begin(); it != irvec.end(); it++){
        std::cout << "irvec: " << MetaUtil::toString((*it)->getInstType()) << std::endl;
        if(cur->hasStrictSameType(*it)){
            //this->MatchedInst.push_back(*it);
            tmp.push_back(*it);
        }
    }

    // check if compiler instrinsics are involved
    // Can only be detected in the case that IR call
    // is the only operand
    if(irvec.size() == 1 && irvec[0]->getInstType()[0] == InstType::CALL){
        std::cout << "irinst = " << MetaUtil::toString(irvec[0]->getInstType()) << std::endl;
        auto vec = irvec[0]->getOperandList();
        std::string funcName = dynamic_cast<MetaCall*>(irvec[0])->getFuncName();
        std::cout << "DEBUG:: CALL Inst " << funcName << " has "<<  irvec[0]->getOperandNum()<< " Operands:\n";

        buildMapping(cur, irvec[0]);


        //std::cout << "DEBUG:: CALL Inst has "<<  irvec[0]->getOperandNum()<< " Operands:\n";

        // for(auto it = vec.begin(); it!= vec.end(); it++){
        //      std::cout << MetaUtil::toString(*it) <<std::endl;
        // }
    }


    // check if fusion is needed
    else if (tmp.size() == 0 ) {    
        std::cout << "DEBUG:: findMatchedInst Checking if fusion is needed!\n";

        for(auto it = irvec.begin(); it != irvec.end(); it++ ){
            int  bits     =  0;
            auto irOpVec  =  (*it)->getInstType();
            irOpCnt       =  irOpVec.size();

            if(asmOpCnt == irOpCnt)
                continue;
            int min = asmOpCnt > irOpCnt? irOpCnt : asmOpCnt;

            for( int i  = 0; i < min; i++ ) bits |= 1 << asmOpVec[i];
            for( int i  = 0; i < min; i++ ) bits ^= 1 << irOpVec[i];
            if ( bits == 0 )
                tmp.push_back((*it));
                //this->MatchedInst.push_back((*it));
        }
    } 

    std::cout << "DEBUG:: Leaving function findMatchedInst().....\n";

    return tmp;

}

int MetaLearner::getFuseID(MetaInst* cur) {
    return fusedMap[cur];
}


MetaInst* MetaLearner::trainEquivClass(MetaInst* cur, MetaInst* irinst) {
    
    std::vector<MetaInst*> asmvec;
    std::vector<MetaInst*> irvec;
    std::vector<MetaInst*> retvec;

    std::cout << "DEBUG:: Enter function trainEquivClass().....\n";
    //if(inst->getInstType()[0] == InstType::LOAD){
    asmvec = cur->getUsers();
    irvec  = irinst->getUsers();
    // this->getUsers();
    // irinst->getUsers();
    std::cout << "DEBUG:: asmvec.size() = "<< asmvec.size() <<std::endl;
    std::cout << "DEBUG:: irvec.size() = "<< irvec.size() <<std::endl;
    std::cout << "DEBUG:: trainEquivClass:: getUsers() completes \n";

    //}
        // SKIP STORE INSTRUCTIONS
        // else{
        //     // TODO: RISC-V places val in rs2 for Store instruction
        //     // This may change for ARM or X86. We may need a unified
        //     // load store protocols for ASM TIR of different ISAs
        //     asmvec = (*inst)->getOperandList()[1]->getUsers();
        //     irvec  = irinst->getOperandList()[0]->getUsers();
        // }

    if(asmvec.size() == 0 || irvec.size()== 0)
        return NULL;   
    
    //std::cout << "DEBUG:: trainEquivClass:: asmvec size check completes \n";

    for(auto it = asmvec.begin(); it != asmvec.end(); it++){
            std::cout << "DEBUG:: trainEquivClass:: enter loops \n";


        if(!(*it)->getInstType().size()){
            std::cout << "DEBUG:: trainEquivClass:: ASM Inst: " << (*it)->getOriginInst() << " has no OP TYPE!\n";
            continue;
        }
        
        //Skipping trained inst
        if(trainedMap[*it]){
            std::cout << "DEBUG:: trainEquivClass:: ASM Inst: " << (*it)->getOriginInst() << " has been trained!\n";
            continue;
        }

        // Skip Load Store & Branch instruction in EquivClass training
        // TODO:: BE CAUTIOUS OF AMO INSTRUCTIONS that have implicit load, store operations!!!
        if( (*it)->getInstType()[0] == InstType::LOAD || (*it)->getInstType()[0] == InstType::STORE )
            continue;
        if((*it)->getInstType().size() == 2 && ((*it)->getInstType()[0] == InstType::COMPARE && (*it)->getInstType()[1] == InstType::BRANCH))
            continue;

        if( (*it)->getInstType()[0] == InstType::PHI)
            (*it)->setOriginInst("PHI");
        

        retvec = findMatchedInst(*it, irvec);

        std::cout << "findMatchedInst returns the vector size = " << retvec.size() << std::endl;

        // TODO: Ambiguous cases can be addressed if adding further hash check
            if(retvec.size() == 1)
                for(auto itt = retvec.begin();itt!=retvec.end();itt++){
                    //if(!(*it)->Trained && !(*itt)->Trained)
                        trainInst(*it, *itt);
                    // else
                    //     std::cout << "DEBUG::trainEquivClass() found ASM inst "<<  (*it)->getOriginInst()
                    //               << " trained status = " << (*it)->Trained << ",  IR inst "
                    //               << (*itt)->getOriginInst() << " trained status = " << (*itt)->Trained 
                    //               << std::endl;
                }
        }

    return cur;
}

MetaInst* MetaLearner::QualifiedForMatch(MetaInst* cur){
    auto        vec     = cur->getUsers();
    //auto        vec     = this->findMatchedInst();
    int         flag    = 0;

    // Skip the instructions that have been matched
    for(auto it = vec.begin(); it!=vec.end(); it++) {
        if (matchedMap[*it]) {
            flag  =  1; 
            return NULL;
        }           
    }

    return cur;
}

std::vector<MetaInst*> MetaLearner::getMatchedInst(MetaInst* cur) {
    if (matchedInstMap.find(cur) != matchedInstMap.end()) return matchedInstMap[cur];
    std::vector<MetaInst*> ret;
    return ret;
}


MetaInst* MetaLearner::trainControlFlow(MetaInst* cur, MetaInst* irinst){
// Used for train branch <-> icmp + branch
    std::vector<MetaInst*> asmvec;
    std::vector<MetaInst*> irvec;
    std::vector<MetaInst*> retvec;
    std::vector<MetaOperand*> asmOperands = cur->getOperandList();
    std::vector<MetaOperand*> irOperands = irinst->getOperandList();
    for(auto it = asmOperands.begin(); it != asmOperands.end(); it++) if((*it)->isMetaInst()) asmvec.push_back((MetaInst*)(*it));
    for(auto it = irOperands.begin(); it != irOperands.end(); it++) if((*it)->isMetaInst()) irvec.push_back((MetaInst*)(*it));

    std::cout << "DEBUG:: Enter function trainControlFlow().....\n";
    std::cout << "DEBUG:: asmvec.size() = "<< asmvec.size() <<std::endl;
    std::cout << "DEBUG:: irvec.size() = "<< irvec.size() <<std::endl;

    if(asmvec.size() == 0 || irvec.size()== 0)
        return NULL;   
    
    std::vector<MetaInst*> operandsOfIcmp;
    // now irvec = ['icmp'], make operands of icmp to match with operands of branch
    for(auto it2 = irvec[0]->getOperandList().begin(); it2 != irvec[0]->getOperandList().end(); it2++) {
        if((*it2)->isMetaInst()) {
            operandsOfIcmp.push_back((MetaInst*)(*it2));
        }
    }
    std::cout << "DEBUG:: operandsOfIcmp.size() = "<< operandsOfIcmp.size() <<std::endl;

    if(asmvec.size() == 1 && (asmvec[0])->isType(InstType::COMPARE)) {
        // Either asmvec is 'icmp' instruction
        retvec = findMatchedInst(asmvec[0], irvec);
    }
    else {
        // asmvec is arithmetic calculate for branch
        // Train the current 'branch' instruction with irvec

        // Firstly, -- train `asmvec` and `irvec` BEGIN --
        for(auto it = asmvec.begin(); it != asmvec.end(); it++){
            std::cout << "DEBUG:: trainControlFlow:: enter loops, find matchedInst for " << (*it)->getOriginInst() << " \n";
            if(!(*it)->getInstType().size())
                continue;
            //Skipping trained inst
            if(trainedMap[*it]) {
                std::cout << "DEBUG:: This asmvec[i] has been already trained" << std::endl;
                continue;
            }
            if( (*it)->getInstType()[0] == InstType::PHI) // FIXME: whether it need ??
                (*it)->setOriginInst("PHI");

            retvec.clear();
            retvec = findMatchedInst(*it, operandsOfIcmp);
            std::cout << "findMatchedInst(operandsOfIcmp) returns the vector size = " << retvec.size() << std::endl;
            if(retvec.size() == 1) {
                for(int i = 0; i < retvec.size(); i++) {
                    MetaInst* cur = retvec[i];
                    std::cout << "auto itt = retvec.begin(); itt != retvec.end(); itt++" << std::endl;
                    std::cout << cur->toString() << std::endl;
                    if(!trainedMap[*it] && !trainedMap[cur]) {
                        trainInst(*it, cur); // Train `asmInst->op[i]` and its matched inst in `operandsOfIcmp`
                    } else {
                        std::cout << "DEBUG::trainControlFlow() found ASM inst "<<  (*it)->getOriginInst()
                                << " trained status = " << trainedMap[*it] << ",  IR inst "
                                << cur->getOriginInst() << " trained status = " << trainedMap[cur] 
                                << std::endl;
                    }
                }
            }
        }
        // second: build `branch` mapping rule
        retvec = findMatchedInst(cur, irvec);
        for(auto itt = retvec.begin(); itt != retvec.end(); itt++)
            trainInst(cur, *itt);
    }
    std::cout << "DEBUG:: retvec.size() = "<< retvec.size() <<std::endl;

    // if(retvec.size() == 1)
        

    return cur;
}


void MetaLearner::pushMatchedInst(MetaInst* key, MetaInst* value) {
    auto it = matchedInstMap.find(key);
    if (it == matchedInstMap.end()) {
        matchedInstMap[key] = { value };
    }
    else {
        std::vector<MetaInst*> vec = it->second;
        vec.push_back(value);
        matchedInstMap[key] = vec;
    }
}



bool MetaLearner::ifOrderMatters(InstType type) {
    
    switch(type){
        case InstType::ADD:
        case InstType::LOAD:
        case InstType::CALL:
        case InstType::JUMP:
        case InstType::MUL:
        case InstType::AND:
        case InstType::OR:
        case InstType::XOR:
        case InstType::SWAP:
        case InstType::MAX:
        case InstType::MIN:
        case InstType::NEG:
            return false;
        default:
            return true;
    }
    
}

}