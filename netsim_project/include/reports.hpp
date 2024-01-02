#ifndef REPORTS_HPP_
#define REPORTS_HPP_

#include <factory.hpp>
#include <ostream>

static void generate_structure_report(const Factory& f, std::ostream& os);

static void generate_simulation_turn_report(const Factory& f, std::ostream& os, Time t);

class IntervalReportNotifier {
public:
    explicit IntervalReportNotifier(TimeOffset to) : to_(to) {}
    bool  should_generate_report(Time t) { return (t - 1) % to_ == 0; }

private:
    TimeOffset to_;
};

class SpecificTurnsReportNotifier {
public:
    explicit SpecificTurnsReportNotifier(std::set<Time> turns) : turns_(std::move(turns)) {}
    bool should_generate_report(Time t) { return turns_.find(t) != turns_.end(); }

private:
    std::set<Time> turns_;
};

#endif /* REPORTS_HPP_ */