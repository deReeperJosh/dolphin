// Copyright 2022 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once
#include "Skylander.h"

namespace IOS::HLE::USB
{
class SkylanderFigure
{
private:
  File::IOFile sky_file;
  std::array<u8, 0x40 * 0x10> data;

  void PopulateSectorTrailers();

public:
  SkylanderFigure(const std::string& file_path);
  bool Create(u16 m_sky_id, u16 m_sky_var);
  void Save();
  std::array<u8, 0x10> GetBlock(u8 index);
  void SetBlock(u8 index, std::array<u8, 0x40 * 0x10> block);
};
}  // namespace IOS::HLE::USB
