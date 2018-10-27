#ifndef METADATA_HPP
#define METADATA_HPP

#include "common.hpp"
#include <ctime>
#include <iomanip>

struct Gen8
{
	bool debug;
	std::string filename;
	std::string config;
	u32 game_id;
	std::string name;
	s32 major, minor, release, build;
	s32 window_width, window_height;
	bool fullscreen, resizeable, borderless, interpolate, show_cursor;
	bool sync_vertex_1, sync_vertex_2, sync_vertex_3;
	bool screen_key;
	u8 studio_version;
	bool is_steam;
	bool local_data_enabled;
	std::time_t timestamp;
	u32 game_targets; // TODO
	u32 steam_appid;

};

inline void read(Gen8& meta, Reader& reader)
{
	meta.debug = reader.read_pod<u32>();
	meta.filename = reader.read_string_reference();
	meta.config = reader.read_string_reference();
	reader.skip(2 * 4);
	meta.game_id = reader.read_pod<u32>();
	reader.skip(4 * 4);
	meta.name = reader.read_string_reference();
	meta.major = reader.read_pod<s32>();
	meta.minor = reader.read_pod<s32>();
	meta.release = reader.read_pod<s32>();
	meta.build = reader.read_pod<s32>();
	meta.window_width = reader.read_pod<s32>();
	meta.window_height = reader.read_pod<s32>();

	{
		auto flags = reader.read_pod<u32>();
		meta.fullscreen          = flags & (1 << 0);
		meta.sync_vertex_1       = flags & (1 << 1);
		meta.sync_vertex_2       = flags & (1 << 2);
		meta.interpolate         = flags & (1 << 3);
		// unknown
		meta.show_cursor         = flags & (1 << 4);
		meta.resizeable          = flags & (1 << 5);
		meta.screen_key          = flags & (1 << 6);
		meta.sync_vertex_3       = flags & (1 << 7);
		meta.studio_version      = (flags >> 9) & 0b111;
		meta.is_steam            = flags & (1 << 12);
		meta.local_data_enabled  = flags & (1 << 13);
		meta.borderless          = flags & (1 << 14);
	}

	reader.skip(10); // License MD5 - what to handle?
	reader.skip(4); // License CRC32 - what to handle?

	meta.timestamp = reader.read_pod<s64>();
	meta.game_targets = reader.read_pod<u32>();

	reader.skip(4 * 4);

	meta.steam_appid = reader.read_pod<u32>();
	fmt::print(
	    "\t.win version {:03b}\n"
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

	    meta.studio_version,
	    meta.debug,
	    meta.filename,
	    meta.config,
	    meta.game_id,
	    meta.name,
	    meta.major,
	    meta.minor,
	    meta.release,
	    meta.build,
	    meta.window_width,
	    meta.window_height,
	    meta.borderless,
	    meta.fullscreen,
	    meta.resizeable,
	    meta.show_cursor,
	    meta.screen_key,
	    meta.sync_vertex_1,
	    meta.sync_vertex_2,
	    meta.sync_vertex_3,
	    meta.interpolate,
	    meta.is_steam,
	    meta.steam_appid,
	    meta.local_data_enabled
	);
}

#endif // METADATA_HPP
