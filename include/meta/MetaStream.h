//===-------------------------------------------------------------------------------===//
/// An implementation of some simple stream operator.
/// author: hrtan
//===-------------------------------------------------------------------------------===//

#include <vector>

namespace MetaTrans {

template<class T>
class Stream {

private:

    std::vector<T>& dataVec;

public:

    Stream(std::vector<T>& v) : dataVec(v) { }

    void forEach(void (*lambda)(T element)) {
        for (T e : dataVec) lambda(e);
    }

    template<class K>
    std::vector<K> map(K (*lambda)(T element)) {
        std::vector<K> mapped;
        for (T e : dataVec) mapped.push_back(lambda(e));
        return mapped;
    }

};

} // end namespace MetaTrans