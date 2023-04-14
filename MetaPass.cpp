#include "meta/MetaPass.h"
#include "meta/MetaFilter.h"
#include "llvm/Support/JSON.h"
#include "llvm/IR/IRBuilder.h"
#include "meta/MetaMatcher.h"
#include "meta/MetaTrans.h"
#include <typeinfo>

#define ULL unsigned long long

using namespace llvm;
int globalColor = 0;
extern MetaTrans::MappingTable* MapTable;

namespace MetaTrans {
    
//===-------------------------------------------------------------------------------===//
/// Meta Function pass implementation, Will be invoked by LLVM pass manager.

    void printName(MetaFunction* func) {
        printf("printing func: %s\n", func->getFunctionName().c_str());
    }

    MetaFunctionPass::~MetaFunctionPass() {
        builder.getMetaUnit()->fillID();
        std::string funcs  = builder.getMetaUnit()->toString();
        std::string name   = getenv("HOME");
        std::string IRJSON = name + "/ir.json";
        MetaUtil::writeToFile(funcs, IRJSON);
        std::cout << "MetaFunctionPass::~MetaFunctionPass ir.json IRJSON :: " << std::endl << funcs << std::endl;
 
        // just test for constructor.
        MetaUnit u(funcs);
        
        // check out functions
        printf("PRINTING FUNC NAME ... \n");

        processMatch();

    }

