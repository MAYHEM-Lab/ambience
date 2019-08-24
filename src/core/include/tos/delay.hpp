//
// Created by fatih on 8/25/18.
//

#pragma once

#include <chrono>

namespace tos {
/**
 * Spin waits for the given duration.
 *
 * The thread will not be suspended during the wait. When waiting
 * for long periods of time, prefer using an alarm.
 *
 * @param us microseconds to delay for
 */
void delay_us(std::chrono::microseconds us);

/**
 * Spin waits for the given duration.
 *
 * The thread will not be suspended during the wait. When waiting
 * for long periods of time, prefer using an alarm.
 *
 * @param ms milliseconds to delay for
 */
void delay_ms(std::chrono::milliseconds ms);
} // namespace tos