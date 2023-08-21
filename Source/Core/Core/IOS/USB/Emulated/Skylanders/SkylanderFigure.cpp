#include "SkylanderFigure.h"
#include <Common/Random.h>
#include <algorithm>
#include "Skylander.cpp"

namespace IOS::HLE::USB
{
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
  sky_file = File::IOFile(file_path, "w+b");
  sky_file.ReadArray(data.data(), data.size());
}

bool SkylanderFigure::Create(u16 m_sky_id, u16 m_sky_var)
{
  if (!sky_file)
  {
    return false;
  }

  std::array<u8, 0x40 * 0x10> buf{};
  const auto file_data = buf.data();

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
  u16 checksum = SkylanderCRC16(0xFFFF, file_data, 0x1E);
  memcpy(&file_data[0x1E], &checksum, sizeof(checksum));

  Save();
  return false;
}
void SkylanderFigure::Save()
{
  sky_file.Seek(0, File::SeekOrigin::Begin);
  sky_file.WriteArray(data);
}

std::array<u8, 0x10> SkylanderFigure::GetBlock(u8 index)
{
  std::array<u8, 0x10> buf = std::array<u8, 0x10>();

  std::copy(data[index * 0x10], data[(index * 0x10) - 1], buf[0]);

  return buf;
}
void SkylanderFigure::SetBlock(u8 index, std::array<u8, 0x40 * 0x10> block)
{
  std::copy(block[0], block[0xF], data[index * 0x10]);
}
}  // namespace IOS::HLE::USB
