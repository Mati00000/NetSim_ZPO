#include "factory.hpp"

template <typename Node>
void NodeCollection<Node>::add(Node&& node) {
    nodes_.emplace_back(std::move(node));
}

template <typename Node>
void NodeCollection<Node>::remove_by_id(ElementID id) {
    auto it = find_by_id(id);
    if (it != nodes_.end()) {
        nodes_.erase(it);
    }
}

template <typename Node>
typename NodeCollection<Node>::iterator NodeCollection<Node>::find_by_id(ElementID id) {
    return std::find_if(nodes_.begin(), nodes_.end(), [id](const Node& node) {
        return node.get_id() == id;
    });
}

template <typename Node>
typename NodeCollection<Node>::const_iterator NodeCollection<Node>::find_by_id(ElementID id) const {
    return std::find_if(nodes_.cbegin(), nodes_.cend(), [id](const Node& node) {
        return node.get_id() == id;
    });
}

void Factory::add_ramp(Ramp&& ramp) {
    ramp_.add(std::move(ramp));
}

void Factory::remove_ramp(ElementID id) {
    ramp_.remove_by_id(id);
}

void Factory::add_worker(Worker&& worker) {
    worker_.add(std::move(worker));
}

void Factory::remove_worker(ElementID id) {
    worker_.remove_by_id(id);
}

void Factory::add_storehouse(Storehouse&& storehouse) {
    storehouse_.add(std::move(storehouse));
}

void Factory::remove_storehouse(ElementID id) {
    storehouse_.remove_by_id(id);
}

enum class node_colour { NOT_VISITED, VISITED, CHECKED };


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

template <typename Node>
void Factory::remove_receiver(NodeCollection<Node>& collection, ElementID id) {
    for (auto& node : collection) {
        node.remove_receiver(id);
    }
}
