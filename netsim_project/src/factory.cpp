#include "factory.hpp"
#include "nodes.hpp"

#include <vector>
#include <istream>
#include <sstream>
#include <map>
#include <stdexcept>
#include <string>

//--RAMP--//
void Factory::add_ramp(Ramp&& ramp) {
    ramp_.add(std::move(ramp));
}

void Factory::remove_ramp(ElementID id) {
    ramp_.remove_by_id(id);
}

//--WORKER--//
void Factory::add_worker(Worker&& worker) {
    worker_.add(std::move(worker));
}

void Factory::remove_worker(ElementID id) {
    auto worker_it = worker_.find_by_id(id);

    if (worker_it != worker_.end()) {
        Worker& worker = *worker_it;

        for (auto& ramp : ramp_) {
            ramp.receiver_preferences_.remove_receiver(&worker);
        }

        worker_.remove_by_id(id);
    }
}

//--STOREHOUSE--//
void Factory::add_storehouse(Storehouse&& storehouse) {
    storehouse_.add(std::move(storehouse));
}

void Factory::remove_storehouse(ElementID id) {
    storehouse_.remove_by_id(id);
}



bool is_storehouse_achievable(const PackageSender *node, std::map<const PackageSender *, node_colour> &node_states) {
    if (node_states[node] == node_colour::CHECKED) {
        return true;
    }
    node_states[node] = node_colour::VISITED;

    if (node->receiver_preferences_.get_preferences().empty()) {
        throw std::logic_error("No receivers");
    }

    bool has_receiver = false;
    for (auto &receiver: node->receiver_preferences_.get_preferences()) {
        if (receiver.first->get_receiver_type() == ReceiverType::STOREHOUSE) {
            has_receiver = true;
        } else if (receiver.first->get_receiver_type() == ReceiverType::WORKER) {
            IPackageReceiver *receiver_ptr = receiver.first;
            auto worker_ptr = dynamic_cast<Worker *>(receiver_ptr);
            auto sendrecv_ptr = dynamic_cast<PackageSender *>(worker_ptr);
            if (sendrecv_ptr == node) {
                continue;
            }
            has_receiver = true;
            if (node_states[sendrecv_ptr] == node_colour::NOT_VISITED) {
                is_storehouse_achievable(sendrecv_ptr, node_states);
            }
        }
    }

    node_states[node] = node_colour::CHECKED;

    if (!has_receiver) {
        throw std::logic_error("No receiver");
    }
    return true;
}

bool Factory::is_consistent() const {
    std::map<const PackageSender *, node_colour> map;
    for (const PackageSender &ramp: ramp_) {
        map[&ramp] = node_colour::NOT_VISITED;
    }

    for (const PackageSender &worker: worker_) {
        map[&worker] = node_colour::NOT_VISITED;
    }
    try {
        for (const PackageSender &ramp: ramp_) {
            is_storehouse_achievable(&ramp, map);
        }
    }
    catch (...) {
        return false;
    }
    return true;
}

void Factory::do_deliveries(Time time) {
    for(auto e = ramp_.begin(); e != ramp_.end(); e++){
        e->deliver_goods(time);
    }
}

void Factory::do_work(Time time) {
    for(auto e = worker_.begin(); e != worker_.end(); e++){
        e->do_work(time);
    }
}

void Factory::do_package_passing() {
    for(auto e = ramp_.begin(); e != ramp_.end(); e++){
        e->send_package();
    }
    for(auto e = worker_.begin(); e != worker_.end(); e++){
        e->send_package();
    }
}

//--------------------------------------------------------------------------------------------------------------------//


struct ParsedLineData {
    ElementType element_type;
    std::map<std::string, std::string> parameters;
};


ElementType eltype (std::string line){
    if (line == "LOADING_RAMP") { return ElementType::RAMP; }
    else if (line == "WORKER") { return ElementType::WORKER; }
    else if (line == "STOREHOUSE") { return ElementType::STOREHOUSE; }
    else if (line == "LINK") { return ElementType::LINK; }

    throw std::invalid_argument ("Non-existent element");
}

PackageQueueType pqttype(std::string line){
    if(line == "FIFO") {return PackageQueueType::FIFO;}
    if(line == "LIFO") {return PackageQueueType::LIFO;}
    throw std::invalid_argument ("Non-existent queue type");
}

std::string str_pqt (PackageQueueType typ){
    switch (typ) {
        case PackageQueueType::LIFO:
            return "LIFO";
        case PackageQueueType::FIFO:
            return "FIFO";
    }
    throw std::invalid_argument("Taki typ kolejki nie istnieje!");
}

std::string str_rt (ReceiverType typ){
    switch(typ) {
        case ReceiverType::WORKER:
            return "worker";
        case ReceiverType::STOREHOUSE:
            return "store";
    }
    throw std::invalid_argument("Non-existent receiver type");
}

std::vector<std::string> split_line(std::string &line, char delimiter){
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream token_stream(line);
    while(std::getline(token_stream, token, delimiter)){
        tokens.push_back(token);
    }
    return tokens;
}

