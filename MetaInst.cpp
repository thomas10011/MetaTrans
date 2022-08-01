#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/DerivedUser.h"
#include "llvm/Analysis/MemorySSA.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/Operator.h"
#include <vector>
#include <list>
#include <unordered_map>
#include <unordered_set>


using namespace llvm;

namespace { 

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

    class MetaOperand {

    };

    class MetaInst;
    class MetaBB;
    class MetaFunction;
    struct MetaInstPass;

    enum InstType {
        // NONE represent a Non-Instruction operand.
        NONE,
        LOAD,
        STORE,
        COMPARE,
        CALL,
        BRANCH,
        JUMP,
        // phi node.
        PHI,
        ADD,
        SUB,
        MUL,
        DIV,
        REMAINDER,
        AND,
        OR,
        XOR,
        SHIFT,
        NEG,
        RET,
        // represent allocation for a data.
        ALLOCATION
    };

    class MetaConstant : public MetaOperand {

    };


    class MetaArgument : public MetaOperand {

    };

    class MetaInst : public MetaOperand {
        private:
        protected:

            // a vector to indicate the real type of a instruction.
            std::vector<InstType> type;
            std::vector<MetaOperand*> operandList;

        public:
            MetaInst() { }

            MetaInst(std::vector<InstType> ty) : type(ty) {

            }

            void addOperand(MetaOperand* op) {
                operandList.push_back(op);
            }

            virtual void processOperand (
                Instruction* curInst, MetaBB* curBB, MetaFunction& f, 
                MetaInstPass& pass
            );

            static MetaInst* createMetaInst(std::vector<InstType> ty);



    };
    
    // represent a phi node.
    class MetaPhi : public MetaInst {
        private:
        protected:

            std::unordered_map<MetaBB*, MetaOperand*> bbValueMap;

        public:

            MetaPhi(std::vector<InstType> ty) : MetaInst(ty) { }

            virtual void processOperand (
                Instruction* curInst, MetaBB* curBB, MetaFunction& f, 
                MetaInstPass& pass
            ) override;

            void addValue(MetaBB* bb, MetaOperand* op) {
                bbValueMap.insert({bb, op});
            }

            MetaOperand* getValue(MetaBB* bb) {
                auto pair = bbValueMap.find(bb);
                if (pair == bbValueMap.end()) return nullptr;
                return pair->second;
            }
    };
    

    class MetaBB {
        private:
        protected:

        std::vector<MetaInst*> instList;
        
        // point to next BB if exists.
        MetaBB* nextBB;
        // possibly a phi node.
        MetaInst* entry;
        // each bb end with a terminator.
        MetaInst* terminator;
        
        // record parent scope
        const MetaFunction* parent;

        public:

        MetaBB(MetaFunction* f) : nextBB(nullptr), entry(nullptr), terminator(nullptr), parent(f) { }

        ~MetaBB() {
            for (auto inst : instList) {
                delete inst;
            }
        }

        MetaInst* addInstruction(std::vector<InstType> ty) {
            MetaInst* newInst = new MetaInst();
            instList.push_back(newInst);
            return newInst;
        }

        void addInstruction(MetaInst* inst) {
            instList.push_back(inst);
        }

        void setNextBB(MetaBB* next) { nextBB = next; }

        MetaBB* getNextBB() { return nextBB; }

        void setEnrty(MetaInst* inst) { entry = inst; }

        MetaInst* getEntry() { return entry; }

        void setTerminator(MetaInst* inst) { terminator = inst; }

        MetaInst* getTerminator() { return terminator; }


    };

    class MetaFunction {
        private:
        protected:

        // a function should contains a set of constants.
        std::unordered_set<MetaConstant*> constants;
        // arguments, as well.
        std::unordered_set<MetaArgument*> args;
        // basic blocks.
        std::vector<MetaBB*> bbs;

        // CFG root
        MetaBB* root;

        public:

        void addConstant(MetaConstant* c) {
            constants.insert(c);
        }
        
        void addArgument(MetaArgument* a) {
            args.insert(a);
        }
    
        // create a new bb at the end of bb list.
        MetaBB* buildBB() {
            MetaBB* newBB = new MetaBB(this);
            bbs.push_back(newBB);
            return newBB;
        }

    };

    class MetaInstGraph {
        private:
        protected:
            std::vector<MetaInst*> vertex;

        public:
            MetaInstGraph() { } 

            ~MetaInstGraph() {
                for (std::vector<MetaInst*>::iterator it = vertex.begin(); it != vertex.end(); ++it) {
                    delete *it;
                }
            }

            void addMetaInst(MetaInst* metaInst) {
                vertex.push_back(metaInst);
            }

            void addMetaInst(Instruction* i) {
                vertex.push_back(new MetaInst());
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
            

            // create all meta basic block and instructions.
            for (auto bb = F.begin(); bb != F.end(); ++bb) {
                MetaBB* newBB = mf.buildBB();
                bbMap.insert({&*bb, newBB});
                for (auto i = bb->begin(); i != bb->end(); ++i) {
                    MetaInst* newInst = MetaInst::createMetaInst(getInstType(&*i));
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

            // construct graph
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
        
            outs() << "\n";
            return true;
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
                curBB->setNextBB(it->second);
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

char MetaInstPass::ID = 0;


static RegisterPass<MetaInstPass> X("meta-trans", "MetaTrans Pass",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);