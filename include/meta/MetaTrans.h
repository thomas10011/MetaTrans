#pragma once

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/JSON.h"
#include "llvm/IR/DerivedUser.h"
#include "llvm/Analysis/MemorySSA.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/Operator.h"

#include "MetaData.h"

#include <vector>
#include <unordered_map>
#include <unordered_set>


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

            int id;

        public:

            MetaOperand& setID(int id);

            int getID();

            virtual bool isMetaConstant();
            
            virtual bool isMetaArgument();

            virtual bool isMetaInst();
            
            virtual ~MetaOperand();

            virtual std::string toString();

    };

    class MetaConstant : public MetaOperand {
        private:
        protected:

            DataType type;

            DataUnion value;

            MetaFunction* parent;

        public:
            
            MetaConstant();

            MetaConstant(MetaFunction* p);

            virtual ~MetaConstant();

            MetaConstant(DataType ty);

            MetaConstant& setDataType(DataType ty);

            DataType getDataType();

            DataUnion getValue();

            MetaFunction* getParent();

            MetaConstant& setValue(int8_t v);

            MetaConstant& setValue(int16_t v);
            
            MetaConstant& setValue(int32_t v);

            MetaConstant& setValue(int64_t v);

            MetaConstant& setValue(float v);
            
            MetaConstant& setValue(double v);

            MetaConstant& setParent(MetaFunction* p);
            
            virtual bool isMetaConstant() override;

            std::string virtual toString() override;

    };
    
    class MetaArgument : public MetaOperand {
        private: 
        protected:

            int width;

            int argIndex;

            int offset;

            DataType type; 

            MetaFunction* parent;
        
        public:

            MetaArgument();

            MetaArgument(MetaFunction* p);

            ~MetaArgument();

            MetaArgument(DataType ty); 

            MetaArgument& setArgIndex(int i);

            MetaArgument& setOffset(int o);

            MetaArgument& setArgType(DataType ty);

            MetaArgument& setWidth(int w);

            MetaArgument& setParent(MetaFunction* f);

            int getArgIndex();

            int getOffest();

            int getWidth();

            DataType getArgType();

            MetaFunction* getParent();

            virtual bool isMetaArgument() override;
    };

    class MetaInst : public MetaOperand {
        private:
        protected:

            MetaBB* parent;

            // a vector to indicate the real type of a instruction.
            std::vector<InstType> type;

            std::vector<MetaOperand*> operandList;

            InstMetaData metaData;

            std::set<ColorData> colors; // color, type(0 data computing 1 addressing 3 control flow)

        public:

            MetaInst();

            virtual ~MetaInst();

            MetaInst(std::vector<InstType> ty); 

            MetaInst& setInstType(std::vector<InstType> ty);

            MetaInst& setInstType(InstType ty);

            MetaInst& addInstType(InstType ty);

            MetaInst& setParent(MetaBB* bb);

            MetaInst& addOperand(MetaOperand* op);

            virtual MetaInst& buildFromJSON(llvm::json::Object JSON, std::unordered_map<int64_t, MetaBB*>& tempBBMap, std::unordered_map<int64_t, MetaOperand*>& tempOperandMap);

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

            bool virtual isMetaInst() override;

            std::string virtual toString() override;

            bool virtual isMetaPhi();

            void addColor(int c, int t);

            std::set<ColorData>& getColors();

            bool hasColor(int c);

    };

    /// represent a phi node.
    class MetaPhi : public MetaInst {
        private:
        protected:

            std::unordered_map<MetaBB*, MetaOperand*> bbValueMap;

        public:

            MetaPhi(std::vector<InstType> ty); 

            MetaPhi();

            MetaPhi& addValue(MetaBB* bb, MetaOperand* op);

            

            bool equals(MetaPhi* phi);

            MetaOperand* getValue(MetaBB* bb);

            int getMapSize();

            std::unordered_map<MetaBB*, MetaOperand*>::iterator begin();

            std::unordered_map<MetaBB*, MetaOperand*>::iterator end();

            bool virtual isMetaPhi() override;

            virtual MetaInst& buildFromJSON(llvm::json::Object JSON, std::unordered_map<int64_t, MetaBB*>& tempBBMap, std::unordered_map<int64_t, MetaOperand*>& tempOperandMap) override;

            std::string virtual toString() override;
    };

    class MetaBB {
        private:
        protected:

            int id;

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

            MetaBB();

            MetaBB(MetaFunction* parent);

            MetaBB(std::string JSON);

            MetaBB(MetaFunction* parent, std::string JSON);

            ~MetaBB(); 

            // Build a new instruction and append to instList.
            MetaInst* buildInstruction(std::vector<InstType> ty);

            MetaInst* buildInstruction();

            MetaPhi* buildPhi(bool insertAtHead = false);

            MetaBB& addPhi(MetaPhi* phi, bool insertAtHead = false);

            MetaBB& addInstruction(MetaInst* inst);

            MetaBB& addNextBB(MetaBB* next);

            MetaBB& setEntry(MetaInst* inst);

            MetaBB& setTerminator(MetaInst* inst);

            MetaBB& setParent(MetaFunction* mF);

            MetaBB& setID(int id);

            MetaBB& buildFromJSON(llvm::json::Object JSON, std::unordered_map<int64_t, MetaBB*>& tempBBMap, std::unordered_map<int64_t, MetaOperand*>& tempOperandMap);

            std::vector<MetaBB*> getNextBB();

            MetaBB* getNextBB(int index);

            MetaInst* getEntry();

            MetaInst* getTerminator();

            MetaFunction* getParent();

            int getID();

            std::vector<MetaInst*>& getInstList();

            int getInstNum();

            std::string toString();

            std::vector<MetaInst*>::iterator begin();

            std::vector<MetaInst*>::iterator end();

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

            DataType returnType;
            
            int stackSize;

            int argNum;

        public:

            MetaFunction();

            MetaFunction(std::string JSON);

            MetaFunction& addConstant(MetaConstant* c);
            
            MetaFunction& addArgument(MetaArgument* a);

            MetaFunction& setRoot(MetaBB* rootBB);

            MetaFunction& setFunctionName(std::string name);

            MetaFunction& setStackSize(int s);

            MetaFunction& setReturnType(DataType ty);

            MetaFunction& expandStackSize(int s);

            MetaArgument* getArgument(int index);

            std::string getFunctionName();

            int getConstNum();

            int getArgNum();

            DataType getReturnType();

            // create a new bb at the end of bb list.
            MetaBB* buildBB();

            MetaArgument* buildArgument();

            MetaConstant* buildConstant();

            std::string toString();

            std::vector<MetaBB*>::iterator begin();

            std::vector<MetaBB*>::iterator end();

            std::vector<MetaBB*>::iterator bb_begin();

            std::vector<MetaBB*>::iterator bb_end();

            std::vector<MetaConstant*>::iterator const_begin();

            std::vector<MetaConstant*>::iterator const_end();

            std::vector<MetaArgument*>::iterator arg_begin();

            std::vector<MetaArgument*>::iterator arg_end();
    
    };
}

    