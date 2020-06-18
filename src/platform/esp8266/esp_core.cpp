//
// Created by fatih on 1/9/19.
//

namespace tos::platform {
[[noreturn]] void force_reset() {
    // esp sdk should reset
    while (true) {
    }
}
} // namespace tos::platform