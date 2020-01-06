//
// Created by fatih on 1/5/20.
//

#pragma once

namespace tos::debug {
template <class StorageT, class EnableT>
class in_memory_logger : StorageT, public EnableT {
public:
    template <class... Ts>
    void log(Ts&&... args) {

    }
private:
};
}