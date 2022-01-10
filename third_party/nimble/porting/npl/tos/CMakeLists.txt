target_include_directories(nimble PUBLIC include)
target_sources(nimble PRIVATE
    mutex.cpp
    event.cpp
    nrf5_isr.cpp
    critical.cpp
    timer.cpp
    callout.cpp
    semaphore.cpp
    os.cpp
)
target_link_libraries(nimble PRIVATE
    tos_ft
    tos_core
)