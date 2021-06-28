#pragma once

namespace tos::ae {
namespace kernel {
struct group;
}
struct group_runner {
    virtual void run(kernel::group& group) = 0;
    virtual ~group_runner() = default;
};
} // namespace tos::ae