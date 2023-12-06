#ifndef PACKAGE_HPP_
#define PACKAGE_HPP_

#include "types.hpp"
#include <set>

class Package {
public:
    Package();
    explicit Package(ElementID ID) : ID_(ID) { assigned_IDs.insert(ID_); }
    Package(Package &&package)  noexcept : ID_(package.ID_) {}
    Package &operator=(Package &&package) noexcept ;
    ElementID get_id() const { return ID_; }
    ~Package();

private:
    ElementID ID_;
    static std::set<ElementID> assigned_IDs;
    static std::set<ElementID> freed_IDs;
};

#endif /* PACKAGE_HPP_ */