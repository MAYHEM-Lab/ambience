//
// Created by fatih on 12/30/19.
//

#pragma once

#include <tos/self_pointing.hpp>
#include <ti/drivers/I2C.h>

namespace tos::cc32xx {
class i2c : public self_pointing<i2c> {
public:

    explicit i2c(int num);

private:
};
}

// implementation

namespace tos::cc32xx {
inline i2c::i2c(int num) {
    static auto _ = []{
      return I2C_init(), 0;
    }();
    I2C_Params params;
    I2C_Params_init(&params);
}
}