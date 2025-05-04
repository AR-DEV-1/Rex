#pragma once

#include "rex_engine/assets/asset.h"

#include "rex_engine/assets/blockset.h"

#include "rex_engine/engine/types.h"
#include "rex_std/string.h"
#include "rex_std/memory.h"

namespace rex
{
	enum class Direction
	{
		North,
		East,
		South,
		West
	};

	struct MapHeader
	{
		rsl::string name;											// name of the map
		Blockset* tileset;										// the blockset to be used by the map
		s8 width;															// represented in block count
		s8 height;														// represented in block count
		s8 border_block_idx;									// Index of the block used for the border if no connection blocks are present
	};

	struct MapConnection
	{
		MapHeader map;
		Direction direction;
		s8 offset;
	};

	struct WarpEvent
	{
		s8 x;
		s8 y;
		s8 dst_map_id;
		s8 dst_warp_id;
	};

	struct TextEvent // BgEvent in pokemon code
	{
		s8 x;
		s8 y;
		rsl::string text;
		s8 sign_id;
	};

	struct MapObject // Characters
	{
		rsl::string name;
	};

	enum class ObjectEventType
	{
		Character, // By default
		Item,
		Trainer,
		Pokemon
	};

	struct ObjectEvent
	{
		s8 x;
		s8 y;
		rsl::string text_id;
		rsl::string direction;
		rsl::string movement; // walk/stay
		rsl::string sprite_id;
	};
	struct CharacterObjectEvent : ObjectEvent
	{
		// Has no extra members
	};
	struct ItemObjectEvent : ObjectEvent
	{
		s8 item;
	};
	struct TrainerObjectEvent : ObjectEvent
	{
		s8 trainer_class;
		s8 trainer_number;
	};
	struct PokemonObjectEvent : ObjectEvent
	{
		s8 pokemon_id;
		s8 pokemon_level;
	};

	struct Trainer
	{
		s8 event_flag;
		s8 view_range;
		s8 text_before_battle;
		s8 text_after_battle;
		s8 text_end_battle;
	};

	struct MapScript
	{
		rsl::string name;
	};

	// This is the data that gets saved to disk
	struct MapDesc
	{
		rsl::unique_array<MapConnection> connections;
		rsl::unique_array<MapObject> objects;
		rsl::unique_array<rsl::unique_ptr<ObjectEvent>> object_events;
		rsl::unique_array<WarpEvent> warps;
		rsl::unique_array<TextEvent> text_events;
		rsl::unique_array<rsl::string> scripts;
		rsl::unique_array<s8> blocks;
		MapHeader map_header;
	};

	class Map : public Asset
	{
	public:
		Map(MapDesc&& desc);

	private:
		MapDesc m_desc;
	};
}