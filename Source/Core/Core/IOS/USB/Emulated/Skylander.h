// Copyright 2022 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <map>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "Common/IOFile.h"
#include "Core/IOS/USB/EmulatedUsbDevice.h"

#define MAX_SKYLANDERS 8

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

protected:
  std::queue<std::array<u8, 64>> q_queries;

  class FakeTransferThread final
  {
  public:
    explicit FakeTransferThread(SkylanderUsb* portal) : m_portal(portal) {}
    ~FakeTransferThread();
    void Start();
    void Stop();
    void AddTransfer(std::unique_ptr<TransferCommand> command, std::array<u8, 64> data);

  private:
    SkylanderUsb* m_portal = nullptr;
    Common::Flag m_thread_running;
    std::thread m_thread;
    Common::Flag m_is_initialized;
    std::map<std::array<u8, 64>, std::unique_ptr<TransferCommand>> m_transfers;
    std::mutex m_transfers_mutex;
  };

private:
  u16 m_vid = 0;
  u16 m_pid = 0;
  u8 m_active_interface = 0;
  bool m_device_attached = false;
  DeviceDescriptor deviceDesc;
  std::vector<ConfigDescriptor> configDesc;
  std::vector<InterfaceDescriptor> interfaceDesc;
  std::vector<EndpointDescriptor> endpointDesc;
  FakeTransferThread m_transfer_thread{this};
  bool m_has_initialised = false;
  FakeTransferThread& GetTransferThread() { return m_transfer_thread; }
};

struct Skylander final
{
  File::IOFile sky_file;
  u8 status = 0;
  std::queue<u8> queued_status;
  std::array<u8, 0x40 * 0x10> data{};
  u32 last_id = 0;
  void save();
};

struct LedColor final
{
  u8 r = 0, g = 0, b = 0;
};

class SkylanderPortal final
{
public:
  void Activate();
  void Deactivate();
  bool IsActivated();
  void UpdateStatus();
  void SetLEDs(u8 side, u8 r, u8 g, u8 b);

  std::array<u8, 64> GetStatus();
  void QueryBlock(u8 sky_num, u8 block, u8* reply_buf);
  void WriteBlock(u8 sky_num, u8 block, const u8* to_write_buf, u8* reply_buf);

  bool RemoveSkylander(u8 sky_num);
  u8 LoadSkylander(u8* buf, File::IOFile in_file);

protected:
  std::mutex sky_mutex;

  bool activated = true;
  bool status_updated = false;
  u8 interrupt_counter = 0;
  LedColor color_right = {};
  LedColor color_left = {};
  LedColor color_trap = {};

  Skylander skylanders[MAX_SKYLANDERS];
};

extern SkylanderPortal g_skyportal;

}  // namespace IOS::HLE::USB
