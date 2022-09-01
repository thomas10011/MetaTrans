
#include "llvm/Support/raw_ostream.h"
#include "MetaTrans.h"


namespace MetaTrans { 

    template<class T>
    class MetaVertex {
        private:
        protected:
            std::vector<MetaVertex<T>*> adjacency;
            T* data;

        public:
            /*
             * create a null vertex
             */
            MetaVertex() {

            }

            MetaVertex(T* t) {
                data = t;
            }

            void addAdjacency(MetaVertex<T>* next) {
                adjacency.push_back(next);
            }

            T* getData() {
                return data;
            }
    };


    template<class T>
    class MetaGraph {
        private:
        protected:
            std::vector<MetaVertex<T>*> vertex;

        public:
            MetaGraph() {

            }

            MetaVertex<T>* addVertex(T* data) {
                MetaVertex<T>* ptr = new MetaVertex<T>(data);
                vertex.push_back(ptr);
                return ptr;
            }

            void addEdge(MetaVertex<T>* from, MetaVertex<T>* to) {
                from->addAdjacency(to);
            }

    };
    
//===-------------------------------------------------------------------------------===//
/// Meta Function pass implementation, Will be invoked by LLVM pass manager.

    MetaFunctionPass::MetaFunctionPass() : FunctionPass(MetaFunctionPass::ID) {
                    
    }

    bool MetaFunctionPass::runOnFunction(Function & F) {
        
        outs() << "running MetaInst pass on function " << F.getName() << " ... " << "\n";

        createMetaElements(F);

        // construct graph
        mf.setRoot(bbMap.find(&*F.begin())->second);
        for (Function::iterator bb = F.begin(); bb != F.end(); ++bb) {
            // we create graph for each basic block
            MetaBB* curBB = bbMap.find(&*bb)->second;
            
            for (BasicBlock::iterator i = bb->begin(); i != bb->end(); ++i) {
                // add this instruction to current basic block.
                MetaInst* curInst = instMap.find(&*i)->second;
                curInst->processOperand(&*i, curBB, mf, *this);
            }
        }
        
        for (auto bb = mf.bb_begin(); bb != mf.bb_end(); ++bb) {
            outs() << "successor amount of " << *bb << " is " << (*bb)->getNextBB().size() << "\n";
            MetaUtil::printInstDependencyGraph(*bb);
        }
        
        outs() << "\n";
        return true;
    }

    bool MetaFunctionPass::createMetaArg(Argument* arg) {
        auto pair = argMap.find(arg);
        if (pair != argMap.end()) return false;
        MetaArgument* metaArg = new MetaArgument();
        mf.addArgument(metaArg);
        assert(argMap.insert({arg, metaArg}).second);
        return true;
    }

    bool MetaFunctionPass::createMetaConstant(Constant* c) {
        auto pair = constantMap.find(c);
        if (pair != constantMap.end()) return false;
        MetaConstant* metaCons = new MetaConstant();
        mf.addConstant(metaCons);
        assert(constantMap.insert({c, metaCons}).second);
        return true;
    }

    void MetaFunctionPass::createMetaElements(Function& F) {
        // create all meta basic block and instructions.
        for (auto bb = F.begin(); bb != F.end(); ++bb) {
            MetaBB* newBB = mf.buildBB();
            assert(bbMap.insert({&*bb, newBB}).second);
            bool first_non_phi = true;
            outs() << "creating instructions in Basic Block " << *bb;
            for (auto i = bb->begin(); i != bb->end(); ++i) {
                outs() << "instruction with type: " << i->getOpcodeName() << "\n";
                MetaInst* newInst = newBB->buildInstruction(getInstType(&*i));
                assert(instMap.insert({&*i, newInst}).second);
                if (first_non_phi && i->getOpcode() != Instruction::PHI) {
                    newBB->setEntry(newInst); first_non_phi = false;
                }

                // create all arg and constant.
                for (auto op = i->op_begin(); op != i->op_end(); ++op) {
                    Value* value = op->get();
                    if (Argument* arg = dyn_cast<Argument>(value)) {
                        createMetaArg(arg);
                    }
                    else if (Constant* c = dyn_cast<Constant>(value)) {
                        createMetaConstant(c);
                    }
                }

            }
        }
    }

