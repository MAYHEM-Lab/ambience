//
// Created by Mehmet Fatih BAKIR on 13/04/2018.
//

#define CATCH_CONFIG_RUNNER
#include "catch.hpp"
#include <tos/ft.hpp>

void tos_main()
{
    tos::launch([](void*){
        Catch::Session().run();
    });
}