#include "nodes.hpp"

void ReceiverPreferences::add_receiver(IPackageReceiver *r) {
    auto num_of_receivers_begin = double(preferences_t_.size());
    if (num_of_receivers_begin == 0) {
        preferences_t_[r] = 1.0;
    } else {
        for (auto &rec: preferences_t_) {
            rec.second = 1 / (num_of_receivers_begin + 1);
        }
        preferences_t_[r] = 1 / (num_of_receivers_begin + 1);
    }
}

void ReceiverPreferences::remove_receiver(IPackageReceiver *r) {
    auto num_of_receivers_begin = double(preferences_t_.size());
    if (num_of_receivers_begin > 1) {
        for (auto &rec: preferences_t_) {
            if (rec.first != r) {
                rec.second = 1 / (num_of_receivers_begin - 1);
            }
        }
    }
    preferences_t_.erase(r);
}

IPackageReceiver *ReceiverPreferences::choose_receiver() {
    auto prob = pg_();
    if (prob >= 0 && prob <= 1) {
        double distribution = 0.0;
        for (auto &rec: preferences_t_) {
            distribution = distribution + rec.second;
            if (distribution < 0 || distribution > 1) {
                return nullptr;
            }
            if (prob <= distribution) {
                return rec.first;
            }
        }
        return nullptr;
    }
    return nullptr;
}

void PackageSender::send_package() {
    IPackageReceiver *receiver;
    if (bufor_) {
        receiver = receiver_preferences_.choose_receiver();
        receiver->receive_package(std::move(*bufor_));
        bufor_.reset();
    }
}

void Ramp::deliver_goods(Time t) {
    if (!bufor_) {
        push_package(Package());
        bufor_.emplace(id_);
        t_ = t;
    } else {
        if (t - di_ == t_) {
            push_package(Package());
        }
    }
}

void Worker::do_work(Time t) {
    if (!bufor_ && !q_->empty()) {
        bufor_.emplace(q_->pop());
        t_ = t;
    } else {
        if (t - t_ + 1 == pd_) {
            push_package(Package(bufor_.value().get_id()));
            bufor_.reset();
            if (!q_->empty()) {
                bufor_.emplace(q_->pop());
            }
        }
    }
}

void Worker::receive_package(Package &&p) {
    q_->push(std::move(p));
}

void Storehouse::receive_package(Package &&p) {
    d_->push(std::move(p));
}