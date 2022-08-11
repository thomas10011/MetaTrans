
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
    
    
    struct MetaInstPass : FunctionPass {

        static char ID;

        /* data */
        MetaInstPass() : FunctionPass(ID) {
                       
        }
        
        // record the reflection between primitive type and Meta type.
        std::unordered_map<BasicBlock*, MetaBB*> bbMap;
        std::unordered_map<Instruction*, MetaInst*> instMap;
        std::unordered_map<Constant*, MetaConstant*> constantMap;
        std::unordered_map<Argument*, MetaArgument*> argMap;

        bool runOnFunction(Function & F) override {
            
            outs() << "running MetaInst pass on function " << F.getName() << " ... " << "\n";

            MetaFunction mf;
            createMetaElements(F, mf);

            // construct graph
            mf.setRoot(bbMap.find(&*F.begin())->second);
            for (Function::iterator bb = F.begin(); bb != F.end(); ++bb) {
                // we create graph for each basic block
                MetaBB* curBB = bbMap.find(&*bb)->second;
                
                for (BasicBlock::iterator i = bb->begin(); i != bb->end(); ++i) {
                    // add this instruction to current basic block.
                    MetaInst* curInst = instMap.find(&*i)->second;
                    outs() << "going to print values..." << "\n";
                    curInst->processOperand(&*i, curBB, mf, *this);
                }
            }
            
            for (auto bb = mf.begin(); bb != mf.end(); ++bb) {
                outs() << "successor amount of " << *bb << " is " << (*bb)->getNextBB().size() << "\n";
            }
            
            outs() << "\n";
            return true;
        }

        void createMetaElements(Function& F, MetaFunction& mf) {
            // create all meta basic block and instructions.
            for (auto bb = F.begin(); bb != F.end(); ++bb) {
                MetaBB* newBB = mf.buildBB();
                bbMap.insert({&*bb, newBB});
                bool first_non_phi = true;
                for (auto i = bb->begin(); i != bb->end(); ++i) {
                    MetaInst* newInst = MetaInst::createMetaInst(getInstType(&*i));
                    if (i->getOpcode() != Instruction::PHI && first_non_phi) {
                        newBB->setEntry(newInst);
                        first_non_phi = false;
                    }
                    newBB->addInstruction(newInst);
                    instMap.insert({&*i, newInst});

                    // create all arg and constant.
                    for (auto op = i->op_begin(); op != i->op_end(); ++op) {
                        Value* value = op->get();
                        if (Argument* arg = dyn_cast<Argument>(value)) {
                            auto pair = argMap.find(arg);
                            if (pair != argMap.end()) break;
                            MetaArgument* metaArg = new MetaArgument();
                            mf.addArgument(metaArg);
                            argMap.insert({arg, metaArg});
                        }
                        else if (Constant* c = dyn_cast<Constant>(value)) {
                            auto pair = constantMap.find(c);
                            if (pair != constantMap.end()) break;
                            MetaConstant* metaCons = new MetaConstant();
                            mf.addConstant(metaCons);
                            constantMap.insert({c, metaCons});
                        }
                    }
                }
            }
        }
        
        // determine the type of a instruction. referenced Instruction.h
        std::vector<InstType> getInstType(Instruction* inst) {
            std::vector<InstType> ty;
            outs() << "type of this instruction is: " << inst->getOpcodeName() << "\n";
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

        // this function is used to print the subclass of a Value in INSTRUCTION level.
        void printType(Value* value) {
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

    };

    MetaInst* MetaInst::createMetaInst(std::vector<InstType> ty) {
        if (ty[0] == InstType::PHI)
            return new MetaPhi(ty);
        else 
            return new MetaInst(ty);
    }

    void MetaInst::processOperand(
        Instruction* curInst, MetaBB* curBB, MetaFunction& f, 
        MetaInstPass& pass
    ) {
        for (auto op = curInst->op_begin(); op != curInst->op_end(); ++op) {
            Value* value = op->get();
            pass.printType(value);
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
    }

    void MetaPhi::processOperand(
        Instruction* curInst, MetaBB* curBB, MetaFunction& f, 
        MetaInstPass& pass
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
            pass.printType(value); 
            outs() << "; operand number: " << op->getOperandNo() << "#" << "\n";
        }
    }

}

char MetaTrans::MetaInstPass::ID = 0;


static RegisterPass<MetaTrans::MetaInstPass> X("meta-trans", "MetaTrans Pass",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);