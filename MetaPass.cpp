#include "meta/MetaPass.h"
#include "meta/MetaFilter.h"

using namespace llvm;

namespace MetaTrans {
    
//===-------------------------------------------------------------------------------===//
/// Meta Function pass implementation, Will be invoked by LLVM pass manager.

    MetaFunctionPass::MetaFunctionPass() : FunctionPass(MetaFunctionPass::ID) {
        typeMap = YamlUtil::parseMapConfig("ir.yml", MetaFunctionPass::str_inst_map);
    }

    bool MetaFunctionPass::runOnFunction(Function & F) {
        outs() << "running MetaInst pass on function " << F.getName() << " ... " << "\n";
        MetaFunction* metaFunc = builder
                                    .setFunction(&F)
                                    .setTypeMap(typeMap)
                                    .build();
        metaFuncs.push_back(metaFunc);
        std::cout << metaFunc->toString() << std::endl;
        return true;
    }

    char MetaFunctionPass::ID = 0;

    std::unordered_map<std::string, unsigned>
    MetaFunctionPass::str_inst_map = {
        { "Ret",             Instruction::Ret                   },
        { "Br",              Instruction::Br                    },
        { "Switch",          Instruction::Switch                },
        { "IndirectBr",      Instruction::IndirectBr            },
        { "Invoke",          Instruction::Invoke                },
        { "Resume",          Instruction::Resume                },
        { "Unreachable",     Instruction::Unreachable           },
        { "CleanupRet",      Instruction::CleanupRet            },
        { "CatchRet",        Instruction::CatchRet              },
        { "CatchPad",        Instruction::CatchPad              },
        { "CatchSwitch",     Instruction::CatchSwitch           },
        { "CallBr",          Instruction::CallBr                },
        { "FNeg",            Instruction::FNeg                  },
        { "Add",             Instruction::Add                   },
        { "FAdd",            Instruction::FAdd                  },
        { "Sub",             Instruction::Sub                   },
        { "FSub",            Instruction::FSub                  },
        { "Mul",             Instruction::Mul                   },
        { "FMul",            Instruction::FMul                  },
        { "UDiv",            Instruction::UDiv                  },
        { "SDiv",            Instruction::SDiv                  },
        { "FDiv",            Instruction::FDiv                  },
        { "URem",            Instruction::URem                  },
        { "SRem",            Instruction::SRem                  },
        { "FRem",            Instruction::FRem                  },
        { "And",             Instruction::And                   },
        { "Or",              Instruction::Or                    },
        { "Xor",             Instruction::Xor                   },
        { "Alloca",          Instruction::Alloca                },
        { "Load",            Instruction::Load                  },
        { "Store",           Instruction::Store                 },
        { "AtomicCmpXchg",   Instruction::AtomicCmpXchg         },
        { "AtomicRMW",       Instruction::AtomicRMW             },
        { "Fence",           Instruction::Fence                 },
        { "GetElementPtr",   Instruction::GetElementPtr         },
        { "Trunc",           Instruction::Trunc                 },
        { "ZExt",            Instruction::ZExt                  },
        { "SExt",            Instruction::SExt                  },
        { "FPTrunc",         Instruction::FPTrunc               },
        { "FPExt",           Instruction::FPExt                 },
        { "FPToUI",          Instruction::FPToUI                },
        { "FPToSI",          Instruction::FPToSI                },
        { "UIToFP",          Instruction::UIToFP                },
        { "SIToFP",          Instruction::SIToFP                },
        { "IntToPtr",        Instruction::IntToPtr              },
        { "PtrToInt",        Instruction::PtrToInt              },
        { "BitCast",         Instruction::BitCast               },
        { "AddrSpaceCast",   Instruction::AddrSpaceCast         },
        { "ICmp",            Instruction::ICmp                  },
        { "FCmp",            Instruction::FCmp                  },
        { "PHI",             Instruction::PHI                   },
        { "Select",          Instruction::Select                },
        { "Call",            Instruction::Call                  },
        { "Shl",             Instruction::Shl                   },
        { "LShr",            Instruction::LShr                  },
        { "AShr",            Instruction::AShr                  },
        { "VAArg",           Instruction::VAArg                 },
        { "ExtractElement",  Instruction::ExtractElement        },
        { "InsertElement",   Instruction::InsertElement         },
        { "ShuffleVector",    Instruction::ShuffleVector          },
        { "ExtractValue",    Instruction::ExtractValue          },
        { "InsertValue",     Instruction::InsertValue           },
        { "LandingPad",      Instruction::LandingPad            },
        { "CleanupPad",      Instruction::CleanupPad            },
        { "Freeze",          Instruction::Freeze                },
        { "GetElementPtr",   Instruction::GetElementPtr         }
    };

//===-------------------------------------------------------------------------------===//
/// Meta Function Builder implementation.

    MetaFunctionBuilder::MetaFunctionBuilder() : F(nullptr), mF(nullptr), typeMap(nullptr) {
        filterManager
            .addFilter(new MetaArgFilter())
            .addFilter(new MetaConstantFilter())
            .addFilter(new MetaInstFilter())
            .addFilter(new MetaBBFilter())
            .addFilter(new MetaFuncFilter())
            ;
    }

    MetaFunctionBuilder& MetaFunctionBuilder::clearAuxMaps() {
        bbMap       .clear();
        instMap     .clear();
        constantMap .clear();
        argMap      .clear();
        return *this;
    }

    MetaFunctionBuilder& MetaFunctionBuilder::setFunction(Function* F) {
        this->F = F;
        clearAuxMaps();
        return *this;
    }

