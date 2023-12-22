#ifndef FACTORY_HPP_
#define FACTORY_HPP_

#include "nodes.hpp"
#include "types.hpp"
#include "algorithm"
#include <stdexcept>

template <typename Node>
class NodeCollection {
public:
    using container_t = typename std::list<Node>;
    using iterator = typename container_t::iterator;
    using const_iterator = typename container_t::const_iterator;

    void add(Node&& node);
    void remove_by_id(ElementID id);

    NodeCollection<Node>::iterator find_by_id(ElementID id);
    NodeCollection<Node>::const_iterator find_by_id(ElementID id) const;

    NodeCollection<Node>::iterator begin() { return nodes_.begin(); }
    NodeCollection<Node>::iterator end() { return nodes_.end(); }

    NodeCollection<Node>::const_iterator cbegin() const { return nodes_.cbegin(); }
    NodeCollection<Node>::const_iterator cend() const { return nodes_.cend(); }
    NodeCollection<Node>::const_iterator begin() const { return nodes_.begin(); }
    NodeCollection<Node>::const_iterator end() const { return nodes_.end(); }

private:
    container_t nodes_;
};


class Factory {
public:
    //---RAMP---//
    void add_ramp(Ramp&& ramp);
    void remove_ramp(ElementID id);

    NodeCollection<Ramp>::iterator find_ramp_by_id(ElementID id) { return ramp_.find_by_id(id); }
    NodeCollection<Ramp>::const_iterator find_ramp_by_id(ElementID id) const { return ramp_.find_by_id(id); }
    NodeCollection<Ramp>::const_iterator ramp_cbegin() const { return ramp_.cbegin(); }
    NodeCollection<Ramp>::const_iterator ramp_cend() const { return ramp_.cend(); }

    //---WORKER---//
    void add_worker(Worker&& worker);
    void remove_worker(ElementID id);

    NodeCollection<Worker>::iterator find_worker_by_id(ElementID id) { return worker_.find_by_id(id); }
    NodeCollection<Worker>::const_iterator find_worker_by_id(ElementID id) const { return worker_.find_by_id(id); }
    NodeCollection<Worker>::const_iterator worker_cbegin() const { return worker_.cbegin(); }
    NodeCollection<Worker>::const_iterator worker_cend() const { return worker_.cend(); }

    //---STOREHOUSE---//
    void add_storehouse(Storehouse&& storehouse);
    void remove_storehouse(ElementID id);

    NodeCollection<Storehouse>::iterator find_storehouse_by_id(ElementID id) { return storehouse_.find_by_id(id); }
    NodeCollection<Storehouse>::const_iterator find_storehouse_by_id(ElementID id) const { return storehouse_.find_by_id(id); }
    NodeCollection<Storehouse>::const_iterator storehouse_cbegin() const { return storehouse_.cbegin(); }
    NodeCollection<Storehouse>::const_iterator storehouse_cend() const { return storehouse_.cend(); }

    bool is_consistent() const;
    void do_deliveries(Time time);
    void do_package_passing();
    void do_work(Time time);

private:
    template<typename Node>
    void remove_receiver(NodeCollection<Node>& collection, ElementID id);

    NodeCollection<Ramp> ramp_;
    NodeCollection<Worker> worker_;
    NodeCollection<Storehouse> storehouse_;
};

#endif /* FACTORY_HPP_ */