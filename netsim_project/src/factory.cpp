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

//------LOAD-SAVE-FACTORY------//

struct ParsedLineData {
    ElementType element_type;
    std::map<std::string, std::string> parameters;
};

ElementType ElementType_ (std::string line){
    if (line == "LOADING_RAMP") { return ElementType::RAMP; }
    else if (line == "WORKER") { return ElementType::WORKER; }
    else if (line == "STOREHOUSE") { return ElementType::STOREHOUSE; }
    else if (line == "LINK") { return ElementType::LINK; }

    throw std::invalid_argument ("Non-existent element");
}

PackageQueueType PackageQueueType_ (std::string line){
    if(line == "FIFO") {return PackageQueueType::FIFO;}
    if(line == "LIFO") {return PackageQueueType::LIFO;}
    throw std::invalid_argument ("Non-existent queue type");
}

std::string PackageQueueType_string_ (PackageQueueType type){
    switch (type) {
        case PackageQueueType::LIFO:
            return "LIFO";
        case PackageQueueType::FIFO:
            return "FIFO";
    }
    throw std::invalid_argument("Non-existent queue type");
}

std::string ReceiverType_string_ (ReceiverType type){
    switch(type) {
        case ReceiverType::WORKER:
            return "worker";
        case ReceiverType::STOREHOUSE:
            return "store";
    }
    throw std::invalid_argument("Non-existent receiver type");
}

std::vector<std::string> SplitLine_(std::string &line, char di){
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream token_stream(line);

    while(std::getline(token_stream, token, di)){
        tokens.push_back(token);
    }

    return tokens;
}

ParsedLineData ParseLine_ (std::string line){
    auto words = SplitLine_(line, ' ');
    std::list<std::string> tokens (std::make_move_iterator(words.begin()),std::make_move_iterator(words.end()));
    ParsedLineData node;
    node.element_type = ElementType_(std::move((tokens.front())));
    tokens.pop_front();

    for (auto &el :tokens){
        auto key_value = SplitLine_(el, '=');
        node.parameters.insert({key_value[0], key_value[1]});
    }

    return node;
}

Factory load_factory_structure(std::istream& is){
    Factory factory;

    std::string line;
    while(std::getline(is, line)){
        if(!line.empty() && line[0] != ';'){
            auto parsed= ParseLine_(line);

            switch (parsed.element_type) {
                case ElementType::RAMP: {
                    factory.add_ramp(Ramp(std::stoi(parsed.parameters["id"]), std::stoi(parsed.parameters["delivery-interval"])));
                    break;
                }

                case ElementType::STOREHOUSE: {
                    factory.add_storehouse(Storehouse(std::stoi(parsed.parameters["id"])));
                    break;
                }

                case ElementType::WORKER: {
                    if (parsed.parameters["queue-type"] == "FIFO"){
                        factory.add_worker(Worker(std::stoi(parsed.parameters["id"]), std::stoi(parsed.parameters["processing-time"]), std::make_unique<PackageQueue>(PackageQueueType::FIFO)));
                    }

                    else if (parsed.parameters["queue-type"]=="LIFO"){
                        factory.add_worker(Worker(std::stoi(parsed.parameters["id"]), std::stoi(parsed.parameters["processing-time"]), std::make_unique<PackageQueue>(PackageQueueType::LIFO)));
                    }
                    break;
                }

                case ElementType::LINK: {
                    std::vector<std::string> src;
                    std::vector<std::string> dest;
                    std::string token;
                    std::istringstream token_src(parsed.parameters["src"]);

                    while (std::getline(token_src, token, '-')){
                        src.push_back(token);
                    }

                    std::istringstream token_dest(parsed.parameters["dest"]);

                    while (std::getline(token_dest, token, '-')){
                        dest.push_back(token);
                    }

                    if (src[0] == "worker" && dest[0] == "store"){
                        auto obj_src = factory.find_worker_by_id(std::stol(src[1]));
                        auto obj_dest = factory.find_storehouse_by_id(std::stol(dest[1]));
                        obj_src->receiver_preferences_.add_receiver(&*obj_dest);
                    }
                    else if (src[0] == "ramp" && dest[0] == "worker"){
                        auto obj_src = factory.find_ramp_by_id(std::stol(src[1]));
                        auto obj_dest = factory.find_worker_by_id(std::stol(dest[1]));
                        obj_src->receiver_preferences_.add_receiver(&*obj_dest);
                    }
                    else if (src[0] == "worker" && dest[0] == "worker") {
                        auto obj_src = factory.find_worker_by_id(std::stol(src[1]));
                        auto obj_dest = factory.find_worker_by_id(std::stol(dest[1]));
                        obj_src->receiver_preferences_.add_receiver(&*obj_dest);
                    }
                    else if(src[0] == "ramp" && dest[0] == "store"){
                        auto obj_src = factory.find_ramp_by_id(std::stol(src[1]));
                        auto obj_dest = factory.find_storehouse_by_id(std::stol(dest[1]));
                        obj_src->receiver_preferences_.add_receiver(&*obj_dest);
                    }

                    break;
                }
            }
        }
    }

    return factory;
}


void save_factory_structure(Factory& factory, std::ostream& os){
    std::ostringstream tm;

    //--LOADING-RAMPS--//
    os << "; == LOADING RAMPS ==" << std::endl << std::endl;
    for(auto iterator = factory.ramp_cbegin(); iterator != factory.ramp_cend(); ++iterator){
        os << "LOADING_RAMP id=" << iterator->get_id() << " delivery-interval="<< iterator->get_delivery_interval() << std::endl;
        for (auto elements : iterator->receiver_preferences_.get_preferences()){
            tm << "LINK src=ramp-" << iterator->get_id() << " dest=" << ReceiverType_string_(elements.first->get_receiver_type()) << "-" << elements.first->get_id() << std::endl;
        }

        tm << std::endl;
    }

    //--WORKERS--//
    os << "; == WORKERS ==" << std::endl << std::endl;
    for(auto iterator = factory.worker_cbegin(); iterator != factory.worker_cend(); ++iterator){
        os << "WORKER id=" << iterator->get_id() << " processing-time="<< iterator->get_processing_duration() << " queue-type=" << PackageQueueType_string_(iterator->get_queue()->get_queue_type()) << std::endl;
        for (auto elements : iterator->receiver_preferences_.get_preferences()){
            tm << "LINK src=worker-" << iterator->get_id() << " dest=" << ReceiverType_string_(elements.first->get_receiver_type()) << "-" << elements.first->get_id() << std::endl;
        }
        tm << std::endl;
    }

    //--STOREHOUSES--//
    os << "; == STOREHOUSES ==" << std::endl << std::endl;
    for(auto iterator = factory.storehouse_cbegin(); iterator != factory.storehouse_cend(); ++iterator){
        os << "STOREHOUSE id=" << iterator->get_id() <<std::endl;
    }

    //--LINKS--//
    os << std::endl << "; == LINKS ==" <<std::endl << std::endl;
    os << tm.str();
}