//
// Created by fatih on 12/30/19.
//

#pragma once

#include <ti/drivers/crypto/CryptoCC32XX.h>
namespace tos::cc32xx {
class crypto {
public:
    crypto() {
        static auto _ = []{return CryptoCC32XX_init(), 0;};
    }
private:
};
}