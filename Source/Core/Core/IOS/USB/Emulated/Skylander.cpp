// Copyright 2017 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include "Core/IOS/USB/Emulated/Skylander.h"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

#include <libusb.h>

#include "Common/Assert.h"
#include "Common/Logging/Log.h"
#include "Common/StringUtil.h"
#include "Core/HW/Memmap.h"
#include "Core/IOS/Device.h"
#include "Core/IOS/IOS.h"

namespace IOS::HLE::USB
{
SkylanderUsb::SkylanderUsb(Kernel& ios, const std::string& device_name)
    : EmulatedUsbDevice(ios, device_name)
{
  m_vid = 0x1430;
  m_pid = 0x150;
  m_device_attached = true;
  deviceDesc = DeviceDescriptor{18, 1, 512, 0, 0, 0, 64, 5168, 336, 256, 1, 2, 0, 1};
  configDesc.emplace_back(ConfigDescriptor{9, 2, 41, 1, 1, 0, 128, 250});
  interfaceDesc.emplace_back(InterfaceDescriptor{9, 4, 0, 0, 2, 3, 0, 0, 0});
  endpointDesc.emplace_back(EndpointDescriptor{7, 5, 129, 3, 64, 1});
  endpointDesc.emplace_back(EndpointDescriptor{7, 5, 2, 3, 64, 1});
}

SkylanderUsb::~SkylanderUsb()
{
}

DeviceDescriptor SkylanderUsb::GetDeviceDescriptor() const
{
  return deviceDesc;
}

std::vector<ConfigDescriptor> SkylanderUsb::GetConfigurations() const
{
  return configDesc;
}

std::vector<InterfaceDescriptor> SkylanderUsb::GetInterfaces(u8 config) const
{
  return interfaceDesc;
}

std::vector<EndpointDescriptor> SkylanderUsb::GetEndpoints(u8 config, u8 interface, u8 alt) const
{
  return endpointDesc;
}

bool SkylanderUsb::Attach()
{
  NOTICE_LOG_FMT(IOS_USB, "[{:04x}:{:04x}] Opening device", m_vid, m_pid);
  return true;
}

bool SkylanderUsb::AttachAndChangeInterface(const u8 interface)
{
  return true;
}

int SkylanderUsb::CancelTransfer(u8 endpoint)
{
  return IPC_SUCCESS;
}

int SkylanderUsb::ChangeInterface(const u8 interface)
{
  INFO_LOG_FMT(IOS_USB, "[{:04x}:{:04x} {}] Changing interface to {}", m_vid, m_pid,
               m_active_interface, interface);
  m_active_interface = interface;
  return 0;
}

int SkylanderUsb::GetNumberOfAltSettings(u8 interface)
{
  return 0;
}

int SkylanderUsb::SetAltSetting(u8 alt_setting)
{
  return 0;
}

int SkylanderUsb::SubmitTransfer(std::unique_ptr<CtrlMessage> cmd)
{
  DEBUG_LOG_FMT(IOS_USB,
                "[{:04x}:{:04x} {}] Control: bRequestType={:02x} bRequest={:02x} wValue={:04x}"
                " wIndex={:04x} wLength={:04x}",
                m_vid, m_pid, m_active_interface, cmd->request_type, cmd->request, cmd->value,
                cmd->index, cmd->length);

  return 0;
}
int SkylanderUsb::SubmitTransfer(std::unique_ptr<BulkMessage> cmd)
{
  DEBUG_LOG_FMT(IOS_USB, "[{:04x}:{:04x} {}] Bulk: length={:04x} endpoint={:02x}", m_vid, m_pid,
                m_active_interface, cmd->length, cmd->endpoint);
  return 0;
}
int SkylanderUsb::SubmitTransfer(std::unique_ptr<IntrMessage> cmd)
{
  DEBUG_LOG_FMT(IOS_USB, "[{:04x}:{:04x} {}] Interrupt: length={:04x} endpoint={:02x}", m_vid,
                m_pid, m_active_interface, cmd->length, cmd->endpoint);
  return 0;
}
int SkylanderUsb::SubmitTransfer(std::unique_ptr<IsoMessage> cmd)
{
  DEBUG_LOG_FMT(IOS_USB,
                "[{:04x}:{:04x} {}] Isochronous: length={:04x} endpoint={:02x} num_packets={:02x}",
                m_vid, m_pid, m_active_interface, cmd->length, cmd->endpoint, cmd->num_packets);
  return 0;
}

}  // namespace IOS::HLE::USB
