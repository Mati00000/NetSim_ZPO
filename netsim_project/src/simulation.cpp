#include "simulation.hpp"
#include "types.hpp"
#include "factory.hpp"

void simulate(Factory& f,TimeOffset d, const std::function<void (Factory&, Time)>& rf) {
    if(!f.is_consistent())
        throw std::logic_error("Not consistent");
    else
        rf(f, d);
    for(int i = 1; i <= d; i++){
        f.do_deliveries(i);
        f.do_package_passing();
        f.do_work(i);
    }
}