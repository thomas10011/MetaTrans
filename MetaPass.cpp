#include "meta/MetaPass.h"
#include "meta/MetaFilter.h"
#include "llvm/Support/JSON.h"
#include "meta/MetaMatcher.h"

using namespace llvm;
int globalColor = 0;
namespace MetaTrans {
    
//===-------------------------------------------------------------------------------===//
/// Meta Function pass implementation, Will be invoked by LLVM pass manager.

    MetaFunctionPass::~MetaFunctionPass() {
        std::string funcs = MetaUtil::vectorToJsonString(metaFuncs);
        std::string name = getenv("HOME");
        std::string IRJSON = name + "/ir.json";
        MetaUtil::writeToFile(funcs, IRJSON);

        processMatch();

        for (MetaFunction* mF : metaFuncs) delete mF;
    }

    void MetaFunctionPass::processMatch() {
        std::string name = getenv("HOME");
        std::string ASMJSON = name + "/asm.json";
        std::string asmStr = MetaUtil::readFromFile(ASMJSON);
        
        llvm::Expected<json::Value> expect = json::parse(asmStr);
        if (Error e = expect.takeError()) {
            // std::cout << "parse function json error!" << "\n";
            logAllUnhandledErrors(std::move(e), outs(), "[JSON Error] ");
            return;
        }
        json::Array& funcArr = *(expect.get().getAsArray());
        for (int i = 0; i < funcArr.size(); i++) {
            MetaFunction f(*(funcArr[i].getAsObject()));
            
            MetaMatcher matcher;
            for (MetaFunction* mF : metaFuncs) {
                if (mF->getFunctionName() == f.getFunctionName()) {
                    std::unordered_map<MetaBB*, MetaBB*>& result = matcher
                                                                    .setX(&f)
                                                                    .setY(mF)
                                                                    .matchBB()
                                                                    .matchInst()
                                                                    .getBBMatchResult()
                                                                    ;
                    // print match result.
                    printf("<<<<<<<<<<<<<<<<<<<< BB Match Result >>>>>>>>>>>>>>>>>>>>\n");
                    printf("\nFunction Name: %s\n\n", mF->getFunctionName().c_str());
                    for (auto pair = result.begin(); pair != result.end(); ++pair) {
                        MetaBB& x = *(pair->first);
                        MetaBB& y = *(pair->second);
                        std::vector<int> x_f = x.getFeature();
                        std::vector<int> y_f = y.getFeature();
                        printf(
                            "ASM BB num: %d --->>> IR BB num: %d  ||  features: [%d, %d, %d, %d], features: [%d, %d, %d, %d]\n",
                            x.getID(), y.getID(),
                            x_f[0], x_f[1], x_f[2], x_f[3],
                            y_f[0], y_f[1], y_f[2], y_f[3]
                        );
                    }
                    printf("\n<<<<<<<<<<<<<<<<<<<< End Match Result >>>>>>>>>>>>>>>>>>>>\n\n");
                    for (auto pair = result.begin(); pair != result.end(); ++pair) {
                        printf("MetaBB: %d <-> %d, Training Strats\n",pair->first->getID(), pair->second->getID() );
                        pair->first->trainBB(pair->second);
                    }
                    printf("-----------------------------------\n");

                }
                

            }
        }

    }

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
        MetaFunction f(metaFunc->toString());
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
            .addFilter(new MetaIDFilter())
            .addFilter(new MetaFeatureFilter())
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
        MetaUtil::paintColor(mF, globalColor++);
        return mF;
    }

    MetaFunctionBuilder& MetaFunctionBuilder::buildMetaFunction() {
        mF = new MetaFunction();
        return *this;
    }

    MetaFunctionBuilder& MetaFunctionBuilder::buildMetaElements() {
        // create all meta basic block and instructions recursively.
        for (auto bb = F->begin(); bb != F->end(); ++bb) {
            (*this)
                .createMetaBB(*bb);
        }
        // create all arguments;
        for (auto arg_iter = F->arg_begin(); arg_iter != F->arg_end(); ++arg_iter) {
            MetaArgument* arg = new MetaArgument();
            argMap[&*arg_iter] = arg;
            mF->addArgument(arg);
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
        MetaBB& newBB = *(mF->buildBB());
        assert(bbMap.insert({&b, &newBB}).second);
        for (auto i = b.begin(); i != b.end(); ++i) {
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
        // create all constants.
        for (auto op = i.op_begin(); op != i.op_end(); ++op) {
            Value* value = op->get();
            // Ugly, but works.
            if (Constant* c = dyn_cast<Constant>(value)) {
                if (constantMap.find(c) != constantMap.end()) continue;
                mF->addConstant(constantMap[c] = new MetaConstant());
            }
        }
        return *this;
    }

    MetaOperand* MetaFunctionBuilder::findMetaOperand(Value* value) {
        if (Argument*    a = dyn_cast<Argument>(value))    return (MetaOperand*)argMap[a];
        if (Constant*    c = dyn_cast<Constant>(value))    return (MetaOperand*)constantMap[c];
        if (Instruction* i = dyn_cast<Instruction>(value)) return (MetaOperand*)instMap[i];
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
        outs() << "\n";
    }

    void MetaFunctionBuilder::copyDependencies(Instruction* curInst) {
        MetaInst* inst = instMap[curInst];
        // deal with phi node particularly.
        if (auto phi = dyn_cast<PHINode>(curInst)) { copyDependencies(phi); }

        MetaBB* curBB = inst->getParent();
        outs() << "inst " << inst << ",  type: " << curInst->getOpcodeName() << "\n";
        for (auto op = curInst->op_begin(); op != curInst->op_end(); ++op) {
            Value* value = op->get();
            MetaUtil::printValueType(value);
            // create CFG here.
            if (BasicBlock* bb = dyn_cast<BasicBlock>(value)) {
                (*curBB)
                    .addNextBB(bbMap[bb])
                    .setTerminator(inst)
                    ;
            }

            if (MetaOperand* metaOp = findMetaOperand(value))
                inst->addOperand(metaOp);

            outs() << "; operand number: " << op->getOperandNo() << "#" << "\n";
        }

    }

} // namespace MetaTrans


static RegisterPass<MetaTrans::MetaFunctionPass> X("meta-trans", "MetaTrans Pass",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);