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

    class MetaFunctionBuilder;   

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

            DataType type; 
        
        public:
            MetaArgument();

            MetaArgument(DataType ty); 

            void setArgIndex(int i);

            int getArgIndex();

            void setArgType(DataType ty);

            DataType getArgType();

    };

    class MetaInst : public MetaOperand {
        private:
        protected:

            // a vector to indicate the real type of a instruction.
            std::vector<InstType> type;

            std::vector<MetaOperand*> operandList;

            InstMetaData metaData;

        public:

            MetaInst();

            MetaInst(std::vector<InstType> ty); 

            void addOperand(MetaOperand* op);

            int getOperandNum();

            std::vector<InstType> getInstType();

            std::vector<MetaOperand*>& getOperandList();

            std::vector<MetaOperand*>::iterator op_begin();
            
            std::vector<MetaOperand*>::iterator op_end();

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

        int getInstNum();

        std::vector<MetaInst*>::iterator inst_begin();

        std::vector<MetaInst*>::iterator inst_end();

    };

    class MetaFunction {
        private:
        protected:

        // a function should contains a set of constants.
        std::unordered_set<MetaConstant*> constants;
        // arguments, as well.
        std::vector<MetaArgument*> args;
        // basic blocks.
        std::vector<MetaBB*> bbs;

        // CFG root
        MetaBB* root;

        std::string funcName;

        DataType outputType;
        
        int argNum;

        public:

        void addConstant(MetaConstant* c);
        
        void addArgument(MetaArgument* a);

        MetaArgument* getArgument(int index);

        int getConstNum();

        int getArgNum();

        // create a new bb at the end of bb list.
        MetaBB* buildBB();

        void setRoot(MetaBB* rootBB);

        std::vector<MetaBB*>::iterator bb_begin();

        std::vector<MetaBB*>::iterator bb_end();

        std::vector<MetaArgument*>::iterator arg_begin();

        std::vector<MetaArgument*>::iterator arg_end();
    
    };

    /// An simple implementation of Builder Pattern.
    /// This class is used to create Instruction Graph for each Function. 
    class MetaFunctionBuilder {
        protected:

            Function* F;

            MetaFunction* mf;

            // Auxiliary map, record the reflection between primitive type and Meta type.
            std::unordered_map<BasicBlock*, MetaBB*> bbMap;

            std::unordered_map<Instruction*, MetaInst*> instMap;

            std::unordered_map<Constant*, MetaConstant*> constantMap;

            std::unordered_map<Argument*, MetaArgument*> argMap;

            void clearMaps();

            bool createMetaArg(Argument* arg);

            bool createMetaConstant(Constant* c);

            void createMetaElements(Function* F);

            void processOperand(MetaInst* inst, Instruction* curInst, MetaBB* curBB);

            void processOperand(MetaPhi* inst, PHINode* curInst, MetaBB* curBB);

        public:
            
            MetaFunctionBuilder();

            MetaFunctionBuilder& setFunction(Function* F);

            MetaFunction* build();

    };
    struct MetaFunctionPass : FunctionPass {

        static char ID;

        MetaFunctionBuilder builder;

        std::vector<MetaFunction*> metaFuncs;

        MetaFunctionPass();

        bool runOnFunction(Function & F) override;

    };


    class MetaUtil {
        
        public:
            
        void static printValueType(Value* value);

        void static printInstDependencyGraph(MetaBB* bb);

        void static printInstOperand(Instruction* inst);
        
        template<typename T>
        std::string static typeVecToString(std::vector<T> type_vector);

        std::string static toString(DataType type);

        std::string static toString(InstType type);

        std::vector<InstType> static getInstType(Instruction* inst);
    };
}

    