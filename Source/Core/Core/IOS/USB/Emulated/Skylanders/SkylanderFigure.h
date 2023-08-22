// Copyright 2022 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once
#include <Common/IOFile.h>

namespace IOS::HLE::USB
{

struct SkylanderData final
{
  u16 money;
  u16 heroLevel;
  u32 playtime;
  // Null-terminated UTF-16 string
  u16 nickname[16];
};

struct FigureData final
{
  u8 normalizedType;
  u16 figureID;
  u16 variantID;
  union
  {
    SkylanderData skylanderData;
  };
};

u16 PointerToU16(u8* pointer);
u32 PointerToU32(u8* pointer);
u64 PointerToU64(u8* pointer);

constexpr u32 FIGURE_SIZE = 0x40 * 0x10;

class SkylanderFigure
{
private:
  File::IOFile sky_file;
  std::array<u8, FIGURE_SIZE> data;

  void PopulateSectorTrailers();
  void GenerateIncompleteHashIn(u8 dest[0x56]);

public:
  SkylanderFigure(const std::string& file_path);
  SkylanderFigure(File::IOFile file);
  bool Create(u16 m_sky_id, u16 m_sky_var);
  void Save();
  void Close();
  bool FileIsOpen();
  void GetBlock(u8 index, u8* dest);
  void SetBlock(u8 block, const u8* buf);
  void DecryptFigure(u8* dest);
  FigureData GetData();
};
}  // namespace IOS::HLE::USB
