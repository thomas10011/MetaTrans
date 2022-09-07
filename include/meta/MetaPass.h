#pragma once

#include "meta/MetaTrans.h"
#include "meta/utils.h"

namespace MetaTrans {
    
    /// An simple implementation of Builder Pattern.
    /// This class is used to create Instruction Graph for each Function. 
    class MetaFunctionBuilder {
        protected:

            Function* F;

            MetaFunction* mF;

            // Auxiliary map, record the reflection between primitive type and Meta type.
            std::unordered_map<BasicBlock*, MetaBB*> bbMap;

            std::unordered_map<Instruction*, MetaInst*> instMap;

            std::unordered_map<Constant*, MetaConstant*> constantMap;

            std::unordered_map<Argument*, MetaArgument*> argMap;

            MetaFunctionBuilder& clearAuxMaps();

            // build denpendency graph between instructions.
            MetaFunctionBuilder& buildGraph();
            
            MetaFunctionBuilder& createMetaElements();
            
            // create a meta bb correspond to a llvm bb insde a meta function.
            MetaFunctionBuilder& createMetaBB(BasicBlock& b, MetaFunction& f);

            // create a meta instruction correspond to a llvm instruction inside a meta bb. 
            MetaFunctionBuilder& createMetaInst(Instruction& i, MetaBB& b);

            // create meta operand for an llvm instruction.
            MetaFunctionBuilder& createMetaOperand(Instruction& i);

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
}