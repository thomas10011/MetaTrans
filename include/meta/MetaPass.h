#pragma once

#include "meta/Filter.h"
#include "meta/MetaTrans.h"
#include "meta/MetaUtils.h"

namespace MetaTrans {
    
    /// An simple implementation of Builder Pattern.
    /// This class is used to create Instruction Graph for each Function. 
    class MetaFunctionBuilder : public FilterTarget {

        protected:

            friend class MetaArgFilter;
            friend class MetaConstantFilter;
            friend class MetaInstFilter;
            friend class MetaBBFilter;
            friend class MetaFuncFilter;

            Function* F;

            MetaFunction* mF;

            std::unordered_map<unsigned, std::vector<InstType>>*    typeMap;

            // Auxiliary map, record the reflection between primitive type and Meta type.
            std::unordered_map<BasicBlock*, MetaBB*>                bbMap;

            std::unordered_map<Instruction*, MetaInst*>             instMap;

            std::unordered_map<Constant*, MetaConstant*>            constantMap;

            std::unordered_map<Argument*, MetaArgument*>            argMap;

            FilterManager                                           filterManager;

            MetaFunctionBuilder&    clearAuxMaps                    ();

            MetaFunctionBuilder&    buildMetaFunction               ();
            
            MetaFunctionBuilder&    buildMetaElements               ();
                                                                    
            // build denpendency graph between instructions.
            MetaFunctionBuilder&    buildGraph                      ();

            // use responsibility pattern to create meta data.
            MetaFunctionBuilder&    buildMetaData                   ();
            
            // create a meta bb correspond to a llvm bb insde a meta function.
            MetaFunctionBuilder&    createMetaBB                    (BasicBlock& b);

            // create a meta instruction correspond to a llvm instruction inside a meta bb. 
            MetaFunctionBuilder&    createMetaInst                  (Instruction& i, MetaBB& b);

            // create meta operand for an llvm instruction.
            MetaFunctionBuilder&    createMetaOperand               (Instruction& i);

            MetaOperand*            findMetaOperand                  (Value* v);

            // copy instruction dependencies from LLVM IR.
            void                    copyDependencies                (Instruction* curInst);

            void                    copyDependencies                (PHINode* curInst);

        public:
            
                                    MetaFunctionBuilder             ();

            MetaFunctionBuilder&    setFunction                     (Function* F);

            MetaFunctionBuilder&    setTypeMap                      (std::unordered_map<unsigned, std::vector<InstType>>* typeMap);

            MetaFunction*           build                           ();

    };

    struct MetaFunctionPass : public llvm::FunctionPass {

        static char ID;

        static std::unordered_map<std::string, unsigned int> str_inst_map;

        // singleton type map.
        // Providing a map from LLVM Instruction type to TIR Instruction type.
        std::unordered_map<unsigned, std::vector<InstType>>* typeMap;

        MetaFunctionBuilder builder;

        std::vector<MetaFunction*> metaFuncs;

        MetaFunctionPass();

        bool runOnFunction(Function & F) override;

    };
}