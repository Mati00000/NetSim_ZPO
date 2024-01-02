#include "reports.hpp"
#include <iostream>
#include <algorithm>

template <typename Node>
void sort_nodes_by_id(Node& nodes) {
    nodes.sort([](const auto& a, const auto& b) {
        return a.get_id() < b.get_id();
    });
}

void sort_receivers(std::vector<IPackageReceiver*>& receivers) {
    std::sort(receivers.begin(), receivers.end(), [](const auto& a, const auto& b) {
        if (a->get_receiver_type() != b->get_receiver_type()) {
            return a->get_receiver_type() < b->get_receiver_type();
        }
        return a->get_id() < b->get_id();
    });
}

void Reports::generate_structure_report(const Factory& f, std::ostream& os) {
    os << "== LOADING RAMPS ==" << std::endl;
    auto ramps = f.ramp_cbegin();
    while (ramps != f.ramp_cend()) {
        os << "LOADING RAMP #" << ramps->get_id() << std::endl;
        os << "  Delivery interval: " << ramps->get_delivery_interval() << std::endl;
        os << "  Receivers:" << std::endl;

        std::vector<IPackageReceiver*> receivers(ramps->receiver_preferences_.cbegin(), ramps->receiver_preferences_.cend());
        sort_receivers(receivers);

        for (const auto& receiver : receivers) {
            os << "    " << receiver->get_id() << std::endl;
        }

        ++ramps;
    }

    os << "== WORKERS ==" << std::endl;
    auto workers = f.worker_cbegin();
    while (workers != f.worker_cend()) {
        os << "WORKER #" << workers->get_id() << std::endl;
        os << "  Processing time: " << workers->get_processing_duration() << std::endl;
        os << "  Queue type: " << (workers->get_queue()->get_queue_type() == PackageQueueType::LIFO ? "LIFO" : "FIFO") << std::endl;
        os << "  Receivers:" << std::endl;

        std::vector<IPackageReceiver*> receivers(workers->receiver_preferences_.cbegin(), workers->receiver_preferences_.cend());
        sort_receivers(receivers);

        for (const auto& receiver : receivers) {
            os << "    " << receiver->get_id() << std::endl;
        }

        ++workers;
    }

    os << "== STOREHOUSES ==" << std::endl;
    auto storehouses = f.storehouse_cbegin();
    while (storehouses != f.storehouse_cend()) {
        os << "STOREHOUSE #" << storehouses->get_id() << std::endl;
        ++storehouses;
    }
}

void Reports::generate_simulation_turn_report(const Factory& f, std::ostream& os, Time t) {
    os << "=== [ Turn: " << t << " ] ===" << std::endl;

    os << "== WORKERS ==" << std::endl;
    auto workers = f.worker_cbegin();
    while (workers != f.worker_cend()) {
        os << "WORKER #" << workers->get_id() << std::endl;
        os << "  PBuffer: #" << (workers->get_processing_buffer().has_value() ? workers->get_processing_buffer()->get_id() : -1)
           << " (pt = " << workers->get_processing_duration() << ")" << std::endl;
        os << "  Queue:";
        for (const auto& package : *workers->get_queue()) {
            os << " #" << package.get_id();
        }
        os << std::endl;
        os << "  SBuffer: #" << (workers->get_sending_buffer().has_value() ? workers->get_sending_buffer()->get_id() : -1) << std::endl;
        ++workers;
    }

    os << "== STOREHOUSES ==" << std::endl;
    auto storehouses = f.storehouse_cbegin();
    while (storehouses != f.storehouse_cend()) {
        os << "STOREHOUSE #" << storehouses->get_id() << std::endl;
        os << "  Stock:";
        for (const auto& package : *storehouses) {
            os << " #" << package.get_id();
        }
        os << std::endl;
        ++storehouses;
    }
}