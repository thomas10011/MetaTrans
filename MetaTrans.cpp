
#include "llvm/Support/raw_ostream.h"
#include "meta/MetaTrans.h"



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
/// Meta Constant implementation.

    MetaConstant::MetaConstant() { }

    MetaConstant::MetaConstant(DataType ty) : type(ty) { }

    void MetaConstant::setDataType(DataType ty) { type = ty; }

    DataType MetaConstant::getDataType() { return type; }

    DataUnion MetaConstant::getValue() { return value; }

    void MetaConstant::setValue(int8_t v) {
        if (type != DataType::INT8) return;
        value.int_8_val = v; 
    }

    void MetaConstant::setValue(int16_t v) {
        if (type != DataType::INT16) return;
        value.int_16_val = v; 
    }
    
    void MetaConstant::setValue(int32_t v) { 
        if (type != DataType::INT32) return;
        value.int_32_val = v; 
    }

    void MetaConstant::setValue(int64_t v) { 
        if (type != DataType::INT64) return;
        value.int_64_val = v; 
    }

    void MetaConstant::setValue(float v) {
        if (type != DataType::FLOAT) return;
        value.float_val = v; 
    }
    
    void MetaConstant::setValue(double v) { 
        if (type != DataType::DOUBLE) return;
        value.double_val = v; 
    }

//===-------------------------------------------------------------------------------===//
/// Meta Argument implementation.

    MetaArgument::MetaArgument() { }

    MetaArgument::MetaArgument(DataType ty) : type(ty) { }

    void MetaArgument::setArgIndex(int i) { argIndex = i; }

    int MetaArgument::getArgIndex() { return argIndex; }

    void MetaArgument::setArgType(DataType ty) { type = ty; }

    DataType MetaArgument::getArgType() { return type; }

//===-------------------------------------------------------------------------------===//
/// Meta Instruction implementation.

    MetaInst::MetaInst() { }

    MetaInst::MetaInst(std::vector<InstType> ty) : type(ty) { }

    MetaInst::~MetaInst() { }

    void MetaInst::addOperand(MetaOperand* op) {
        operandList.push_back(op);
    }

    int MetaInst::getOperandNum() { return operandList.size(); }

    std::vector<InstType> MetaInst::getInstType() { return type; }

    std::vector<MetaOperand*>& MetaInst::getOperandList() { return operandList; }

    std::vector<MetaOperand*>::iterator MetaInst::op_begin() { return operandList.begin(); }

    std::vector<MetaOperand*>::iterator MetaInst::op_end() { return operandList.end(); }

    MetaInst* MetaInst::createMetaInst(std::vector<InstType> ty) {
        if (ty[0] == InstType::PHI)
            return new MetaPhi(ty);
        else 
            return new MetaInst(ty);
    }

//===-------------------------------------------------------------------------------===//
/// Meta Phi Instruction implementation.

    MetaPhi::MetaPhi(std::vector<InstType> ty) : MetaInst(ty) { }


    void MetaPhi::addValue(MetaBB* bb, MetaOperand* op) {
        bbValueMap.insert({bb, op});
    }

    MetaOperand* MetaPhi::getValue(MetaBB* bb) {
        auto pair = bbValueMap.find(bb);
        if (pair == bbValueMap.end()) return nullptr;
        return pair->second;
    }

//===-------------------------------------------------------------------------------===//
/// Meta Basic Block implementation.

    MetaBB::MetaBB(MetaFunction* f) : entry(nullptr), terminator(nullptr), parent(f) { }

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

    MetaBB& MetaBB::addInstruction(MetaInst* inst) {
        instList.push_back(inst);
        return *this;
    }

    MetaBB& MetaBB::addNextBB(MetaBB* next) {
        successors.push_back(next);
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

    std::vector<MetaBB*> MetaBB::getNextBB() { return successors; }

    MetaBB* MetaBB::getNextBB(int index) { return successors[index]; }

    MetaInst* MetaBB::getEntry() { return entry; }

    MetaInst* MetaBB::getTerminator() { return terminator; }

    std::vector<MetaInst*>& MetaBB::getInstList() { return instList; }

    int MetaBB::getInstNum() { return instList.size(); }

    std::vector<MetaInst*>::iterator MetaBB::inst_begin() { return instList.begin(); }

    std::vector<MetaInst*>::iterator MetaBB::inst_end() { return instList.end(); }

//===-------------------------------------------------------------------------------===//
/// Meta Function implementation.

    void MetaFunction::addConstant(MetaConstant* c) {
        constants.insert(c);
    }
    
    void MetaFunction::addArgument(MetaArgument* a) {
        args.push_back(a);
    }

    int MetaFunction::getArgNum() { return args.size(); }

    int MetaFunction::getConstNum() { return constants.size(); }

    MetaArgument* MetaFunction::getArgument(int index) { return args[index]; }

    // create a new bb at the end of bb list.
    MetaBB* MetaFunction::buildBB() {
        MetaBB* newBB = new MetaBB(this);
        bbs.push_back(newBB);
        return newBB;
    }

    void MetaFunction::setRoot(MetaBB* rootBB) {
        root = rootBB;
    }

    std::vector<MetaBB*>::iterator MetaFunction::bb_begin() { return bbs.begin(); }

    std::vector<MetaBB*>::iterator MetaFunction::bb_end() { return bbs.end(); }

    std::vector<MetaArgument*>::iterator MetaFunction::arg_begin() { return args.begin(); }

    std::vector<MetaArgument*>::iterator MetaFunction::arg_end() { return args.end(); }

}