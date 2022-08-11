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

namespace MetaTrans {

    class MetaInst;
    class MetaBB;
    class MetaFunction;
    struct MetaInstPass;


    class MetaData {

    };

    // record offset in stack.
    class MetaOffset : public MetaData {
        private:
        protected:
            int offset;

        public:
            MetaOffset () : offset(0) { }

            int getOffset();
            void setOffset(int offs);
    }

    class MetaOperand {

    };
    

    class MetaConstant : public MetaOperand {
        private:
        protected:
            longlong value;

    };


    enum DataType {
        INT,
        UINT,
        FLOAT,
        DOUBLE,
    };

    class MetaArgument : public MetaOperand {
        private: 
        protected:

            int width;

            int argIndex;

            DataType type; 
        
        public:
            MetaArgument(DataType ty) : type(ty) {}

            void setArgIndex(int i);
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

            virtual ~MetaInst() { }


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
        std::vector<MetaBB*> successors;
        // possibly a phi node.
        MetaInst* entry;
        // each bb end with a terminator.
        MetaInst* terminator;
        
        // record parent scope
        const MetaFunction* parent;

        public:

        MetaBB(MetaFunction* f) : entry(nullptr), terminator(nullptr), parent(f) { }

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

        void addNextBB(MetaBB* next) { successors.push_back(next); }

        std::vector<MetaBB*> getNextBB() { return successors; }

        MetaBB* getNextBB(int index) { return successors[index]; }

        void setEntry(MetaInst* inst) { entry = inst; }

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

        void setRoot(MetaBB* rootBB) {
            root = rootBB;
        }

        std::vector<MetaBB*>::iterator begin() { return bbs.begin(); }

        std::vector<MetaBB*>::iterator end() { return bbs.end(); }

    };
}

    