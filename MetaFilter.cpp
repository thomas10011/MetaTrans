#include "meta/MetaFilter.h"
#include "meta/MetaPass.h"


namespace MetaTrans {


    void MetaArgFilter::doFilter(FilterTarget& target, FilterChain& chain) {
        MetaFunctionBuilder& builder = dynamic_cast<MetaFunctionBuilder&>(target);

        int index = 0, offset = 0; 
        for (auto it = builder.F->arg_begin(); it != builder.F->arg_end(); ++it) {
            MetaArgument& arg = *(builder.argMap[&(*it)]);
            arg
                .setArgIndex(index++)
                .setOffset(offset)
                .setParentScope(builder.mF)
                .setDataType(MetaUtil::extractDataType(*(it->getType())))
                .setWidth(MetaUtil::extractDataWidth(*(it->getType())))
                ;
            offset += arg.getWidth();
        }

        chain.doFilter(target);
    }

    void MetaConstantFilter::doFilter(FilterTarget& target, FilterChain& chain) {
        MetaFunctionBuilder& builder = dynamic_cast<MetaFunctionBuilder&>(target);
        // do something
        Module& module = *(builder.F->getParent());
        std::unordered_map<Constant*, MetaConstant*> map = builder.constantMap;
        
        printf("number of global values: %d\nnumber of alias: %d\n", module.getGlobalList().size(), module.getAliasList().size());
        for (auto& global : module.getGlobalList()) {
            printf("meta address for global var %s is: %d\n", global.getName().str().c_str(), map[&global]);
        }
        
        chain.doFilter(target);
    }

    void MetaInstFilter::doFilter(FilterTarget& target, FilterChain& chain) {
        MetaFunctionBuilder& builder = dynamic_cast<MetaFunctionBuilder&>(target);
        
        // TODO - scan alloca inst and add statck size in meta function.
        // Problem - if compile with -o3 then ir may have no alloca instruction.

        BidirectionMap<Instruction*, MetaInst*>& instMap = builder.instMap;

        for (auto iter = instMap.begin(); iter != instMap.end(); ++iter) {
            Instruction* irInst = iter->first;
            MetaInst* metaInst = iter->second;

            std::string type = irInst->getOpcodeName();
            if (CmpInst* cmp = dyn_cast<CmpInst>(irInst)) {
                std::string pred = cmp->getPredicateName(cmp->getPredicate()).str();
                printf("type of cmp: %s\n", pred.c_str());
                type = type + " " + pred;
            }
            metaInst->setOriginInst(type);
        }

        chain.doFilter(target);
    }

    void MetaBBFilter::doFilter(FilterTarget& target, FilterChain& chain) {
        MetaFunctionBuilder& builder = dynamic_cast<MetaFunctionBuilder&>(target);
        for (auto it = builder.F->begin(); it != builder.F->end(); ++it) {
            BasicBlock& b  = *it;
            MetaBB&     mB = *(builder.bbMap[&b]);
            std::vector<MetaInst*>& instList = mB.getInstList();
            assert(b.getFirstNonPHI());
            mB
                .setEntry       (builder.instMap.get(b.getFirstNonPHI()))
                .setParentScope (builder.mF)
                .setTerminator  (instList[instList.size() - 1])
                ;
        }
        chain.doFilter(target);
    }

    void MetaFuncFilter::doFilter(FilterTarget& target, FilterChain& chain) {
        MetaFunctionBuilder& builder = dynamic_cast<MetaFunctionBuilder&>(target);

        Function&       func     = *(builder.F);
        MetaFunction&   metaFunc = *(builder.mF);

        metaFunc
            .setStackSize(0)
            .setRoot(builder.bbMap[&*(func.begin())])
            .setFunctionName(func.getName().str())
            .setReturnType(MetaUtil::extractDataType(*(func.getReturnType()))) // TODO 修改为适配器模式
            ;

        chain.doFilter(target);
    }

    void MetaIDFilter::doFilter(FilterTarget& target, FilterChain& chain) {
        MetaFunctionBuilder&                        builder     =   dynamic_cast<MetaFunctionBuilder&>(target);
        MetaFunction&                               metaFunc    =   *(builder.mF);

        // set id fro each meta basic block and meta operand.
        int operand_id = 0, bb_id = 0;

        for (auto const_iter = metaFunc.const_begin(); const_iter != metaFunc.const_end(); ++const_iter) {
            (**const_iter).setID(operand_id++);
        }

        for (auto arg_iter = metaFunc.arg_begin(); arg_iter != metaFunc.arg_end(); ++arg_iter) {
            (**arg_iter).setID(operand_id++);
        }

        for (auto bb_iter = metaFunc.begin(); bb_iter != metaFunc.end(); ++bb_iter) {
            MetaBB& bb = **bb_iter;
            bb.setID(bb_id++);
            for (auto inst_iter = bb.begin(); inst_iter != bb.end(); ++inst_iter) {
                (**inst_iter).setID(operand_id++);
            }
        }

        chain.doFilter(target);

    }

    void MetaFeatureFilter::doFilter(FilterTarget& target, FilterChain& chain) {
        MetaFunctionBuilder&                        builder     =   dynamic_cast<MetaFunctionBuilder&>(target);
        MetaFunction&                               metaFunc    =   *(builder.mF);

        std::unordered_map<MetaBB*, std::vector<int>> degreeMap;

        for (auto bb_iter = metaFunc.begin(); bb_iter != metaFunc.end(); ++bb_iter) {
            degreeMap[*bb_iter] = std::vector<int>(2, 0);
        }

        for (auto bb_iter = metaFunc.begin(); bb_iter != metaFunc.end(); ++bb_iter) {
            MetaBB& bb = **bb_iter;
            // self loop只统计入度
            for (auto next_bb = bb.next_begin(); next_bb != bb.next_end(); ++next_bb) {
                degreeMap[*next_bb][0]++;
                if (*next_bb != *bb_iter) degreeMap[*bb_iter][1]++;
            }
            // 统计 load store 数量
            int load_count = 0, store_count = 0;
            for (auto inst_iter = bb.inst_begin(); inst_iter != bb.inst_end(); ++inst_iter) {
                MetaInst& inst = **inst_iter;
                if (inst.isLoad()) load_count++;
                if (inst.isStore()) store_count++;
            }
            
            bb
                .addFeature(load_count)
                .addFeature(store_count)
                ;
        }

        for (auto bb_iter = metaFunc.begin(); bb_iter != metaFunc.end(); ++bb_iter) {
            int in  = degreeMap[*bb_iter][0];
            int out = degreeMap[*bb_iter][1];

            // CFG应当只有一个出口，出度为0。
            if (out == 0) metaFunc.setExit(*bb_iter);

            (**bb_iter)
                .addFeature(in)
                .addFeature(out)
                ;
        }

        chain.doFilter(target);

    }
}