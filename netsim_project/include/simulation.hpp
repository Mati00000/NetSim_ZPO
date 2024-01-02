#ifndef SIMULATION_HPP_
#define SIMULATION_HPP_

#include "factory.hpp"

void simulate(Factory& f,TimeOffset d,const std::function<void (Factory&, Time)>& rf);

#endif /* SIMULATION_HPP_ */