void load(Factory &factory_, std::vector<std::string> &sender, std::vector<std::string> &rec){
    if (sender[0] == "worker"){
        auto iterator_worker = factory_.find_worker_by_id(static_cast<ElementID>(std::stoi(sender[1])));

        if (rec[0] == "store") {
            auto it_receiver_store = factory_.find_storehouse_by_id(static_cast<ElementID>(std::stoi(rec[1])));
            auto receiver = &(*it_receiver_store);
            iterator_worker->receiver_preferences_.add_receiver(receiver);
        }
        else if (rec[0] == "worker") {
            auto it_receiver_worker = factory_.find_worker_by_id(static_cast<ElementID>(std::stoi(rec[1])));
            auto receiver = &(*it_receiver_worker);
            iterator_worker->receiver_preferences_.add_receiver(receiver);
        }
        else {
            throw std::invalid_argument("Taki typ odbiorcy nie istnieje");
        }
    }
    if (sender[0] == "ramp"){
        auto iterator_ramp = factory_.find_ramp_by_id(static_cast<ElementID>(std::stoi(sender[1])));

        if (rec[0] == "store") {
            auto it_receiver_store = factory_.find_storehouse_by_id(static_cast<ElementID>(std::stoi(rec[1])));
            auto receiver = &(*it_receiver_store);
            iterator_ramp->receiver_preferences_.add_receiver(receiver);
        }
        else if (rec[0] == "worker") {
            auto it_receiver_worker = factory_.find_worker_by_id(static_cast<ElementID>(std::stoi(rec[1])));
            auto receiver = &(*it_receiver_worker);
            iterator_ramp->receiver_preferences_.add_receiver(receiver);
        }
        else {
            throw std::invalid_argument("Taki typ odbiorcy nie istnieje");
        }
    }
    else{
        throw std::invalid_argument("Taki typ nadawcy nie istnieje!");
    }
}

ParsedLineData parse_line (std::string line){
    auto words = split_line(line, ' ');
    std::list<std::string> tokens (std::make_move_iterator(words.begin()),std::make_move_iterator(words.end()));
    ParsedLineData node;
    node.element_type = eltype(std::move((tokens.front())));
    tokens.pop_front();
    for (auto &el :tokens){
        auto key_value = split_line(el, '=');
        node.parameters.insert({key_value[0], key_value[1]});
    }
    return node;
}

Factory load_factory_structure(std::istream& is){
    Factory factory_;
    std::string line;
    while(std::getline(is, line)){
        if (line.empty() || isblank(line[0])) continue;
        else if (!line.empty() && line[0] == ';') continue;
        else{
            auto parsed = parse_line(line);

            if (parsed.element_type == ElementType::LINK) {
                continue;
            }

            switch (parsed.element_type){
                case ElementType::RAMP: {
                    Ramp ramp_(static_cast<ElementID>(std::stoi(parsed.parameters["id"])), std::stoi(parsed.parameters["delivery-interval"]));
                    factory_.add_ramp(std::move(ramp_));
                    break;
                }
                case ElementType::STOREHOUSE: {
                    Storehouse storehouse_(static_cast<ElementID>(std::stoi(parsed.parameters["id"])));
                    factory_.add_storehouse(std::move(storehouse_));
                    break;
                }
                case ElementType::WORKER: {
                    Worker worker_(static_cast<ElementID>(std::stoi(parsed.parameters["id"])), std::stoi(parsed.parameters["processing-time"]), std::make_unique<PackageQueue>(pqttype(parsed.parameters["queue-type"])));
                    factory_.add_worker(std::move(worker_));
                    break;
                }
                case ElementType::LINK: {
                    auto sender_val = split_line(parsed.parameters["src"], '-');
                    auto receiver_val = split_line(parsed.parameters["dest"], '-');
                    load(factory_, sender_val, receiver_val);
                    break;
                }
            }
        }
    }
    return factory_;
}

void save_factory_structure(Factory& factory, std::ostream& os){
    os << "; == LOADING RAMPS ==" << std::endl << std::endl;
    for(auto iterator = factory.ramp_cbegin(); iterator != factory.ramp_cend(); ++iterator){
        os << "LOADING_RAMP id=" << iterator->get_id() << " delivery-interval="<< iterator->get_delivery_interval() << std::endl;
        // Pominięto zapis LINKS, zostaną zapisane na końcu
    }

    os << "; == WORKERS ==" << std::endl << std::endl;
    for(auto iterator = factory.worker_cbegin(); iterator != factory.worker_cend(); ++iterator){
        os << "WORKER id=" << iterator->get_id() << " processing-time="<< iterator->get_processing_duration() << " queue-type=" << str_pqt(iterator->get_queue()->get_queue_type()) << std::endl;
        // Pominięto zapis LINKS, zostaną zapisane na końcu
    }

    os << "; == STOREHOUSES ==" << std::endl << std::endl;
    for(auto iterator = factory.storehouse_cbegin(); iterator != factory.storehouse_cend(); ++iterator){
        os << "STOREHOUSE id=" << iterator->get_id() <<std::endl;
    }

    os << std::endl << "; == LINKS ==" <<std::endl << std::endl;
    for(auto iterator = factory.ramp_cbegin(); iterator != factory.ramp_cend(); ++iterator){
        for (auto elements : iterator->receiver_preferences_.get_preferences()){
            os << "LINK src=ramp-" << iterator->get_id() << " dest=" << str_rt(elements.first->get_receiver_type()) << "-" << elements.first->get_id() << std::endl;
        }
    }

    for(auto iterator = factory.worker_cbegin(); iterator != factory.worker_cend(); ++iterator){
        for (auto elements : iterator->receiver_preferences_.get_preferences()){
            os << "LINK src=worker-" << iterator->get_id() << " dest=" << str_rt(elements.first->get_receiver_type()) << "-" << elements.first->get_id() << std::endl;
        }
    }
}