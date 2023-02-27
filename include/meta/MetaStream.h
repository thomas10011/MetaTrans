//===-------------------------------------------------------------------------------===//
/// An implementation of some simple stream operator.
/// author: hrtan
//===-------------------------------------------------------------------------------===//

#include <vector>
#include <algorithm>

namespace MetaTrans {

template<class T>
class Stream {

private:

    std::vector<T> s;

public:

    Stream(std::vector<T>& v) : s(v) { }

    void forEach(void (*consumer)(T element)) {
        for (T e : s) consumer(e);
    }

    void forEach(const std::function<void(T)>& consumer) {
        for (T e : s) consumer(e);
    }

    template<class UnaryPredicate>
    Stream<T>& filter(UnaryPredicate predicate) {
        std::vector<T> filtered;
        for (T e : s) if (predicate(e)) filtered.push_back(e);
        s = filtered;
        return *this;
    }

    Stream<T>& filter(bool (*predicate)()) {
        std::vector<T> filtered;
        for (T e : s) if (predicate(e)) filtered.push_back(e);
        s = filtered;
        return *this;
    }

    template<class K>
    Stream<K> map(const std::function<K(T)>& mapper) {
        std::vector<K> mapped;
        for (T e : s) mapped.push_back(mapper(e));
        Stream<K> newStream(mapped);
        return newStream;
    }

    template<class K>
    Stream<K> map(K (*mapper)(T element)) {
        std::vector<K> mapped;
        for (T e : s) mapped.push_back(mapper(e));
        Stream<K> newStream(mapped);
        return newStream;
    }

    std::vector<T> collect() {
        return s;
    }

};

} // end namespace MetaTrans