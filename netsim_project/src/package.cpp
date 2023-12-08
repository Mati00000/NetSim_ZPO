#include "package.hpp"

Package::~Package() {
    assigned_IDs.erase(ID_);
    freed_IDs.insert(ID_);
}
Package::Package() {
    if(!freed_IDs.empty()){
        if(!assigned_IDs.empty()){
            ID_ = *(assigned_IDs.rbegin()) + 1;
            assigned_IDs.insert(ID_);
        } else {
            ID_ = 1;
            assigned_IDs.insert(ID_);
        }
    } else {
        ID_ = *(freed_IDs.begin());
        freed_IDs.erase(ID_);
        assigned_IDs.insert(ID_);
    }
}

Package &Package::operator=(Package &&package) noexcept {
    if (this == &package)
        return *this;
    assigned_IDs.erase(this->ID_);
    freed_IDs.insert(this->ID_);
    this->ID_ = package.ID_;
    assigned_IDs.insert(this->ID_);
    return *this;
}

