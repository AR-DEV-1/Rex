#include "defines.hlsl"

// This buffers holds the indices of each tile in the texture.
// Using this information, we can calculate the UV for each vertex
// Each index is only 1 byte, so you need to use bit shifting to unpack it
// TLDR: this maps instance id --> tile id in texture
ByteAddressBuffer TileIndexIntoTextureBuffer : register(t0, RENDER_PASS_REGISTER_SPACE);

// Every constant buffer needs to be 256 byte aligned
// So we try to pack as much data together into 1 buffer
cbuffer RenderingMetaData : register(b0, RENDER_PASS_REGISTER_SPACE)
{
  // Tile texture data
  int texture_tiles_per_row;     // the number of tiles per tile row in the texture
  float inv_tile_texture_width;  // the inverse of the width of a single tile in the texture
  float inv_tile_texture_height; // the inverse of the height of a single tile in the texture

  // Render target data
  uint screen_tiles_per_row;     // the number of tiles per tile row on the screen
  float inv_tile_screen_width;   // the inverse of the width of a single tile on the screen
  float inv_tile_screen_height;  // the inverse of the height of a single tile on the screen
};

struct VertexIn
{
  float2 PosL : POSITION;       // The position of the vertex in local space
  float2 Uv : TEXCOORD0;        // The UV of the vertex

  uint instanceId : SV_InstanceID;  // The instance the vertex belongs to
};

struct VertexOut
{
  float4 PosH : SV_POSITION;
  float2 Uv : TEXCOORD0;
};

// Convert a index into a 2D coordinate.
uint2 index_to_coord(uint tileIdx, uint width)
{
  uint2 coord;

  coord.x = tileIdx % width;
  coord.y = tileIdx / width;

  return coord;
}

float4 calculate_vertex_position(VertexIn vin)
{
  VertexOut vout;

  // Calculate where on the screen the tile needs to go
  // The result holds 2D coordinate as indices
  // eg (x: 1, y: 3) -> tile on the 4th row, at 2nd column
  uint2 screen_tile_coord_idx = index_to_coord(vin.instanceId, screen_tiles_per_row);

  // Calculate the position of this cell, starting from top left
  float2 pos = { -1, 1 };
  pos.x += screen_tile_coord_idx.x * inv_tile_screen_width;
  pos.y -= screen_tile_coord_idx.y * inv_tile_screen_height;

  // If we ever want to render the tilemap at an offset from the top left, this is how that'd be done
  // pos.x += screen_start_offset.x;
  // pos.x += screen_start_offset.y;
  
  // Offset the position to this position
  return float4(vin.PosL + pos, 0.0, 1.0f);
}

float2 calculate_vertex_uv(VertexIn vin)
{
  // For example:
  // Let's say you have the following TileIndexIntoTextureBuffer
  // +-------+-------+-------+-------+-------+-------+-------+-------+
  // |   10  |   10  |  11   |   11  |   5   |   5   |   5   |   5   |
  // +-------+-------+-------+-------+-------+-------+-------+-------+
  // and you're currently processing a tile instance of idx 1
  // That'd mean you're expecting a final result of tile idx == 10
  // As you're processing a tile which is the 10th tile in the texture

  // The offset value from which we load needs to be 4 byte aligned.
  // see: https://learn.microsoft.com/en-us/windows/win32/direct3d11/direct3d-11-advanced-stages-cs-resources#byte-address-buffer
  // That's why we load 4 bytes from the lower aligned value based on the instance ID
  // Then we shift it so we get correct byte out and that's what we use as index
  uint tile_idx_offset = vin.instanceId & ~3; // mask away the lower bits, so we're always aligning down to a multiple of 4
  uint result = TileIndexIntoTextureBuffer.Load(tile_idx_offset); // This always loads 4 bytes, not 1 byte
  
  // Using the above example, we'd expect to have the following
  // tile_idx_offset == 0 (as 1 & ~3 == 0)
  // result == { 10, 10, 11, 11 } == 0x0A0A0B0B <-- array of 4 bytes, 1 index per byte

  uint byte_offset = vin.instanceId & 3; // limit to the value { 0, 1, 2, 3 }
  uint bit_offset = byte_offset * 8;

  // Using the above example, we'd expect to have the following
  // byte_offset == 1 (as 1 & 3 == 1)
  // bit_offset == 1 * 8 == 8

  // Now shift the byte we care about to the front and mask it off
  uint tile_idx = result >> bit_offset;
  tile_idx &= 0x000000FF;

  // Using the above example, we'd expect the following
  // tile_idx == 0x0A0B0B (after shift)
  // tile_idx == 0x0A == 10 (after mask)
  
  // We know that the tile instance we're currently processing maps to the 10th tile in the texture
  // which is exactly what we're expecting
  // We now have to move our UV coordinate to the correct location so that also maps to the UV location of the 10th tile

  // Calculate the 2D coordinate of the tile in the texture
  // The result holds 2D coordinate as indices
  // eg (x: 1, y: 3) -> tile on the 4th row, at 2nd column
  uint2 tex_tile_coord_idx = index_to_coord(tile_idx, texture_tiles_per_row);
  
  // Calculate the the top left uv coordinate of the tile we want
  float2 tile_uv_start;
  tile_uv_start.x = tex_tile_coord_idx.x * inv_tile_texture_width;
  tile_uv_start.y = tex_tile_coord_idx.y * inv_tile_texture_height;

  // Add the offset to the original uv offset
  return vin.Uv + tile_uv_start;
}

VertexOut main(VertexIn vin)
{
  VertexOut vout;
  
  vout.PosH = calculate_vertex_position(vin);
  vout.Uv = calculate_vertex_uv(vin);  

  return vout;
}


