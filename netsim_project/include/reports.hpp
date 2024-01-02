#ifndef REPORTS_HPP_
#define REPORTS_HPP_

#include <factory.hpp>
#include <ostream>

void generate_structure_report(const Factory& f, std::ostream& os);
void generate_simulation_turn_report(const Factory& f, std::ostream& os, Time t);

#endif /* REPORTS_HPP_ */