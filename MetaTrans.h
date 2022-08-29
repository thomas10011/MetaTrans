#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/DerivedUser.h"
#include "llvm/Analysis/MemorySSA.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/Operator.h"
#include <vector>
#include <list>
#include <unordered_map>
#include <unordered_set>

#include "MetaData.h"

using namespace llvm;

namespace MetaTrans {

    class MetaInst;

    class MetaBB;

    class MetaFunction;

    struct MetaFunctionPass;

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

    class MetaOperand {

    };
    class MetaConstant : public MetaOperand {
        private:
        protected:
            long value;

    };
    class MetaArgument : public MetaOperand {
        private: 
        protected:

            int width;

            int argIndex;

            DataType type; 
        
        public:
            MetaArgument();

            MetaArgument(DataType ty); 

            void setArgIndex(int i);
    };
    class MetaInst : public MetaOperand {
        private:
        protected:

            // a vector to indicate the real type of a instruction.
            std::vector<InstType> type;
            std::vector<MetaOperand*> operandList;

        public:
            MetaInst();

            MetaInst(std::vector<InstType> ty); 

            void addOperand(MetaOperand* op);

            std::vector<MetaOperand*>& getOperandList();

            std::vector<MetaOperand*>::iterator op_begin();
            
            std::vector<MetaOperand*>::iterator op_end();

            virtual void processOperand (
                Instruction* curInst, MetaBB* curBB, MetaFunction& f, 
                MetaFunctionPass& pass
            );

            static MetaInst* createMetaInst(std::vector<InstType> ty);

            virtual ~MetaInst();
    };

    /// represent a phi node.
    class MetaPhi : public MetaInst {
        private:
        protected:

            std::unordered_map<MetaBB*, MetaOperand*> bbValueMap;

        public:

            MetaPhi(std::vector<InstType> ty); 

            virtual void processOperand (
                Instruction* curInst, MetaBB* curBB, MetaFunction& f, 
                MetaFunctionPass& pass
            ) override;

            void addValue(MetaBB* bb, MetaOperand* op);

            MetaOperand* getValue(MetaBB* bb);

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

        MetaBB(MetaFunction* f);

        ~MetaBB(); 

        // Build a new instruction and append to instList.
        MetaInst* buildInstruction(std::vector<InstType> ty);

        void addInstruction(MetaInst* inst);

        void addNextBB(MetaBB* next);

        std::vector<MetaBB*> getNextBB();

        MetaBB* getNextBB(int index);

        void setEntry(MetaInst* inst);

        MetaInst* getEntry();

        void setTerminator(MetaInst* inst);

        MetaInst* getTerminator();

        std::vector<MetaInst*>& getInstList();

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

        void addConstant(MetaConstant* c);
        
        void addArgument(MetaArgument* a);
    
        // create a new bb at the end of bb list.
        MetaBB* buildBB();

        void setRoot(MetaBB* rootBB);

        std::vector<MetaBB*>::iterator bb_begin();

        std::vector<MetaBB*>::iterator bb_end();

    };

    struct MetaFunctionPass : FunctionPass {

        static char ID;

        // record the reflection between primitive type and Meta type.
        std::unordered_map<BasicBlock*, MetaBB*> bbMap;

        std::unordered_map<Instruction*, MetaInst*> instMap;

        std::unordered_map<Constant*, MetaConstant*> constantMap;

        std::unordered_map<Argument*, MetaArgument*> argMap;

        MetaFunction mf;

        MetaFunctionPass();

        bool runOnFunction(Function & F) override;

        bool createMetaArg(Argument* arg);

        bool createMetaConstant(Constant* c);

        void createMetaElements(Function& F);

        std::vector<InstType> getInstType(Instruction* inst);

        void printType(Value* value);

        void printInstDependencyGraph(MetaBB* bb);
    };
}

    