#include "reports.hpp"
#include <iostream>
#include <algorithm>

void generate_structure_report(const Factory& f, std::ostream& os){
    //------RAMPS------//
    os  << std::endl << "== LOADING RAMPS =="<< std::endl;
    std::vector<ElementID> sorted_ramps_IDs;

    std::transform(f.ramp_cbegin(), f.ramp_cend(), std::back_inserter(sorted_ramps_IDs), [](const Ramp& ramp_) {
        return ramp_.get_id();
    });

    std::sort(sorted_ramps_IDs.begin(), sorted_ramps_IDs.end());

    for (const auto& id : sorted_ramps_IDs) {
        const auto& ramp_ = *f.find_ramp_by_id(id);
        os << std::endl << "LOADING RAMP #" << ramp_.get_id() << std::endl;
        os << "  Delivery interval: " << ramp_.get_delivery_interval();
        os << std::endl << "  Receivers:";

        std::vector<ElementID> sortedReceiverIDs;
        std::transform(ramp_.receiver_preferences_.cbegin(), ramp_.receiver_preferences_.cend(), std::back_inserter(sortedReceiverIDs),
                       [](const auto& receiver) { return receiver.first->get_id(); });

        std::sort(sortedReceiverIDs.begin(), sortedReceiverIDs.end());
        for(auto& receiverID: sortedReceiverIDs){
            os<<std::endl << "    worker #" << receiverID;
        }
        os<<std::endl;
    };

    //------WORKERS------//
    os << std::endl << std::endl << "== WORKERS ==" << std::endl;

    std::vector<ElementID> sorted_workers_IDs;
    std::transform(f.worker_cbegin(), f.worker_cend(), std::back_inserter(sorted_workers_IDs), [](const Worker& w) {
        return w.get_id();
    });
    std::sort(sorted_workers_IDs.begin(), sorted_workers_IDs.end());

    auto os_WORKER = [&os](const Worker& worker_){
        os << std::endl << "WORKER #" << worker_.get_id() << std::endl;
        os << "  Processing time: " << worker_.get_processing_duration() << std::endl;
        os  << "  Queue type: " << (worker_.get_queue()->get_queue_type() == PackageQueueType::LIFO ? "LIFO" : "FIFO");
        os << std::endl << "  Receivers:";

        std::vector<ElementID> worker_receiverIDs;
        std::vector<ElementID> storehouse_receiverIDs;

        for (const auto& receiver : worker_.receiver_preferences_.get_preferences()) {
            const auto& receiver_id = receiver.first->get_id();
            if (receiver.first->get_receiver_type() == ReceiverType::WORKER) {
                worker_receiverIDs.emplace_back(receiver_id);
            } else if (receiver.first->get_receiver_type() == ReceiverType::STOREHOUSE) {
                storehouse_receiverIDs.emplace_back(receiver_id);
            }
        }

        std::sort(worker_receiverIDs.begin(), worker_receiverIDs.end());
        std::sort(storehouse_receiverIDs.begin(), storehouse_receiverIDs.end());

        for (const auto& receiver_id : worker_receiverIDs) {
            os << std::endl << "    worker #" << receiver_id;
        }

        for (const auto& receiver_id : storehouse_receiverIDs) {
            os << std::endl << "    storehouse #" << receiver_id;
        }

        os << std::endl;
    };
    for (std::vector<ElementID>::const_iterator it = sorted_workers_IDs.begin(); it != sorted_workers_IDs.end(); ++it) {
        os_WORKER(*f.find_worker_by_id(*it));
    }


    //------STOREHOUSES------//
    os << std::endl << std::endl << "== STOREHOUSES ==" <<std::endl;
    std::vector<ElementID> sorted_storehouses_IDs;
    std::transform(f.storehouse_cbegin(), f.storehouse_cend(), std::back_inserter(sorted_storehouses_IDs), [](const Storehouse& storehouse_) {
        return storehouse_.get_id();
    });
    std::sort(sorted_storehouses_IDs.begin(), sorted_storehouses_IDs.end());

    for (const auto& ID : sorted_storehouses_IDs) {
        os << std::endl << "STOREHOUSE #" << ID << std::endl;
    }
    os << std::endl;
}