    MetaFunctionBuilder& MetaFunctionBuilder::setTypeMap(std::unordered_map<unsigned, std::vector<InstType>>* typeMap) {
        this->typeMap = typeMap;
        return *this;
    }

    MetaFunction* MetaFunctionBuilder::build() {
        (*this)
            .buildMetaFunction()
            .buildMetaElements()
            .buildGraph()
            .buildMetaData();
        return mF;
    }

    MetaFunctionBuilder& MetaFunctionBuilder::buildMetaFunction() {
        mF = new MetaFunction();
        return *this;
    }

    MetaFunctionBuilder& MetaFunctionBuilder::buildMetaElements() {
        // create all meta basic block and instructions recursively.
        for (auto bb = F->begin(); bb != F->end(); ++bb) {
            (*this).createMetaBB(*bb);
        }
        return *this;
    }

    MetaFunctionBuilder& MetaFunctionBuilder::buildGraph() {
        outs() << "creating edges in graph..." << "\n";
        for (Function::iterator bb = F->begin(); bb != F->end(); ++bb) {
            for (BasicBlock::iterator i = bb->begin(); i != bb->end(); ++i) {
                // add this instruction to current basic block.
                copyDependencies(&*i);
            }
        }
        // print some info   
        for (auto bb = mF->bb_begin(); bb != mF->bb_end(); ++bb) {
            outs() << "successor number of " << *bb << " is " << (*bb)->getNextBB().size() << "\n";
            MetaUtil::printInstDependencyGraph(*bb);
        }
        outs() << "\n";
        return *this;
    }

    MetaFunctionBuilder& MetaFunctionBuilder::buildMetaData() {
        filterManager.filter(*this);
        return *this;
    }

    MetaFunctionBuilder& MetaFunctionBuilder::createMetaBB(BasicBlock& b) {
        outs() << "creating instructions in Basic Block " << b;
        MetaBB& newBB = *(mF->buildBB());
        assert(bbMap.insert({&b, &newBB}).second);
        for (auto i = b.begin(); i != b.end(); ++i) {
            outs() << "instruction with type: " << i->getOpcodeName() << "\n";
            (*this)
                .createMetaInst(*i, newBB)
                .createMetaOperand(*i);
        }
        return *this;
    }
    
    MetaFunctionBuilder& MetaFunctionBuilder::createMetaInst(Instruction& i, MetaBB& b) {
        MetaInst* newInst = b.buildInstruction((*typeMap)[i.getOpcode()]);
        newInst->setParent(&b);
        assert(instMap.insert({&i, newInst}).second);
        return *this;
    }

    MetaFunctionBuilder& MetaFunctionBuilder::createMetaOperand(Instruction& i) {
        // create all arg and constant.
        for (auto op = i.op_begin(); op != i.op_end(); ++op) {
            Value* value = op->get();
            // Ugly, but works.
            if (Argument* a = dyn_cast<Argument>(value)) {
                if (MetaArgument* mA = MetaUtil::createValue(a, argMap))
                    mF->addArgument(mA);
            }
            else if (Constant* c = dyn_cast<Constant>(value)) {
                if (MetaConstant* mC = MetaUtil::createValue(c, constantMap))
                    mF->addConstant(mC);
            }
        }
        return *this;
    }

    MetaOperand* MetaFunctionBuilder::findMetaOperand(Value* value) {
        if (Argument* arg = dyn_cast<Argument>(value)) {
            return (MetaOperand*)argMap[arg];
        }
        else if (Constant* c = dyn_cast<Constant>(value)) {
            return (MetaOperand*)constantMap[c];
        }
        else if (Instruction* i = dyn_cast<Instruction>(value)) {
            return (MetaOperand*)instMap[i];
        }
        return nullptr;
    }

    void MetaFunctionBuilder::copyDependencies(PHINode* phi) {
        MetaPhi* inst = (MetaPhi*)instMap[phi];
        outs() << "copying phi node chain ..." << "\n";
        for (auto bb = phi->block_begin(); bb != phi->block_end(); ++bb) {
            Value* value = phi->getIncomingValueForBlock(*bb);
            MetaOperand* op = findMetaOperand(value);
            assert(op);
            inst->addValue(bbMap[*bb], op);
            outs() << "basic block: "<< bb << "; value: " << value << "; ";
        }
        outs() << "\n";
        
        for (auto op = phi->op_begin(); op != phi->op_end(); ++op) {
            Value* value = op->get();
            MetaUtil::printValueType(value); 
            outs() << "; operand number: " << op->getOperandNo() << "#" << "\n";
        }
    }

    void MetaFunctionBuilder::copyDependencies(Instruction* curInst) {
        MetaInst* inst = instMap[curInst];
        // deal with phi node particularly.
        if (auto phi = dyn_cast<PHINode>(curInst)) { copyDependencies(phi); }

        MetaBB* curBB = inst->getParent();
        outs() << "inst type: " << curInst->getOpcodeName() << "\n";
        for (auto op = curInst->op_begin(); op != curInst->op_end(); ++op) {
            Value* value = op->get();
            MetaUtil::printValueType(value);
            // create CFG here.
            if (BasicBlock* bb = dyn_cast<BasicBlock>(value)) {
                (*curBB)
                    .addNextBB(bbMap[bb])
                    .setTerminator(inst);
            }

            if (MetaOperand* metaOp = findMetaOperand(value))
                inst->addOperand(metaOp);

            outs() << "; operand number: " << op->getOperandNo() << "#" << "\n";
        }

    }

}


static RegisterPass<MetaTrans::MetaFunctionPass> X("meta-trans", "MetaTrans Pass",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);