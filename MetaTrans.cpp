#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/JSON.h"
#include <math.h>
#include <stdlib.h>
#include "meta/MetaTrans.h"
#include "meta/MetaUtils.h"
#include <utility>
#include <iostream>
#include <fstream>
#include <string>

const std::string RED("\033[31m");
const std::string GRN("\033[32m");
const std::string YEL("\033[33m");
const std::string CYN("\033[36m");
const std::string MAG("\033[35m");
const std::string RST("\033[0m");
const std::string BOLD("\033[1m");


// Create a Global Map Table;
MetaTrans::MappingTable* MapTable = NULL;

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

    MetaOperand& MetaOperand::setParentScope(MetaScope* scope) { parentScope = scope; return *this; }

    MetaScope* MetaOperand::getParentScope() { return parentScope; }

    MetaUnit& MetaOperand::getMetaUnit() { return *((MetaUnit*)parentScope->getRootScope()); }

    MetaOperand& MetaOperand::registerToMetaUnit() {
        getMetaUnit().registerOperand(this);
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

    MetaConstant::MetaConstant() : global(false), imm(false) { }

    MetaConstant::MetaConstant(MetaUnitBuildContext& context) {
        auto    object = context.getHoldObject();
        int64_t id     = object.getInteger("id")      .getValue();
        bool    global = object.getBoolean("isGloabl").getValue();
        bool    imm    = object.getBoolean("isImm")   .getValue();

        (*this)
            .setGlobal(global)
            .setImm(imm)
            .setParentScope(context.getCurScope())
            .registerToMetaUnit()
            .setID(id)
            ;
        context.addMetaOperand(id, this);
    }

    MetaConstant::MetaConstant(MetaFunction* p) : global(false), imm(false) {
        setParentScope(p);
    }

    MetaConstant::~MetaConstant() { }

    MetaConstant::MetaConstant(DataType ty) : type(ty), global(false), imm(false) { }

    MetaConstant& MetaConstant::setDataType(DataType ty) { type = ty; }

    DataType MetaConstant::getDataType() { return type; }

    DataUnion MetaConstant::getValue() { return value; }

    MetaConstant& MetaConstant::setName(std::string name) { this->name = name; return *this; }

    MetaConstant& MetaConstant::setGlobal(bool v) { global = v; return *this; }

    MetaConstant& MetaConstant::setImm(bool v) { imm = v; return *this; }

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

    MetaConstant& MetaConstant::setParentScope(MetaScope* scope) {
        MetaOperand::setParentScope(scope);
        return *this;
    }

    bool MetaConstant::isMetaConstant() { return true; }

    bool MetaConstant::isGlobal() { return global; }

    bool MetaConstant::isImm() { return imm; }

    std::string MetaConstant::getName() { return name; }

    std::string MetaConstant::toString() {
        std::string str = "";
        return str + "{" 
            + "\"id\":" + std::to_string(id) + ","
            + "\"isGloabl\":" + (global ? "true" : "false") + ","
            + "\"isImm\":" + (imm ? "true" : "false")
            + "}";
    }

//===-------------------------------------------------------------------------------===//
/// Meta Argument implementation.

    MetaArgument::MetaArgument() { }

    MetaArgument::MetaArgument(MetaFunction* p) {
        setParentScope(p);
    }

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

    MetaArgument& MetaArgument::setParentScope(MetaScope* scope) {
        MetaOperand::setParentScope(scope);
        return *this;
    }

    int MetaArgument::getArgIndex() { return argIndex; }

    int MetaArgument::getOffest() { return offset; }

    int MetaArgument::getWidth() { return width; }

    DataType MetaArgument::getArgType() { return type; }

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

    MetaInst& MetaInst::setParentScope(MetaScope* scope) {
        MetaOperand::setParentScope(scope);
        return *this;
    }

    MetaInst& MetaInst::addOperand(MetaOperand* op) {
        operandList.push_back(op);
        op->addUser(this);
        return *this;
    }

    MetaInst& MetaInst::buildFromJSON(MetaUnitBuildContext& context) {
       llvm::json::Object JSON  = context.getHoldObject();

        std::string     originInst          = JSON["originInst"]         .getAsString().getValue().str();
        std::string     dataRoot            = JSON["dataRoot"]           .getAsString().getValue().str();
        std::string     globalSymbolName    = JSON["globalSymbolName"]   .getAsString().getValue().str();
        unsigned long   hashCode            = JSON.getInteger("hashCode").getValue();
        int64_t         id                  = JSON.getInteger("id")      .getValue();
        int64_t         addr                = JSON.getInteger("address") .getValue();

        (*this)
            .setOriginInst          (originInst)
            .setHashcode            (hashCode)
            .setDataRoot            (dataRoot)
            .setGlobalSymbolName    (globalSymbolName)
            .setAddress             (address)
            .setID                  (id)
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
            if(this->isType(InstType::LOAD) && i == 1) {
                if(paths.size() > i) {
                    Path *p = paths[i];
                    if(p) path += "[" + std::to_string(p->type) + "," +std::to_string(p->numLoad) + "," +std::to_string(p->numStore) + "," +std::to_string(p->numPHI) + "," +std::to_string(p->numGEP) + "]";
                }
                break;
            } else if (this->isType(InstType::STORE) && (i == 0 || i == 1)) {
                if(paths.size() > i) {
                    if (i == 1 && paths[0] && paths[i]) path += ",";
                    Path *p = paths[i];
                    if(p) path += "[" + std::to_string(p->type) + "," +std::to_string(p->numLoad) + "," +std::to_string(p->numStore) + "," +std::to_string(p->numPHI) + "," +std::to_string(p->numGEP) + "]";
                }
            } else if (this->isType(InstType::BRANCH) && (i == 2)) {
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
        std::string str = "{";
        str = str + 
            "\"id\":" + std::to_string(id) + "," +
            "\"address\":" + std::to_string(address) + "," +
            "\"originInst\":" + "\"" +originInst + "\"" + "," +
            "\"isMetaPhi\":false,\"type\":" + MetaUtil::toString(type) + "," +
            "\"operandList\":" + opList + "," + 
            "\"userList\":" + userList + "," + 
            "\"path\":" + path + "," +
            "\"hashCode\":" + std::to_string(hashCode) + "," + 
            "\"dataRoot\":\"" + dataRoot + "\"," + 
            "\"globalSymbolName\":\"" + globalSymbolName + "\"" +
            "}"
            ;
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

    int MetaInst::getAddress() {
        return address;
    }

    MetaInst& MetaInst::setAddress(int address) {
        this->address = address;
    }

    MetaInst& MetaInst::setHashcode(unsigned long _hashCode) {
        hashCode = _hashCode;
        return *this;
    }

    std::string MetaInst::getDataRoot() {return dataRoot;}

    std::string MetaInst::getGlobalSymbolName() {return globalSymbolName;}

    MetaInst& MetaInst::setDataRoot(std::string s) { dataRoot = s; return *this; }

    MetaInst& MetaInst::setGlobalSymbolName(std::string s) { globalSymbolName = s; return *this; }

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

    // According to <Learning-Phase Algorithm> - Match Load/Store
    std::vector<MetaInst *> MetaInst::findTheSameInst(MetaBB *bb) {
        std::cout << "Enter findTheSameInst for asmInst: " << std::hex << this << std::oct << std::endl;
        std::vector<MetaInst *> ans;

        // 1. if only one load/store/branch, directly match
        // Each bb only have 1 branch or jump and it's in the end instruction
        if(this->isType(InstType::BRANCH)) {
            if(bb->inst_last()->isType(InstType::BRANCH)) {
                ans.push_back(bb->inst_last());
                return ans;
            }
        }

        // 2. TODO: Compare with dataRoot: if global->match name;
        if(dataRoot == "RISCV_GLOBAL") {
            std::string asmGlobalName = globalSymbolName;
            for (auto it = bb->inst_begin(); it != bb->inst_end(); it++) {
                MetaInst *IRinst = *it;
                // std::string rootAns = MetaUtil::findDataRoot(IRinst);
                if(this->hasSameType(IRinst) && IRinst->getDataRoot() == "TIR_GLOBAL") {
                    std::string rootAns = this->getGlobalSymbolName();
                    if(rootAns == asmGlobalName) {
                        std::cout << "findTheSameGlobalVariable " << rootAns << ": " << std::hex << IRinst << std::oct << std::endl;
                        ans.push_back(IRinst);
                        return ans;
                    }
                }
            }
        }

        // 3. Compare with hashCode
        if(hashCode != 0) {
            for (auto it = bb->inst_begin(); it != bb->inst_end(); it++) {
                MetaInst *inst = *it;
                if(inst->getHashcode() == hashCode) {
                    std::cout << "findTheSameHash: " << hashCode << std::endl;
                    ans.push_back(inst);
                }
            }
        }

        // 4. (Optional) Find the instruction has same path: each data compute/addressing/control flow path has the same numLoad, numStore, numPHI, numGEP be seen as the same Path
        for (auto it = bb->inst_begin(); it != bb->inst_end(); it++) {
            MetaInst *inst = *it;
            std::vector<Path *> anotherPath = inst->getAllPath();
            // std::cout << inst->toString() << std::endl;
            if (inst->isType(type[0])) { // TODO: fix if is the same type, not to use index 0 in case of AMO instr.
                if(this->isType(InstType::BRANCH)) {
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
                            if(this->isType(InstType::STORE)) {
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
        auto bb  = (MetaBB*)(this->getParentScope());
        this->EquivClassTag = 1;
        for(auto i = vec.begin(); i!=vec.end();i++) {
               if((*i)->isMetaInst() && (MetaBB*)(dynamic_cast<MetaInst*>(*i)->getParentScope()) == bb)
                    (dynamic_cast<MetaInst*>(*i))->buildEquivClass(); 
        }
    }

    void MetaInst::resetEquivClass(){
        auto vec = this->getOperandList();
        auto bb  = (MetaBB*)(this->getParentScope());
        this->EquivClassTag = 0;
        for(auto i = vec.begin(); i!=vec.end();i++){
               if((*i)->isMetaInst() && (MetaBB*)(dynamic_cast<MetaInst*>(*i)->getParentScope()) == bb)
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

    bool ifOrderMatters(InstType type){
        
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

        std::cout <<  "\nDEBUG:: Create Instruction Mapping:\n" << BOLD << GRN << mapping << RST <<std::endl;
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
        std::string str = "";
        this->Matched   = true;
        inst->Matched   = true;


        this->MatchedInst.push_back(inst);
        inst->MatchedInst.push_back(this);
    
        std::cout <<"DEBUG:: Calling function buildOperandMapping().....\n";

        if(this->getOriginInst()!="PHI")
            str = this->buildOperandMapping(inst);
        else{ 
            std::cout << "DEBUG:: Encounter PHI Node, Skip Operand Mapping\n";
            dumpMapping(this->getOriginInst() + " : " + inst->getOriginInst());
        }
        

        //Update MTable of 1-N mapping
        auto it = MapTable->MTable[1].find(this->getOriginInst());
        // std:: cout << "DEBUG::buildMapping() checked the MTable\n";
        // std:: cout << "DEBUG::buildOperandMapping returns string = " << str << std::endl;
        if(it != MapTable->MTable[1].end())
            std:: cout << "DEBUG::MTable contains the mapping for " << this->getOriginInst() 
                       << " : "<<it->second << std::endl;

        if(str != "" && it == MapTable->MTable[1].end()){
            std:: cout << "DEBUG::buildMapping() Writing to MTable " << MapTable->getTableName(1) << std::endl;
            MapTable->MTable[1][this->getOriginInst()] = inst->getOriginInst();
            MetaUtil::writeMapping(str, MapTable->getTableName(1));
        }

        std::cout <<"DEBUG:: Leaving function buildMapping().....\n";

        return *this;

    }


    //std::pair<std::string, std::string> 
    std::string MetaInst::buildOperandMapping(std::vector<MetaInst*> Fuse, std::string ASMorIR){

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
        return str;

    }


    //std::pair<std::string, std::string> 
    std::string MetaInst::buildOperandMapping(MetaInst* inst){
        
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
        std::cout << "DEBUG:: buildOperandMapping() This->getOperandNum = " << opNum << std::endl;
        std::cout << "DEBUG:: buildOperandMapping() inst->getOperandNum = " << inst->getOperandNum() << std::endl;

        if(opNum != inst->getOperandNum()){
            std::cout << BOLD << RED << "ERROR:: Operand Number mismatched between IR & ASM! STOP buildOperandMapping()!\n" << RST;
            return "";
        }
           

        if(this->getInstType().size() == 1 && inst->getInstType().size() == 1)
            // Unordered Operations can directly dump sequence
            if(!ifOrderMatters(this->getInstType()[0])){
                std::cout << "DEBUG:: Operand Ordering Unnecessary according to InstType check\n";
                for(int i = 0; i < opNum; i++)
                    str +=  std::to_string(i+1) + " ";
                dumpMapping(str);
                return str;
            }  



        // 1-1 Mapping
        if(asmVec.size() == 1 && irVec.size() == 1){
            for(int id = 0; id < asmOpVec.size(); id++){
                auto vec = dynamic_cast<MetaInst*>(asmOpVec[id])->getMatchedInst();
                for(int ir = 0; ir < irOpVec.size(); ir++){
                    if (ifFind (dynamic_cast<MetaInst*>(irOpVec[ir]), vec) != -1){
                        mapping.insert(std::make_pair(id, ir));
                        std::cout << "DEBUG:: buildOperandMapping() builds operand pair < "
                        << id << ", " << ir << " >"<<std::endl;
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
                return "";
            }
        }

        dumpMapping(str);
        std::cout <<"DEBUG:: Leaving function buildOperandMapping().....\n";

        return str;
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


    MetaInst* MetaInst::trainControlFlow(MetaInst* irinst){
    // Used for train branch <-> icmp + branch
        std::vector<MetaInst*> asmvec;
        std::vector<MetaInst*> irvec;
        std::vector<MetaInst*> retvec;
        std::vector<MetaOperand*> asmOperands = this->getOperandList();
        std::vector<MetaOperand*> irOperands = irinst->getOperandList();
        for(auto it = asmOperands.begin(); it != asmOperands.end(); it++) if((*it)->isMetaInst()) asmvec.push_back((MetaInst*)(*it));
        for(auto it = irOperands.begin(); it != irOperands.end(); it++) if((*it)->isMetaInst()) irvec.push_back((MetaInst*)(*it));

        std::cout << "DEBUG:: Enter function trainControlFlow().....\n";
        std::cout << "DEBUG:: asmvec.size() = "<< asmvec.size() <<std::endl;
        std::cout << "DEBUG:: irvec.size() = "<< irvec.size() <<std::endl;

        if(asmvec.size() == 0 || irvec.size()== 0)
            return NULL;   
        
        if(asmvec.size() == 1 && (asmvec[0])->isType(InstType::COMPARE)) {
            // Either asmvec is 'icmp' instruction
            retvec = (asmvec[0])->findMatchedInst(irvec);
        }else {
            // asmvec is arithmetic calculate for branch
            // Train the current 'branch' instruction with irvec
            retvec = this->findMatchedInst(irvec);
        }

        if(retvec.size() == 1)
            for(auto itt = retvec.begin();itt!=retvec.end();itt++)
                this->trainInst(*itt);

        return this;
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

        std::string str = "{";
        return str + 
            "\"id\":" + std::to_string(id) + "," +
            "\"address\":" + std::to_string(MetaInst::getAddress()) + "," + 
            "\"isMetaPhi\":true,\"type\":" + MetaUtil::toString(type) + 
            ",\"operandList\":" + opList + 
            ",\"bbValueMap\":" + phiMapStr +
            "}";
    }

    MetaInst& MetaPhi::buildFromJSON(MetaUnitBuildContext& context) {
        llvm::json::Object JSON = context.getHoldObject();
        
        int64_t id = JSON.getInteger("id").getValue();
        int64_t address = JSON.getInteger("address").getValue();

        setID(id);
        setAddress(address);


        return *this;
    }

    bool MetaPhi::isLoad() { return false; }

    bool MetaPhi::isStore() { return false; }


//===-------------------------------------------------------------------------------===//
/// Meta Basic Block implementation.

    
    MetaBB::MetaBB() : entry(nullptr), terminator(nullptr) { }

    MetaBB::MetaBB(MetaFunction* f) : entry(nullptr), terminator(nullptr) {
        setParentScope(f);
    }

    MetaBB::~MetaBB() {
        for (auto inst : instList) {
            delete inst;
        }
    }

    MetaBB& MetaBB::setParentScope(MetaScope* scope) {
        MetaScope::setParentScope(scope);
        return *this;
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

    MetaBB& MetaBB::setID(int64_t id) {
        MetaScope::setID(id);
        return *this;
    }

    MetaBB& MetaBB::registerToMetaUnit() {
        getMetaUnit().registerScope(this);
        return *this;
    }

    MetaBB& MetaBB::buildInstGraphFromJSON(MetaUnitBuildContext& context) {
        json::Array insts = context.getJsonArray("instList");

        for (int i = 0; i < insts.size(); ++i) {
            json::Object inst = *(insts[i].getAsObject());
            json::Array ops  = *(inst.getArray("operandList"));
            
            // 连上 Meta Instruction 之间的边
            for (int j = 0; j < ops.size(); ++j) {
                int64_t op_id = ops[j].getAsInteger().getValue();
                assert(instList[i]);
                assert(context.getMetaOperand(op_id));
                MetaOperand* operand = context.getMetaOperand(op_id);
                instList[i]->addOperand(operand);
            }
                
            // 如果是phi节点 则添加bb和value的映射
            if (instList[i]->isMetaPhi()) {
                json::Object kv = *(inst["bbValueMap"].getAsObject());
                for (auto iter = kv.begin(); iter != kv.end(); ++iter) {
                    int key = std::stoi(iter->first.str());
                    int val = iter->second.getAsInteger().getValue();
                    printf("%d %d\n", key, val);
                    assert(context.getMetaOperand(val));
                    assert(context.getMetaBB(key));
                    ((MetaPhi*)(instList[i]))->addValue(context.getMetaBB(key), context.getMetaOperand(val));
                }
            }
        }

    }

    MetaBB& MetaBB::buildInstFromJSON(MetaUnitBuildContext& context) {

        json::Array insts = context.getJsonArray("instList");
        
        for (auto iter = insts.begin(); iter != insts.end(); ++iter) {
            int64_t     inst_id     = (*iter).getAsObject()->getInteger("id").getValue();
            bool        isMetaPhi   = (*iter).getAsObject()->getBoolean("isMetaPhi").getValue();
            MetaInst*   newInst     = isMetaPhi ? buildPhi(true) : buildInstruction();
            context.addMetaOperand(inst_id, newInst);

            context.saveContext().setHoldObject(iter->getAsObject());
            // 构建 Meta Instruction
            (*newInst)
                .buildFromJSON      (context)
                .setParentScope     (this)
                .registerToMetaUnit ()
                ;
            context.restoreContext();
        }
        
        
        llvm::json::Object object = context.getHoldObject();
        (*this)
            .setID          (object["id"]     .getAsInteger().getValue())
            .addFeature     (object["load"]   .getAsInteger().getValue())
            .addFeature     (object["store"]  .getAsInteger().getValue())
            .addFeature     (object["in"]     .getAsInteger().getValue())
            .addFeature     (object["out"]    .getAsInteger().getValue())
            .setEntry       (context.getMetaInst(object["entry"]     .getAsInteger().getValue()))
            .setTerminator  (context.getMetaInst(object["terminator"].getAsInteger().getValue()))
            ;
    }

    std::vector<MetaBB*> MetaBB::getNextBB() { return successors; }

    std::vector<int> MetaBB::getFeature() { return features; }

    MetaBB* MetaBB::getNextBB(int index) { return successors[index]; }

    MetaInst* MetaBB::getEntry() { return entry; }

    MetaInst* MetaBB::getTerminator() { return terminator; }

    std::vector<MetaInst*>& MetaBB::getInstList() { return instList; }

    int MetaBB::getInstNum() { return instList.size(); }

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

    MetaUnit& MetaBB::getMetaUnit() {
        return *((MetaUnit*)getRootScope());
    }

    Stream<MetaInst*> MetaBB::stream() { Stream<MetaInst*> s(instList); return s; }

    std::vector<MetaInst*>::iterator MetaBB::begin() { return inst_begin(); }

    std::vector<MetaInst*>::iterator MetaBB::end() { return inst_end(); }

    std::vector<MetaInst*>::iterator MetaBB::inst_begin() { return instList.begin(); }

    std::vector<MetaInst*>::iterator MetaBB::inst_end() { return instList.end(); }
    
    MetaInst* MetaBB::inst_last() { return instList.back(); }

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
            "\"id\":" + std::to_string(getID()) + "," +
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
               
            if((*inst)->isType(InstType::BRANCH)) {
                auto matchvec = (*inst)->findTheSameInst(irbb);
                if(matchvec.size() != 1 ){
                    if(matchvec.size() > 1)
                        std::cout << "DEBUG:: Matched more than one branch instruction!\n";
                    else
                        std::cout << "DEBUG:: No branch instruction is matched!\n";
                    continue;
                }
                irinst = matchvec[0]; 
                std::cout << "DEBUG:: Calling function trainControlFlow().....\n";
                (*inst)->trainControlFlow(irinst);
            }
            
            if((*inst)->getInstType().size() == 0){
                std::cout << "DEBUUG:: trainBB meets instruction " << (*inst)->getOriginInst() 
                          << ", which has no instruction type recorded\n";
                continue;
            }

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

    MetaFunction::MetaFunction(MetaUnitBuildContext& context) {
        json::Array blocks    = context.getJsonArray("basicBlocks");
        json::Array arguments = context.getJsonArray("arguments");
        json::Array constants = context.getJsonArray("constants");


        json::Object object = context.getHoldObject();
        (*this)
            .setParentScope(context.getCurScope())
            .registerToMetaUnit()
            .setFunctionName(object["funcName"].getAsString().getValue().str())
            .setStackSize(object.getInteger("stackSize").getValue())
            .setReturnType(MetaUtil::stringToDataType(object["returnType"].getAsString().getValue().str()))
            .setID(object.getInteger("id").getValue())
            ;
        
        context.addMetaScope(getID(), this);

        // 构建 Meta Argument
        for (auto iter = arguments.begin(); iter != arguments.end(); ++iter) {
            MetaArgument* newArg = buildArgument();
            int64_t arg_id = (*iter).getAsObject()->getInteger("id").getValue();
            context.addMetaOperand(arg_id, newArg);
            (*newArg)
                .setParentScope     (this)
                .registerToMetaUnit ()
                .setID              (arg_id)
                ;
        }

        // 构建 Meta Constant
        for (auto iter = constants.begin(); iter != constants.end(); ++iter) {
            MetaConstant* newConst = buildConstant();
            int64_t constant_id = (*iter).getAsObject()->getInteger("id").getValue();
            context.addMetaOperand(constant_id, newConst);
            (*newConst)
                .setParentScope     (this)
                .registerToMetaUnit ()
                .setID              (constant_id)
                ;
        }

        // 构建 Meta BB
        for (auto iter = blocks.begin(); iter != blocks.end(); ++iter) {
            MetaBB* newBB = buildBB();
            int64_t bbID = (*iter).getAsObject()->getInteger("id").getValue();
            context.addMetaScope(bbID, newBB);
            (*newBB)
                .setParentScope     (this)
                .registerToMetaUnit ()
                .setID              (bbID)
                ;
            // 递归构建 Meta BB
            context.saveContext().setCurScope(this).setHoldObject(iter->getAsObject());
            newBB->buildInstFromJSON(context);
            context.restoreContext();
        }
        
        (*this)
            .setRoot(context.getMetaBB(object["rootBB"].getAsInteger().getValue()))
            ;

        for (int i = 0; i < blocks.size(); ++i) {
            json::Object block = *(blocks[i].getAsObject());
            json::Array suc = *(block.getArray("successors"));
            // 连上 Meta BB 之间的边
            for (int j = 0; j < suc.size(); ++j) {
                int64_t suc_id = suc[j].getAsInteger().getValue();
                assert(context.getMetaBB(suc_id));
                bbs[i]->addNextBB(context.getMetaBB(suc_id));
            }
            context.saveContext().setCurScope(this).setHoldObject(block);
            bbs[i]->buildInstGraphFromJSON(context);
            context.restoreContext();
        }

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
    
    MetaFunction& MetaFunction::registerToMetaUnit() {
        getMetaUnit().registerScope(this);
        return *this;
    }

    MetaFunction& MetaFunction::setParentScope(MetaScope* scope) {
        MetaScope::setParentScope(scope);
        return *this;
    }

    MetaUnit& MetaFunction::getMetaUnit() {
        return *((MetaUnit*)getRootScope());
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
            "\"id\":" + std::to_string(getID()) + "," +
            "\"funcName\":" + "\"" + funcName + "\"" + "," +
            "\"stackSize\":" + std::to_string(stackSize) + "," +
            "\"rootBB\":" + std::to_string(root->getID()) + "," +
            "\"returnType\":" + "\"" + MetaUtil::toString(returnType) + "\"" + "," +
            "\"arguments\":" + argStr + "," +
            "\"constants\":" + constStr + "," +
            "\"basicBlocks\":" + bbStr +
            "}";
    }

    Stream<MetaBB*> MetaFunction::stream() { Stream<MetaBB*> s(bbs); return s; }

    std::vector<MetaBB*>::iterator MetaFunction::begin() { return bb_begin(); }

    std::vector<MetaBB*>::iterator MetaFunction::end() { return bb_end(); }

    std::vector<MetaBB*>::iterator MetaFunction::bb_begin() { return bbs.begin(); }

    std::vector<MetaBB*>::iterator MetaFunction::bb_end() { return bbs.end(); }

    std::vector<MetaConstant*>::iterator MetaFunction::const_begin() { return constants.begin(); }

    std::vector<MetaConstant*>::iterator MetaFunction::const_end() { return constants.end(); }

    std::vector<MetaArgument*>::iterator MetaFunction::arg_begin() { return args.begin(); }

    std::vector<MetaArgument*>::iterator MetaFunction::arg_end() { return args.end(); }



//===-------------------------------------------------------------------------------===//
/// Mapping Table implementation.


    MappingTable* MappingTable::initTableMeta(){

        std::ifstream file(this->MappingName[0]);
        if(!file){
            std::ofstream new_file(this->MappingName[0]);
            new_file.close();
            std::cout << "Mapping Table " << this->MappingName[0] << "does not exist! Creating a new one! \n";
            return this;
        }
        // Check if the file was opened successfully
        if (!file.is_open()) {
            std::cout << "Error: Could not open file " << this->MappingName[0] << std::endl;
            return NULL;
        }

        std::string line;
        while (std::getline(file, line)) {
        // Split the line into two parts - the string and the integer
            std::string::size_type pos = line.find(' ');
            if (pos == std::string::npos) {
                std::cerr << "Error: Invalid line format." << std::endl;
                continue;
            }
            std::string str = line.substr(0, pos);
            std::string int_str = line.substr(pos+1);

            // Convert the string to a std::string and the integer to an int
            int value;
            try {
                value = std::stoi(int_str);
            } catch (std::invalid_argument& e) {
                std::cerr << "Error: Invalid integer value." << std::endl;
                continue;
            } catch (std::out_of_range& e) {
                std::cerr << "Error: Integer value out of range." << std::endl;
                continue;
            }

            // Store the mapping in the map
            this->TableMata[str] = value;
        }
        return this;
    }

    MappingTable* MappingTable::loadMappingTable(){    

        std::cout << "\n\n\n\n"
                  << "======================"
                  << "Loading Mapping Table"
                  << "======================"
                  << "\n\n\n\n";

        // Fill the MTable[0] first
        std::map<std::string, std::string> filler;
        filler[" "] = " ";
        this->MTable.push_back(filler);

        for(int i = 1; i < this->max; i++){

            std::ifstream file(this->MappingName[i]);
            if (!file) {
                std::ofstream new_file(this->MappingName[i]);
                new_file.close();
                std::cout << "Mapping Table " << this->MappingName[i] << "does not exist! Creating a new one! \n";
                continue;
            }
            // Check if the file was opened successfully
            if (!file.is_open()) {
                std::cerr << "Error: Could not open file "<< this->MappingName[i] << std::endl;
                continue;
            }

            std::map<std::string, std::string> map;
            std::string line;
            while (std::getline(file, line)) {
            // Split the line into two parts - the string and the integer
                std::string::size_type pos = line.find(' : ');
                if (pos == std::string::npos) {
                    std::cerr << "Error: Invalid line format." << std::endl;
                    continue;
                }
                std::string str = line.substr(0, pos);
                std::string ir =  line.substr(pos+3);
                std::cout << "DEBUG:: loadMappingTable() is Parsing string " << str
                          << " and " << ir << std::endl;  

                // Store the mapping in the map
                map[str] = ir;
            }
            this->MTable.push_back(map);
        }


        std::cout << "\n\n\n\n"
                  << "==========================="
                  << "Mapping Table Loading Done!"
                  << "==========================="
                  << "\n\n\n\n";

        return this;
    }


    int MappingTable::locateMappingTable(std::string InstName){

        auto ptr = this->TableMata.find(InstName);

        if(ptr != TableMata.end())
            return ptr->second;
        else
            return 0;

    }

    MappingTable* MappingTable::setName(std::string path){

        this->MappingName.push_back(path + "TableMata.mapping");
        for(int i = 1; i <= this->max; i++)
            this->MappingName.push_back(path + std::to_string(i) + "-N.mapping");
        return this;
    }


    std::string MappingTable::getTableName(int id){
        if(id > this->max){
            std::cout << "ERROR:: getTableName() exceeds the range of vector MappingName!\n";
            return "";
        }
        return MappingName[id];
    }

//===-------------------------------------------------------------------------------===//
/// MetaUnit implementation.

    MetaUnit::MetaUnit() { }

    MetaUnit::MetaUnit(std::string& JSON) {
        MetaUnitBuildContext context(JSON);

        json::Array funcObj = context.getJsonArray("funcs");
        json::Array globalVarObj = context.getJsonArray("globalVar");

        for (auto iter = globalVarObj.begin(); iter != globalVarObj.end(); ++iter) {
            context.saveContext().setCurScope(this).setHoldObject(iter->getAsObject());
            MetaConstant* c = new MetaConstant(context);
            addGlobalVar(c);
            context.restoreContext();
        }

        for (auto iter = funcObj.begin(); iter != funcObj.end(); ++iter) {
            context
                .saveContext()
                .setHoldObject(iter->getAsObject())
                .setCurScope(this)
                ;
            MetaFunction* f = new MetaFunction(context);
            addFunc(f);
            context.restoreContext();
        }

        printf("size of func: %d size of global: %d\n", funcs.size(), globalVar.size());
            
    }

    MetaUnit::~MetaUnit() { }

    MetaUnit& MetaUnit::addFunc(MetaFunction* f) { funcs.push_back(f); return *this; }

    MetaUnit& MetaUnit::addGlobalVar(MetaConstant* c) { globalVar.push_back(c); return *this; }

    MetaUnit& MetaUnit::fillID() {
        int op_id = 0, scope_id = 0, inst_address = 0;
        for (auto operand : operands) {
            operand->setID(op_id++);
            if (operand->isMetaInst())
                ((MetaInst*)operand)->setAddress(inst_address++);
        }
        for (auto scope : scopes) scope->setID(scope_id++);
    }

    MetaUnit& MetaUnit::registerOperand(MetaOperand* operand) {
        operands.push_back(operand);
        if (operand->isMetaInst()) insts.push_back((MetaInst*)operand);
    }

    MetaUnit& MetaUnit::registerScope(MetaScope* scope) {
        scopes.push_back(scope);
    }

    Stream<MetaFunction*> MetaUnit::func_stream() { return std::move(Stream<MetaFunction*>(funcs)); } 

    Stream<MetaConstant*> MetaUnit::global_stream() { return std::move(Stream<MetaConstant*>(globalVar)); } 

    Stream<MetaInst*> MetaUnit::inst_stream() { return std::move(Stream<MetaInst*>(insts)); } 

    std::vector<MetaFunction*>& MetaUnit::getFuncList() { return funcs; }

    std::vector<MetaConstant*>& MetaUnit::getGlobalVarList() { return globalVar; }
    
    std::vector<MetaFunction*>::iterator MetaUnit::begin() { return funcs.begin(); }

    std::vector<MetaFunction*>::iterator MetaUnit::end() { return funcs.end(); }

    std::string MetaUnit::toString() {
        std::string&& str           = "{";
        std::string&& funcStr       = MetaUtil::vectorToJsonString(funcs);
        std::string&& globalVarStr  = MetaUtil::vectorToJsonString(globalVar);
        printf("size of func: %d size of global: %d\n", funcs.size(), globalVar.size());

        return str
            + "\"funcs\":" + funcStr + ","
            + "\"globalVar\":" + globalVarStr
            + "}"
            ;
    }


//===-------------------------------------------------------------------------------===//
/// MetaScope implementation.
    
    MetaScope::MetaScope() : parent(nullptr), id(-1) { }

    int MetaScope::getID() { return id; }

    MetaScope& MetaScope::setParentScope(MetaScope* scope) { parent = scope; return *this; }

    MetaScope& MetaScope::setID(int id) { this->id = id; return *this; }

    MetaScope* MetaScope::getParentScope() { return parent; }

    MetaScope* MetaScope::getRootScope() {
        if (parent == nullptr) return this;
        return parent->getRootScope();
    }

//===-------------------------------------------------------------------------------===//
/// MetaUnitBuildContext implementation.

    MetaUnitBuildContext::MetaUnitBuildContext(std::string str) : JSON(str), scope(nullptr) {
        llvm::Expected<json::Value> expect = json::parse(str);
        if (expect.takeError()) {
            std::cout << "parse function json error!" << "\n";
            return;
        }
        object = *(expect.get().getAsObject());
    }

    llvm::json::Object MetaUnitBuildContext::getHoldObject() {
        return object;
    }

    llvm::json::Array MetaUnitBuildContext::getJsonArray(std::string key) {
        return *(object[key].getAsArray());
    }

    llvm::json::Object MetaUnitBuildContext::getJsonObject(std::string key) {
        return *(object[key].getAsObject());
    }

    MetaScope* MetaUnitBuildContext::getScope(int64_t id) { return scopeMap[id]; }
    
    MetaScope* MetaUnitBuildContext::getCurScope() { return scope; }
    
    MetaUnitBuildContext& MetaUnitBuildContext::setCurScope(MetaScope* scope) {
        this->scope = scope;
        return *this;
    }

    MetaUnitBuildContext& MetaUnitBuildContext::setHoldObject(llvm::json::Object& o) {
        object = o;
        return *this;
    }

    MetaUnitBuildContext& MetaUnitBuildContext::setHoldObject(llvm::json::Object* o) {
        object = *o;
        return *this;
    }

    MetaUnitBuildContext& MetaUnitBuildContext::addMetaOperand(int64_t id, MetaOperand* operand) {
        operandMap[id] = operand;
        return *this;
    }

    MetaUnitBuildContext& MetaUnitBuildContext::addMetaScope(int64_t id, MetaScope* scope) {
        scopeMap[id] = scope; 
        return *this;
    }

    MetaUnitBuildContext& MetaUnitBuildContext::saveContext() {
        objectStack.push_back(object);
        scopeStack.push_back(scope);
        return *this;
    }

    MetaUnitBuildContext& MetaUnitBuildContext::restoreContext() {
        object = objectStack.back();
        scope = scopeStack.back();
        objectStack.pop_back();
        scopeStack.pop_back();
        return *this;
    }

    MetaOperand* MetaUnitBuildContext::getMetaOperand(int64_t id) {
        return operandMap[id];
    }

    MetaConstant* MetaUnitBuildContext::getMetaConstant(int64_t id) {
        return (MetaConstant*) operandMap[id];
    }

    MetaInst* MetaUnitBuildContext::getMetaInst(int64_t id) {
        return (MetaInst*) operandMap[id];
    }

    MetaBB* MetaUnitBuildContext::getMetaBB(int64_t id) {
        return (MetaBB*) scopeMap[id];
    }

    MetaFunction* MetaUnitBuildContext::getMetaFunction(int64_t id) {
        return (MetaFunction*) scopeMap[id];
    }


} // end namespace MetaTrans
