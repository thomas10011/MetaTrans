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

    Stream(std::vector<T>&& v) : s(v) { }

    Stream<T>& forEach(void (*consumer)(T element)) {
        for (T& e : s) consumer(e);
        return *this;
    }

    Stream<T>& forEach(const std::function<void(T)>& consumer) {
        for (T& e : s) consumer(e);
        return *this;
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

    // the `mapper` parameter is a forwarding reference, which means it can accept both lvalues and rvalues.
    // use `std::declval` to get the type of the return value of the mapper function when passed an object of type T. 
    // use decltype, which returns the type of its argument.
    template<class Mapper>
    auto map(Mapper&& mapper) -> Stream<decltype(mapper(std::declval<T>()))> {
        // Get the return type of the mapper function using `std::declval` and `decltype`.
        using K = decltype(mapper(std::declval<T>()));
        std::vector<K> mapped;
        for (T e : s) mapped.push_back(mapper(e));
        // create a new string using perfect forward.
        return Stream<K>(std::move(mapped));
    }

    template<class K>
    Stream<K> map(K (*mapper)(T element)) {
        std::vector<K> mapped;
        for (T e : s) mapped.push_back(mapper(e));
        return Stream<K>(std::move(mapped));
    }

    std::vector<T> collect() {
        return std::move(s);
    }

};

} // end namespace MetaTrans