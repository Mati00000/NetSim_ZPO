#ifndef TYPES_HPP_
#define TYPES_HPP_

#include <functional>

using ElementID = unsigned long long;
using Time = unsigned long long;
using TimeOffset = unsigned long long;
using ProbabilityGenerator = std::function<double()>;

#endif /* TYPES_HPP_ */