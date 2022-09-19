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


        chain.doFilter(target);
    }

    void MetaBBFilter::doFilter(FilterTarget& target, FilterChain& chain) {
        MetaFunctionBuilder& builder = dynamic_cast<MetaFunctionBuilder&>(target);
        for (auto it = builder.F->begin(); it != builder.F->end(); ++it) {
            BasicBlock& b  = *it;
            MetaBB&     mB = *(builder.bbMap[&b]);
            assert(b.getFirstNonPHI());
            mB
                .setEntry(builder.instMap[b.getFirstNonPHI()])
                .setParent(builder.mF);
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
}