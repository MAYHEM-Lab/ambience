//
// Created by Mehmet Fatih BAKIR on 14/06/2018.
//

#pragma once

namespace tos
{
  namespace vcs
  {
    /**
     * This constant contains the GIT hash signature when the OS was built
     */
    static constexpr char commit_hash[] = TOS_GIT_SHA1;
  } // namespace vcs
} // namespace tos