void generate_simulation_turn_report(const Factory& f, std::ostream& os, Time t) {
    os << "=== [ Turn: " << t << " ] ===" << std::endl;

    //------WORKERS------//
    os << std::endl << "== WORKERS ==" << std::endl;
    std::vector<ElementID> sorted_workers_IDs;
    std::transform(f.worker_cbegin(), f.worker_cend(), std::back_inserter(sorted_workers_IDs), [](const Worker &worker_) {
        return worker_.get_id();
    });
    std::sort(sorted_workers_IDs.begin(), sorted_workers_IDs.end());

    auto os_WORKER = [&os, t](const Worker& worker_) {
        os << std::endl << "WORKER #" << worker_.get_id() << std::endl;

        //----PBuffer----//
        if (worker_.get_processing_buffer()) {
            if (t - worker_.get_package_processing_start_time() == 0) {
                os << "  PBuffer: (empty)" << std::endl;
            } else {
                os << "  PBuffer: #" << worker_.get_processing_buffer()->get_id() << " (pt = "
                   << (t - worker_.get_package_processing_start_time()) << ")"
                   << std::endl;
            }
        } else {
            os << "  PBuffer: (empty)" << std::endl;
        }

        //----QUEUE----//
        os << "  Queue: ";
        const auto& worker_queue = worker_.get_queue();
        if (!worker_queue->empty()) {
            std::vector<ElementID> queue_ids;
            std::transform(worker_queue->cbegin(), worker_queue->cend(), std::back_inserter(queue_ids),
                           [](const Package &p) { return p.get_id(); });
            std::sort(queue_ids.begin(), queue_ids.end());
            os << "#" << queue_ids[0];
            if (queue_ids.size() > 1) {
                std::for_each(queue_ids.cbegin() + 1, queue_ids.cend(),
                              [&os](const ElementID &id) { os << ", #" << id; });
            }
        } else {
            os << "(empty)";
        }

        //----SBuffer----//
        os << std::endl << "  SBuffer: "
           << (worker_.get_sending_buffer().has_value() ? "#" + std::to_string(worker_.get_sending_buffer()->get_id()) : "(empty)")
           << std::endl;

    };
    for (const auto &id : sorted_workers_IDs) {
        os_WORKER(*f.find_worker_by_id(id));
    }

    //------STOREHOUSES------//
    os << std::endl << std::endl << "== STOREHOUSES ==" << std::endl << std::endl;
    std::vector<ElementID> sorted_storehouse_IDs;
    std::transform(f.storehouse_cbegin(), f.storehouse_cend(), std::back_inserter(sorted_storehouse_IDs), [](const Storehouse &s) {
        return s.get_id();
    });
    std::sort(sorted_storehouse_IDs.begin(), sorted_storehouse_IDs.end());

    auto os_STOREHOUSE =[&os](const Storehouse& storehouse_){
        os << "STOREHOUSE #" << storehouse_.get_id();

        if (storehouse_.cbegin() != storehouse_.cend()){
            std::vector<ElementID> sorted_elements_IDs;
            std::for_each(storehouse_.cbegin(), storehouse_.cend(), [&sorted_elements_IDs](const Package& p){sorted_elements_IDs.emplace_back(p.get_id());});
            std::sort(sorted_elements_IDs.begin(), sorted_elements_IDs.end());
            os << std::endl << "  Stock: #" << sorted_elements_IDs[0];

            if(sorted_elements_IDs.size() > 1){
                std::for_each(sorted_elements_IDs.cbegin(), sorted_elements_IDs.cend(), [&os](const ElementID& id){os<<", #"<<id;});
            }

            os << std::endl << std::endl;
        }
        else{
            os << std::endl << "  Stock: (empty)" << std::endl << std::endl;
        }
    };

    for (const auto& id : sorted_storehouse_IDs) {
        os_STOREHOUSE(*f.find_storehouse_by_id(id));
    }

}