//
// Created by fatih on 4/14/20.
//

#include <ios>
extern "C"
{
[[gnu::weak]]
void __cxa_pure_virtual()
{
}

void __cxa_atexit(void (*) (void *), void *, void *) {
}
}

namespace std {
ios_base::Init::Init() {}
ios_base::Init::~Init() {}
}