    // determine the type of a instruction. refer to Instruction.h
    std::vector<InstType> MetaFunctionPass::getInstType(Instruction* inst) {
        std::vector<InstType> ty;
        switch (inst->getOpcode()) {
            // Terminators
            case Instruction::Ret:   
            case Instruction::Br:
            case Instruction::Switch:
            case Instruction::IndirectBr:
            case Instruction::Invoke:
            case Instruction::Resume:
            case Instruction::Unreachable:
            case Instruction::CleanupRet:
            case Instruction::CatchRet:
            case Instruction::CatchPad:
            case Instruction::CatchSwitch:
            case Instruction::CallBr:
                ty.push_back(InstType::JUMP);
                break;

            // Standard unary operators...
            case Instruction::FNeg:
                ty.push_back(InstType::NEG);
                break;

            // Standard binary operators...
            case Instruction::Add:
            case Instruction::FAdd:
                ty.push_back(InstType::ADD);
                break;
            case Instruction::Sub:
            case Instruction::FSub:
                ty.push_back(InstType::SUB);
                break;
            case Instruction::Mul:
            case Instruction::FMul:
                ty.push_back(InstType::MUL);
                break;
            case Instruction::UDiv:
            case Instruction::SDiv:
            case Instruction::FDiv:
                ty.push_back(InstType::DIV);
                break;
            case Instruction::URem:
            case Instruction::SRem:
            case Instruction::FRem:
                ty.push_back(InstType::REMAINDER);
                break;

            // Logical operators...
            case Instruction::And:
                ty.push_back(InstType::AND);
                break;
            case Instruction::Or :
                ty.push_back(InstType::OR);
                break;
            case Instruction::Xor:
                ty.push_back(InstType::XOR);
                break;

            // Memory instructions...
            case Instruction::Alloca:
                ty.push_back(InstType::ALLOCATION);
                break;
            case Instruction::Load:
                ty.push_back(InstType::LOAD);
                break;
            case Instruction::Store:
                ty.push_back(InstType::STORE);
                break;
            case Instruction::AtomicCmpXchg:
            case Instruction::AtomicRMW:
            case Instruction::Fence:
            case Instruction::GetElementPtr:
                break;

            // Convert instructions...
            case Instruction::Trunc:
            case Instruction::ZExt:
            case Instruction::SExt:
            case Instruction::FPTrunc:
            case Instruction::FPExt:
            case Instruction::FPToUI:
            case Instruction::FPToSI:
            case Instruction::UIToFP:
            case Instruction::SIToFP:
            case Instruction::IntToPtr:
            case Instruction::PtrToInt:
            case Instruction::BitCast:
            case Instruction::AddrSpaceCast:
                break;
            
            // Other instructions...
            case Instruction::ICmp:
            case Instruction::FCmp:
                ty.push_back(InstType::COMPARE);
                break;
            case Instruction::PHI:
                ty.push_back(InstType::PHI);
                break;
            case Instruction::Select:
                break;
            case Instruction::Call:
                ty.push_back(InstType::JUMP);
                break;
            case Instruction::Shl:
            case Instruction::LShr:
            case Instruction::AShr:
                ty.push_back(InstType::SHIFT);
                break;
            case Instruction::VAArg:
            case Instruction::ExtractElement:
            case Instruction::InsertElement:
            case Instruction::ShuffleVector:
            case Instruction::ExtractValue:
            case Instruction::InsertValue:
            case Instruction::LandingPad:
            case Instruction::CleanupPad:
            case Instruction::Freeze:
                break;

            default:
                break;
        }

        return ty;

    } 

//===-------------------------------------------------------------------------------===//
/// Meta Util implementation.

