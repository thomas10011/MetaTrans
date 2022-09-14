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
        
        // do something


        chain.doFilter(target);
    }

    void MetaBBFilter::doFilter(FilterTarget& target, FilterChain& chain) {
        MetaFunctionBuilder& builder = dynamic_cast<MetaFunctionBuilder&>(target);
        
        // do something


        chain.doFilter(target);
    }

    void MetaFuncFilter::doFilter(FilterTarget& target, FilterChain& chain) {
        MetaFunctionBuilder& builder = dynamic_cast<MetaFunctionBuilder&>(target);
        
        // do something


        chain.doFilter(target);
    }
}