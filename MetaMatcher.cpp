#include "meta/MetaMatcher.h"


namespace MetaTrans {

MetaMatcher::MetaMatcher() : x(nullptr), y(nullptr) { }

MetaMatcher& MetaMatcher::setX(MetaFunction* x) {
    this->x = x;
    return *this;
}

MetaMatcher& MetaMatcher::setY(MetaFunction* y) {
    this->y = y;
    return *this;
}

MetaMatcher& MetaMatcher::match() {
    for ()


    return *this;
}

std::unordered_map<MetaBB*, MetaBB*>& MetaMatcher::getBBMatchResult() {
    return bbMap;
}

}