    // this function is used to print the subclass of a Value in INSTRUCTION level.
    void MetaUtil::printValueType(Value* value) {
        outs() << "value address: " << value << ";";
        if (dyn_cast<Argument>(value)) { 
            outs() << " real type: Argument";
        }
        else if (dyn_cast<BasicBlock>(value)) {
            outs() << " real type: BasicBlock";
        }
        else if (dyn_cast<InlineAsm>(value)) {
            outs() << " real type: nlineAsm";
        }
        else if (dyn_cast<MetadataAsValue>(value)) {
            outs() << " real type: MetadataAsValue";
        }
        else if (dyn_cast<Constant>(value)) {
            outs() << " real type: Constant";
        }
        else if (dyn_cast<MemoryAccess>(value)) {
            outs() << " real type: MemoryAccess";
        }
        else if (dyn_cast<Instruction>(value)) {
            outs() << " real type: Instruction";
        }
        else if (dyn_cast<Operator>(value)) {
            outs() << " real type: Operator";
        }
    }
    
    void MetaUtil::printInstDependencyGraph(MetaBB* bb) {
        std::vector<MetaInst*> instList = bb->getInstList();
        std::unordered_map<MetaOperand*, int> deg;
        std::unordered_map<MetaOperand*, int> op_num;
        for (auto iter = instList.begin(); iter != instList.end(); ++iter) {
            deg[*iter] = 0; op_num[*iter] = (*iter)->getOperandNum();
        }
        for (auto iter = instList.begin(); iter != instList.end(); ++iter) {
            std::vector<MetaOperand*> operandList = (*iter)->getOperandList();
            for (auto op_iter = operandList.begin(); op_iter != operandList.end(); ++op_iter) {
                deg[*op_iter]++;
            }
        }
        for (auto iter = instList.begin(); iter != instList.end(); ++iter) {
            outs() << "degree of " << MetaUtil::typeToString((*iter)->getInstType()) << " is " << deg[*iter] << "; oprand number is " << op_num[*iter] << '\n';
        } 

    }

    void MetaUtil::printInstOperand(Instruction* inst) {

    }

    template<typename T>
    std::string MetaUtil::typeToString(std::vector<T> type_vector) {
        static_assert(
            std::is_same<T, InstType>::value ||
            std::is_same<T, DataType>::value ,
            "Type of template not support!"
        );
        std::string s = "{ " + MetaUtil::toString(type_vector[0]);
        for (int i = 1; i < type_vector.size(); i++) { s += ", " + type_vector[i]; }
        s += " }";
        return s;
    }

    std::string MetaUtil::toString(DataType type) {
        switch (type) {
            case DataType::INT8: return "INT8";
            case DataType::INT16: return "INT16";
            case DataType::INT32: return "INT32";
            case DataType::INT64: return "INT64";
            case DataType::FLOAT: return "FLOAT";
            case DataType::DOUBLE: return "DOUBLE";
        }
    }

    std::string MetaUtil::toString(InstType type) {
        switch (type) {
            case InstType::NONE: return "none";
            case InstType::LOAD: return "load";
            case InstType::STORE: return "store";
            case InstType::COMPARE: return "compare";
            case InstType::CALL: return "call";
            case InstType::BRANCH: return "branch";
            case InstType::JUMP: return "jump";
            case InstType::PHI: return "phi";
            case InstType::ADD: return "add";
            case InstType::SUB: return "sub";
            case InstType::MUL: return "mul";
            case InstType::DIV: return "div";
            case InstType::REMAINDER: return "remainder";
            case InstType::AND: return "and";
            case InstType::OR: return "or";
            case InstType::XOR: return "xor";
            case InstType::SHIFT: return "shift";
            case InstType::NEG: return "neg";
            case InstType::RET: return "ret";
            case InstType::ALLOCATION: return "allocation";
        }
    }

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

