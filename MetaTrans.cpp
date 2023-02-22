#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/JSON.h"
#include <math.h>
#include <stdlib.h>
#include "meta/MetaTrans.h"
#include "meta/MetaUtils.h"
#include <utility>

const std::string RED("\033[31m");
const std::string GRN("\033[32m");
const std::string YEL("\033[33m");
const std::string CYN("\033[36m");
const std::string MAG("\033[35m");
const std::string RST("\033[0m");
const std::string BOLD("\033[1m");


namespace MetaTrans { 
    
//===-------------------------------------------------------------------------------===//
/// Function Mata Data implementation.

    FuncMetaData::FuncMetaData() { }

    void FuncMetaData::setFunctionName(std::string name) { funcName = name; }

    void FuncMetaData::setArgAmount(int amt) { argAmount = amt; }

    void FuncMetaData::setOutputType(DataType type) { outputType = type; }
    
    void FuncMetaData::setFunction(MetaFunction* mf) { func = mf; }

    std::string FuncMetaData::getFunctionName() { return funcName; }

    DataType FuncMetaData::getOutputType() { return outputType; }

    MetaFunction* FuncMetaData::getFunction() { return func; }
    
    int FuncMetaData::getArgAmount() { return argAmount; }
    

//===-------------------------------------------------------------------------------===//
/// Instruction Mata Data implementation.

    InstMetaData::InstMetaData() { }

    void InstMetaData::setInst(MetaInst* inst) { this->inst = inst; }

    void InstMetaData::setOperandAmount(int amt) { operandAmount = amt; }

    MetaInst* InstMetaData::getInst() { return inst; }

    int InstMetaData::getOperandAmount() { return operandAmount; }

//===-------------------------------------------------------------------------------===//
/// Meta Operand implementation.

    MetaOperand& MetaOperand::addUser(MetaInst* user) {
        users.push_back(user);
        return *this;
    }

    MetaOperand& MetaOperand::removeUser(MetaInst* user) {
        for (auto it = users.begin(); it != users.end(); ++it) {
            if (*it == user) {
                printf("removed user %d!\n", user);
                users.erase(it);
                break;
            }
        }
        return *this;
    }
    
    std::vector<MetaInst*> MetaOperand::getUsers() {
        return users;
    }

    std::vector<MetaInst*>::iterator MetaOperand::users_begin() {
        return users.begin();
    }

    std::vector<MetaInst*>::iterator MetaOperand::users_end() {
        return users.end();
    }

    MetaOperand& MetaOperand::setID(int id) { this->id = id; return* this; }

    int MetaOperand::getID() { /*assert(id != -1);*/ return id; }

    bool MetaOperand::isMetaConstant() { return false; }

    bool MetaOperand::isMetaArgument() { return false; }

    bool MetaOperand::isMetaInst() { return false; }

    MetaOperand::~MetaOperand() { }

    std::string MetaOperand::toString() { return "\"Operand\""; }

//===-------------------------------------------------------------------------------===//
/// Meta Constant implementation.

    MetaConstant::MetaConstant() : parent(nullptr) { }

    MetaConstant::MetaConstant(MetaFunction* p) : parent(p) { }

    MetaConstant::~MetaConstant() { }

    MetaConstant::MetaConstant(DataType ty) : type(ty) { }

    MetaConstant& MetaConstant::setDataType(DataType ty) { type = ty; }

    DataType MetaConstant::getDataType() { return type; }

    DataUnion MetaConstant::getValue() { return value; }

    MetaFunction* MetaConstant::getParent() { return parent; }

    MetaConstant& MetaConstant::setValue(int8_t v) {
        if (type != DataType::INT) return *this;
        value.int_8_val = v; 
        return *this;
    }

    MetaConstant& MetaConstant::setValue(int16_t v) {
        if (type != DataType::INT) return *this;
        value.int_16_val = v; 
        return *this;
    }
    
    MetaConstant& MetaConstant::setValue(int32_t v) { 
        if (type != DataType::INT) return *this;
        value.int_32_val = v; 
        return *this;
    }

    MetaConstant& MetaConstant::setValue(int64_t v) { 
        if (type != DataType::INT) return *this;
        value.int_64_val = v; 
        return *this;
    }

    MetaConstant& MetaConstant::setValue(float v) {
        if (type != DataType::FLOAT) return *this;
        value.float_val = v; 
        return *this;
    }
    
    MetaConstant& MetaConstant::setValue(double v) { 
        if (type != DataType::FLOAT) return *this;
        value.double_val = v; 
        return *this;
    }

    MetaConstant& MetaConstant::setParent(MetaFunction* p) { 
        parent = p;
        return *this;
    }

    bool MetaConstant::isMetaConstant() { return true; }

    std::string MetaConstant::toString() {
        std::string str = "";
        return str + "{" 
            + "\"id\":" + std::to_string(id) +
            "}";
    }

//===-------------------------------------------------------------------------------===//
/// Meta Argument implementation.

    MetaArgument::MetaArgument() { }

    MetaArgument::MetaArgument(MetaFunction* p) : parent(p) { }

    MetaArgument::~MetaArgument() { }

    MetaArgument::MetaArgument(DataType ty) : type(ty) { }

    MetaArgument& MetaArgument::setArgIndex(int i) {
        argIndex = i;
        return *this;
    }

    MetaArgument& MetaArgument::setOffset(int o) {
        offset = o;
        return *this;
    }
    
    MetaArgument& MetaArgument::setArgType(DataType ty) {
        type = ty;
        return *this;
    }

    MetaArgument& MetaArgument::setWidth(int w) {
        width = w;
        return *this;
    }

    MetaArgument& MetaArgument::setParent(MetaFunction* f) {
        parent = f;
        return *this;
    }

    int MetaArgument::getArgIndex() { return argIndex; }

    int MetaArgument::getOffest() { return offset; }

    int MetaArgument::getWidth() { return width; }

    DataType MetaArgument::getArgType() { return type; }

    MetaFunction* MetaArgument::getParent() { return parent; }

    bool MetaArgument::isMetaArgument() { return true; }

    std::string MetaArgument::toString() {
        std::string str = "";
        return str + "{" 
            + "\"id\":" + std::to_string(id) +
            "}";
    }

//===-------------------------------------------------------------------------------===//
/// Meta Instruction implementation.

    MetaInst::MetaInst() { 
        paths.resize(3);
    }

