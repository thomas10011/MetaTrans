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
                .setParent(builder.mF)
                .setArgType(MetaUtil::extractDataType(*(it->getType())))
                .setWidth(MetaUtil::extractDataWidth(*(it->getType())))
                ;
            offset += arg.getWidth();
        }

        chain.doFilter(target);
    }

    void MetaConstantFilter::doFilter(FilterTarget& target, FilterChain& chain) {
        MetaFunctionBuilder& builder = dynamic_cast<MetaFunctionBuilder&>(target);
        // do something

        chain.doFilter(target);
    }

    void MetaInstFilter::doFilter(FilterTarget& target, FilterChain& chain) {
        MetaFunctionBuilder& builder = dynamic_cast<MetaFunctionBuilder&>(target);
        
        // TODO - scan alloca inst and add statck size in meta function.
        // Problem - if compile with -o3 then ir may have no alloca instruction.

        std::unordered_map<Instruction*, MetaInst*>& instMap = builder.instMap;

        for (auto iter = instMap.begin(); iter != instMap.end(); ++iter) {
            iter->second->setOriginInst(iter->first->getOpcodeName());
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
                .setEntry(builder.instMap[b.getFirstNonPHI()])
                .setParent(builder.mF)
                .setTerminator(instList[instList.size() - 1])
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
            (**bb_iter)
                .addFeature(degreeMap[*bb_iter][0])
                .addFeature(degreeMap[*bb_iter][1])
                ;
        }

        chain.doFilter(target);

    }
}