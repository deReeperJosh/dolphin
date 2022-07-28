// Copyright 2022 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <queue>

#include "Common/CommonTypes.h"
#include "Core/IOS/USB/Common.h"
#include "Core/IOS/USB/EmulatedUsbDevice.h"
#include "Core/LibusbUtils.h"

namespace IOS::HLE::USB
{
class SkylanderUsb final : public EmulatedUsbDevice
{
public:
  SkylanderUsb(Kernel& ios, const std::string& device_name);
  ~SkylanderUsb();
  DeviceDescriptor GetDeviceDescriptor() const override;
  std::vector<ConfigDescriptor> GetConfigurations() const override;
  std::vector<InterfaceDescriptor> GetInterfaces(u8 config) const override;
  std::vector<EndpointDescriptor> GetEndpoints(u8 config, u8 interface, u8 alt) const override;
  bool Attach() override;
  bool AttachAndChangeInterface(u8 interface) override;
  int CancelTransfer(u8 endpoint) override;
  int ChangeInterface(u8 interface) override;
  int GetNumberOfAltSettings(u8 interface) override;
  int SetAltSetting(u8 alt_setting) override;
  int SubmitTransfer(std::unique_ptr<CtrlMessage> message) override;
  int SubmitTransfer(std::unique_ptr<BulkMessage> message) override;
  int SubmitTransfer(std::unique_ptr<IntrMessage> message) override;
  int SubmitTransfer(std::unique_ptr<IsoMessage> message) override;

private:
  u16 m_vid = 0;
  u16 m_pid = 0;
  u8 m_active_interface = 0;
  bool m_device_attached = false;
  DeviceDescriptor deviceDesc;
  std::vector<ConfigDescriptor> configDesc;
  std::vector<InterfaceDescriptor> interfaceDesc;
  std::vector<EndpointDescriptor> endpointDesc;

protected:
  std::queue<std::array<u8, 32>> q_queries;
};

}  // namespace IOS::HLE::USB
