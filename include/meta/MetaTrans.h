#pragma once

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/DerivedUser.h"
#include "llvm/Analysis/MemorySSA.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/Operator.h"
#include "MetaData.h"

#include <vector>
#include <unordered_map>
#include <unordered_set>

using namespace llvm;

namespace MetaTrans {
    
    class MetaInst;

    class MetaBB;

    class MetaFunction;

    class MetaFunctionBuilder;   

    struct MetaFunctionPass;

    enum InstType {
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
        ALLOCATION,
        ADDRESSING,
        EXCEPTION,
        SWAP,
        MIN,
        MAX,
        SQRT,
        FENCE,
        CONVERT,
        HINT
    };

    class MetaOperand {
        private:
        protected:
        public:
        
    };

    class MetaConstant : public MetaOperand {
        private:
        protected:

            DataType type;

            DataUnion value;

        public:
            
            MetaConstant();

            MetaConstant(DataType ty);

            void setDataType(DataType ty);

            DataType getDataType();

            DataUnion getValue();

            void setValue(int8_t v);

            void setValue(int16_t v);
            
            void setValue(int32_t v);

            void setValue(int64_t v);

            void setValue(float v);
            
            void setValue(double v);

    };
    
    class MetaArgument : public MetaOperand {
        private: 
        protected:

            int width;

            int argIndex;

            int offset;

            DataType type; 
        
        public:

            MetaArgument();

            MetaArgument(DataType ty); 

            MetaArgument& setArgIndex(int i);

            MetaArgument& setOffset(int o);

            MetaArgument& setArgType(DataType ty);

            int getArgIndex();

            int getOffest();

            DataType getArgType();

    };

    class MetaInst : public MetaOperand {
        private:
        protected:

            MetaBB* parent;

            // a vector to indicate the real type of a instruction.
            std::vector<InstType> type;

            std::vector<MetaOperand*> operandList;

            InstMetaData metaData;

        public:

            MetaInst();

            MetaInst(std::vector<InstType> ty); 

            MetaInst& setParent(MetaBB* bb);

            MetaInst& addOperand(MetaOperand* op);

            int getOperandNum();

            MetaBB* getParent();

            std::vector<InstType> getInstType();

            std::vector<MetaOperand*>& getOperandList();

            std::vector<MetaOperand*>::iterator op_begin();
            
            std::vector<MetaOperand*>::iterator op_end();
                
            // return true if has same type with i.
            bool hasSameType(MetaInst* i);
            
            // return true if ty is in type vector.
            bool isType(InstType ty);

            // return trie of this instruction only has single type amd same with ty.
            bool isSingleType(InstType ty);

            virtual ~MetaInst();

    };

    /// represent a phi node.
    class MetaPhi : public MetaInst {
        private:
        protected:

            std::unordered_map<MetaBB*, MetaOperand*> bbValueMap;

        public:

            MetaPhi(std::vector<InstType> ty); 

            void addValue(MetaBB* bb, MetaOperand* op);

            MetaOperand* getValue(MetaBB* bb);

    };

    class MetaBB {
        private:
        protected:

            std::vector<MetaInst*> instList;
            
            // point to next BB if exists.
            std::vector<MetaBB*> successors;

            // first non phi instruction.
            MetaInst* entry;

            // each bb end with a terminator.
            MetaInst* terminator;
            
            // record parent scope
            MetaFunction* parent;

        public:

            MetaBB(MetaFunction* f);

            ~MetaBB(); 

            // Build a new instruction and append to instList.
            MetaInst* buildInstruction(std::vector<InstType> ty);

            MetaBB& addInstruction(MetaInst* inst);

            MetaBB& addNextBB(MetaBB* next);

            MetaBB& setEntry(MetaInst* inst);

            MetaBB& setTerminator(MetaInst* inst);

            MetaBB& setParent(MetaFunction* mF);

            std::vector<MetaBB*> getNextBB();

            MetaBB* getNextBB(int index);

            MetaInst* getEntry();

            MetaInst* getTerminator();

            MetaFunction* getParent();

            std::vector<MetaInst*>& getInstList();

            int getInstNum();

            std::vector<MetaInst*>::iterator inst_begin();

            std::vector<MetaInst*>::iterator inst_end();

    };

    class MetaFunction {
        private:
        protected:

            // a function should contains a set of constants.
            std::vector<MetaConstant*> constants;
            // arguments, as well.
            std::vector<MetaArgument*> args;
            // basic blocks.
            std::vector<MetaBB*> bbs;

            // CFG root
            MetaBB* root;

            std::string funcName;

            DataType outputType;
            
            int argNum;

            int stackSize;

        public:

            MetaFunction& addConstant(MetaConstant* c);
            
            MetaFunction& addArgument(MetaArgument* a);

            MetaFunction& setRoot(MetaBB* rootBB);

            MetaArgument* getArgument(int index);

            int getConstNum();

            int getArgNum();

            // create a new bb at the end of bb list.
            MetaBB* buildBB();

            std::vector<MetaBB*>::iterator bb_begin();

            std::vector<MetaBB*>::iterator bb_end();

            std::vector<MetaConstant*>::iterator const_begin();

            std::vector<MetaConstant*>::iterator const_end();

            std::vector<MetaArgument*>::iterator arg_begin();

            std::vector<MetaArgument*>::iterator arg_end();
    
    };
}

    