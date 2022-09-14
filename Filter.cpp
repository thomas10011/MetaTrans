#include "meta/Filter.h"

namespace MetaTrans {

//===-------------------------------------------------------------------------------===//
/// Filter Target implementation.

    FilterTarget::FilterTarget() { }

    FilterTarget::~FilterTarget() { }

//===-------------------------------------------------------------------------------===//
/// Filter implementation.

    Filter::Filter() { }

    Filter::~Filter() { }

//===-------------------------------------------------------------------------------===//
/// Filter Chain implementation.

    FilterChain::FilterChain() {

    }

    std::vector<Filter*>::iterator FilterChain::filters_begin() {
        return filters.begin();
    }

    std::vector<Filter*>::iterator FilterChain::filters_end() {
        return filters.end();
    }

    void FilterChain::doFilter(FilterTarget& target) {
        if (++index >= filters.size()) { return; }
        filters[index]->doFilter(target, *this);
    }

    FilterChain& FilterChain::setIndex(int index) {
        this->index = index;
        return *this;
    }

    FilterChain& FilterChain::addFilter(Filter* filter) {
        filters.push_back(filter);
        return *this;
    }


    Filter& FilterChain::getFilter(int index) {
        return *filters[index];
    }


    int FilterChain::getFiltersNum() {
        return filters.size();
    }

//===-------------------------------------------------------------------------------===//
/// Filter Manager implementation.

    FilterManager::FilterManager() { }

    FilterManager::~FilterManager() {
        clear();
    }

    FilterManager& FilterManager::addFilter(Filter* filter) {
        chain.addFilter(filter);
        return *this;
    }

    FilterManager& FilterManager::clear() {
        for (auto it = chain.filters_begin(); it != chain.filters_end(); ++it)
            delete *it;
        chain.filters.clear();
        return *this;
    }

    FilterManager& FilterManager::filter(FilterTarget& target) {
       if (chain.getFiltersNum() == 0) return *this;
       chain
            .setIndex(0)
            .getFilter(0)
            .doFilter(target, chain)
            ;
        return *this;
    }
}