    MetaInst::MetaInst(std::vector<InstType> ty) : type(ty) {
        paths.resize(3);
    }

    MetaInst::~MetaInst() { }

    MetaInst& MetaInst::setOriginInst(std::string name) {
        originInst = name;
        return *this;
    }

    MetaInst& MetaInst::setInstType(std::vector<InstType> ty) {
        type = ty;
        return *this;
    }

    MetaInst& MetaInst::setInstType(InstType ty) {
        std::vector<InstType> tmp(1, ty);
        type = tmp;
        return *this;
    }

    MetaInst& MetaInst::addInstType(InstType ty) {
        type.push_back(ty);
        return *this;
    }

    MetaInst& MetaInst::setParent(MetaBB* bb) {
        parent = bb;
        return *this;
    }

    MetaInst& MetaInst::addOperand(MetaOperand* op) {
        operandList.push_back(op);
        op->addUser(this);
        return *this;
    }

    MetaInst& MetaInst::buildFromJSON(llvm::json::Object JSON, std::unordered_map<int64_t, MetaBB*>& tempBBMap, std::unordered_map<int64_t, MetaOperand*>& tempOperandMap) {

        std::string     originInst  = JSON["originInst"].getAsString().getValue().str();
        std::string     dataRoot    = JSON["dataRoot"]  .getAsString().getValue().str();
        unsigned long   hashCode    = JSON.getInteger("hashCode").getValue();
        int64_t         id          = JSON.getInteger("id")      .getValue();

        (*this)
            .setOriginInst (originInst)
            .setHashcode   (hashCode)
            .setDataRoot   (dataRoot)
            .setID         (id)
            ;

        json::Array& ops = *(JSON["type"].getAsArray());
        for (int i = 0; i < ops.size(); ++i) {
            std::string op_str = ops[i].getAsString().getValue().str();
            addInstType(MetaUtil::stringToInstType(op_str));
        }

        this->paths.resize(3);
        json::Array* paths = JSON.getArray("path");
        if(paths) {
            for(int i = 0; i < (*paths).size(); i++) {
                json::Array& path = *((*paths)[i].getAsArray());
                this->paths[path[0].getAsInteger().getValue()] = new Path{nullptr, path[0].getAsInteger().getValue(), path[1].getAsInteger().getValue(),  path[2].getAsInteger().getValue(), path[3].getAsInteger().getValue(), path[4].getAsInteger().getValue()};
                // printf("path.size() = %d\n", path.size());
                // std::cout << "type: " << path[0].getAsInteger().getValue() << " numLoad: " <<  path[1].getAsInteger().getValue() << " numStore: " <<  path[2].getAsInteger().getValue()<< " numPHI: " <<  path[3].getAsInteger().getValue() << " numGEP: " <<  path[4].getAsInteger().getValue() << "\n";
            }
        }

        return *this;
    }

    std::string MetaInst::getOriginInst() {
        return originInst;
    }

    bool MetaInst::isLoad() { for (auto ty : type) if (ty == InstType::LOAD) return true; return false; }

    bool MetaInst::isStore() { for (auto ty : type) if (ty == InstType::STORE) return true; return false; }

    int MetaInst::getOperandNum() { return operandList.size(); }

    MetaBB* MetaInst::getParent() { return parent; }

    std::vector<InstType> MetaInst::getInstType() { return type; }

    std::vector<MetaOperand*>& MetaInst::getOperandList() { return operandList; }

    std::vector<MetaOperand*>::iterator MetaInst::op_begin() { return operandList.begin(); }

    std::vector<MetaOperand*>::iterator MetaInst::op_end() { return operandList.end(); }

    bool MetaInst::hasSameType(MetaInst* i) {
        int64_t ty = 0;
        for (InstType t : type) ty |= (1 << t);
        for (InstType t : i->getInstType()) ty ^= (1 << t);
        return ty == 0;
    }

    bool MetaInst::isType(InstType ty) {
        for (InstType t : type) if (t == ty) return true;
        return false;
    }

    bool MetaInst::isSingleType(InstType ty) {
        return type.size() == 1 && type[0] == ty;
    }

    bool MetaInst::isMetaInst() { return true; }

    bool MetaInst::isMetaPhi() { return false; }

    std::string MetaInst::toString() {
        std::string opList = operandList.size() == 0 ? "[]" : "[";
        for (MetaOperand* oprand : operandList) { opList = opList + std::to_string(oprand->getID()) + ","; }
        std::string userList = users.size() == 0 ? "[]" : "[";
        for (MetaOperand* user : users) { userList = userList + std::to_string(user->getID()) + ","; }
        std::string path = "[";
        for (int i = 0; i < 3; i++) {
            if(type[0] == InstType::LOAD && i == 1) {
                if(paths.size() > i) {
                    Path *p = paths[i];
                    if(p) path += "[" + std::to_string(p->type) + "," +std::to_string(p->numLoad) + "," +std::to_string(p->numStore) + "," +std::to_string(p->numPHI) + "," +std::to_string(p->numGEP) + "]";
                }
                break;
            } else if (type[0] == InstType::STORE && (i == 0 || i == 1)) {
                if(paths.size() > i) {
                    if (i == 1 && paths[0] && paths[i]) path += ",";
                    Path *p = paths[i];
                    if(p) path += "[" + std::to_string(p->type) + "," +std::to_string(p->numLoad) + "," +std::to_string(p->numStore) + "," +std::to_string(p->numPHI) + "," +std::to_string(p->numGEP) + "]";
                }
            } else if (type[0] == InstType::BRANCH && (i == 2)) {
                if(paths.size() > i) {
                    Path *p = paths[i];
                    if(p) path += "[" + std::to_string(p->type) + "," +std::to_string(p->numLoad) + "," +std::to_string(p->numStore) + "," +std::to_string(p->numPHI) + "," +std::to_string(p->numGEP) + "]";
                }
                break;
            }
        }
        path += "]";
        opList[opList.length() - 1] = ']';
        userList[userList.length() - 1] = ']';
        std::string str = "";
        str = str + "{" + "\"id\":" + std::to_string(id) + ",\"originInst\":" + "\"" +
            originInst + "\"" +
            ",\"isMetaPhi\":false,\"type\":" + MetaUtil::toString(type) + "," +
            "\"operandList\":" + opList + "," + "\"userList\":" + userList + "," + "\"path\":" + path + ",\"hashCode\":" + std::to_string(hashCode) + ",\"dataRoot\":\"" + dataRoot + "\"}";
        return str;
    }

