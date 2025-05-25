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
		s8 width_in_blocks;										// represented in block count
		s8 height_in_blocks;									// represented in block count
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
		// The connections of a map. This means where a player can walk to from this map without going through a warp
		rsl::unique_array<MapConnection> connections;

		// The objects within a map (characters, items, pokemon, ...)
		// This should be merged with object and text events
		// An object is "something" in a map the player can interact with
		rsl::unique_array<MapObject> objects;
		rsl::unique_array<rsl::unique_ptr<ObjectEvent>> object_events;
		rsl::unique_array<TextEvent> text_events;

		// A warp is a space that teleports to the player into another map
		// A good example of this is a door
		rsl::unique_array<WarpEvent> warps;

		// Scripts that run within a map (currently not implemented yet)
		rsl::unique_array<rsl::string> scripts;

		// The block indices of the map. This is required for rendering
		rsl::unique_array<s8> blocks;

		// The map header, holding high level info of the map
		// This holds all the information to identify the map without needing to load all the objects of the map
		MapHeader map_header;
	};

	class Map : public Asset
	{
	public:
		Map(MapDesc&& desc);

		s32 width_in_blocks() const;
		s32 height_in_blocks() const;
		s32 border_block_idx() const;

		const rsl::unique_array<MapConnection>& connections() const;

	private:
		MapDesc m_desc;
	};
}