#include "meta/MetaData.h"
#include "meta/MetaUtils.h"


namespace MetaTrans {


MetaPrimType::MetaPrimType() : type(DataType::VOID), width(32), ptrLevel(0) { }

MetaPrimType::MetaPrimType(std::string s) {
    std::vector<std::string> tmp = MetaUtil::split("_", s);
    type = MetaUtil::stringToDataType(tmp[0]);
    width = std::stoi(tmp[1]);
    ptrLevel = tmp[2].length();
}

MetaPrimType::MetaPrimType(DataType ty, int width, int ptrLevel) {
    this->type = ty;
    this->width = width;
    this->ptrLevel = ptrLevel;
}

MetaPrimType& MetaPrimType::setType(DataType ty) {
    type = ty;
    return *this;
}

MetaPrimType& MetaPrimType::setWidth(int w) {
    width = w;
    return *this;
}

MetaPrimType& MetaPrimType::setPrtLevel(int l) {
    ptrLevel = l;
    return *this;
}

DataType MetaPrimType::getType() { return type; }

int MetaPrimType::getWidth() { return width; }

int MetaPrimType::getPtrLevel() { return ptrLevel; }

std::string MetaPrimType::toString() {
    std::string ret = "";
    ret += MetaUtil::toString(type) + "_";
    ret += std::to_string(width) + "_";
    for (int i = 0; i < ptrLevel; ++i) ret += "*";
    return ret;
}


} // namespace MetaTrans