    void MetaFunctionPass::processMatch() {
        std::string name    = getenv("HOME");
        std::string ASMJSON = name + "/asm.json";
        std::string asmStr  = MetaUtil::readFromFile(ASMJSON);

        MapTable = new MappingTable();

        AddrMappingTable& addrMapTable = AddrMappingTable::getInstanceRef();

        MapTable->setName(name+"/")
                ->loadMappingTable()
                ->initTableMeta();


        MetaUnit* asmUnit = new MetaUnit(asmStr);

        std::cout << asmUnit->toString() << std::endl;
        
        // 染色，为了matching
        std::vector<MetaFunction*> funcs = asmUnit->getFuncList();
        for (int i = 0, color = 0; i < funcs.size(); ++i, ++color) {
            color = MetaUtil::paintColor(funcs[i], color);
            MetaUtil::paintColorCheck(funcs[i]);
        }

        for (auto iter = asmUnit->begin(); iter != asmUnit->end(); ++iter) {
            MetaFunction* f = *iter;
            // MetaBBMatcher* matcher = new LinearMetaBBMatcher();
            MetaBBMatcher* matcher = new CraphBasedBBMatcher();
            MetaAddressMatcher* addrMatcher = new MetaAddressMatcher();

            auto predicate = [&] (MetaFunction* mF) { return mF->getFunctionName() == f->getFunctionName(); };
            auto match_inner = [&] (MetaFunction* mF) {
                std::unordered_map<MetaBB*, MetaBB*>& result = (*matcher)
                                                                .setAsmMetaFunc(f)
                                                                .setIrMetaFunc(mF)
                                                                .match()
                                                                .getResult()
                                                                ;
                // print match result.
                printf("<<<<<<<<<<<<<<<<<<<< BB Match Result >>>>>>>>>>>>>>>>>>>>\n");
                printf("\nFunction Name: %s\n", mF->getFunctionName().c_str());
                printf("\nBB Match Precision: %.2f%!\n\n", 100.0 * result.size() / f->getBB().size());
                for (auto pair = result.begin(); pair != result.end(); ++pair) {
                    MetaBB& x = *(pair->first);
                    MetaBB& y = *(pair->second);
                    std::vector<int> x_f = x.getFeature();
                    std::vector<int> y_f = y.getFeature();
                    printf(
                        "ASM BB ID: %d --->>> IR BB ID: %d  ||  features: [%d, %d, %d, %d], features: [%d, %d, %d, %d]\n",
                        x.getID(), y.getID(),
                        x_f[0], x_f[1], x_f[2], x_f[3],
                        y_f[0], y_f[1], y_f[2], y_f[3]
                    );
                }
                printf("\n<<<<<<<<<<<<<<<<<<<< End Match Result >>>>>>>>>>>>>>>>>>>>\n\n");

                // 开始Addressing的Match
                for (auto pair = result.begin(); pair != result.end(); ++pair) {
                    printf("MetaBB: %d <-> %d, Training Address Strats\n",pair->first->getID(), pair->second->getID() );
                    for (auto it = pair->first->begin(); it != pair->first->end(); ++it) {
                        if ((*it)->isLoad())
                            (*addrMatcher).setAsmInst(*it).setIrBB(&*(pair->second)).match();
                    }
                }
                printf("<<<<<<<<<<<<<<<<<<<< Addr Match Result >>>>>>>>>>>>>>>>>>>>\n\n");
                std::vector<CodePiecePair> addrRes = addrMatcher->getResult();
                for (CodePiecePair pair : addrRes) {
                    printf("%s : %s\n", pair.first.toString().c_str(), pair.second.toString().c_str());
                }
                addrMapTable.update(addrMatcher->getResultMap()).flush();
                printf("\n<<<<<<<<<<<<<<<<<<<< End Addr Match Result >>>>>>>>>>>>>>>>>>>>\n\n");


                // train BB
                printf("-----------------------------------\n");
                for (auto pair = result.begin(); pair != result.end(); ++pair) {
                    printf("MetaBB: %d <-> %d, Training Strats\n",pair->first->getID(), pair->second->getID() );
                    pair->first->trainBB(pair->second);
                }
                printf("-----------------------------------\n");
            };

            
            builder
                .getMetaUnitRef()
                .func_stream()
                .filter(predicate)
                .forEach(match_inner)
                ;

        };
        MapTable->updateTableMeta();

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

    MetaFunctionBuilder::MetaFunctionBuilder() : F(nullptr), mF(nullptr), typeMap(nullptr), buildCount(0) {
        unit = new MetaUnit();
        filterManager
            .addFilter(new MetaArgFilter())
            .addFilter(new MetaConstantFilter())
            .addFilter(new MetaInstFilter())
            .addFilter(new MetaBBFilter())
            .addFilter(new MetaFuncFilter())
            // .addFilter(new MetaIDFilter())
            .addFilter(new MetaFeatureFilter())
            ;
    }

    MetaFunctionBuilder::~MetaFunctionBuilder() {
        filterManager.clear();
        delete unit;
    }

    MetaFunctionBuilder& MetaFunctionBuilder::clearAuxMaps() {
        bbMap       .clear();
        instMap     .clear();
        argMap      .clear();
        // constantMap .clear();
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
            // .translateGEP()
            .buildMetaElements()
            .buildGraph()
            .buildMetaData();

        globalColor = MetaUtil::paintColor(mF, globalColor++);
        MetaUtil::setDataRoot(mF);

        buildCount++;

        return mF;
    }

    void unrollingGEP(GetElementPtrInst* gep) {
        IRBuilder<> builder(gep->getContext());

        builder.SetInsertPoint(gep);
        BasicBlock& bb = *(gep->getParent());

        Value* base = gep->getPointerOperand();
        
        // Get the base pointer type
        Type* ptrType = gep->getPointerOperandType();
        Type* type = gep->getType();
        Type* resType = gep->getResultElementType();
        Type* srcType = gep->getSourceElementType();

        // print GEP type information
        std::string tyStr;
        raw_string_ostream ros(tyStr);
        ros << "ptr type: " << *ptrType << "\t, && id: " << ptrType->getTypeID() << "\n";
        ros << "gep type: " << *type    << "\t, && id: " << type->getTypeID()    << "\n";
        ros << "src type: " << *srcType << "\t, && id: " << srcType->getTypeID() << "\n";
        ros << "res type: " << *resType << "\t, && id: " << resType->getTypeID() << "\n";

        printf(tyStr.c_str());

        // 目前仅支持固定大小数组的gep翻译 如a[1024][1024]
        // 获取元素个数，最后一位存储位宽
        std::vector<ULL> arrayLength;
        Type* iter = srcType;
        while (iter != resType) {
            printf("%d ", iter->getArrayNumElements());
            arrayLength.push_back((ULL)iter->getArrayNumElements());
            iter = iter->getArrayElementType();
        }
        // 根据不同类型存储位宽
        if (iter->isIntegerTy()) {
            printf("%d\n", iter->getIntegerBitWidth());
            arrayLength.push_back((ULL)iter->getIntegerBitWidth());
        }
        else if (iter->isFloatingPointTy()) {
            printf("%d\n", iter->getFPMantissaWidth());
            arrayLength.push_back((ULL)iter->getFPMantissaWidth());
        }
        
        // offsets存储偏移量
        int n = arrayLength.size();
        ULL offsets[n];
        offsets[n - 1] = arrayLength[n - 1];
        for (int i = n - 2; i >= 0; --i) {
            offsets[i] = offsets[i + 1] * arrayLength[i];
        }
        for (int i = 0; i < n; ++i) {
            printf("%lld ", offsets[i]);
        }
        printf("\n");

        // 从此处开始翻译
        Value* pti = builder.CreatePtrToInt(base, builder.getInt64Ty());

        // Iterate through the indices to compute the final type
        Value* address = pti;
        for (int i = 1; i <= gep->getNumIndices(); ++i) {
            printf("operand no of idx: %d / %d\n", i, gep->getNumIndices());
            Value* idx = gep->getOperand(i);
            Constant* size = builder.getInt64(offsets[i - 1]);
            Value* offset = builder.CreateMul(idx, size);
            Value* add = builder.CreateAdd(address, offset);
            address = add;
        }

        Value* itp = builder.CreateIntToPtr(address, type);

        for (auto iter = gep->user_begin(); iter != gep->user_end(); ++iter) {
            User* memOp = *iter;
            if (LoadInst* load = dyn_cast<LoadInst>(memOp)) { printf("Find laod operation after GEP.\n"); }
            else if (StoreInst* store = dyn_cast<StoreInst>(memOp)) { printf("Find laod operation after GEP.\n"); }
            else { printf("WARN: no load or store after GEP.\n"); outs() << *memOp << "\n"; } 
        }
        gep->replaceAllUsesWith(itp);
        gep->eraseFromParent();
    }

    MetaFunctionBuilder& MetaFunctionBuilder::translateGEP() {
        
        printf("Finding gep\n");
        for(BasicBlock& bb : F->getBasicBlockList()) {
            // 必须把gep单独摘出来处理 否则段错误
            std::vector<GetElementPtrInst*> geps;
            for (Instruction& inst : bb.getInstList()) {
                // TODO: 使用GEPOperator，因为GetElementPtrInst和ConstantExpr都可以表示GEP操作
                if (GetElementPtrInst* gep = dyn_cast<GetElementPtrInst>(&inst)) {
                    geps.push_back(gep);
                }
            }
            // 为GEP的User（Load / Store）设置GlobalVar 然后将GEP展开。
            for (auto gep : geps) {
                Value* var = gep->getOperand(0);
                outs() << "Type of GEP[0]: " << *(var->getType()) << " " << "\n";
                for (auto it = gep->user_begin(); it != gep->user_end(); ++it) {
                    Instruction* inst = (Instruction*)*it;
                    if (isa<LoadInst>(inst) || isa<StoreInst>(inst)) {
                        printf("LOAD / STORE: %d,  Type of GEP[0]: %s, %d\n", inst->getName().str().c_str(), var->getName().str().c_str(), var->getType()->getTypeID());
                        LoadStoreVarMap[inst] = var;
                    }
                }
                printf("Unrolling gep\n");
                unrollingGEP(gep);
            }
        }
        
        return *this;
    }

    MetaFunctionBuilder& MetaFunctionBuilder::buildMetaElements() {
        (*this)
            .createGlobalVar()
            .createMetaFunction()
            .createMetaArgs()
            .createMetaBB()
            ;
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

    MetaFunctionBuilder& MetaFunctionBuilder::createGlobalVar() {
        if (buildCount > 0) return *this;

        Module& m = *(F->getParent());

        for (auto& global : m.getGlobalList()) {
            MetaConstant* mc = new MetaConstant();
            (*mc)
                .setGlobal(true)
                .setName(global.getName().str())
                .setParentScope(unit)
                .registerToMetaUnit()
                ;
            printf("INFO: Creating Global Var named %s\n", mc->getName().c_str());
            unit->addGlobalVar(constantMap[&global] = mc);
            // printf("meta address for global var %s is: %d\n", global.getName().str().c_str(), map[&global]);
        }

        return *this;
    }

    MetaFunctionBuilder& MetaFunctionBuilder::createMetaFunction() {
        mF = new MetaFunction();
        (*mF)
            .setParentScope(unit)
            .registerToMetaUnit()
            ;
        (*unit)
            .addFunc(mF)
            ;
        return *this;
    }

    MetaFunctionBuilder& MetaFunctionBuilder::createMetaArgs() {
        // create all arguments;
        for (auto arg_iter = F->arg_begin(); arg_iter != F->arg_end(); ++arg_iter) {
            MetaArgument* arg = mF->buildArgument();
            argMap[&*arg_iter] = arg;
            (*arg)
                .setParentScope(mF)
                .registerToMetaUnit()
                ;
        }
        return *this;

    }

    MetaFunctionBuilder& MetaFunctionBuilder::createMetaBB() {
        // create all meta basic block and instructions recursively.
        for (auto bb = F->begin(); bb != F->end(); ++bb) {
            BasicBlock& bbRef   = *bb;
            MetaBB*     newBB   = mF->buildBB();
            bbMap[&bbRef]       = newBB;
            (*newBB)
                .setParentScope(mF)
                .registerToMetaUnit()
                ;

            for (auto i = bbRef.begin(); i != bbRef.end(); ++i) {
                Instruction& inst  = *i;
                (*this)
                    .createMetaInst(inst, *newBB)
                    .createMetaOperand(inst)
                    ;
            }
        }
        return *this;
    }
    
    MetaFunctionBuilder& MetaFunctionBuilder::createMetaInst(Instruction& i, MetaBB& b) {
        MetaInst* newInst = b.buildInstruction((*typeMap)[i.getOpcode()]);

        std::string globalVarNmae = "";
        // set global var name for Load / Store.
        if (isa<LoadInst>(i) || isa<StoreInst>(i)) {
            std::string tyStr;
            raw_string_ostream ros(tyStr);
            i.getOperand(0)->getType()->print(ros);
            Value* first = i.getOperand(0);
            printf("Type of OP[0]: %s\n", tyStr.c_str());

            for (auto it = i.op_begin(); it != i.op_end(); ++it) {
                Value* op = *it;
                // 如果是gep
                if (GEPOperator* gep = dyn_cast<GEPOperator>(op)) {
                    outs() << "Find nested GEP\n";
                    globalVarNmae = gep->getPointerOperand()->getName().str();
                }
                else if (Value* var = LoadStoreVarMap[&i]) {
                    globalVarNmae = var->getName().str();
                }
                printf("Set gloabl var %s for : ", globalVarNmae.c_str());
                outs() << i << "\n";
            }

        }

        (*newInst)
            .setGlobalSymbolName(globalVarNmae)
            .setDataRoot("TIR_GLOBAL")
            .setParentScope(&b)
            .registerToMetaUnit()
            ;

        instMap[&i] = newInst;
        return *this;
    }

    MetaFunctionBuilder& MetaFunctionBuilder::createMetaOperand(Instruction& i) {
        // create all constants.
        for (auto op = i.op_begin(); op != i.op_end(); ++op) {
            Value* value = op->get();
            // Ugly, but works.
            if (Constant* c = dyn_cast<Constant>(value)) {
                // function 也是 constant的子类 跳过不添加
                if (Function* func = dyn_cast<Function>(value)) continue;
                if (constantMap.find(c) != constantMap.end()) continue;
                MetaConstant* newConst = constantMap[c] = mF->buildConstant();
                (*newConst)
                    .setParentScope(mF)
                    .registerToMetaUnit()
                    ;
                if (ConstantData* cd = dyn_cast<ConstantData>(c)) {
                    (*newConst)
                        .setImm(true)
                        .setName("")
                        .setValueStr(cd->getNameOrAsOperand())
                        ;
                    printf("INFO: Creating Immm Constant with value %s.\n", newConst->getValueStr().c_str());
                }
            }
            else {
                printf("find operand type: %d\n", value->getType()->getTypeID());
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
        
        // for (auto op = phi->op_begin(); op != phi->op_end(); ++op) {
        //     Value* value = op->get();
        //     MetaUtil::printValueType(value); 
        //     outs() << "; operand number: " << op->getOperandNo() << "#" << "\n";
        // }
        // outs() << "\n";
    }

    void MetaFunctionBuilder::copyDependencies(Instruction* curInst) {
        MetaInst* inst = instMap[curInst];
        // deal with phi node particularly.
        if (auto phi = dyn_cast<PHINode>(curInst)) { copyDependencies(phi); }

        MetaBB* curBB = (MetaBB*)(inst->getParentScope());
        outs() << "inst " << inst << ",  type: " << curInst->getOpcodeName() << "\n";
        for (auto op = curInst->op_begin(); op != curInst->op_end(); ++op) {
            Value* value = op->get();
            // MetaUtil::printValueType(value);
            // create CFG here.
            if (BasicBlock* bb = dyn_cast<BasicBlock>(value)) {
                (*curBB)
                    .addNextBB(bbMap[bb])
                    .setTerminator(inst)
                    ;
            }

            // only call instruction has function as operand
            else if (Function* func = dyn_cast<Function>(value)) {
                ((MetaCall*)inst)->setFuncName(func->getName().str());
            }

            else if (MetaOperand* metaOp = findMetaOperand(value))
                inst->addOperand(metaOp);

            // outs() << "; operand number: " << op->getOperandNo() << "#" << "\n";
        }

    }

    std::string MetaFunctionBuilder::getMetaUnitStr() {
        return std::move(unit->toString());
    }
        
    MetaUnit* MetaFunctionBuilder::getMetaUnit() { return unit; }

    MetaUnit& MetaFunctionBuilder::getMetaUnitRef() { return *unit; }

} // namespace MetaTrans


static RegisterPass<MetaTrans::MetaFunctionPass> X("meta-trans", "MetaTrans Pass",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);