#pragma once

#include <string>

#include "Common/CommonTypes.h"
#include "Core/IOS/USB/Common.h"

namespace IOS::HLE::USB
{
class EmulatedUsbDevice : public Device
{
public:
  EmulatedUsbDevice(Kernel& ios, const std::string& device_name);
  virtual ~EmulatedUsbDevice();

private:
  Kernel& m_ios;
};
}  // namespace IOS::HLE::USB
