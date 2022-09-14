#pragma once
#include <vector>

namespace MetaTrans {

    class FilterTarget;
    class Filter;
    class FilterChain;
    class FilterManager;

    class FilterTarget {

        public:

            FilterTarget();

            virtual ~FilterTarget();

    };

    class Filter {
        public:

            Filter();

            virtual ~Filter();

            virtual void doFilter(FilterTarget &target, FilterChain &chain) = 0;

    };

    class FilterChain {
        protected:
            
            friend class FilterManager;

            unsigned index;

            std::vector<Filter*> filters;

            std::vector<Filter*>::iterator filters_begin();

            std::vector<Filter*>::iterator filters_end();

            FilterChain();

            FilterChain& setIndex(int index);

            FilterChain& addFilter(Filter* filter);

            Filter& getFilter(int index);

            int getFiltersNum();

        public:

            void doFilter(FilterTarget& target);

    };

    class FilterManager {

        protected:

            FilterChain chain;

        public:

            FilterManager();

            ~FilterManager();

            FilterManager& addFilter(Filter* filter);

            FilterManager& clear();

            FilterManager& filter(FilterTarget& target);

    };

    
}
