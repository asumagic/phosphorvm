#pragma once

#include "common.hpp"
#include <ctime>
#include <iomanip>

struct Gen8 : Chunk
{
	u8 debug;
	u8 bytecode_version;
	std::string filename;
	std::string config;
	u32 game_id;
	std::string name;
	s32 major, minor, release, build;
	s32 window_width, window_height;

	struct Flags
	{
		bool
			fullscreen, sync_vertex_1, sync_vertex_2, interpolate, show_cursor,
			resizeable, screen_key, sync_vertex_3;
		u8 studio_version;
		bool is_steam, local_data_enabled, borderless;

	} flags;

	std::time_t timestamp;
	u32 game_targets; // TODO
	u32 steam_appid;

	void debug_print() const
	{
		fmt::print(
			"\t.win version {:03b}, bytecode version ${:02x}\n"
			"\tDebug:    {}\n"
			"\tFile:     {}\n"
			"\tConfig:   {}\n"
			"\tGameID:   ${:08x}\n"
			"\tName:     {}\n"
			"\tVersion:  {}.{}.{}.{}\n" // TODO: convert timestamp to str
			"\tWindow:   {}x{}, borderless={}, fullscreen={}, resizeable={}\n"
			"\tControls: cursor={} screenkey={}\n"
			"\tGraphics: syncvtx=({}; {}; {}), interpolate={}\n"
			"\tSteam:    enabled={} ${:08x}\n"
			"\tMisc:     localdata={}\n"
			"",

			flags.studio_version,
			bytecode_version,
			debug,
			filename,
			config,
			game_id,
			name,
			major,
			minor,
			release,
			build,
			window_width,
			window_height,
			flags.borderless,
			flags.fullscreen,
			flags.resizeable,
			flags.show_cursor,
			flags.screen_key,
			flags.sync_vertex_1,
			flags.sync_vertex_2,
			flags.sync_vertex_3,
			flags.interpolate,
			flags.is_steam,
			steam_appid,
			flags.local_data_enabled
		);
	}
};

inline void user_reader(Gen8& target, Reader& reader)
{
	u32 flags;

	reader
		>> target.debug
		>> target.bytecode_version
		>> skip(2)
		>> string_reference(target.filename)
		>> string_reference(target.config)
		>> skip(2 * 4)
		>> target.game_id
		>> skip(4 * 4)
		>> string_reference(target.name)
		>> target.major
		>> target.minor
		>> target.release
		>> target.build
		>> target.window_width
		>> target.window_height
		>> flags
		>> skip(10) // License MD5
		>> skip(4) // License CRC32
		>> target.timestamp
		>> target.game_targets
		>> skip(4 * 4)
		>> target.steam_appid;

	auto f = [&](unsigned byte_offset) {
		return (flags & (1 << byte_offset)) != 0;
	};

	target.flags = {
		.fullscreen          = f(0),
		.sync_vertex_1       = f(1),
		.sync_vertex_2       = f(2),
		.interpolate         = f(3),
		// unknown
		.show_cursor         = f(4),
		.resizeable          = f(5),
		.screen_key          = f(6),
		.sync_vertex_3       = f(7),
		.studio_version      = u8((flags >> 9) & 0b111),
		.is_steam            = f(12),
		.local_data_enabled  = f(13),
		.borderless          = f(14)
	};
}
