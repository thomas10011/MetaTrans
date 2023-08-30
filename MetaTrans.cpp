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

    MetaOperand& MetaOperand::addUsers(std::vector<MetaInst*> vec) {
        std::unordered_set<MetaInst*> sset(users.begin(), users.end());
        for(MetaInst* user : vec) {
            if(sset.count(user) == 0)
                users.push_back(user);
        }
        return *this;
    }

    MetaOperand& MetaOperand::removeUser(MetaInst* user) {
        for (auto it = users.begin(); it != users.end(); ) {
            if (*it == user) {
                printf("removed user %d!\n", user);
                it = users.erase(it);
            }
            else ++it;
        }
        return *this;
    }


    MetaOperand& MetaOperand::copyUsers(MetaOperand* op) {
        this->users = op->getUsers();
        return *this;
    }

    MetaOperand& MetaOperand::replaceUser(MetaInst* src, MetaInst* dest) {
        for (int i = 0; i < users.size(); ++i) {
            if (users[i] == src) { users[i] = dest; }
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

    MetaOperand& MetaOperand::unregisterFromMetaUnit() {
        getMetaUnit().unregisterOperand(this);
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

    MetaPrimType MetaOperand::getDataType() {
        MetaPrimType ty;
        return ty;
    }

    MetaOperand::~MetaOperand() { }

    std::string MetaOperand::toString() { return "\"Operand\""; }

//===-------------------------------------------------------------------------------===//
/// Meta Constant implementation.

    MetaConstant::MetaConstant() : global(false), imm(false), primaryType(DataType::INT, 64, 0) {

    }

    MetaConstant::MetaConstant(MetaUnitBuildContext& context) {
        auto        object = context.getHoldObject();
        std::string name   = object.getString ("name")    .getValue().str();
        std::string value  = object.getString ("value")   .getValue().str();
        int64_t     id     = object.getInteger("id")      .getValue();
        bool        global = object.getBoolean("isGloabl").getValue();
        bool        imm    = object.getBoolean("isImm")   .getValue();
        MetaPrimType type(object.getString("type").getValue().str());

        (*this)
            .setName(name)
            .setValueStr(value)
            .setGlobal(global)
            .setImm(imm)
            .setType(type)
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

    MetaConstant& MetaConstant::setType(MetaPrimType ty) { primaryType = ty; }

    MetaPrimType MetaConstant::getDataType() { return primaryType; }

    DataUnion MetaConstant::getValue() { return value; }

    MetaConstant& MetaConstant::setName(std::string name) { this->name = name; return *this; }

    MetaConstant& MetaConstant::setGlobal(bool v) { global = v; return *this; }

    MetaConstant& MetaConstant::setImm(bool v) { imm = v; return *this; }

    MetaConstant& MetaConstant::setValue(int8_t v) {
        primaryType.setType(DataType::INT);
        primaryType.setWidth(8);
        value.int_8_val = v; 
        return *this;
    }

    MetaConstant& MetaConstant::setValue(int16_t v) {
        primaryType.setType(DataType::INT);
        primaryType.setWidth(16);
        value.int_16_val = v; 
        return *this;
    }
    
    MetaConstant& MetaConstant::setValue(int32_t v) { 
        primaryType.setType(DataType::INT);
        primaryType.setWidth(32);
        value.int_32_val = v; 
        return *this;
    }

    MetaConstant& MetaConstant::setValue(int64_t v) { 
        primaryType.setType(DataType::INT);
        primaryType.setWidth(64);
        value.int_64_val = v; 
        return *this;
    }

    MetaConstant& MetaConstant::setValue(float v) {
        primaryType.setType(DataType::FLOAT);
        primaryType.setWidth(32);
        value.float_val = v; 
        return *this;
    }
    
    MetaConstant& MetaConstant::setValue(double v) { 
        primaryType.setType(DataType::FLOAT);
        primaryType.setWidth(64);
        value.double_val = v; 
        return *this;
    }

    MetaConstant& MetaConstant::setParentScope(MetaScope* scope) {
        MetaOperand::setParentScope(scope);
        return *this;
    }

    MetaConstant& MetaConstant::setValueStr(std::string str) {
        valueStr = str;
        if (MetaUtil::isNumber(str)) {
            if (MetaUtil::contain(".", str)) {
                primaryType.setType(DataType::FLOAT);
            }
            else {
                primaryType.setType(DataType::INT);
            }
            primaryType.setWidth(64); 
        }
        return *this;
    }

    MetaConstant& MetaConstant::setDataType(DataType ty) {
        primaryType.setType(ty);
        return *this;
    }

    MetaConstant& MetaConstant::setWidth(int w) {
        primaryType.setWidth(w);
        return *this;
    }

    MetaConstant& MetaConstant::abs() {
        if (MetaUtil::startwith("-", valueStr)) {
            valueStr = valueStr.substr(1);
        }
    }

    std::string MetaConstant::getValueStr() {
        return valueStr;
    }

    bool MetaConstant::isMetaConstant() { return true; }

    bool MetaConstant::isGlobal() { return global; }

    bool MetaConstant::isImm() { return imm; }

    std::string MetaConstant::getName() { return name; }

    std::string MetaConstant::toString() {
        std::string str = "";
        return str + "{" 
            + "\"type\":" + "\"" + primaryType.toString() + "\"" + ","
            + "\"name\":" + "\"" + getName() + "\"" + ","
            + "\"value\":" + (valueStr.length() ? "\"" + valueStr + "\"" : "\"\"") + "," 
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

    MetaArgument::MetaArgument(DataType ty) {
        type.setType(ty);
    }

    MetaArgument& MetaArgument::setArgIndex(int i) {
        argIndex = i;
        return *this;
    }

    MetaArgument& MetaArgument::setOffset(int o) {
        offset = o;
        return *this;
    }
    
    MetaArgument& MetaArgument::setDataType(DataType ty) {
        type.setType(ty);
        return *this;
    }

    MetaArgument& MetaArgument::setWidth(int w) {
        type.setWidth(w);
        return *this;
    }

    MetaArgument& MetaArgument::setPtrLevel(int l) {
        type.setPrtLevel(l);
        return *this;
    }

    MetaArgument& MetaArgument::setType(MetaPrimType type) {
        this->type = type;
        return *this;
    }

    MetaArgument& MetaArgument::setParentScope(MetaScope* scope) {
        MetaOperand::setParentScope(scope);
        return *this;
    }

    int MetaArgument::getArgIndex() { return argIndex; }

    int MetaArgument::getOffest() { return offset; }

    int MetaArgument::getWidth() { return width; }

    MetaPrimType MetaArgument::getDataType() { return type; }

    bool MetaArgument::isMetaArgument() { return true; }

    std::string MetaArgument::toString() {
        std::string str = "";
        return str + "{" 
            + "\"id\":" + std::to_string(id) + ","
            + "\"type\":" + "\"" + type.toString() + "\""
            "}";
    }

//===-------------------------------------------------------------------------------===//
/// Meta Instruction implementation.

    MetaInst::MetaInst() : color(-1, -1) { 
        paths.resize(3);
    }

    MetaInst::MetaInst(std::vector<InstType> ty) : type(ty), color(-1, -1) {
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

    MetaInst& MetaInst::setDataType(MetaPrimType ty) {
        this->dataType = ty;
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
    
    MetaInst& MetaInst::initOperands(int num) {
        while (num-- > 0) operandList.push_back(nullptr);
        return *this;
    }

    MetaInst& MetaInst::addOperand(MetaOperand* op) {
        operandList.push_back(op);
        op->addUser(this);
        return *this;
    }

    MetaInst& MetaInst::addOperandAt(MetaOperand* op, int index) {
        // while (operandList.size() <= index) operandList.push_back(nullptr);
        operandList.at(index) = op;
        op->addUser(this);
        return *this;
    }

    MetaInst& MetaInst::addOperandAtLast(MetaOperand* op) {
        operandList.at(operandList.size() - 1) = op;
        op->addUser(this);
        return *this;
    }

    bool MetaInst::checkOperands() {
        int validNum = 0;
        // This will contains rs1/2/3 and immediate
        for (int i = 0; i < operandList.size(); ++i) {
            MetaOperand* op = operandList[i];
            if (op) {
                validNum++;
                printf("operandList[%d]: %s \n", i, op->toString().c_str());
            }
        }
        std::cout << "INFO: " << this << " " << originInst.c_str() << " checkOperands validNum " << validNum << " " << (validNum != this->NumOperands ? '!' : '=') << " this->NumOperands " << this->NumOperands << std::endl;

        return validNum == this->NumOperands;
    }

    MetaInst& MetaInst::checkNullOperands() {
        for (MetaOperand* op : operandList) {
            if (op == nullptr) printf("ERROR: Find nullptr operand with inst %s (id=%d)!\n", getOriginInst().c_str(), getID());
        }
        return *this;
    }

    MetaInst& MetaInst::filterNullOperands() {
        auto iter = operandList.begin();
        while (iter != operandList.end()) {
            if (*iter == nullptr) iter = operandList.erase(iter);
            else ++iter;
        }
        return *this;
    }

    MetaInst& MetaInst::buildFromJSON(MetaUnitBuildContext& context) {
       llvm::json::Object JSON  = context.getHoldObject();

        std::string     originInst          = JSON["originInst"]          .getAsString().getValue().str();
        std::string     dataRoot            = JSON["dataRoot"]            .getAsString().getValue().str();
        std::string     globalSymbolName    = JSON["globalSymbolName"]    .getAsString().getValue().str();
        unsigned long   hashCode            = JSON.getInteger("hashCode") .getValue();
        int64_t         id                  = JSON.getInteger("id")       .getValue();
        int64_t         addr                = JSON.getInteger("address")  .getValue();
        bool            isAddrGen           = JSON.getBoolean("isAddrGen").getValue();
        bool            isfake              = JSON.getBoolean("isFake").getValue();

        (*this)
            .setOriginInst          (originInst)
            .setHashcode            (hashCode)
            .setDataRoot            (dataRoot)
            .setGlobalSymbolName    (globalSymbolName)
            .setAddress             (address)
            .setAddrGen             (isAddrGen)
            .setID                  (id)
            ;

        if(isfake)
            this->setFake();

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

    MetaPrimType MetaInst::getDataType() {
        return dataType;
    }

    bool MetaInst::isLoad() { for (auto ty : type) if (ty == InstType::LOAD) return true; return false; }

    bool MetaInst::isStore() { for (auto ty : type) if (ty == InstType::STORE) return true; return false; }

    bool MetaInst::isMemOp() { return isLoad() || isStore(); }

    bool MetaInst::isAddressing() { return getColor()->type == 0; }

    bool MetaInst::isComputing() { return getColor()->type == 1; }

    bool MetaInst::isControlFlow() { return getColor()->type == 2; }

    int MetaInst::getOperandNum() { return operandList.size(); }

    int MetaInst::getNumOperands() {return NumOperands;}

    int MetaInst::getValidOperandNum() {
        int ans = 0;
        for (int i = 0; i < operandList.size(); i++)
            if (operandList[i]) ans++; 
        return ans;
    }

    void MetaInst::setNumOperands(int i) {NumOperands = i;}

    std::vector<InstType> MetaInst::getInstType() { return type; }

    std::vector<MetaOperand*>& MetaInst::getOperandList() {
        return operandList;
    }

    std::vector<MetaInst*> MetaInst::getOperandOnlyInstList() {
        std::vector<MetaInst*> ans;
        for(int i = 0; i < operandList.size(); i++) if(operandList[i] && operandList[i]->isMetaInst()) ans.push_back(dynamic_cast<MetaInst*>(operandList[i]));
        return ans;
    }

    std::vector<MetaOperand*>::iterator MetaInst::op_begin() { return operandList.begin(); }

    std::vector<MetaOperand*>::iterator MetaInst::op_end() { return operandList.end(); }

    bool MetaInst::hasStrictSameType(MetaInst* i) {
        int64_t ty = 0;
        for (InstType t : type) ty |= (1 << t);
        for (InstType t : i->getInstType()) ty ^= (1 << t);
        return ty == 0;
    }

    bool MetaInst::hasRelaxedSameType(MetaInst* i) {
        std::unordered_set<InstType> set;
        for (InstType t : type) set.insert(t);
        for (InstType t : i->getInstType()) {
            if(set.count(t)) return true;
        }
        return false;
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

    bool MetaInst::isMetaCall() { return false; }

    bool MetaInst::isSigned() { return sign; }

    bool MetaInst::isUnsigned() { return !sign; }

    MetaInst& MetaInst::setSigned(bool sign) {
        this->sign = sign;
        return *this;
    }

    MetaInst& MetaInst::setAddrGen(bool b){
        this->AddrGenFlag = b;
        return *this;
    }

    bool MetaInst::ifAddrGen(){
        return this->AddrGenFlag;
    }

    std::string MetaInst::toString() {
        std::string opList = operandList.size() == 0 ? "[]" : "[";
        for (MetaOperand* oprand : operandList) { if(oprand) opList = opList + std::to_string(oprand->getID()) + ","; }
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
            "\"isMetaCall\":" + "false" + "," +
            "\"isMetaPhi\":false,\"type\":" + MetaUtil::toString(type) + "," +
            "\"isAddrGen\":" + (ifAddrGen() ? "true" : "false") + "," +
            "\"operandList\":" + opList + "," + 
            "\"userList\":" + userList + "," + 
            "\"path\":" + path + "," +
            "\"hashCode\":" + std::to_string(hashCode) + "," + 
            "\"dataRoot\":\"" + dataRoot + "\"," + 
            "\"globalSymbolName\":\"" + globalSymbolName + "\"" + "," + 
            "\"isFake\":" + (isFake() ? "true" : "false") +
            "}"
            ;
        return str;
    }

    void MetaInst::setColor(int c, int t) {
        if (color.type != -1) {
            printf("WARN: Trying to change inst(id = %d) color from %d to %d! This operation will be ignored.\n", getID(), color.type, t);
            return;
        }
        color.color = c; color.type = t; 
    }

    ColorData* MetaInst::getColor() { return &color; }

    unsigned long MetaInst::getHashcode() {return hashCode;}

    uint64_t MetaInst::getAddress() {
        return address;
    }

    MetaInst& MetaInst::setAddress(uint64_t address) {
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

    MetaInst& MetaInst::setTypeSrc(std::vector<std::vector<int>> src){
        typeSrc = src;
        return (*this);
    }

    llvm::Instruction* MetaInst::getTransInst(){
        return this->TransInst;
    }

    void MetaInst::setTransInst(llvm::Instruction* inst){
        this->TransInst = inst;
    }

    bool MetaInst::fold() {
        if (operandList.size() != 1) {
            return false;
        }
        MetaOperand* origin = operandList.at(0);
        origin->removeUser(this);
        origin->addUsers(this->getUsers());
        for (auto user : users) {
            user->replaceOperand(this, origin);
        }
        this->erase();
        return true;
    }

    MetaInst&  MetaInst::removeOperand(MetaOperand* op) {
        for (auto it = operandList.begin(); it != operandList.end(); ) {
            if (*it == op) it = operandList.erase(it);
            else ++it;
        }
        return *this;
    }

    MetaInst&  MetaInst::replaceOperand(MetaOperand* src, MetaOperand* dest) {
        for (int i = 0; i < operandList.size(); ++i) {
            if (operandList[i] == src) operandList[i] = dest;
        }
        return *this;
    }


    MetaInst& MetaInst::erase() {
        MetaBB* parent = (MetaBB*)(this->getParentScope());

        parent->removeInst(this);
        unregisterFromMetaUnit();

        return *this;
    }

    MetaOperand* MetaInst::getOperand(int idx) { return operandList.at(idx); }


    bool MetaInst::isFake(){
        return this->fake;
    }

    MetaInst& MetaInst::setFake(){
        this->fake = true;
    }



//===-------------------------------------------------------------------------------===//
/// Meta Phi Instruction implementation.

    MetaPhi::MetaPhi() {
        type.push_back(InstType::PHI);
        setOriginInst("phi");
        MetaInst(type);
    }

    MetaPhi::MetaPhi(std::vector<InstType> ty) : MetaInst(ty) {
        setOriginInst("phi");
    }

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
            "\"isMetaCall\":false," + 
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

    MetaInst& MetaPhi::replaceOperand(MetaOperand* src, MetaOperand* dest) {
        MetaInst::replaceOperand(src, dest);
        for (auto it = bbValueMap.begin(); it != bbValueMap.end(); ++it) {
            if (it->second == src) bbValueMap[it->first] = dest;
        }
        return *this;
    }

    std::unordered_map<MetaBB*, MetaOperand*> MetaPhi::getMapping(){return this->bbValueMap;}


//===-------------------------------------------------------------------------------===//
/// Meta Basic Block implementation.

    MetaCall::MetaCall() : func(nullptr) {
        type.push_back(InstType::CALL);
    }

    MetaCall& MetaCall::setFuncName(std::string name) {
        funcName = name;
        return *this;
    }

    std::string MetaCall::getFuncName() {
        return funcName;
    }

    MetaCall& MetaCall::setMetaFunction(MetaFunction* mF) {
        func = mF;
        return *this;
    }

    MetaFunction* MetaCall::getMetaFunction() { return func; }

    bool MetaCall::isMetaCall() { return true; }

    bool MetaCall::isTailCall() { return tailCall; }

    MetaCall& MetaCall::setTailCall(bool flag) {
        tailCall = flag;
        return *this;
    }

    MetaInst& MetaCall::buildFromJSON(MetaUnitBuildContext& context) {
        llvm::json::Object JSON = context.getHoldObject();
        
        std::string     func                = JSON.getString("funcName").getValue().str();
        std::string     originInst          = JSON["originInst"]          .getAsString().getValue().str();
        std::string     dataRoot            = JSON["dataRoot"]            .getAsString().getValue().str();
        std::string     globalSymbolName    = JSON["globalSymbolName"]    .getAsString().getValue().str();
        unsigned long   hashCode            = JSON.getInteger("hashCode") .getValue();
        int64_t         id                  = JSON.getInteger("id")       .getValue();
        int64_t         addr                = JSON.getInteger("address")  .getValue();
        bool            isAddrGen           = JSON.getBoolean("isAddrGen").getValue();

        setFuncName(func);
        setOriginInst(originInst);
        setDataRoot(dataRoot);
        setGlobalSymbolName(globalSymbolName);
        setHashcode(hashCode);
        setID(id);
        setAddress(address);
        setAddrGen(isAddrGen);

        return *this; 
    }

    std::string MetaCall::toString() {
        std::string opList = operandList.size() == 0 ? "[]" : "[";
        for (MetaOperand* oprand : operandList) { if(oprand) opList = opList + std::to_string(oprand->getID()) + ","; }
        opList[opList.length() - 1] = ']'; 
        
        std::string userList = users.size() == 0 ? "[]" : "[";
        for (MetaOperand* user : users) { userList = userList + std::to_string(user->getID()) + ","; }
        userList[userList.length() - 1] = ']';

        std::string str = "{";
        return str + 
            "\"id\":" + std::to_string(id) + "," +
            "\"address\":" + std::to_string(MetaInst::getAddress()) + "," + 
            "\"originInst\":" + "\"" + getOriginInst() + "\"" + "," +
            "\"isMetaCall\":true," + 
            "\"isMetaPhi\":false," + 
            "\"type\":" + MetaUtil::toString(type) + "," +
            "\"isAddrGen\":" + (ifAddrGen() ? "true" : "false") + "," +
            "\"operandList\":" + opList + "," +
            "\"userList\":" + userList + "," + 
            "\"funcName\":" + "\"" + funcName + "\"" + "," +
            "\"hashCode\":" + std::to_string(hashCode) + "," + 
            "\"dataRoot\":\"" + dataRoot + "\"," + 
            "\"globalSymbolName\":\"" + globalSymbolName + "\"" +
            "}"
            ;
    }

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
        else if (ty[0] == InstType::CALL)
            newInst = new MetaCall();
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

    MetaInst* MetaBB::buildCall() {
        MetaInst* newInst = new MetaCall();
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
        next->addPrevBB(this);
        return *this;
    }


    MetaBB& MetaBB::addPrevBB(MetaBB* prev) {
        predecessors.push_back(prev);
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
                    assert(context.getMetaOperand(val));
                    assert(context.getMetaBB(key));
                    ((MetaPhi*)(instList[i]))->addValue(context.getMetaBB(key), context.getMetaOperand(val));
                }
            }
        }

        return *this;

    }

    MetaBB& MetaBB::buildInstFromJSON(MetaUnitBuildContext& context) {

        json::Array insts = context.getJsonArray("instList");
        
        for (auto iter = insts.begin(); iter != insts.end(); ++iter) {
            int64_t     inst_id     = (*iter).getAsObject()->getInteger("id").getValue();
            bool        isMetaPhi   = (*iter).getAsObject()->getBoolean("isMetaPhi").getValue();
            bool        isMetaCall  = (*iter).getAsObject()->getBoolean("isMetaCall").getValue();
            MetaInst*   newInst     = isMetaPhi ? buildPhi(true) : isMetaCall ? buildCall() : buildInstruction();
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

    std::vector<MetaBB*> MetaBB::getPrevBB() { return predecessors; }

    std::vector<int> MetaBB::getFeature() { return features; }

    MetaBB* MetaBB::getNextBB(int index) { return successors[index]; }

    MetaInst* MetaBB::getEntry() { return entry; }

    MetaInst* MetaBB::getTerminator() { return terminator; }

    std::vector<MetaInst*>& MetaBB::getInstList() { return instList; }

    int MetaBB::getNumInst() { return instList.size(); }

    int MetaBB::getNumLoad() { return features[0]; }

    int MetaBB::getNumStore() { return features[1]; }

    int MetaBB::getNumMemOp() { return features[0] + features[1]; }

    int MetaBB::isLinearInCFG() {

        return features[2] <= 1 && features[3] == 1;
    }

    int MetaBB::isSelfLoop() {
        for (MetaBB* suc : successors) if (suc == this) return 1;
        return 0;
    }

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
    
    double MetaBB::memOpSimilarity(MetaBB* bb) {
        std::vector<int> v = bb->getFeature();

        double mod_a = sqrt(features[0] * features[0] + features[1] * features[1]);
        double mod_b = sqrt(v[0] * v[0] + v[1] * v[1]);
        if (!mod_a && !mod_b) return 1.0;
        if (!mod_a || !mod_b) return 0.0;
        
        int dot = 0;
        for (int i = 0; i < 2; ++i) {
            dot += v[i] * features[i];
        }

        return dot / (mod_a * mod_b);
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

    MetaBB& MetaBB::swapSuccessors() {
        if (successors.size() < 2) return *this;
        std::swap(successors[0], successors[1]);
        return *this;
    }

    MetaBB& MetaBB::removeInst(MetaInst* inst) {
        for (auto it = inst_begin(); it != inst_end(); ++it) {
            if (*it != inst) continue;
            instList.erase(it);
            break;
        }
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
            context.saveContext().setCurScope(this).setHoldObject(iter->getAsObject());
            MetaConstant* c = new MetaConstant(context);
            addConstant(c);
            context.restoreContext();
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
            .setExit(context.getMetaBB(object["exitBB"].getAsInteger().getValue()))
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

    MetaFunction& MetaFunction::setExit(MetaBB* exitBB) {
        exit = exitBB;
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

    MetaBB* MetaFunction::getExit() { return exit; }

    std::vector<MetaBB*>& MetaFunction::getBB() { return bbs; }

    MetaArgument* MetaFunction::getArgument(int index) { return args.at(index); }

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
            "\"exitBB\":" + std::to_string(exit->getID()) + "," +
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
/// Addressing Mapping Table implementation.

    AddrMappingTable* AddrMappingTable::table = nullptr;

    AddrMappingTable::AddrMappingTable() {
        std::string home = getenv("HOME");
        std::string data = MetaUtil::readFromFile(home + "/Address.mapping");
        if (data.length() == 0) return;
        std::vector<std::string> lines = MetaUtil::split("\n", data);
        
        for (std::string line : lines) {
            std::vector<std::string> kv = MetaUtil::split(" : ", line);
            CodePiece asmCodes(kv[0]), irCodes(kv[1]);
            map[asmCodes.hashCode()] = { asmCodes, irCodes };
        }
    }


    AddrMappingTable::~AddrMappingTable() { }

	AddrMappingTable::AddrMappingTable(const AddrMappingTable&) { }

	AddrMappingTable& AddrMappingTable::operator=(const AddrMappingTable&) { }

    AddrMappingTable& AddrMappingTable::getInstanceRef() {
        return *getInstance();
    }

    // 单线程懒汉式
    AddrMappingTable* AddrMappingTable::getInstance() {
        if (AddrMappingTable::table == nullptr) AddrMappingTable::table = new AddrMappingTable();
        return AddrMappingTable::table;
    }

    AddrMappingTable& AddrMappingTable::update(const std::unordered_map<uint64_t, CodePiecePair>& m) {
        using iterator = std::unordered_map<uint64_t, CodePiecePair>::const_iterator;
        for (iterator it = m.begin(); it != m.end(); ++it) {
            map[it->first] = it->second;
        }
        return *this;
    }

    AddrMappingTable& AddrMappingTable::update(const std::vector<CodePiecePair>& pairs) {
        for (auto pair : pairs) map[pair.first.hashCode()] = pair;
        return *this;
    }

    AddrMappingTable& AddrMappingTable::addMapping(CodePiece asmCodes, CodePiece irCodes) {
        map[asmCodes.hashCode()] = { asmCodes, irCodes };
        return *this;
    }

    AddrMappingTable& AddrMappingTable::flush() {
        std::vector<std::string> lines;
        using iterator = std::unordered_map<uint64_t, CodePiecePair>::iterator;
        for (iterator it = map.begin(); it != map.end(); ++it) {
            CodePiecePair pair = it->second;
            std::string line = pair.first.toString() + " : " + pair.second.toString();
            lines.push_back(line);
        }
        std::string home = getenv("HOME");
        MetaUtil::writeToFile(MetaUtil::join("\n", lines), home + "/Address.mapping");
        return *this;
    }


//===-------------------------------------------------------------------------------===//
/// Mapping Table implementation.

    MappingTable* MappingTable::initTableMeta(){

        // std::ifstream file(this->MappingName[0]);
        // if(!file){
        //     std::ofstream new_file(this->MappingName[0]);
        //     new_file.close();
        //     std::cout << "Mapping Table " << this->MappingName[0] << "does not exist! Creating a new one! \n";
        //     return this;
        // }
        // // Check if the file was opened successfully
        // if (!file.is_open()) {
        //     std::cout << "Error: Could not open file " << this->MappingName[0] << std::endl;
        //     return NULL;
        // }

        // std::string line;
        // while (std::getline(file, line)) {
        // // Split the line into two parts - the string and the integer
        //     std::string::size_type pos = line.find(' ');
        //     if (pos == std::string::npos) {
        //         std::cerr << "Error: Invalid line format." << std::endl;
        //         continue;
        //     }
        //     std::string str = line.substr(0, pos);
        //     std::string int_str = line.substr(pos+1);

        //     // Convert the string to a std::string and the integer to an int
        //     int value;
        //     try {
        //         value = std::stoi(int_str);
        //     } catch (std::invalid_argument& e) {
        //         std::cerr << "Error: Invalid integer value." << std::endl;
        //         continue;
        //     } catch (std::out_of_range& e) {
        //         std::cerr << "Error: Integer value out of range." << std::endl;
        //         continue;
        //     }

        //     // Store the mapping in the map
        //     this->TableMeta[str] = value;
        // }

        int bitmap = 1;
        std::string  firstInst;
        for(int i = 1; i < this->max; i++){
            bitmap <<= i - 1;
            for(auto it = this->MTable[i].begin(); it!= this->MTable[i].end(); it++){
                if(i == 1){
                    firstInst = it->first;
                }
                else{
                     std::string::size_type pos =  it->first.find(' ');
                    if (pos == std::string::npos) {
                        std::cout << "DEBUG:: Incorrect formatting in MTable[" << i << "], Key ="
                                  << it->first << std::endl;
                        continue;
                    }
                    firstInst = it->first.substr(0, pos);
                }

                if(this->TableMeta.find(firstInst) != this->TableMeta.end())
                    TableMeta[firstInst] |= bitmap;
                else
                    TableMeta.insert (std::make_pair(firstInst, bitmap));
            }            
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
            std::string tableName = getTableName(i);
            std::ifstream file(tableName);
            if (!file) {
                std::ofstream new_file(tableName);
                new_file.close();
                std::cout << "Mapping Table " << tableName << " does not exist! Creating a new one! \n";
                continue;
            }
            // Check if the file was opened successfully
            if (!file.is_open()) {
                std::cerr << "Error: Could not open file "<< tableName << std::endl;
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
                str = MetaUtil::lower(str);
                std::string ir =  line.substr(pos+3);
                ir = MetaUtil::lower(ir);
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

        auto ptr = this->TableMeta.find(InstName);

        if(ptr != TableMeta.end())
            return ptr->second;
        else
            return 0;

    }

    MappingTable* MappingTable::setArch(std::string arch) {
        this->arch = arch;
        return this;
    }

    MappingTable* MappingTable::setPath(std::string path) {
        this->path = path;
        initName();
        return this;
    }

    MappingTable* MappingTable::initName() {
        this->MappingName.push_back("TableMeta.mapping");
        for(int i = 1; i <= this->max; i++)
            this->MappingName.push_back(std::to_string(i) + "-N.mapping");
        return this;
    }

    std::string MappingTable::getTableName(int id) {
        if(id > this->max){
            std::cout << "ERROR:: getTableName() exceeds the range of vector MappingName!\n";
            return "";
        }
        return path + arch + "." + MappingName[id];
    }


    std::string MappingTable::getTableMetaName() {
        return getTableName(0);
    }


    MappingTable* MappingTable::updateTableMeta(){

        std::ofstream file(getTableMetaName());
       
        for(auto pair: TableMeta){
            file << pair.first << " " << pair.second << std::endl;
        }
        return this;
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

        for (MetaOperand* operand : operands) {
            if (MetaCall* call = dynamic_cast<MetaCall*>(operand)) {
                for (auto func : funcs) {
                    if (func->getFunctionName() == call->getFuncName()) {
                        call->setMetaFunction(func);
                        break;
                    }
                }
            }
        }

            
    }

    MetaUnit::~MetaUnit() { }

    MetaUnit& MetaUnit::addFunc(MetaFunction* f) { funcs.push_back(f); return *this; }

    MetaUnit& MetaUnit::addGlobalVar(MetaConstant* c) { globalVar.push_back(c); return *this; }

    MetaUnit& MetaUnit::fillID(bool fillAddress) {
        int op_id = 0, scope_id = 0, inst_address = 0;
        for (auto operand : operands) {
            operand->setID(op_id++);
            printf("ID = %d, operand = %s\n", operand->getID(), operand->toString().c_str());
            if (fillAddress && operand->isMetaInst())
                ((MetaInst*)operand)->setAddress(inst_address++);
        }
        for (auto scope : scopes) scope->setID(scope_id++);
        return *this;
    }

    MetaUnit& MetaUnit::registerOperand(MetaOperand* operand) {
        operands.push_back(operand);
        if (operand->isMetaInst()) insts.push_back((MetaInst*)operand);
        return *this;
    }

    MetaUnit& MetaUnit::registerScope(MetaScope* scope) {
        scopes.push_back(scope);
        return *this;
    }

    MetaUnit& MetaUnit::unregisterOperand(MetaOperand* operand) {
        for (auto it = operands.begin(); it != operands.end(); ++it) {
            if (*it == operand) {
                operands.erase(it);
                break;
            }
        }

        if (!operand->isMetaInst()) return *this;

        for (auto it = insts.begin(); it != insts.end(); ++it) {
            if (*it == operand) {
                insts.erase(it);
                break;
            }
        }

        return *this;
    }

    MetaUnit& MetaUnit::unregisterScope(MetaScope* scope) {
        for (auto it = scopes.begin(); it != scopes.end(); ++it) {
            if (*it == scope) {
                scopes.erase(it);
                break;
            }
        }
        return *this;
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

        return str
            + "\"funcs\":" + funcStr + ","
            + "\"globalVar\":" + globalVarStr
            + "}"
            ;
    }


//===-------------------------------------------------------------------------------===//
/// MetaScope implementation.
    
    MetaScope::MetaScope() : parent(nullptr), id(-1) { }

    MetaScope::~MetaScope() { }

    int MetaScope::getID() { return id; }

    MetaScope& MetaScope::setParentScope(MetaScope* scope) { parent = scope; return *this; }

    MetaScope& MetaScope::setID(int id) { this->id = id; return *this; }

    MetaScope* MetaScope::getParentScope() { return parent; }

    MetaScope* MetaScope::getRootScope() {
        if (parent == nullptr) return this;
        return parent->getRootScope();
    }


//===-------------------------------------------------------------------------------===//
/// CodePiece implementation.

CodePiece::CodePiece() { }

CodePiece::CodePiece(std::string s) {
    instList = MetaUtil::split(" ", s);
}

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

std::string CodePiece::toString() {
    return MetaUtil::join(" ", instList);
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

//===-------------------------------------------------------------------------------===//



} // end namespace MetaTrans
