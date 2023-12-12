#ifndef NODES_HPP_
#define NODES_HPP_

#include "types.hpp"
#include "package.hpp"
#include "storage_types.hpp"
#include "helpers.hpp"

#include <map>

class IPackageReceiver{
public:
    using const_iterator = typename IPackageStockpile::const_iterator;

    virtual const_iterator cbegin() const = 0;
    virtual const_iterator cend() const = 0;
    virtual const_iterator begin() const = 0;
    virtual const_iterator end() const = 0;

    virtual void receive_package(Package &&p) = 0;
    virtual ElementID get_id() const = 0;

    virtual ~IPackageReceiver() = default;
};

template <typename PreferenceType = double>

class ReceiverPreferences {
public:
    using preferences_t = std::map<IPackageReceiver*, PreferenceType>;
    using const_iterator = typename preferences_t::const_iterator;

    explicit ReceiverPreferences(ProbabilityGenerator pg = probability_generator): pg_(std::move(pg)) {};

    const_iterator cbegin() const {return preferences_t_.cbegin(); }
    const_iterator cend() const {return preferences_t_.cend(); }
    const_iterator begin() const {return preferences_t_.begin(); }
    const_iterator end() const {return preferences_t_.end(); }

    void add_receiver(IPackageReceiver *r);
    void remove_receiver(IPackageReceiver *r);
    IPackageReceiver* choose_receiver();
    const preferences_t& get_preferences() const {return preferences_t_;}

private:
    ProbabilityGenerator pg_;
    preferences_t preferences_t_;
};



#endif /* NODES_HPP_ */