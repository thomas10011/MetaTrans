#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
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
                from.addAdjacency(to);
            }

    };

    class MetaOperand {

    };

    enum InstType {
        // NONE represent a Non-Instruction operand.
        NONE,
        LOAD,
        STORE,
        COMPARE,
        CALL,
        BRANCH,
        JUMP,
        ADD,
        SUB,
        MUL,
        DIV,
        AND,
        OR,
        XOR,
        SHIFT,
        NEG,
        RET
    };

    class MetaConstant : MetaOperand {

    }


    class MetaArgument : MetaOperand {

    }


    class MetaInst : MetaOperand {
        private:
        protected:

            // a vector to indicate the real type of a instruction.
            std::vector<InstType> type;
            std::vector<MetaOperand*> operandList;

        public:
            MetaInst() { }

            MetaInst(Instruction* i) {

            }

    };

    class MetaBB {
        private:
        protected:

        std::list<MetaInst> instList;

    };

    class MetaFunction {
        private:
        protected:

        // a function should contains a set of constants.
        std::unordered_set<MetaConstant> constants;
        // arguments, as well.
        std::unordered_set<MetaArgument> args;

        public:

        void addConstant(const Constant & c) {
            constants.insert(MetaConstant());
        }
        
        void addArgument(const Argument & a) {
            args.insert(MetaArgument());
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
                vertex.push_back(new MetaInst(i));
            }
    };
    
    struct MetaInstPass : FunctionPass {

        static char ID;



        /* data */
        MetaInstPass() : FunctionPass(ID) {
                       
        }
        
        bool runOnFunction(Function & F) override {
            
            outs() << "running MetaInst pass on function " << F.getName() << " ... " << "\n";

            MetaFunction mf;

            for (Function::iterator bb = F.begin(); bb != F.end(); ++bb) {
                // we create graph for each basic block
                for (BasicBlock::iterator i = bb->begin(); i != bb->end(); ++i) {
                    // 笑死 乱打属于是 先dereference再取地址
                    // if this instruction never appear, then add it to vertex list.
                    Use* edges = i->getOperandList();
                    outs() << "going to print values..." << "\n";
                    while (edges) {
                        Value* value = edges->get();
                        printType(value);
                        outs() << "the operand number of current use is " << edges->getOperandNo() << "#" << "\n";
                        edges = edges->getNext();
                    }
                }
            }
            outs() << "\n";
        }


        // this function is used to print the subclass of a Value in INSTRUCTION level.
        void printType(Value* value) {
            outs() << "value address: " << value;
            if (dyn_cast<Argument>(value)) { 
                outs() << " real type: Argument" << "\n";
            }
            else if (dyn_cast<BasicBlock>(value)) {
                outs() << " real type: BasicBlock" << "\n";
            }
            else if (dyn_cast<InlineAsm>(value)) {
                outs() << " real type: nlineAsm" << "\n";
            }
            else if (dyn_cast<MetadataAsValue>(value)) {
                outs() << " real type: MetadataAsValue" << "\n";
            }
            else if (dyn_cast<Constant>(value)) {
                outs() << " real type: Constant" << "\n";
            }
            else if (dyn_cast<MemoryAccess>(value)) {
                outs() << " real type: MemoryAccess" << "\n";
            }
            else if (dyn_cast<Instruction>(value)) {
                outs() << " real type: Instruction" << "\n";
            }
            else if (dyn_cast<Operator>(value)) {
                outs() << " real type: Operator" << "\n";
            }
        }

    };


}

char MetaInstPass::ID = 0;


static RegisterPass<MetaInstPass> X("meta-inst", "MetaInst Pass",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);