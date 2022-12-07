#include "MetaTrans.h"

#include <unordered_map>

namespace MetaTrans {

class MetaMatcher {

private:

protected:

MetaFunction* x, y;

std::unordered_map<MetaBB*, MetaBB*> bbMap;

public:

MetaMatcher();

MetaMatcher& setX(MetaFunction* x);

MetaMatcher& setY(MetaFunction* y);

MetaMatcher& match();

std::unordered_map<MetaBB*, MetaBB*>& getBBMatchResult();




};


} // namespace MetaTrans