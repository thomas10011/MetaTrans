#pragma once
#include "meta/Filter.h"

namespace MetaTrans {

    class MetaArgFilter : public Filter {

        public:

            virtual void doFilter(FilterTarget& target, FilterChain& chain) override;

    };

    class MetaConstantFilter : public Filter {

        public:

            virtual void doFilter(FilterTarget& target, FilterChain& chain) override;

    };

    class MetaInstFilter : public Filter {

        public:

            virtual void doFilter(FilterTarget& target, FilterChain& chain) override;

    };

    class MetaBBFilter : public Filter {

        public:

            virtual void doFilter(FilterTarget& target, FilterChain& chain) override;

    };

    class MetaFuncFilter : public Filter {

        public:

            virtual void doFilter(FilterTarget& target, FilterChain& chain) override;

    };

    class MetaIDFilter : public Filter {

        public:

            virtual void doFilter(FilterTarget& target, FilterChain& chain) override;

    };
    
    class MetaFeatureFilter : public Filter {

        public:

            virtual void doFilter(FilterTarget& target, FilterChain& chain) override;

    }

}