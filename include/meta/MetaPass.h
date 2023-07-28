#pragma once

#include "meta/Filter.h"
#include "meta/MetaTrans.h"
#include "meta/MetaUtils.h"

extern int globalColor;

namespace MetaTrans {
    
/// An simple implementation of Builder Pattern.
/// This class is used to create Instruction Graph for each Function. 
class MetaFunctionBuilder : public FilterTarget {

private:

    int buildCount = 0;

protected:

    friend class MetaArgFilter;
    friend class MetaConstantFilter;
    friend class MetaInstFilter;
    friend class MetaBBFilter;
    friend class MetaFuncFilter;
    friend class MetaIDFilter;
    friend class MetaFeatureFilter;

    MetaUnit* unit;

    Function* F;

    MetaFunction* mF;

    std::unordered_map<unsigned, std::vector<InstType>>*    typeMap;

    // Auxiliary map, record the reflection between primitive type and Meta type.
    std::unordered_map<BasicBlock*, MetaBB*>                bbMap;

    BidirectionMap<Instruction*, MetaInst*>                 instMap;

    std::unordered_map<Instruction*, Value*>                LoadStoreVarMap;

    std::unordered_map<Constant*, MetaConstant*>            constantMap;

    std::unordered_map<Argument*, MetaArgument*>            argMap;

    FilterManager                                           filterManager;

    MetaFunctionBuilder&    clearAuxMaps                    ();
    
    // 将GEP转换成inttoptr和ptrtoint
    MetaFunctionBuilder&    translateGEP                    ();

    MetaFunctionBuilder&    buildMetaElements               ();
                                                            
    // build denpendency graph between instructions.
    MetaFunctionBuilder&    buildGraph                      ();

    // use responsibility pattern to create meta data.
    MetaFunctionBuilder&    buildMetaData                   ();

    MetaFunctionBuilder&    createMetaArgs                  ();

    MetaFunctionBuilder&    createGlobalVar                 ();
    
    MetaFunctionBuilder&    createMetaFunction              ();

    // create a meta bb correspond to a llvm bb insde a meta function.
    MetaFunctionBuilder&    createMetaBB                    ();

    // create a meta instruction correspond to a llvm instruction inside a meta bb. 
    MetaFunctionBuilder&    createMetaInst                  (Instruction& i, MetaBB& b);

    // create meta operand for an llvm instruction.
    MetaFunctionBuilder&    createMetaOperand               (Instruction& i);

    MetaOperand*            findMetaOperand                  (Value* v);

    int                     getNumFuncs                     ();

    // copy instruction dependencies from LLVM IR.
    void                    copyDependencies                (Instruction* curInst);

    void                    copyDependencies                (PHINode* curInst);

public:

                            MetaFunctionBuilder             ();

                            ~MetaFunctionBuilder            ();

    MetaFunctionBuilder&    setFunction                     (Function* F);

    MetaFunctionBuilder&    setTypeMap                      (std::unordered_map<unsigned, std::vector<InstType>>* typeMap);

    MetaFunction*           build                           ();

    std::string             getMetaUnitStr                  ();

    MetaUnit*               getMetaUnit                     ();

    MetaUnit&               getMetaUnitRef                  ();

};

struct MetaFunctionPass : public llvm::FunctionPass {

    static char ID;

    static std::unordered_map<std::string, unsigned int> str_inst_map;

    // singleton type map.
    // Providing a map from LLVM Instruction type to TIR Instruction type.
    std::unordered_map<unsigned, std::vector<InstType>>* typeMap;

    MetaFunctionBuilder builder;

    MetaFunctionPass();

    ~MetaFunctionPass();

    bool runOnFunction(Function & F) override;

    void processMatch();

};
}