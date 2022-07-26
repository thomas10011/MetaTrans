#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/JSON.h"
#include <math.h>
#include "meta/MetaTrans.h"
#include "meta/MetaUtils.h"

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

    MetaOperand& MetaOperand::setID(int id) { this->id = id; return* this; }

    int MetaOperand::getID() { return id; }

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

    MetaInst::MetaInst() : paths(3) { }

    MetaInst::MetaInst(std::vector<InstType> ty) : type(ty) { }

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
        return *this;
    }

    MetaInst& MetaInst::buildFromJSON(llvm::json::Object JSON, std::unordered_map<int64_t, MetaBB*>& tempBBMap, std::unordered_map<int64_t, MetaOperand*>& tempOperandMap) {
        json::Array& ops = *(JSON["type"].getAsArray());
        std::string originInst = JSON["originInst"].getAsString().getValue().str();
        (*this).setOriginInst(originInst);
        for (int i = 0; i < ops.size(); ++i) {
            std::string op = ops[i].getAsString().getValue().str();
            addInstType(MetaUtil::stringToInstType(op));
        }
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
                    if (i == 1 && paths[0]) path += ",";
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
        std::string str = "";
        str = str + "{" + "\"id\":" + std::to_string(id) + ",\"originInst\":" + "\"" +
            originInst + "\"" +
            ",\"isMetaPhi\":false,\"type\":" + MetaUtil::toString(type) + "," +
            "\"operandList\":" + opList + "," + "\"path\":" + path + "}";
        std::cout << str << std::endl;
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

    std::vector<Path *>& MetaInst::getAllPath() { return paths; }

    Path* MetaInst::getPath(int type) { return paths[type]; }

    void MetaInst::dumpPath(int index) {
        printf("%x, type: %d, numLoad: %d, numStore: "
            "%d, numPHI: %d, numGEP: %d\n",
            paths[index]->firstNode, paths[index]->type, paths[index]->numLoad,
            paths[index]->numStore, paths[index]->numPHI, paths[index]->numGEP);
    }

    void MetaInst::addToPath(Path* p) {
        paths.push_back(p);
    }

    std::vector<MetaInst *> MetaInst::findTheSameInst(MetaBB *bb) {
        // Find the instruction has same path: each /data compute/addressing/control flow/ path has the same numLoad, numStore, numPHI, numGEP
        std::vector<MetaInst *> ans;
        for (auto it = bb->inst_begin(); it != bb->inst_end(); it++) {
            MetaInst *inst = *it;
            std::vector<Path *> anotherPath = inst->getAllPath();
            std::cout << inst->toString() << std::endl;
            if (inst->isType(type[0])) {
                if(type[0] == InstType::BRANCH) {
                  inst->dumpPath(2);
                  if (*(paths[2]) == *(anotherPath[2])) { // control flow
                    std::cout << "findTheSamePath" << std::endl;
                    ans.push_back(inst);
                  }
                }else {
                    inst->dumpPath(1);
                    if(*(paths[1]) == *(anotherPath[1])) { // addressing
                        if(type[0] == InstType::STORE) {
                            inst->dumpPath(0);
                            if(*(paths[0]) == *(anotherPath[0])) { // data compute
                                std::cout << "findTheSamePath" << std::endl;
                                ans.push_back(inst);
                            }
                        }else {
                            std::cout << "findTheSamePath" << std::endl;
                            ans.push_back(inst);
                        }
                    }
                }
            }
        }
        std::cout << "Exit findTheSameInst" << std::endl;
        return ans;
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

    MetaBB& MetaBB::buildFromJSON(json::Object JSON, std::unordered_map<int64_t, MetaBB*>& tempBBMap, std::unordered_map<int64_t, MetaOperand*>& tempOperandMap) {

        json::Array& insts = *(JSON["instList"].getAsArray());

        for (auto iter = insts.begin(); iter != insts.end(); ++iter) {
            int64_t inst_id = (*iter).getAsObject()->getInteger("id").getValue();
            bool isMetaPhi = (*iter).getAsObject()->getBoolean("isMetaPhi").getValue();
            MetaInst& newInst = isMetaPhi ? *buildPhi(true) : *buildInstruction();
            tempOperandMap[inst_id] = &newInst;
            // 构建 Meta Instruction
            newInst
                .setParent(this)
                .setID(inst_id)
                ;
        }

        for (int i = 0; i < insts.size(); ++i) {
            json::Object& inst = *(insts[i].getAsObject());
            json::Array& ops = *(inst.getArray("operandList"));
            
            // 连上 Meta Instruction 之间的边
            for (int j = 0; j < ops.size(); ++j) {
                int64_t op_id = ops[j].getAsInteger().getValue();
                instList[i]->addOperand(tempOperandMap[op_id]);
            }
            instList[i]->buildFromJSON(*(insts[i].getAsObject()), tempBBMap, tempOperandMap);
        }

        (*this)
            .addFeature(JSON["load"].getAsInteger().getValue())
            .addFeature(JSON["store"].getAsInteger().getValue())
            .addFeature(JSON["in"].getAsInteger().getValue())
            .addFeature(JSON["out"].getAsInteger().getValue())
            .setEntry((MetaInst*)tempOperandMap[JSON["entry"].getAsInteger().getValue()])
            .setTerminator((MetaInst*)tempOperandMap[JSON["terminator"].getAsInteger().getValue()])
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

    int MetaBB::getID() { return id; }

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
            // 递归构建 Meta BB
            bbs[i]->buildFromJSON(*(blocks[i].getAsObject()), tempBBMap, tempOperandMap);
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
        std::cout << JSON << std::endl;
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