#include <arch/drivers.hpp>
#include <common/clock.hpp>
#include <numeric>
#include <tos/debug/assert.hpp>
#include <tos/debug/log.hpp>
#include <tos/function_ref.hpp>
#include <tos/soc/bcm2837.hpp>
#include <uspios.h>

namespace global {
extern tos::raspi3::interrupt_controller* ic;
extern tos::any_clock* clk;
extern tos::any_alarm* alarm;
} // namespace global

volatile uint64_t _usb_irq_count = 0;

namespace {
void delay(uint64_t count) {
    asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
                 : "=r"(count)
                 : [count] "0"(count)
                 : "cc");
}

tos::function_ref<void()> usb_handler{[](void*) {}};

tos::raspi3::irq_handler usb_irq_handler{tos::function_ref<bool()>([](void*) {
    _usb_irq_count = _usb_irq_count + 1;
    usb_handler();
    return true;
})};
struct timer;
std::vector<timer> timers;

struct timer {
    TKernelTimerHandler* pHandler;
    void* pParam;
    void* pContext;

    std::unique_ptr<tos::sleeper> m_sleeper;
    unsigned m_handle;

    timer(unsigned ticks, unsigned handle)
        : m_sleeper(std::make_unique<tos::sleeper>(
              ticks, tos::mem_function_ref<&timer::call>(*this)))
        , m_handle{handle} {
    }

    void call() {
        std::invoke(pHandler, m_handle, pParam, pContext);
        timers.erase(timers.begin() + std::distance(timers.data(), this));
    }
};

unsigned next_timer;
} // namespace

extern "C" {
void* tos_malloc(size_t sz) {
    return new char[sz];
}

void tos_free(void* data) {
    delete[] reinterpret_cast<char*>(data);
}

void MsDelay(unsigned nMilliSeconds) {
    usDelay(nMilliSeconds * 1000);
    //    tos::delay(*global::clk, std::chrono::microseconds(nMilliSeconds * 1000),
    //    false);
}

void usDelay(unsigned nMicroSeconds) {
    //    tos::delay(*global::clk, std::chrono::microseconds(nMicroSeconds), false);
    delay(uint64_t(nMicroSeconds) * ((1200 / 3) / 8) / 10);
}

unsigned
StartKernelTimer(unsigned nHzDelay, // in HZ units (see "system configuration" above)
                 TKernelTimerHandler* pHandler,
                 void* pParam,
                 void* pContext) {
    Expects(nHzDelay <= 65'535);

    auto handle = next_timer++;

    timers.emplace_back(nHzDelay, handle);
    timers.back().pHandler = pHandler;
    timers.back().pParam = pParam;
    timers.back().pContext = pContext;

    global::alarm->set_alarm(*timers.back().m_sleeper);
    return handle;
}

void CancelKernelTimer(unsigned hTimer) {
    auto it = std::find_if(
        timers.begin(), timers.end(), [=](auto& tim) { return tim.m_handle == hTimer; });

    global::alarm->cancel(tos::ad_hoc_list_iter(*it->m_sleeper));
    timers.erase(it);
}

void ConnectInterrupt([[maybe_unused]] unsigned nIRQ,
                      TInterruptHandler* pHandler,
                      void* pParam) {
    usb_handler = {pHandler, pParam};
    global::ic->register_handler(tos::bcm283x::irq_channels::usb, usb_irq_handler);
    tos::bcm2837::INTERRUPT_CONTROLLER->enable_irq_1 = 1 << 9;
}

int SetPowerStateOn(unsigned nDeviceId) {
    using namespace tos::raspi3;
    property_channel_tags_builder builder;
    auto buf = builder.add(0x00028001, {nDeviceId, 0b11}).end();
    property_channel property;
    if (!property.transaction(buf)) {
        tos::debug::panic("Can't set power state on");
    }

    return 1;
}

int GetMACAddress(unsigned char Buffer[6]) {
    std::iota(Buffer, Buffer + 6, 1);
    return 1;
}

void LogWrite([[maybe_unused]] const char* pSource, // short name of module
              [[maybe_unused]] unsigned Severity,   // see above
              [[maybe_unused]] const char* pMessage,
              ...) {
    //    LOG("LOG", pMessage);
}

void uspi_assertion_failed(const char* pExpr,
                           [[maybe_unused]] const char* pFile,
                           [[maybe_unused]] unsigned nLine) {
    tos::debug::default_assert_handler(pExpr);
}

void DebugHexdump([[maybe_unused]] const void* pBuffer,
                  [[maybe_unused]] unsigned nBufLen,
                  [[maybe_unused]] const char* pSource /* = 0 */) {
    //    LOG("Called hex dump");
}

void uspi_EnterCritical(void) {
    tos::kern::disable_interrupts();
}

void uspi_LeaveCritical(void) {
    tos::kern::enable_interrupts();
}
}