    void MetaInst::processOperand (
        Instruction* curInst, MetaBB* curBB, MetaFunction& f, 
        MetaFunctionPass& pass
    ) {
        unsigned num_op = curInst->getNumOperands();
        outs() << "inst type: " << curInst->getOpcodeName() << "\n";
        for (auto op = curInst->op_begin(); op != curInst->op_end(); ++op, --num_op) {
            Value* value = op->get();
            MetaUtil::printValueType(value);
            if (Argument* arg = dyn_cast<Argument>(value)) {
                MetaArgument* metaArg = pass.argMap.find(arg)->second;
                addOperand((MetaOperand*)metaArg);
            }
            // create CFG here.
            else if (BasicBlock* bb = dyn_cast<BasicBlock>(value)) {
                auto it = pass.bbMap.find(bb);
                curBB->addNextBB(it->second);
                curBB->setTerminator(this);
            }
            else if (Constant* c = dyn_cast<Constant>(value)) {
                MetaConstant* metaCons = pass.constantMap.find(c)->second;
                addOperand((MetaOperand*)metaCons);
            }
            else if (Instruction* inst = dyn_cast<Instruction>(value)) {
                MetaInst* usedInst = pass.instMap.find(inst)->second;
                addOperand((MetaOperand*)usedInst);
            }
            outs() << "; operand number: " << op->getOperandNo() << "#" << "\n";
        }
        assert(!num_op);
    }

    MetaInst* MetaInst::createMetaInst(std::vector<InstType> ty) {
        if (ty[0] == InstType::PHI)
            return new MetaPhi(ty);
        else 
            return new MetaInst(ty);
    }

//===-------------------------------------------------------------------------------===//
/// Meta Phi Instruction implementation.

    MetaPhi::MetaPhi(std::vector<InstType> ty) : MetaInst(ty) { }

    void MetaPhi::processOperand(
        Instruction* curInst, MetaBB* curBB, MetaFunction& f, 
        MetaFunctionPass& pass
    ) {
        // invoke parent class's method first.
        MetaInst::processOperand(curInst, curBB, f, pass);
        outs() << "processing meta phi node's operands..." << "\n";
        auto phi = dyn_cast<PHINode>(curInst);
        for (auto bb = phi->block_begin(); bb != phi->block_end(); ++bb) {
            Value* value = phi->getIncomingValueForBlock(*bb);
            MetaOperand* op = nullptr;
            if (Argument* arg = dyn_cast<Argument>(value)) {
                op = (MetaOperand*)pass.argMap.find(arg)->second;
            }
            else if (Constant* c = dyn_cast<Constant>(value)) {
                op = (MetaOperand*)pass.constantMap.find(c)->second;
            }
            else if (Instruction* i = dyn_cast<Instruction>(value)) {
                op = (MetaOperand*)pass.instMap.find(i)->second;
            }
            assert(op);
            addValue(pass.bbMap.find(*bb)->second, op);
            outs() << "basic block: "<< bb << "; value: " << value << "; ";
        }
        outs() << "\n";
        
        for (auto op = curInst->op_begin(); op != curInst->op_end(); ++op) {
            Value* value = op->get();
            MetaUtil::printValueType(value); 
            outs() << "; operand number: " << op->getOperandNo() << "#" << "\n";
        }
    }

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

    void MetaBB::addInstruction(MetaInst* inst) {
        instList.push_back(inst);
    }

    void MetaBB::addNextBB(MetaBB* next) { successors.push_back(next); }

    std::vector<MetaBB*> MetaBB::getNextBB() { return successors; }

    MetaBB* MetaBB::getNextBB(int index) { return successors[index]; }

    void MetaBB::setEntry(MetaInst* inst) { entry = inst; }

    MetaInst* MetaBB::getEntry() { return entry; }

    void MetaBB::setTerminator(MetaInst* inst) { terminator = inst; }

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

char MetaTrans::MetaFunctionPass::ID = 0;


static RegisterPass<MetaTrans::MetaFunctionPass> X("meta-trans", "MetaTrans Pass",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);