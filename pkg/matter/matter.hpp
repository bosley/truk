#pragma once

#include <cstddef>
#include <cstdint>
#include <array>
#include <string_view>

namespace matter {

/*
   Each substance of matter (deliniation that makes forms) will be unique
   they will be unique in ways they are accessed/utilized despite the fact
   that they may have same width. The SUBSTANCE_ID is a means to aide in
   contextualizing the data, but it may need futher contextualization for
   interaction
*/
template <std::uint8_t SUBSTANCE_ID, std::size_t WIDTH> class matter_base_c {
  static_assert(WIDTH >= 1, "WIDTH must be at least 1");

public:
  matter_base_c( ) = default;
  constexpr std::uint8_t id() const { return SUBSTANCE_ID; }


private:
  std::array<std::uint8_t, WIDTH> _data{0};
};

} // namespace matter