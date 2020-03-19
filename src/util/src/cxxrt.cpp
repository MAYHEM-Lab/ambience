//
// Created by fatih on 5/17/18.
//

#include <ios>
extern "C"
{
    [[gnu::weak]]
    void __cxa_pure_virtual()
    {
    }

    void __cxa_atexit(void (*destructor) (void *), void *arg, void *__dso_handle) {

    }
}

namespace std {
ios_base::Init::Init() {}
ios_base::Init::~Init() {
}
}
