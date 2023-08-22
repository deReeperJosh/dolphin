#include "SkylanderFigure.h"
#include <Common/Logging/Log.h>
#include <Common/Random.h>
#include <Common/StringUtil.h>
#include <algorithm>
#include <mbedtls/aes.h>
#include <mbedtls/md5.h>
#include "Common/IOFile.h"
#include "Skylander.h"
#include "SkylanderCrypto.h"

namespace IOS::HLE::USB
{
u16 PointerToU16(u8* pointer)
{
  return *(u16*)pointer;
}
u32 PointerToU32(u8* pointer)
{
  return *(u32*)pointer;
}
u64 PointerToU64(u8* pointer)
{
  return *(u64*)pointer;
}
void SkylanderFigure::PopulateSectorTrailers()
{
  // Set the sector permissions
  u32 first_block = 0x690F0F0F;
  u32 other_blocks = 0x69080F7F;
  memcpy(&data[0x36], &first_block, sizeof(first_block));
  for (u32 index = 1; index < 0x10; index++)
  {
    memcpy(&data[(index * 0x40) + 0x36], &other_blocks, sizeof(other_blocks));
  }
}

SkylanderFigure::SkylanderFigure(const std::string& file_path)
{
  sky_file = std::move(File::IOFile(file_path, "w+b"));
  data = {};
}
// Generate a AES key without the block filled in
void SkylanderFigure::GenerateIncompleteHashIn(u8 dest[0x56])
{
  u8 hashIn[0x56] = {};

  // copy first 2 blocks into hash
  GetBlock(0, hashIn);
  GetBlock(1, hashIn + 0x10);

  // Skip 1 byte. Is a block index that needs to be set per block.

  // Byte array of ascii string " Copyright (C) 2010 Activision.All Rights Reserved.". The space at
  // the start of the string is intentional
  const u8 HASH_CONST[0x35] = {0x20, 0x43, 0x6F, 0x70, 0x79, 0x72, 0x69, 0x67, 0x68, 0x74, 0x20,
                               0x28, 0x43, 0x29, 0x20, 0x32, 0x30, 0x31, 0x30, 0x20, 0x41, 0x63,
                               0x74, 0x69, 0x76, 0x69, 0x73, 0x69, 0x6F, 0x6E, 0x2E, 0x20, 0x41,
                               0x6C, 0x6C, 0x20, 0x52, 0x69, 0x67, 0x68, 0x74, 0x73, 0x20, 0x52,
                               0x65, 0x73, 0x65, 0x72, 0x76, 0x65, 0x64, 0x2E, 0x20};

  memcpy(hashIn + 0x21, HASH_CONST, 0x35);

  memcpy(hashIn, dest, 0x56);
}
SkylanderFigure::SkylanderFigure(File::IOFile file)
{
  sky_file = std::move(file);
  sky_file.Seek(0, File::SeekOrigin::Begin);
  sky_file.ReadBytes(data.data(), data.size());
}
bool SkylanderFigure::Create(u16 m_sky_id, u16 m_sky_var)
{
  if (!sky_file)
  {
    return false;
  }

  const auto file_data = data.data();

  memset(data.data(), 0, data.size());

  PopulateSectorTrailers();

  // Set the NUID of the figure
  Common::Random::Generate(&file_data[0], 4);

  // The BCC (Block Check Character)
  file_data[4] = file_data[0] ^ file_data[1] ^ file_data[2] ^ file_data[3];

  // ATQA
  file_data[5] = 0x81;
  file_data[6] = 0x01;

  // SAK
  file_data[7] = 0x0F;

  // Set the skylander info
  memcpy(&file_data[0x10], &m_sky_id, sizeof(m_sky_id));
  memcpy(&file_data[0x1C], &m_sky_var, sizeof(m_sky_var));

  // Set checksum
  u16 checksum = SkylanderCrypt::ComputeCRC16(0xFFFF, file_data, 0x1E);
  memcpy(&file_data[0x1E], &checksum, sizeof(checksum));

  Save();
  return true;
}
void SkylanderFigure::Save()
{
  sky_file.Seek(0, File::SeekOrigin::Begin);
  sky_file.WriteBytes(data.data(), 0x40 * 0x10);
}

void SkylanderFigure::GetBlock(u8 index, u8* dest)
{
  memcpy(dest, data.data() + (index * 0x10), 0x10);
}
FigureData SkylanderFigure::GetData()
{
  FigureData figureData = {.figureID = PointerToU16(data.data() + 0x10),
                           .variantID = PointerToU16(data.data() + 0x1C)};

  auto filter = std::make_pair(figureData.figureID, figureData.variantID);
  Type type = Type::Item;
  if (IOS::HLE::USB::list_skylanders.count(filter) != 0)
  {
    auto found = IOS::HLE::USB::list_skylanders.at(filter);
    type = found.type;
  }

  figureData.normalizedType = (u8)NormalizeSkylanderType(type);
  if ((Type)figureData.normalizedType == Type::Skylander)
  {
    u8 decrypted[FIGURE_SIZE] = {};

    DecryptFigure(decrypted);

    u8 areaOffset = (decrypted[0x112] > decrypted[0x2D2]) ? 0x80 : 0x240;

    figureData.skylanderData = {.money = PointerToU16(decrypted + areaOffset + 0x3),
                                .heroLevel = PointerToU16(decrypted + areaOffset + 0x5A),
                                .playtime = PointerToU32(decrypted + areaOffset + 0x5)};

    u16 nickname[16] = {};

    // First nickname half
    for (u8 i = 0; i < 8; i++)
    {
      nickname[i] = PointerToU16(decrypted + areaOffset + 0x20 + (i * 2));
    }

    // Second nickname half
    for (u8 i = 0; i < 8; i++)
    {
      nickname[i + 8] = PointerToU16(decrypted + areaOffset + 0x40 + (i * 2));
    }

    memcpy(figureData.skylanderData.nickname, nickname, 2 * 16);
  }

  return figureData;
}
void SkylanderFigure::DecryptFigure(u8 dest[FIGURE_SIZE])
{
  u8 hashIn[0x56] = {};

  // copy first 2 blocks into hash
  GetBlock(0, hashIn);
  GetBlock(1, hashIn + 0x10);

  // Skip 1 byte. Is a block index that needs to be set per block.

  // Byte array of ascii string " Copyright (C) 2010 Activision.All Rights Reserved.". The space at
  // the start of the string is intentional
  const u8 HASH_CONST[0x35] = {0x20, 0x43, 0x6F, 0x70, 0x79, 0x72, 0x69, 0x67, 0x68, 0x74, 0x20,
                               0x28, 0x43, 0x29, 0x20, 0x32, 0x30, 0x31, 0x30, 0x20, 0x41, 0x63,
                               0x74, 0x69, 0x76, 0x69, 0x73, 0x69, 0x6F, 0x6E, 0x2E, 0x20, 0x41,
                               0x6C, 0x6C, 0x20, 0x52, 0x69, 0x67, 0x68, 0x74, 0x73, 0x20, 0x52,
                               0x65, 0x73, 0x65, 0x72, 0x76, 0x65, 0x64, 0x2E, 0x20};

  memcpy(hashIn + 0x21, HASH_CONST, 0x35);

  u8 decrypted[FIGURE_SIZE] = {};

  u8 currentBlock[0x10] = {};

  // Run for every block
  for (u8 i = 0; i < 64; i++)
  {
    GetBlock(i, currentBlock);

    // Skip sector trailer and the first 8 blocks
    if (((i + 1) % 4 == 0) || i < 8)
    {
      memcpy(decrypted + (i * 0x10), currentBlock, 0x10);
      continue;
    }

    // Check if block is all 0
    u16 total = 0;
    for (u8 j = 0; j < 0x10; j++)
    {
      total += currentBlock[j];
    }
    if (total == 0)
    {
      continue;
    }

    // Block index
    hashIn[0x20] = i;

    u8 hashOut[0x10] = {};

    mbedtls_md5_ret(hashIn, 0x56, hashOut);

    mbedtls_aes_context aes_context = {};

    mbedtls_aes_setkey_dec(&aes_context, hashOut, 128);

    mbedtls_aes_crypt_ecb(&aes_context, MBEDTLS_AES_DECRYPT, currentBlock, decrypted + (i * 0x10));
  }

  memcpy(dest, decrypted, FIGURE_SIZE);

  NOTICE_LOG_FMT(IOS_USB, "Decrypted skylander data: \n{}", HexDump(decrypted, FIGURE_SIZE));
}
void SkylanderFigure::Close()
{
  sky_file.Close();
}
void SkylanderFigure::SetBlock(u8 block, const u8* buf)
{
  memcpy(data.data() + (block * 16), buf, 16);
}
bool SkylanderFigure::FileIsOpen()
{
  return sky_file.IsOpen();
}
}  // namespace IOS::HLE::USB
