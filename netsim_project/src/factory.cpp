#include "factory.hpp"

template <typename Node>
void NodeCollection<Node>::add(Node&& node) {
    nodes_.emplace_back(std::move(node));
}

template <typename Node>
void NodeCollection<Node>::remove_by_id(ElementID id) {
    auto it = find_by_id(id);
    if (it != end()) {
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

enum class node_colour { NOT_VISITED, VISITED, CHECKED }; /// nwm czy to ma być zdefiniowane w metodzie czy poza nią i czy w tym pliku czy w nagłówkowym

bool Factory::is_consistent() const { /// not finished


    return false;
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
