#ifndef DRS4lib_EventHeader_h
#define DRS4lib_EventHeader_h

#include <array>
#include <cstdint>
#include <vector>

namespace drs4 {
  class EventHeader : private std::array<uint32_t, 4> {
  public:
    explicit EventHeader(const std::array<uint32_t, 4>& words = {0, 0, 0, 0}) : std::array<uint32_t, 4>(words) {}
    EventHeader(const EventHeader& oth) : std::array<uint32_t, 4>(oth) {}

    inline uint8_t init() const { return at(0) >> 28; }
    inline uint32_t eventSize() const { return at(0) & 0xfffffff; }
    inline bool boardFail() const { return (at(1) >> 26) & 0x1; }
    inline std::vector<bool> groupMask() const {
      const auto mask = at(1) & 0x3;
      return std::vector<bool>{bool(mask & 0x1), bool((mask >> 1) & 0x1)};
    }
    inline uint32_t eventNumber() const { return at(2) & 0xffffff; }
    inline uint32_t eventTimeTag() const { return at(3) & 0x7fffffff; }
    inline bool eventTimeOverflow() const { return bool((at(3) >> 31) & 0x1); }
  };
}  // namespace drs4

#endif