    void MetaInst::addColor(int c, int t) { colors.insert(ColorData(c,t)); }

    std::set<ColorData>& MetaInst::getColors() { return colors; }

    bool MetaInst::hasColor(int c) {
        for(auto & each : colors) {
            if (each.color == c) return true;
        }
        return false;
    }

    unsigned long MetaInst::getHashcode() {return hashCode;}

    MetaInst& MetaInst::setHashcode(unsigned long _hashCode) {
        hashCode = _hashCode;
        return *this;
    }

    std::string MetaInst::getDataRoot() {return dataRoot;}

    MetaInst& MetaInst::setDataRoot(std::string s) { dataRoot = s; return *this; }

    std::vector<Path *>& MetaInst::getAllPath() { return paths; }

    Path* MetaInst::getPath(int type) { return paths[type]; }

    void MetaInst::dumpPath(int index) {
        std::cout << "type: , numLoad: , numStore: , numPHI: , numGEP: " <<
            paths[index]->type << paths[index]->numLoad <<
            paths[index]->numStore << paths[index]->numPHI << paths[index]->numGEP << std::endl;
    }

    MetaInst& MetaInst::addToPath(Path* p) {
        paths[p->type] = p;
        return *this;
    }

    std::vector<MetaInst *> MetaInst::findTheSameInst(MetaBB *bb) {
        std::cout << "Enter findTheSameInst" << std::endl;
        std::vector<MetaInst *> ans;
        // Compare with hashCode
        if(hashCode != 0) {
            for (auto it = bb->inst_begin(); it != bb->inst_end(); it++) {
                MetaInst *inst = *it;
                if(inst->getHashcode() == hashCode) {
                    std::cout << "findTheSameHash: " << hashCode << std::endl;
                    ans.push_back(inst);
                }
            }
        }
        // Find the instruction has same path: each /data compute/addressing/control flow/ path has the same numLoad, numStore, numPHI, numGEP
        for (auto it = bb->inst_begin(); it != bb->inst_end(); it++) {
            MetaInst *inst = *it;
            std::vector<Path *> anotherPath = inst->getAllPath();
            // std::cout << inst->toString() << std::endl;
            if (inst->isType(type[0])) {
                if(type[0] == InstType::BRANCH) {
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
                            if(type[0] == InstType::STORE) {
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

    void MetaInst::buildEquivClass(){
        auto vec = this->getOperandList();
        auto bb  = this->getParent();
        this->EquivClassTag = 1;
        for(auto i = vec.begin(); i!=vec.end();i++){
               if((*i)->isMetaInst() && dynamic_cast<MetaInst*>(*i)->getParent() == bb)
                    (dynamic_cast<MetaInst*>(*i))->buildEquivClass(); 
        }
    }

    void MetaInst::resetEquivClass(){
        auto vec = this->getOperandList();
        auto bb  = this->getParent();
        this->EquivClassTag = 0;
        for(auto i = vec.begin(); i!=vec.end();i++){
               if((*i)->isMetaInst() && dynamic_cast<MetaInst*>(*i)->getParent() == bb)
                    (dynamic_cast<MetaInst*>(*i))->resetEquivClass();     
        }
    }

    bool MetaInst::ifMatched(){
        return this->Matched;
    }

    
    MetaInst& MetaInst::setTypeSrc(std::vector<std::vector<int>> src){
        typeSrc = src;
        return (*this);
    }

    MetaInst* fuseInst(std::vector<InstType> vec, MetaInst* inst, std::vector<MetaInst*> fused){
        auto        OpVec    = inst->getInstType();
        int         OpCnt    = OpVec.size();
        int         cnt      = vec.size();
        int         range    = abs(cnt-OpCnt);
        MetaInst*   ret      = inst;

        std::cout << "DEBUG:: Entering function fuseInst().....\n";

        
        //TODO: Currently skip the bidirectional fusion
        if(OpCnt > cnt)
            return NULL;
        
        //int max = cnt >= OpCnt? cnt:OpCnt;

        for(int i = 0; i < OpCnt; i++){
            if(vec[i]!=OpVec[i])
                return NULL;
        }
        
        // Comply with the instruction ordering
        fused.push_back(inst);

        // Recursive Instruction Fusion
        if(range){
            std::vector<InstType> v(vec.begin()+OpCnt, vec.end()-1);
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

    int ifFind(MetaInst* inst, std::vector<MetaInst*> vec){
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

    int ifFind(MetaOperand* inst, std::vector<MetaOperand*> vec){
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

    void dumpMapping(std::string mapping){

        std::cout <<  "\nCreate Instruction Mapping:\n" << BOLD << GRN << mapping << RST <<std::endl;
        std::cout << std::endl;
    }

    MetaInst& MetaInst::buildMapping(std::vector<MetaInst*> fused, std::string ASMorIR){
        std::cout <<"DEBUG:: Enterng function buildMapping().....\n";
        if(!fused.size()){
            std::cout << "\n\nERROR:: In MetaInst::buildMapping(), Empty Fused Instruction Vector!\n\n";
            return *this;
        }

        int fuseID      = fused[0]->getID();
        this->Matched   = true;
        for(auto it = fused.begin(); it!= fused.end(); it++){
            (*it)->Matched = true;
            (*it)->FuseID  = fuseID;
            this->MatchedInst.push_back((*it));
            (*it)->MatchedInst.push_back(this);
        }
        std::cout <<"DEBUG:: Leaving function buildMapping().....\n";

        return *this;
    }

    MetaInst& MetaInst::buildMapping(MetaInst* inst){
        

        std::cout <<"DEBUG:: Enterng function buildMapping().....\n";

        this->Matched   = true;
        inst->Matched   = true;

        this->MatchedInst.push_back(inst);
        inst->MatchedInst.push_back(this);
    
        std::cout <<"DEBUG:: Calling function buildOperandMapping().....\n";

        if(this->getOriginInst()!="PHI")
            this->buildOperandMapping(inst);
        else{ 
            std::cout << "DEBUG:: Encounter PHI Node, Skip Operand Mapping\n";
            dumpMapping(this->getOriginInst() + " : " + inst->getOriginInst());
        }
            
       

        std::cout <<"DEBUG:: Leaving function buildMapping().....\n";

        return *this;

    }


    //std::pair<std::string, std::string> 
    MetaInst& MetaInst::buildOperandMapping(std::vector<MetaInst*> Fuse, std::string ASMorIR){

        std::cout <<"DEBUG:: Entering function buildOperandMapping().....\n";

        int                         opNum      = this->getOperandNum();
        MetaInst*                   match      = NULL;
        int                         find       = -1;
        std::vector<MetaOperand*>   asmOpVec;
        std::vector<MetaOperand*>   irOpVec;
        std::string                 str;
        std::string                 tmp;
        std::vector<MetaInst*>      fused;

        // Mapping dump instruction information
        if(ASMorIR == "IR"){
            str         =  this->getOriginInst() + " : ";
            asmOpVec    =  this->getOperandList();
        }
        else{
            irOpVec     =  this->getOperandList();
            for(auto it =  Fuse.begin(); it != Fuse.end(); it++)
                str     += (*it)->getOriginInst() + "  ";
            str         += " : " + this->getOriginInst() + "  ";
        }

        // Fused Insts are LLVM IR
        // 1 ASM inst <-> N IR Insts
        if(ASMorIR == "IR"){
            fused = Fuse;
        }
        // Fused Insts are ASM
        // N ASM insts <-> 1 IR int
        else
            fused.push_back(this);

            for(int i = 0; i < fused.size(); i++){
                str         += fused[i]->getOriginInst() + "  ";
                // irOpVec  =  fused[i]->getOperandList();
                auto vec    =  dynamic_cast<MetaInst*>(fused[i])->getOperandList();
                auto tmpvec =  asmOpVec;

                for( int id = 0; id < vec.size(); id++ ){
                    // Check if RS points to any RD in the fused instructions
                    int ret = ifFind(dynamic_cast<MetaInst*>(vec[id]),fused);
                    if( ret != -1 )
                        str += std::to_string(ret+1)+ "." + "rd  ";
                    else if (ret > i)
                        std::cout << "ERROR:: Incorrect Parent Edge detected in buildOperandMapping()\n";

                    // Find matched ASM instruction of such "operand" 
                    auto AsmMatch  = dynamic_cast<MetaInst*>(vec[i])->getMatchedInst();

                    // Check RS matching between ASM and LLVM IR
                    if(ASMorIR == "IR"){
                        
                        for(int idd = 0; idd < tmpvec.size(); idd++){

                            find = ifFind (dynamic_cast<MetaInst*>(tmpvec[idd]), AsmMatch);
                     
                            if (find !=-1){
                                str += std::to_string(idd+1) + " ";
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

                                find  = ifFind(dynamic_cast<MetaInst*>(asmOpVec[opid]), AsmMatch);

                                if ( find != -1 ){
                                    // Skip the mapped operands of the same instruction
                                    // to address the case of like add a1, a0, a0, 
                                    // wherein a0 has been used twice
                                    if(ifFind( asmOpVec[opid],tmpvec)){
                                        asmOpVec[opid]  = NULL;
                                        continue;
                                    }
                                    // Instruction Id + "." + Op ID.
                                    // E.g. 1.1, 2.1, 2.2, etc.
                                    str += std::to_string(idd+1) + "." + std::to_string(opid+1);
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
            }

        dumpMapping(str);
        std::cout <<"DEBUG:: Leaving function buildOperandMapping().....\n";

    }


    //std::pair<std::string, std::string> 
    MetaInst& MetaInst::buildOperandMapping(MetaInst* inst){
        
        std::cout <<"DEBUG:: Entering function buildOperandMapping().....\n";

        int         opNum      = this->getOperandNum();
        auto        asmOpVec   = this->getOperandList();
        auto        irOpVec    = inst->getOperandList();
        auto        asmVec     = this->getMatchedInst();
        auto        irVec      = inst->getMatchedInst();
        std::string str        = this->getOriginInst();
        MetaInst*   match      = NULL;
        int         find       = 0;

        // Mapping dump instruction information
        str += " : ";
        str += inst->getOriginInst() + "  ";

        // <ASM OP ID, IR OP ID>
        std::map<int, int> mapping;

        // 1-1 Mapping
        if(asmVec.size() == 1 && irVec.size() == 1){
            for(int id = 0; id < asmOpVec.size(); id++){
                auto vec = dynamic_cast<MetaInst*>(asmOpVec[id])->getMatchedInst();
                for(int ir = 0; ir < irOpVec.size(); ir++){
                    if (ifFind (dynamic_cast<MetaInst*>(irOpVec[ir]), vec) != -1){
                        mapping.insert(std::make_pair(id, ir));
                        find++;
                        break;
                    }
                }
            }
        }

        // Handle the case that only one LOAD parent instruction is matched
        // And two RS cannot be naturally matched
        if(find == 1 && opNum == 2){
            int i  = opNum - mapping.begin()->first - 1;
            int ii = opNum - mapping.begin()->second - 1 ;
            mapping.insert(std::make_pair(i, ii));
        }

        for(int i = 0; i < opNum; i++){
            auto it = mapping.find(i);
            if(it!=mapping.end())
                str += std::to_string(it->second+1) + "  ";
            else{
                std::cout << BOLD << RED << "\nERROR!! Unmapped Operand!! Invalid Mapping of "
                          << this->getOriginInst() << " : " << inst->getOriginInst() << RST << std::endl;
                break;
            }
        }

        dumpMapping(str);
        std::cout <<"DEBUG:: Leaving function buildOperandMapping().....\n";

    }



    MetaInst& MetaInst::trainInst(MetaInst* irinst){
        
        std::cout << "DEBUG:: Entering function trainInst().....\n";

        int  flag       = 0;
        auto asmOpVec   = this->getInstType();
        auto irOpVec    = irinst->getInstType();
        int  asmOpCnt   = asmOpVec.size();
        int  irOpCnt    = irOpVec.size();
        // std::vector<MetaInst*> childVec;
        // std::vector<MetaInst*> tmp;
        std::vector<MetaInst*> fused;



        //Perfect Match Case: 1-1 or N-N operation mapping
        // CASE: (asmOpCnt  ==  irOpCnt)
        if(this->hasSameType(irinst)){
            std::cout << "DEBUG:: trainInst() found the Same Type between IR and ASM \n";
            this->buildMapping(irinst);
        }

        // Fuse IR TIR instructions
        else if(asmOpCnt > irOpCnt){
            std::cout << "DEBUG:: asmOpCnt = " << asmOpCnt
                      << ", irOpCnt = " << irOpCnt
                      << "\n        asminst = " << this->getOriginInst()
                      << ", asm->type[0] = " << this->getInstType()[0]
                      << "\n        irinst = " << irinst->getOriginInst()
                      << ", ir->type[0] = " << irinst->getInstType()[0]
                      << std::endl;

            if(fuseInst(asmOpVec, irinst, fused))
                this->buildMapping(fused,"IR");
            std::cout << "DEBUG:: Leaving function fuseInst().....\n";

        }
        // Fuse ASM TIR instructions
        else{
            std::cout << "DEBUG:: asmOpCnt = " << asmOpCnt
                      << ", irOpCnt = " << irOpCnt
                      << "\n        asminst = " << this->getOriginInst()
                      << ", asm->type[0] = " << this->getInstType()[0]
                      << "\n        irinst = " << irinst->getOriginInst()
                      << ", ir->type[0] = " << irinst->getInstType()[0]
                      << std::endl;

            if(fuseInst(irOpVec, this ,fused))
                irinst->buildMapping(fused, "ASM");
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

        std::cout << "DEBUG:: Leaving function trainInst().....\n";

        return (*this);
    }

    std::vector<MetaInst*> MetaInst::findMatchedInst(std::vector<MetaInst*> irvec){
        
        std::cout << "DEBUG:: Enter function findMatchedInst().....\n";

        auto asmOpVec   = this->getInstType();
        int  asmOpCnt   = asmOpVec.size();
        int  irOpCnt    = 0;
        
        std::vector<MetaInst*> tmp;

        // Perfect 1-1 or N-N mapping 
        for( auto it = irvec.begin(); it != irvec.end(); it++){
            if(this->hasSameType(*it)){
                //this->MatchedInst.push_back(*it);
                tmp.push_back(*it);
            }
        }

        // check if fusion is needed
        if( this->MatchedInst.size() == 0 ){        
            for(auto it = irvec.begin(); it != irvec.end(); it++ ){
                int  bits     =  0;
                auto irOpVec  =  (*it)->getInstType();
                irOpCnt       =  irOpVec.size();

                if(asmOpCnt == irOpCnt)
                    continue;
                int max = asmOpCnt > irOpCnt? asmOpCnt:irOpCnt;

                for( int i  = 0; i < max; i++ ) bits |= 1 << asmOpVec[i];
                for( int i  = 0; i < max; i++ ) bits ^= 1 << irOpVec[i];
                if ( bits == 0 )
                    tmp.push_back((*it));
                    //this->MatchedInst.push_back((*it));
            }
        } 

        std::cout << "DEBUG:: Leaving function findMatchedInst().....\n";

        return tmp;

    }

    int MetaInst::getFuseID(){return this->FuseID;}


    MetaInst* MetaInst::trainEquivClass(MetaInst* irinst){
        
        std::vector<MetaInst*> asmvec;
        std::vector<MetaInst*> irvec;
        std::vector<MetaInst*> retvec;

        std::cout << "DEBUG:: Enter function trainEquivClass().....\n";
        //if(inst->getInstType()[0] == InstType::LOAD){
        asmvec = this->getUsers();
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
            
            if(!(*it)->getInstType().size())
                continue;

            // Skip Load Store & Branch instruction in EquivClass training
            // TODO:: BE CAUTIOUS OF AMO INSTRUCTIONS that have implicit load, store operations!!!
            if( (*it)->getInstType()[0] == InstType::LOAD || (*it)->getInstType()[0] == InstType::STORE || 
                (*it)->getInstType()[0] == InstType::BRANCH )
                continue;

            if( (*it)->getInstType()[0] == InstType::PHI)
                (*it)->setOriginInst("PHI");
            

            retvec = (*it)->findMatchedInst(irvec);

            std::cout << "findMatchedInst returns the vector size = " << retvec.size() << std::endl;

            // TODO: Ambiguous cases can be addressed if adding further hash check
                if(retvec.size() == 1)
                    for(auto itt = retvec.begin();itt!=retvec.end();itt++)
                            (*it)->trainInst(*itt);
            }

        return this;
    }

    MetaInst* MetaInst::QualifiedForMatch(){
        auto        vec     = this->getUsers();
        //auto        vec     = this->findMatchedInst();
        int         flag    = 0;

        // Skip the instructions that have been matched
        for(auto it = vec.begin(); it!=vec.end();it++){
            if((*it)->Matched){
                flag  =  1; 
                return NULL;
            }           
        }

        return this;

    }

    std::vector<MetaInst*> MetaInst::getMatchedInst(){return this->MatchedInst;}

    MetaInst* MetaInst::trainControlFlow(){
        
    }


//===-------------------------------------------------------------------------------===//
/// Meta Phi Instruction implementation.

    MetaPhi::MetaPhi() {
        type.push_back(InstType::PHI);
        MetaInst(type);
    }
    MetaPhi::MetaPhi(std::vector<InstType> ty) : MetaInst(ty) { }

    MetaPhi& MetaPhi::addValue(MetaBB* bb, MetaOperand* op) {
        bbValueMap.insert({bb, op});
        return *this;
    }

    bool MetaPhi::equals(MetaPhi* phi) {
        if (bbValueMap.size() != phi->getMapSize()) return false;
        for (auto it = bbValueMap.begin(); it != bbValueMap.end(); ++it) {
            MetaBB* key = it->first;
            if (this->getValue(key) != phi->getValue(key)) return false;
        }
        return true;
    }

    int MetaPhi::getMapSize() {
        return bbValueMap.size();
    }

    MetaOperand* MetaPhi::getValue(MetaBB* bb) {
        auto pair = bbValueMap.find(bb);
        if (pair == bbValueMap.end()) return nullptr;
        return pair->second;
    }

    std::unordered_map<MetaBB*, MetaOperand*>::iterator MetaPhi::begin() {
        return bbValueMap.begin();
    }

    std::unordered_map<MetaBB*, MetaOperand*>::iterator MetaPhi::end() {
        return bbValueMap.end();
    }

    bool MetaPhi::isMetaPhi() { return true; }

    std::string MetaPhi::toString() {

        std::string opList = operandList.size() == 0 ? "[]" : "[";
        for (MetaOperand* oprand : operandList) { opList = opList + std::to_string(oprand->getID()) + ","; }
        opList[opList.length() - 1] = ']';
        
        std::string phiMapStr = bbValueMap.size() == 0 ? "{}" : "{";
        for (auto pair = bbValueMap.begin(); pair != bbValueMap.end(); ++pair) {
            phiMapStr = phiMapStr + "\"" + std::to_string(pair->first->getID()) + "\":" + std::to_string(pair->second->getID()) + ",";
        }
        phiMapStr[phiMapStr.length() - 1] = '}'; 

        std::string str = "";
        return str + 
            "{" + "\"id\":" + std::to_string(id) + 
            ",\"isMetaPhi\":true,\"type\":" + MetaUtil::toString(type) + 
            ",\"operandList\":" + opList + 
            ",\"bbValueMap\":" + phiMapStr +
            "}";
    }

    MetaInst& MetaPhi::buildFromJSON(llvm::json::Object JSON, std::unordered_map<int64_t, MetaBB*>& tempBBMap, std::unordered_map<int64_t, MetaOperand*>& tempOperandMap) {
        json::Object kv = *(JSON["bbValueMap"].getAsObject());
        for (auto iter = kv.begin(); iter != kv.end(); ++iter) {
            int key = std::stoi(iter->first.str());
            int val = iter->second.getAsInteger().getValue();
            bbValueMap[tempBBMap[key]] = tempOperandMap[val];
        }
        return *this;
    }

    bool MetaPhi::isLoad() { return false; }

    bool MetaPhi::isStore() { return false; }


//===-------------------------------------------------------------------------------===//
/// Meta Basic Block implementation.

    
    MetaBB::MetaBB() : entry(nullptr), terminator(nullptr), parent(nullptr) { }

    MetaBB::MetaBB(MetaFunction* f) : entry(nullptr), terminator(nullptr), parent(f) { }

    MetaBB::MetaBB(std::string JSON) {

    }

    MetaBB::MetaBB(MetaFunction* parent, std::string JSON) : parent(parent) {

    }

    MetaBB::~MetaBB() {
        for (auto inst : instList) {
            delete inst;
        }
    }

    MetaInst* MetaBB::buildInstruction(std::vector<InstType> ty) {
        MetaInst* newInst = nullptr;
        if (ty[0] == InstType::PHI)
            newInst = new MetaPhi(ty);
        else 
            newInst =  new MetaInst(ty);
        instList.push_back(newInst);
        return newInst;
    }

    MetaInst* MetaBB::buildInstruction() {
        MetaInst* newInst = new MetaInst();
        instList.push_back(newInst);
        return newInst;
    }

    MetaPhi* MetaBB::buildPhi(bool insertAtHead) {
        MetaPhi* newPhi = new MetaPhi();
        addPhi(newPhi, insertAtHead);
        return newPhi;
    }

    MetaBB& MetaBB::addPhi(MetaPhi* phi, bool insertAtHead) {
        auto it = instList.begin();
        if(!insertAtHead) {
            while ((*it)->isMetaPhi()) ++it;
        }
        instList.insert(it, (MetaInst*)phi);
        return *this;
    }

    MetaBB& MetaBB::addInstruction(MetaInst* inst) {
        instList.push_back(inst);
        return *this;
    }

    MetaBB& MetaBB::addNextBB(MetaBB* next) {
        successors.push_back(next);
        return *this;
    }

    MetaBB& MetaBB::addFeature(int f) {
        features.push_back(f);
        modular = sqrt((modular * modular + f * f) * 1.0);
        return *this;
    }

    MetaBB& MetaBB::setEntry(MetaInst* inst) {
        entry = inst;
        return *this;
    }

    MetaBB& MetaBB::setTerminator(MetaInst* inst) {
        terminator = inst;
        return *this;
    }

    MetaBB& MetaBB::setParent(MetaFunction* mF) {
        parent = mF;
        return *this;
    }

    MetaBB& MetaBB::setID(int id) { this->id = id; return* this; }

    MetaBB& MetaBB::buildInstGraphFromJSON(json::Object JSON, std::unordered_map<int64_t, MetaBB*>& tempBBMap, std::unordered_map<int64_t, MetaOperand*>& tempOperandMap) {
        json::Array& insts = *(JSON["instList"].getAsArray());

        for (int i = 0; i < insts.size(); ++i) {
            json::Object& inst = *(insts[i].getAsObject());
            json::Array&  ops  = *(inst.getArray("operandList"));
            
            // 连上 Meta Instruction 之间的边
            for (int j = 0; j < ops.size(); ++j) {
                int64_t op_id = ops[j].getAsInteger().getValue();
                assert(instList[i]);
                instList[i]->addOperand(tempOperandMap[op_id]);
            }
        }

    }

    MetaBB& MetaBB::buildInstFromJSON(json::Object JSON, std::unordered_map<int64_t, MetaBB*>& tempBBMap, std::unordered_map<int64_t, MetaOperand*>& tempOperandMap) {

        json::Array& insts = *(JSON["instList"].getAsArray());

        for (auto iter = insts.begin(); iter != insts.end(); ++iter) {
            int64_t     inst_id     = (*iter).getAsObject()->getInteger("id").getValue();
            bool        isMetaPhi   = (*iter).getAsObject()->getBoolean("isMetaPhi").getValue();
            MetaInst*   newInst     = isMetaPhi ? buildPhi(true) : buildInstruction();
            tempOperandMap[inst_id] = newInst;
            // 构建 Meta Instruction
            (*newInst)
                .setParent     (this)
                .buildFromJSON (*(iter->getAsObject()), tempBBMap, tempOperandMap)
                ;
        }

        (*this)
            .addFeature     (JSON["load"]   .getAsInteger().getValue())
            .addFeature     (JSON["store"]  .getAsInteger().getValue())
            .addFeature     (JSON["in"]     .getAsInteger().getValue())
            .addFeature     (JSON["out"]    .getAsInteger().getValue())
            .setEntry       ((MetaInst*)tempOperandMap[JSON["entry"]     .getAsInteger().getValue()])
            .setTerminator  ((MetaInst*)tempOperandMap[JSON["terminator"].getAsInteger().getValue()])
            ;
    }

    std::vector<MetaBB*> MetaBB::getNextBB() { return successors; }

    std::vector<int> MetaBB::getFeature() { return features; }

    MetaBB* MetaBB::getNextBB(int index) { return successors[index]; }

    MetaInst* MetaBB::getEntry() { return entry; }

    MetaInst* MetaBB::getTerminator() { return terminator; }

    std::vector<MetaInst*>& MetaBB::getInstList() { return instList; }

    int MetaBB::getInstNum() { return instList.size(); }

    MetaFunction* MetaBB::getParent() { return parent; }

    int MetaBB::getID() { assert(id != -1); return id; }

    double MetaBB::getModular() { return modular; }

    double MetaBB::similarity(MetaBB& bb) {
        if (!modular && !bb.getModular()) return 1.0;
        if (!modular || !bb.getModular()) return 0.0;
        std::vector<int> v = bb.getFeature();
        int dot = 0;
        for (int i = 0; i < features.size(); ++i) {
            dot += v[i] * features[i];
        }
        return dot / (bb.getModular() * bb.getModular());
    }

    std::vector<MetaInst*>::iterator MetaBB::begin() { return inst_begin(); }

    std::vector<MetaInst*>::iterator MetaBB::end() { return inst_end(); }

    std::vector<MetaInst*>::iterator MetaBB::inst_begin() { return instList.begin(); }

    std::vector<MetaInst*>::iterator MetaBB::inst_end() { return instList.end(); }

    std::vector<MetaBB*>::iterator MetaBB::next_begin() { return successors.begin(); }

    std::vector<MetaBB*>::iterator MetaBB::next_end() { return successors.end(); }

    std::string MetaBB::toString() {

        std::string instListStr = instList.empty() ? "[]" : "[";
        for (MetaInst* inst : instList) instListStr = instListStr + inst->toString() + ",";
        instListStr[instListStr.length() - 1] = ']';

        std::string sucStr = successors.empty() ? "[]" : "[";
        for (MetaBB* suc : successors) sucStr = sucStr + std::to_string(suc->getID()) + ",";
        sucStr[sucStr.length() - 1] = ']'; 

        std::string bbStr = "";

        return bbStr + "{" +
            "\"id\":" + std::to_string(id) + "," +
            "\"entry\":" + (entry == nullptr ? "null" : std::to_string(entry->getID())) + "," +
            "\"terminator\":" + (terminator == nullptr ? "null" : std::to_string(terminator->getID())) + "," +
            "\"load\":" + std::to_string(features[0]) + "," +
            "\"store\":" + std::to_string(features[1]) + "," +
            "\"in\":" + std::to_string(features[2]) + "," +
            "\"out\":" + std::to_string(features[3]) + "," +
            "\"instList\":" + instListStr + "," +
            "\"successors\":" + sucStr + 
            "}";
    }

    MetaBB* MetaBB::trainBB(MetaBB* irbb){
        std::cout << "\nDEBUG:: Entering function trainBB.....\n";
        MetaInst* irinst = NULL;
        std::vector<MetaInst*> asmvec;
        std::vector<MetaInst*> irvec;
        std::vector<MetaInst*> retvec;

        for(auto inst= this->begin(); inst!= this->end(); inst++){
            // Visit all the load and store instructions
            //if((*inst)->getInstType()[0] !=InstType::LOAD && (*inst)->getInstType()[0] !=InstType::STORE)
            std::cout << "DEBUG:: Checking Inst: "<< (*inst)->getOriginInst()<< std::endl;

            if((*inst)->ifMatched() && (*inst)->getInstType()[0] != InstType::LOAD){
                
                std::cout << "DEBUG:: Calling function QualifiedForMatch().....\n";
                auto res = (*inst)->QualifiedForMatch();
                if(res){
                    std::cout << "DEBUG:: MatchedInst() size = "<< (*inst)->getMatchedInst().size() << std::endl;
                    if((*inst)->getMatchedInst().size() == 0)
                        continue;

                    auto it = (*inst)->getMatchedInst()[(*inst)->getMatchedInst().size()-1];
                    std::cout << "DEBUG:: The end of MatchedInst ID is = "<< it->getID()
                              << ", Type is "
                              <<  it->getInstType()[0] 
                              << std::endl;
                    if(!it)
                        std::cout << "DEBUG:: getMatchedInst() returns NULL!\n";
                    if(it)
                        (*inst)->trainEquivClass(it);
                }
            }
               
            if((*inst)->getInstType()[0] == InstType::BRANCH)
                (*inst)->trainControlFlow();
            

            if((*inst)->getInstType()[0] == InstType::LOAD ){
                std::vector<MetaInst*> matchvec;
                // TODO:: CHECK If implict LOAD/STORE operations within an instruction can be traversed correctly
                if((*inst)->ifMatched()){
                    matchvec = (*inst)->getMatchedInst();
                    std::cout << "DEBUG:: In trainBB():: ASM instruction: " << (*inst)->getOriginInst()
                              << " has been mathced to IR instruction: " <<  matchvec[0]->getOriginInst()
                              << std::endl;
                }                 
                else{
                    matchvec = (*inst)->findTheSameInst(irbb);
                    if(matchvec.size() != 0 )
                        std::cout   << "DEBUG:: In trainBB():: ASM LOAD: " << (*inst)->getOriginInst()
                                    << " Find a new match in IR: " <<  matchvec[0]->getOriginInst()
                                    << std::endl;
                }
                // Skip unmatched or ambiguous instructions 
                // Can be optimized to address ambiguity if needed
                if(matchvec.size() != 1 ){
                    if(matchvec.size() > 1)
                        std::cout << "DEBUG:: Matched more than one load instruction!\n";
                    else
                        std::cout << "DEBUG:: No load instruction is matched!\n";
                    continue;
                }

                irinst = matchvec[0]; 
                std::cout << "DEBUG:: Calling function trainEquivClass().....\n";
                (*inst)->trainEquivClass(irinst);
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

        return this;
    }


//===-------------------------------------------------------------------------------===//
/// Meta Function implementation.

    void MetaFunction::init(llvm::json::Object& object) {
        json::Array& blocks =  *(object["basicBlocks"].getAsArray());
        json::Array& arguments = *(object["arguments"].getAsArray());
        json::Array& constants = *(object["constants"].getAsArray());

        std::unordered_map<int64_t, MetaOperand*> tempOperandMap;
        std::unordered_map<int64_t, MetaBB*> tempBBMap;

        // 构建 Meta Argument
        for (auto iter = arguments.begin(); iter != arguments.end(); ++iter) {
            MetaArgument& newArg = *buildArgument();
            int64_t arg_id = (*iter).getAsObject()->getInteger("id").getValue();
            tempOperandMap[arg_id] = &newArg;
            newArg
                .setParent(this)
                .setID(arg_id)
                ;
        }

        // 构建 Meta Constant
        for (auto iter = constants.begin(); iter != constants.end(); ++iter) {
            MetaConstant& newConst = *buildConstant();
            int64_t constant_id = (*iter).getAsObject()->getInteger("id").getValue();
            tempOperandMap[constant_id] = &newConst;
            newConst
                .setParent(this)
                .setID(constant_id)
                ;
        }

        // 构建 Meta BB
        for (auto iter = blocks.begin(); iter != blocks.end(); ++iter) {
            MetaBB& newBB = *buildBB();
            int64_t bbID = (*iter).getAsObject()->getInteger("id").getValue();
            tempBBMap[bbID] = &newBB;
            newBB
                .setParent(this)
                .setID(bbID)
                ;
            // 递归构建 Meta BB
            newBB.buildInstFromJSON(*((*iter).getAsObject()), tempBBMap, tempOperandMap);
        }

        for (int i = 0; i < blocks.size(); ++i) {
            json::Object& block = *(blocks[i].getAsObject());
            json::Array& suc = *(block.getArray("successors"));
            // 连上 Meta BB 之间的边
            for (int j = 0; j < suc.size(); ++j) {
                int64_t suc_id = suc[j].getAsInteger().getValue();
                assert(tempBBMap[suc_id]);
                bbs[i]->addNextBB(tempBBMap[suc_id]);
            }
            bbs[i]->buildInstGraphFromJSON(*(blocks[i].getAsObject()), tempBBMap, tempOperandMap);
        }

        (*this)
            .setRoot(tempBBMap[object["rootBB"].getAsInteger().getValue()])
            .setFunctionName(object["funcName"].getAsString().getValue().str())
            .setStackSize(object["stackSize"].getAsInteger().getValue())
            .setReturnType(MetaUtil::stringToDataType(object["returnType"].getAsString().getValue().str()))
            ;

    }

    MetaFunction::MetaFunction(std::string JSON) {
        std::cout << "MetaFunction::MetaFunction(std::string JSON)" << std::endl;
        llvm::Expected<json::Value> expect = json::parse(JSON);
        if (expect.takeError()) {
            std::cout << "parse function json error!" << "\n";
            return;
        }
        json::Object& object = *(expect.get().getAsObject());
        init(object);
    }

    MetaFunction::MetaFunction(llvm::json::Object& object) {
        init(object);
    }

    MetaFunction::MetaFunction() : stackSize(0), argNum(0) {

    }

    MetaFunction& MetaFunction::addConstant(MetaConstant* c) {
        constants.push_back(c);
        return *this;
    }
    
    MetaFunction& MetaFunction::addArgument(MetaArgument* a) {
        args.push_back(a);
        return *this;
    }

    MetaFunction& MetaFunction::setFunctionName(std::string name) {
        funcName = name;
        return *this;  
    }

    MetaFunction& MetaFunction::setRoot(MetaBB* rootBB) {
        root = rootBB;
        return *this;
    }

    MetaFunction& MetaFunction::setStackSize(int s) {
        stackSize = s;
        return *this;
    }
    
    MetaFunction& MetaFunction::setReturnType(DataType ty) {
        returnType = ty;
        return *this;
    }

    MetaFunction& MetaFunction::expandStackSize(int s) {
        stackSize += s;
        return *this;
    }

    int MetaFunction::getArgNum() { return args.size(); }

    int MetaFunction::getConstNum() { return constants.size(); }

    MetaBB* MetaFunction::getRoot() { return root; }

    std::vector<MetaBB*>& MetaFunction::getBB() { return bbs; }

    MetaArgument* MetaFunction::getArgument(int index) { return args[index]; }

    std::string MetaFunction::getFunctionName() { return funcName; }

    DataType MetaFunction::getReturnType() { return returnType; }

    // create a new bb at the end of bb list.
    MetaBB* MetaFunction::buildBB() {
        MetaBB* newBB = new MetaBB(this);
        bbs.push_back(newBB);
        return newBB;
    }

    MetaArgument* MetaFunction::buildArgument() {
        MetaArgument* newArg = new MetaArgument(this);
        args.push_back(newArg);
        return newArg;
    }

    MetaConstant* MetaFunction::buildConstant() {
        MetaConstant* newConst = new MetaConstant(this);
        constants.push_back(newConst);
        return newConst;
    }

    std::string MetaFunction::toString() {
        std::string funcStr = "{";

        std::string bbStr = bbs.empty() ? "[]" : "[";
        for (MetaBB* bb : bbs) {
            bbStr = bbStr + bb->toString() + ",";
        }
        bbStr[bbStr.length() - 1] = ']';

        std::string constStr = constants.empty() ? "[]" : "[";
        for (MetaConstant* constant : constants) {
            constStr = constStr + constant->toString() + ",";
        }
        constStr[constStr.length() - 1] = ']'; 

        std::string argStr = args.empty() ? "[]" : "[";
        for (MetaArgument* arg : args) {
            argStr = argStr + arg->toString() + ",";
        }
        argStr[argStr.length() - 1] = ']';
        
        return funcStr +
            "\"funcName\":" + "\"" + funcName + "\"" + "," +
            "\"stackSize\":" + std::to_string(stackSize) + "," +
            "\"rootBB\":" + std::to_string(root->getID()) + "," +
            "\"returnType\":" + "\"" + MetaUtil::toString(returnType) + "\"" + "," +
            "\"arguments\":" + argStr + "," +
            "\"constants\":" + constStr + "," +
            "\"basicBlocks\":" + bbStr +
            "}";
    }

    std::vector<MetaBB*>::iterator MetaFunction::begin() { return bb_begin(); }

    std::vector<MetaBB*>::iterator MetaFunction::end() { return bb_end(); }

    std::vector<MetaBB*>::iterator MetaFunction::bb_begin() { return bbs.begin(); }

    std::vector<MetaBB*>::iterator MetaFunction::bb_end() { return bbs.end(); }

    std::vector<MetaConstant*>::iterator MetaFunction::const_begin() { return constants.begin(); }

    std::vector<MetaConstant*>::iterator MetaFunction::const_end() { return constants.end(); }

    std::vector<MetaArgument*>::iterator MetaFunction::arg_begin() { return args.begin(); }

    std::vector<MetaArgument*>::iterator MetaFunction::arg_end() { return args.end(); }

}