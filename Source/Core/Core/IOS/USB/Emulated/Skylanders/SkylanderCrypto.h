#pragma once
#include <Common/CommonTypes.h>
#include <array>

namespace IOS::HLE::USB
{
class SkylanderCrypt final
{
public:
  static u16 ComputeCRC16(u16 init_value, const u8* buffer, u32 size);
};
}  // namespace IOS::HLE::USB
