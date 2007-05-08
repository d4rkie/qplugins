//-----------------------------------------------------------------------------
//
// File:    QCDModDefs.h
//
// About:   Module definitions file.  Miscellanious definitions used by different
//          module types.  This file is published with the plugin SDKs.
//
// Authors: Written by Paul Quinn
//
// Copyright:
//
//  Quintessential Player Plugin Development Kit
//
//  Copyright (C) 1997-2006 Quinnware
//
//  This code is free.  If you redistribute it in any form, leave this notice 
//  here.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//-----------------------------------------------------------------------------

#ifndef QCDMODDEFS_H
#define QCDMODDEFS_H

#include <windows.h>

#ifdef __cplusplus
#define PLUGIN_API extern "C" __declspec(dllexport)
#else
#define PLUGIN_API __declspec(dllexport)
#endif

// Plugin Prefix (for branded players)
// The prefix will make plugins from other brands incompatible

#if defined GNPLAYER
	#define PLUGIN_PREFIX		GN
#elif defined DDIPLAYER
	#define PLUGIN_PREFIX		DDi
#elif defined LIVEDOOR
	#define PLUGIN_PREFIX		LD
#elif defined TRANS
	#define PLUGIN_PREFIX		Trans
#elif defined QCDPLAYER
	#define PLUGIN_PREFIX		Q
#else
	#error No player brand defined
#endif

#define PLUGIN_ENTRY_POINT_PREFIX(prefix, name)		prefix ## name
#define PLUGIN_ENTRY_POINT_EXPAND(prefix, name)		PLUGIN_ENTRY_POINT_PREFIX(prefix, name)
#define PLUGIN_ENTRY_POINT(name)					PLUGIN_ENTRY_POINT_EXPAND(PLUGIN_PREFIX, name)

// Current plugin version

// use this version for old style API calls (all returned text in native encoding)
#define PLUGIN_API_VERSION              300

// use this version for new style API calls (all returned text in UTF8 encoding on WinNT/2K/XP (native encoding on Win9x))
#define PLUGIN_API_VERSION_NTUTF8		((PLUGIN_API_NTUTF8<<16)|PLUGIN_API_VERSION)
#define PLUGIN_API_VERSION_UNICODE		((PLUGIN_API_UNICODE<<16)|PLUGIN_API_VERSION)
#define PLUGIN_API_NTUTF8				100
#define PLUGIN_API_UNICODE				1000

//-----------------------------------------------------------------------------
// QCDModInfo
// Populated by plugins on initialization to give player description of 
// plugin and indicate supported functionality. Populating this struct
// differs based on plugin type
//
// For Input Plugins
//		moduleString - string to display for plugin in player
//		moduleExtensions - colon delmited list of supported extensions
//		moduleCategory - category for extensions (audio or video). If null, defaults to audio
//
// For Encoder Plugins
//		moduleString - string to display for plugin in player
//		moduleExtensions - string to display for encoder format
//		moduleCategory -  ignored
//
// For All other plugins
//		moduleString - string to display for plugin in player
//		moduleExtensions - ignored
//		moduleCategory -  ignored
//

struct QCDModInfo
{
    char*        moduleString;                  // display string for plugin
    char*        moduleExtensions;              // file extensions (colon delimited)
    char*        moduleCategory;                // "audio" or "video"
    int          moduleParam;                   // rank or other parameter for the plugin (TBD)
    int          moduleFlags;                   // MODINFO_FLAGS_*
};

#define MODINFO_VALID_CATEGORY          0x100   // moduleCategory field exists
#define MODINFO_VALID_PARAM			    0x200   // moduleParam field exists
#define MODINFO_VALID_FLAGS			    0x400   // moduleFlags field exists
#define MODINFO_VALID_CURRENT          (MODINFO_VALID_CATEGORY|MODINFO_VALID_PARAM|MODINFO_VALID_FLAGS)


#define MODINFO_FLAGS_LOADONSTARTUP		0x1000	// Many plugins are not loaded until needed. This flag
                                                // will tell the player to load it on startup. Only
                                                // use if truly need to initialize on startup.


//-----------------------------------------------------------------------------
// Services (ops) provided by the Player
// 
// Parameters to service ops are buffer, param1, param2.
// Relevant parameters for each op listed with op. Unmentioned
// parameters for an op are unused (leave 0).
// 
// For parameters defined as type (char*), the string should be a UTF8 string
// if you specified PLUGIN_API_VERSION_NTUTF8 for the plugin version AND
// the current system is Windows NT (NT/2000/XP)
//-----------------------------------------------------------------------------
enum PluginServiceOp
{
//----Player and Window States-------------------------------------------------

    opGetPlayerVersion = 0,         // Returns version (high word = major version (eg 3.01 is 3), low word = minor (eg 3.01 = 1))
                                    //      param1 = 1, returns build number

    opGetPlayerString = 3,          // Get name of player
                                    //      buffer = (char*)buffer for string
                                    //      param1 = size of buffer in bytes
                                    //      param2 = 0 - friendly name, 1 - short name

    opGetPlayerInstance = 2,        // Returns HINSTANCE to player executable
    opGetPlayerState = 9,           // Returns current state of the player (1 = stopped, 2 = playing, 3 = paused, 0 = failed))
    opGetEncoderState = 5,          // Returns current state of the encoder (1 = stopped, 2 = encoding, 0 = failed))

    opGetAlwaysOnTop = 22,          // Returns 1 if player is set to 'Always on Top'

    opGetParentWnd = 1,             // Returns HWND to player window
    opGetExtensionWnd = 30,         // Returns handle to the draggable window extension (only available on some skins)
                                    //      param1 = extension number (0 - 9), param2 unused. 

    opGetMusicBrowserWnd = 32,      // Returns HWND to music browser window (returns 0 if window is closed)
    opGetSkinPreviewWnd = 33,       // Returns HWND to the skin preview window (returns 0 if window is closed)
    opGetPropertiesWnd = 34,        // Returns HWND to preferences window (returns 0 if window is closed)
    opGetAboutWnd = 36,             // Returns HWND to the about window (returns 0 if window is closed)
    opGetEQPresetsWnd = 38,         // Returns HWND to the EQ presets window (returns 0 if window is closed)

    opGetVisualsWnd = 31,            // Returns HWND to the external visual window (returns 0 if window is closed)
    opGetVisTarget = 21,            // Returns where visual effect is being drawn (1 - internal to skin, 2 - external window, 3 - fullscreen)
    opGetVisDimensions = 50,        // Returns width and height of visual window (HEIGHT in high word, WIDTH in low word )
                                    //      param1 = -1 current vis window, 1 internal vis, 2 external vis, 3 fullscreen

    opGetVideoWnd = 39,             // Returns HWND to the video window (where the actual video appears, internal to skin or in extenal window)
    opGetVideoTarget = 40,          // Returns where video is being drawn (1 - internal to skin, 2 - external window, 3 - fullscreen)
    opShowVideoWindow = 55,         // Show or Close video window
                                    //      param1 = 0 - close, 1 - create, 2 - show (create if needed), 3 - fullscreen
									//      (when param1 == 2) param2 = 0 - default, 1 - force embedded video, 2 - force external video (returns 0 if cannot show)
									//      (when param1 == 3) param2 = 0 - disable fullscreen, 1 - enable fullscreen

    opGetLibraryWnd = 70,           // Returns HWND to the library parent window
    opShowLibraryWindow = 71,       // Show or Close library window
                                    //      param1 = 0 - close, 1 - create, 2 - show (create if needed)

//-----Internet Connection State-----------------------------------------------

    opGetOffline = 20,              // Returns 1 if client is in Offline Mode (0 - client is online)
    opGetProxyInfo = 202,           // Returns 1 if proxy is being used
                                    //      buffer = ProxyInfo*

//-----Playlist Info/Manipulation----------------------------------------------

    opGetNumTracks = 10,            // Returns number of tracks in playlist
                                    //      param2 = 8 - apply to encoder list

    opGetCurrentIndex = 11,         // Returns index of current track in playlist (0 based)
                                    //      param2 = 8 - apply to encoder list

    opGetNextIndex = 12,            // Returns index of next track to play (0 based)
                                    //      param1 = start index. -1 for after current
                                    //      param2 = (int*)didCycleFlag, set to 1 if returned index is start of repeated playlist

    opGetTrackNum = 13,             // Returns track number of index
                                    //      param1 = index of track in playlist, -1 for current
                                    //          The 'track number' is the number of the track in it's respective album, as opposed to playlist number
                                    //          The track number for digital files will be 1 if the tag is not set or the file is not identified
                                    //      param2 = 8 - apply to encoder list

    opGetTrackLength = 14,          // Returns track length
                                    //      param1 = index of track in playlist, -1 for current
                                    //      param2 = 0 for seconds, 1 for milliseconds, 8 - apply to encoder list (options can be OR'd together)

    opGetTime = 15,                 // Returns time on player (in seconds)
                                    //      param1 = 0 for time displayed, 1 for track time, 2 for playlist time
                                    //      param2 = 0 for elapsed, 1 for remaining

    opGetOutputTime = 29,           // Returns position of current track (in milliseconds)

    opGetTrackState = 16,           // Returns whether track is selected
                                    //      param1 = index of track, -1 for current
                                    //      param2 = 8 - apply to encoder list

    opGetPlaylistNum = 17,          // Returns playlist number of index
                                    //      param1 = index of track in playlist, -1 for current
                                    //      param2 = 8 - apply to encoder list

    opGetIndexFromPLNum = 28,       // Returns index from playlist number
                                    //      param1 = playlist number
                                    //      param2 = 8 - apply to encoder list

    opGetIndexFromFilename = 210,   // Returns the index of a filename that exists in current playlist (-1 if not in playlist)
                                    //      buffer = (char*)full path of file
                                    //      param1 = startindex (index to start searching on)
                                    //      param2 = 8 - apply to encoder list

    opUpdateIndexMetadata = 88,     // Forces player to update a track's metadata (ie: requery's lib plugins)
                                    //      param1 = index

    opUpdateIndexExts = 89,         // Forces player to update a track's TrackExtents (ie: to recalc file length)
                                    //      param1 = index
                                    //      param2 = 0 - update if not currently updated, 1 - updated required

    opDeleteIndex = 90,             // Delete index from playlist
                                    //      param1 = index
                                    //      param2 = 8 - apply to encoder list

    opSelectIndex = 91,             // Set index as selected
                                    //      param1 = index
                                    //      param2 = 0 - unset, 1 - set, 8 - apply to encoder list (options can be OR'd together)

    opBlockIndex = 92,              // Set index as blocked
                                    //      param1 = index
                                    //      param2 = 0 - unset, 1 - set, 8 - apply to encoder list (options can be OR'd together)

    opSetPlayNext = 1009,           // Set the next index to be played
                                    //      param1 = index, -1 unsets playnext

    opSetPlaylist = 1006,           // Clear current playlist, add files to playlist or load playlist with new files 
                                    //      buffer = (char*)file list (each file in quotes, string null terminated) Eg; buffer="\"file1.mp3\" \"file2.mp3\"\0" - NULL to clear playlist
                                    //      param1 = (char*)originating path (can be NULL if paths included with files) 
                                    //      param2 = 0 - add files, 1 - clear playlist flag, 2 - add to top of playlist, 4 - add unique files only, 8 - apply to encoder list (options can be OR'd together)

    opInsertPlaylist = 1011,        // Insert tracks into playlist 
                                    //      buffer = (char*)file list (each file in quotes, string null terminated) Eg; buffer="\"file1.mp3\" \"file2.mp3\"\0"
                                    //      param1 = (char*)originating path (can be NULL if paths included with files) 
                                    //      param2 = index location to insert tracks (-1 to insert at end)

    opMovePlaylistTrack = 1012,     // Move a track in playlist 
                                    //      param1 = index of track to move
                                    //      param2 = destination index (move shifts tracks between param1 and param2)

    opSwapPlaylistTracks = 1013,    // Swap two tracks in playlist
                                    //      param1 = index of first track
                                    //      param2 = index of second track (swap only switches indecies param1 and param2)

    opGetQueriesComplete = 60,      // Returns status on whether all tracks in playlist have been queryied for their info (1 yes, 0 no)

    opLoadMediaList = 1030,         // Load media files into playlist using IQCDMediaList* interface (alternative to opSetPlaylist)
                                    //      buffer = IQCDMediaList* interface pointer
                                    //      param1 = unused, set to 0
                                    //      param2 = 0 - add files, 1 - clear playlist flag, 2 - add to top of playlist, 4 - add unique files only, 8 - apply to encoder list, 16 - wait for add complete (options can be OR'd together)

    opEditMediaList = 1031,         // Edit media files contained in IQCDMediaList* interface
                                    //      Use to bring up editor (e.g.: tag editor) for set of files contained in IQCDMediaList)
                                    //      Which editor is launched depends on file types and installed plugins
                                    //      buffer = IQCDMediaList* interface pointer

    opSetIndexFilename = 1010,      // Set the filename (or URL) that an index in the current playlist refers to
                                    //      buffer = (char*)filename
                                    //      param1 = index
                                    //      param2 = 8 - apply to encoder list

    opMediaNameChange = 1020,       // Notifies player (which notifies library plugins) that a filename has changed
                                    //      buffer = (char*)oldMediaName
                                    //      param1 = (char*)newMediaName
                                    //      param2 = (MediaTypes)mediaType (can set if media type changed)

//----Playback control---------------------------------------------------------

    opSetSeekPosition = 1100,       // Seek to position during playback
                                    //     param1 = position
                                    //     param2 = 0 - position is in seconds, 1 - position is in milliseconds, 2 - position is in percent (use (float)param1))

    opGetRepeatState = 23,          // Returns playlist repeat state: 0 - repeat off, 1 - repeat track, 2 - repeat all
    opGetShuffleState = 27,         // Returns playlist shuffle state: 0 - shuffle off, 1 - shuffle enabled

    opSetRepeatState = 1110,        // Set playlist repeat state
                                    //      param1 = 0 - off, 1 - repeat track, 2 - repeat playlist

    opSetShuffleState = 1111,       // Set playlist shuffle state
                                    //      param1 = 0 - off, 1 - on

    opGetTimerState = 24,           // Returns timer display state
                                    //      (low word: 0 - track ascend, 1 - playlist ascend, 2 - track descend, 3 - playlist descend)
                                    //      (hi word: 1 if 'show hours' is set)

    opGetVolume = 25,               // Returns player volume level (0 - 100)
                                    //      param1: 0 = combined, 1 = left, 2 = right

    opSetVolume = 26,               // Set player volume level
                                    //      param1: vol level 0 - 100
                                    //      param2: balance (-100 left, 0 center, 100 right)

//----Track Info---------------------------------------------------------------

    opGetMediaType = 18,            // Returns media type of track
                                    //      param1 = index if track in playlist, -1 for current
                                    //          See MediaTypes below for return values
                                    //      param2 = 8 - apply to encoder list

    opGetAudioInfo = 19,            // Returns format info about currently playing track
                                    //      param1 = 0 for samplerate, 1 for bitrate, 2 for num channels

    opGetMediaID = 115, // legacy
    opGetPlaylistMediaID = 115,     // Returns media identifier of index in playlist
    opGetEncoderMediaID = 130,      // Returns media identifier of index in encoder
                                    //      buffer = (char*)buffer for media id
                                    //      param1 = size of buffer in bytes
                                    //      param2 = index of track in playlist, -1 for current
                                    //          for CD's it's the TOC - for anything else, right now it's 0

    opGetTrackFile = 103, // legacy
    opGetPlaylistFile = 103,        // Returns filename of track in playlist
    opGetEncoderFile = 131,         // Returns filename of track in encoder
                                    //      buffer = (char*)buffer for filename
                                    //      param1 = size of buffer in bytes
                                    //      param2 = index of track in playlist, -1 for current

//-----Current Skin Info-------------------------------------------------------

    opGetSkinName = 104,            // Returns current skin name
                                    //      buffer = (char*)buffer for filename
                                    //      param1 = size of buffer in bytes

    opGetSkinFile = 109,            // Returns current skin filename
                                    //      buffer = (char*)buffer for filename
                                    //      param1 = size of buffer in bytes

    opGetSkinMode = 118,            // Returns current skin mode

//-----Folder Settings---------------------------------------------------------

    opGetPlayerFolder = 113,        // Returns folder where player is launched from
                                    //      buffer = (char*)buffer for folder
                                    //      param1 = size of buffer in bytes

    opGetPluginFolder = 105,        // Returns a set plugin folder
                                    //      buffer = (char*)buffer for folder
                                    //      param1 = size of buffer in bytes
									//      param2 = 0 - default plugin folder, 100 - Gracenote folder
                                    //          or param2 = (char*)path to plugin making call to get related folder

    opGetPluginSettingsFile = 106,  // Returns settings file (plugins.ini) that plugin should save settings to
                                    //      buffer = (char*)buffer for filename
                                    //      param1 = size of buffer in bytes

    opGetPluginCacheFile = 107,     // Returns file that describes plugin validity, functions and names
                                    //      buffer = (char*)buffer for filename
                                    //      param1 = size of buffer in bytes

    opGetPlayerSettingsFile = 108,  // Returns settings file (qcd.ini) that player saves it settings to (should use for read-only)
                                    //      buffer = (char*)buffer for filename
                                    //      param1 = size of buffer in bytes

    opGetSettingsFolder = 110,      // Returns folder suitable for settings
                                    //      buffer = (char*)buffer for folder
                                    //      param1 = size of buffer in bytes
									//      param2 = 0 - default folder, 1 - force non multi-user folder, 100 - Gracenote folder

    opGetPlaylistFolder = 111,      // Returns current playlist folder
                                    //      buffer = (char*)buffer for folder
                                    //      param1 = size of buffer in bytes

    opGetSkinFolder = 112,          // Returns a set skin folder
                                    //      buffer = (char*)buffer for folder
                                    //      param1 = size of buffer in bytes
                                    //      param2 = skin folder index (0 based)

    opGetCurrentPlaylist = 114,     // Returns full pathname of playlist currently loaded 
                                    //      buffer = (char*)buffer for filename
                                    //      param1 = size of buffer in bytes

//-----Supported File Types----------------------------------------------------

    opGetSupportedExtensions = 116, // Returns file extensions supported by the player
                                    //      buffer = (char*)buffer for extensions
                                    //      param1 = size of buffer in bytes
                                    //      params = flags (see opGetSupportedExtensions flags below)
                                    //          returned extensions will be colon ':' delimited

    opGetMediaSupported = 119,      // Returns MediaTypes of filename (UNKNOWN_MEDIA means file is unsupported)
                                    //      buffer = (char*)medianame

    opGetDefaultPlaylistExt = 122,  // Returns default extension for playlists
                                    //      buffer = (char*)buffer for extensions
                                    //      param1 = size of buffer in bytes

//-----Menu Handles------------------------------------------------------------

    opShowMainMenu = 120,           // Display main menu
                                    //      buffer = POINT* - location to display menu

    opGetMainMenu = 121,            // Returns copy of HMENU handle to QCD Menu (use DestroyMenu on handle when complete)

    opShowQuickTrack = 125,         // Display QuickTrack menu
                                    //      buffer = POINT* - location to display menu

    opGetQuickTrack = 126,          // Returns copy of HMENU handle to QuickTrack menu (must use DestroyMenu on handle when complete)
                                    //      To use if QuickTrack item selected: PostMessage(hwndPlayer, WM_COMMAND, menu_id, 0);

//-----EQ State----------------------------------------------------------------

    opGetEQVals = 200,              // Returns current EQ levels/on/off
                                    //      buffer = EQInfo* (see EQInfo below)

    opSetEQVals = 201,              // Sets EQ levels/on/off
                                    //      buffer = EQInfo* (see EQInfo below)

    opGetPlaybackEQ = 203,          // Returns if EQ is to be handled by playback plugin or not
	                                //      returns 1 - playback plugin eq, 0 not playback plugin eq (handled elsewhere)

//-----Browser, Status and Audio Info msgs-------------------------------------

    opSetStatusMessage = 1000,      // Sets message for status area
                                    //      buffer = (char*)msg buffer
                                    //      param1 = TEXT_* flags (see definitions below)

    opSetBrowserUrl = 1001,         // Set music browser URL 
                                    //      buffer = (char*)url (buffer = 0 closes browser)
                                    //      param1 = 0 - normal browser behavior, 1 - force browser open

    opSetAudioInfo = 1002,          // set the current music bitrate/khz
                                    //      buffer = AudioInfo* (see AudioInfo below)

//----Track Metadata-----------------------------------------------------------

    opGetTrackName = 100,           // Returns track name
                                    //      buffer = (char*)buffer for name
                                    //      param1 = size of buffer in bytes
                                    //      param2 = index of track in playlist, -1 for current

    opGetArtistName = 101,          // Returns artist name
                                    //      buffer = (char*)buffer for name
                                    //      param1 = size of buffer in bytes
                                    //      param2 = index of track in playlist, -1 for current

    opGetDiscName = 102, // legacy
    opGetAlbumName = 102,            // Returns disc name
                                    //      buffer = (char*)buffer for name
                                    //      param1 = size of buffer in bytes
                                    //      param2 = index of track in playlist, -1 for current

    opGetPlaylistString = 117,      // Returns string for index as it appears in playlist
                                    //      buffer = (char*)buffer for string
                                    //      param1 = size of buffer in bytes
                                    //      param2 = index of track in playlist, -1 for current

    opSetTrackAlbum = 1003,         // Set track ablum name
                                    //      buffer = (char*)album
                                    //      param1 = (char*)file name
                                    //      param2 = MediaTypes

    opSetTrackTitle = 1004,         // Set track title
                                    //      buffer = (char*)title
                                    //      param1 = (char*)file name
                                    //      param2 = MediaTypes

    opSetTrackArtist = 1005,        // Set track artist name
                                    //      buffer = (char*)artist
                                    //      param1 = (char*)file name
                                    //      param2 = MediaTypes

//----Track Params-------------------------------------------------------------

    opSetTrackExtents = 1007,       // Set track TrackExtents info
                                    //      buffer = TrackExtents* (see TrackExtents below)
                                    //      param1 = (char*)filename

    opSetTrackSeekable = 1008,      // Set track seekable flag
                                    //      buffer = (char*)filename
                                    //      param1 = 1 - can seek, 0 not seekable

//----Plugin Menu, Preferences, and Acclerators--------------------------------

    opSetPluginMenuItem = 2000,     // Set entry for plugin in Plugin Menu
                                    //      buffer = HINSTANCE of plugin (for Configure callback) or HWND for msg
                                    //      param1 = item id
                                    //      param2 = (char*)string to display
                                    //          param2 = 0 to remove 'item id' from menu
                                    //          param1 = 0 and param2 = 0 to remove whole menu

    opSetPluginMenuState = 2001,    // Set state of entry in Plugin Menu
                                    //      buffer = HINSTANCE of plugin (for Configure callback) or HWND for msg
                                    //      param1 = item id
                                    //      param2 = menu flags (same as windows menu flags - eg: MF_CHECKED)

    opSetContextMenuItem = 2005,    // Set entry for plugin in Playlist Context Menu
                                    //      buffer = HINSTANCE of plugin (for Configure callback) or HWND for msg
                                    //      param1 = item id
                                    //      param2 = (char*)string to display
                                    //          param2 = 0 to remove 'item id' from menu
                                    //          param1 = 0 and param2 = 0 to remove whole menu

    opSetContextMenuState = 2006,   // Set state of entry in Playlist Context Menu
                                    //      buffer = HINSTANCE of plugin (for Configure callback) or HWND for msg
                                    //      param1 = item id
                                    //      param2 = menu flags (same as windows menu flags - eg: MF_CHECKED)

    opSetContextMenuContext = 2007, // Set media types menu item should be enabled for
                                    //      buffer = HINSTANCE of plugin (for Configure callback) or HWND for msg
                                    //      param1 = item id
                                    //      param2 = flags (see opSetContextMenuContext flags below)

    opSetMainMenuItem = 2010,       // Set entry for plugin in Main Menu
                                    //      buffer = HINSTANCE of plugin (for Configure callback) or HWND for msg
                                    //      param1 = item id
                                    //      param2 = (char*)string to display
                                    //          param2 = 0 to remove 'item id' from menu
                                    //          param1 = 0 and param2 = 0 to remove whole menu

    opSetMainMenuState = 2011,      // Set state of entry in Main Menu
                                    //      buffer = HINSTANCE of plugin (for Configure callback) or HWND for msg
                                    //      param1 = item id
                                    //      param2 = menu flags (same as windows menu flags - eg: MF_CHECKED)

    opSetEncSrcMenuItem = 2015,     // Set entry for plugin in Encoder Source Menu
                                    //      buffer = HINSTANCE of plugin (for Configure callback) or HWND for msg
                                    //      param1 = item id
                                    //      param2 = (char*)string to display
                                    //          param2 = 0 to remove 'item id' from menu
                                    //          param1 = 0 and param2 = 0 to remove whole menu

    opSetEncSrcMenuState = 2016,    // Set state of entry in Encoder Source Menu
                                    //      buffer = HINSTANCE of plugin (for Configure callback) or HWND for msg
                                    //      param1 = item id
                                    //      param2 = menu flags (same as windows menu flags - eg: MF_CHECKED)

    opSetPluginPage = 2020,         // Set page in preferences for plugin (returns page id)
                                    //      buffer = PluginPrefPage*

    opRemovePluginPage = 2021,      // Remove page in preferences for plugin
                                    //      buffer = HINSTANCE of plugin
                                    //      param1 = prefPage id (return value from opSetPluginPage)

    opShowPluginPage = 2022,        // Display page in preferences
                                    //      buffer = HINSTANCE of plugin
                                    //      param1 = prefPage id (return value from opSetPluginPage)
                                    //      param2 = (HWND)handle to owner window - 0 for default

    opSetAccelerator = 2030,        // Sets a local keyboard accelerator
                                    //      buffer = HINSTANCE of plugin
                                    //      param1 = item id
                                    //      param2 = AccelInfo* (see AccelInfo below)
                                    //          param2 = 0 to remove 'item id' from menu
                                    //          param1 = 0 and param2 = 0 to remove all plugin's accelerators

//----Filename templates-------------------------------------------------------

    opShowTemplateEditor = 2100,    // Display template editor dialog
                                    //      param1 = (HWND)parent window
                                    //      param2 = 1 - modal, 0 modeless

    opLoadTemplate = 2101,          // Loads saved template
                                    //      buffer = (char*)template buffer
                                    //      param1 = size of buffer in bytes
                                    //      param2 = index of template (index < 0 for default formats, index >= 0 for user made formats)

    opRenderTemplate = 2102,        // Render string based on template and metadata
                                    //      buffer = (char*)template
                                    //      param1 = FormatMetaInfo*
                                    //      param2 = (char*)string buffer (*** must be min 1024 bytes ***)

//----Language and Resource Loading--------------------------------------------

    opGetCurLangID = 3000,          // Returns current LANGID of set language in player

    opLoadResString = 3001,         // Load string resource from plugin with same language id as player
                                    //      buffer = (char*)string buffer
                                    //      param1 - size of buffer in bytes
                                    //      param2 - ResInfo* (see ResInfo below)

    opLoadResDialog = 3002,         // Load dialog resource from plugin with same language id as player
                                    // Returns LPCDLGTEMPLATE of dialog (use with DialogBoxIndirect, or CreateDialogIndirect)
                                    //      buffer - ResInfo* (see ResInfo below)

    opLoadResMenu = 3003,           // Load menu resource from plugin with same language id as player
                                    // Returns HMENU of loaded menu (call DestroyMenu when finished with handle)
                                    //      buffer - ResInfo* (see ResInfo below)

//----Plugin-Interfaces--------------------------------------------------------

    opGetIQCDTagInfo = 4000,        // Get pointer to tag editing interface for given file
                                    // Returns IQCDTagInfo* if filename has tag editing support, NULL if not
                                    //      buffer = (char*)filename
                                    //      param2 = 1 - return IQCDTagInfo* regardless of support
                                    //      call Release on interface when done

    opGetIQCDFileInfo = 4010,       // Get pointer to file info interface for given file
                                    // Returns IQCDFileInfo* if filename has file info support, NULL if not
                                    //      buffer = (char*)filename
                                    //      call Release on interface when done

    opGetIQCDMediaInfo = 4020,      // Get pointer to metadata interface (for info being used for given file)
                                    // Returns IQCDMediaInfo* if filename supported
                                    //      buffer = (char*)filename
                                    //      param1 = (MediaTypes)mediaType (set if creating new IQCDMediaInfo)
                                    //      call Release on interface when done

    opGetIQCDMediaList = 4030,      // Get pointer to media list interface
                                    // Returns IQCDMediaList*
                                    //      call Release on interface when done

    opGetIQCDPlaylistInfo = 4040,   // Get pointer to playlist info interface
                                    // Returns IQCDPlaylistInfo* if playlistname supported
                                    //      buffer = (char*)playlistname
                                    //      call Release on interface when done

    opGetIQCDSkinHelper = 4050,     // Get pointer to skinhelper info interface
                                    // Returns IQCDSkinHelper*
                                    //      call Release on interface when done

    opGetIQCDMediaDecoder = 4060,   // Get pointer to mediadecoder info interface
                                    // Returns IQCDMediaDecoder* if filename supported
                                    //      buffer = (char*)filename
                                    //      call Release on interface when done

    opGetIQCDMediaSource = 4070,    // Get pointer to skinhelper info interface
                                    // Returns IQCDMediaSource* if filename supported
                                    //      buffer = (char*)filename
                                    //      param1 = MEDIASOURCE_* creation flags
                                    //      param2 = (char*)name of source module (optional: request named implementation)
                                    //      call Release on interface when done

    opGetIQCDMediaLibrary = 4080,   // Get pointer to media library interface
                                    // Returns IQCDMediaLibrary*
                                    //      call Release on interface when done

    opGetIQCDBrowserHost = 4090,    // Get pointer to hosted web browser interface
                                    // Return IQCDBrowserHost*
                                    //      buffer = (IQCDBrowserHostEvents*)pointer to events interface (optional)
                                    //      call Release on interface when done

    opGetIQCDHttpGet = 4100,        // Get pointer to http data retrieval interface
                                    // Return IQCDHttpGet*
                                    //      buffer = (IQCDHttpEvent*)pointer to events interface (optional)
                                    //      param1 = (BOOL*)pointer to cancel var
                                    //      call Release on interface when done

//----Common Dialogs----------------------------------------------------------

    opOpenFilesDlg = 5000,          // Launches player's file selection dialog with filetype/URL chooser
                                    // Returns IQCDMediaList* if folder selected, 0 otherwise
                                    //      buffer = (char*)initial folder
                                    //      param1 = see Common Dialog flags below

    opOpenFolderDlg = 5001,         // Launches player's folder selection dialog with filetype chooser
                                    // Returns IQCDMediaList* if folder selected, 0 otherwise
                                    //      buffer = (char*)initial folder
                                    //      param1 = see Common Dialog flags below

    opSavePlaylistDlg = 5010,       // Launches player's playlist name selection dialog
                                    // Returns IQCDMediaList* if playlist name selected
                                    //      buffer = (char*)initial folder
                                    //      param1 = (char*)initial playlist name

//----Custom Skin Controls----------------------------------------------------------

    opAttachCustomCtrl = 6000,      // Set message window to respond to custom controls defined by the skin
                                    //      buffer = (WCHAR*)name of control (defined by skin)
                                    //      param1 = (HWND)handle to window to receive control msgs
                                    //      param2 = application defined value (sent as lParam in msg)
                                    // Returns 0 if no control found, > 0 if found

//----Unicode-UTF8 conversion--------------------------------------------------

    opUTF8toUCS2 = 9000,            // Convert UTF8 string to UCS2 (Unicode) string
                                    //      buffer = null terminated utf8 string
                                    //      param1 = (WCHAR*)result string buffer
                                    //      param2 = size of result buffer (int WCHARs)

    opUCS2toUTF8 = 9001,            // Convert UCS2 (Unicode) string to UTF8 string
                                    //      buffer = null terminated ucs2 string
                                    //      param1 = (char*)result string buffer
                                    //      param2 = size of result buffer (in bytes)

    opMBtoUCS2 = 9002,              // Convert MultiByte string to UCS2 (Unicode) string
                                    // The multibyte string will be UTF8 encoded on NT systems, and ACP encoded on Win9x systems
                                    // (ACP refers to 'active code page')
                                    //      buffer = null terminated multibyte string
                                    //      param1 = (WCHAR*)result string buffer
                                    //      param2 = size of result buffer (int WCHARs)

    opUCS2toMB = 9003,              // Convert UCS2 (Unicode) string to MultiByte string
                                    // The multibyte string will be UTF8 encoded on NT systems, and ACP encoded on Win9x systems
                                    // (ACP refers to 'active code page')
                                    //      buffer = null terminated ucs2 string
                                    //      param1 = (char*)result string buffer
                                    //      param2 = size of result buffer (in bytes)

//----Miscellaneous------------------------------------------------------------------

    opSafeWait = 10000,             // Plugins can use this to wait on an object without worrying about deadlocking the player.
                                    // This should only be called by the thread that enters the plugin, not by any plugin-created threads

    opGetAccountType = 11000,       // Returns current user's account permissions
                                    //     1 = Guest
									//     2 = User
									//     3 = Power User
									//     4 = Administrator (this value always returned on Win9x systems)

    opGetNetworkLogin = 11100,      // Shows Network Login dialog
                                    // Returns 1 on OK, 0 on Cancel
                                    //     buffer = (char*)result str buffer (result will be formatted username:password)
                                    //     param1 = sizeof result buffer (in bytes)
                                    //     param2 = (char*)null separated string of initialization values (double null terminated)
                                    //         hostname\0realm\0username\0password\0\0
};


//-----------------------------------------------------------------------------
// Info services api provided by the Player, called by Plugin.
//-----------------------------------------------------------------------------
typedef long (*PluginServiceFunc)(enum PluginServiceOp op, void *buffer, long param1, long param2);

// Use to retrieve service func for DSP plugins (or other inproc process that doesn't have access to PluginServiceFunc)
// Eg: PluginServiceFunc Service = (PluginServiceFunc)SendMessage(hwndPlayer, WM_GETSERVICEFUNC, 0, 0);
// Set WPARAM = PLUGIN_API_WANTUTF8 for UTF8 string parameters
#define WM_GETSERVICEFUNC               (WM_USER+1)


//-----------------------------------------------------------------------------
// Types for QCD callbacks
//-----------------------------------------------------------------------------


//----WriteDataStruct----------------------------------------------------------

struct WriteDataStruct  // for Output Plugin Write callback
{
    void    *data;               // pointer to valid data
    int     bytelen;             // length of data pointed to by 'data' in bytes
    UINT    numsamples;          // number of samples represented by 'data'
    UINT    bps;                 // bits per sample
    UINT    nch;                 // number of channels
    UINT    srate;               // sample rate

    UINT    markerstart;         // Marker position at start of data (marker is time value of data) 
                                 // (set to WAVE_VIS_DATA_ONLY to not have data sent to output plugins)
    UINT	markerend;           // Marker position at end of data (not currently used, set to 0)
};


//----TrackExtents-------------------------------------------------------------

struct TrackExtents     // for GetTrackExtents Input Plugin callback
{
    UINT track;                  // for CD's, set the track number. Otherwise set to 1.
    UINT start;                  // for CD's or media that doesn't start at the beginning 
                                 // of the file, set to start position. Otherwise set to 0.
    UINT end;                    // set to end position of media.
    UINT unitpersec;             // whatever units are being used for this media, how many
                                 // of them per second. 
                                 // (Note: ((end - start) / unitpersecond) = file length
    UINT bytesize;               // size of file in bytes (if applicable, otherwise 0).
};


//----AudioInfo----------------------------------------------------------------

struct AudioInfo        // for opSetAudioInfo service
{
    long struct_size;            // sizeof(AudioInfo)
    long level;                  // MPEG level (1 for MPEG1, 2 for MPEG2, 3 for MPEG2.5, 7 for MPEGpro)
    long layer;                  // and layer (1, 2 or 3)
    long bitrate;                // audio bitrate in bits per second
    long frequency;              // audio freq in Hz
    long mode;                   // 0 for stereo, 1 for joint-stereo, 2 for dual-channel, 3 for mono, 4 for multi-channel
    char text[8];                // up to eight characters to identify format (will override level and layer settings)
};


//-----EQInfo------------------------------------------------------------------

struct EQInfo           // for EQ settings
{
    long struct_size;            // sizeof(EQInfo)
    char enabled;                // eq is enabled, 0 is disabled
    char preamp;                 // -128 to 127, 0 is even
    char bands[10];              // -128 to 127, 0 is even
};


//----ProxyInfo----------------------------------------------------------------

struct ProxyInfo        // for opGetProxyInfo
{
    long struct_size;            // sizeof(ProxyInfo)
    char hostname[200];
    long port;
    char username[100];
    char password[100];
    long usePort80Only;
	char proxyauth[200];
};


//----MediaTypes---------------------------------------------------------------

enum MediaTypes        // for MediaInfo.mediaType
{
    UNKNOWN_MEDIA				= 0x00000000,

	// Supported media is assigned one of these MediaTypes
	CD_AUDIO_MEDIA				= 0x00000100,

    DIGITAL_AUDIOFILE_MEDIA		= 0x00001000,
	DIGITAL_AUDIOSTREAM_MEDIA	= 0x00002000,

    DIGITAL_VIDEOFILE_MEDIA		= 0x00010000,
    DIGITAL_VIDEOSTREAM_MEDIA	= 0x00020000,

	PLAYLIST_FILE_MEDIA			= 0x00100000,
	PLAYLIST_STREAM_MEDIA		= 0x00200000,

	DIGITAL_PHOTOFILE_MEDIA		= 0x01000000,
	DIGITAL_PHOTOSTREAM_MEDIA	= 0x02000000,

	// MediaType groups, for comparisons
    DIGITAL_AUDIO_MEDIA			= (DIGITAL_AUDIOFILE_MEDIA|DIGITAL_AUDIOSTREAM_MEDIA),
    DIGITAL_VIDEO_MEDIA			= (DIGITAL_VIDEOFILE_MEDIA|DIGITAL_VIDEOSTREAM_MEDIA),
    DIGITAL_PHOTO_MEDIA			= (DIGITAL_PHOTOFILE_MEDIA|DIGITAL_PHOTOSTREAM_MEDIA),
	PLAYLIST_MEDIA				= (PLAYLIST_FILE_MEDIA|PLAYLIST_STREAM_MEDIA),

	DIGITAL_FILE_MEDIA			= (DIGITAL_AUDIOFILE_MEDIA|DIGITAL_VIDEOFILE_MEDIA),
	DIGITAL_STREAM_MEDIA		= (DIGITAL_AUDIOSTREAM_MEDIA|DIGITAL_VIDEOSTREAM_MEDIA),

	MEDIATYPE_ANY				= (CD_AUDIO_MEDIA|DIGITAL_AUDIO_MEDIA|DIGITAL_VIDEO_MEDIA|PLAYLIST_MEDIA),

	// legacy values, do not use
	LEGACY_MEDIATYPE_MASK				= 0x000000FF,

    LEGACY_CD_AUDIO_MEDIA				= 1,
    LEGACY_DIGITAL_AUDIOFILE_MEDIA		= 2,
    LEGACY_DIGITAL_AUDIOSTREAM_MEDIA	= 3,
    LEGACY_DIGITAL_VIDEOFILE_MEDIA		= 4,
    LEGACY_DIGITAL_VIDEOSTREAM_MEDIA	= 5,
	LEGACY_PLAYLIST_FILE_MEDIA			= 10

};


//----MediaInfo----------------------------------------------------------------

#define MAX_TOC_LEN         2048
struct MediaInfo
{
    // media descriptors
    char		     mediaFile[MAX_PATH];
    enum MediaTypes  mediaType;

    // cd audio media info
    char             cd_mediaTOC[MAX_TOC_LEN];
    int              cd_numTracks;
    int              cd_hasAudio;

    // operation info
    int              op_canSeek;

    // not used
    int              reserved[4];
};


//----FormatMetaInfo-----------------------------------------------------------

struct FormatMetaInfo
{
    long        struct_size;        // sizeof(FormatMetaInfo)
    LPCWSTR     title;
    LPCWSTR     artalb;
    LPCWSTR     album;
    LPCWSTR     genre;
    LPCWSTR     year;
    LPCWSTR     tracknum;
    LPCWSTR     filename;
    LPCWSTR     arttrk;
    LPCWSTR     plnum;
    LPCWSTR     time;
    LPCWSTR     discnum;
};


//----PluginPrefPage-----------------------------------------------------------

struct PluginPrefPage      // for opSetPluginPage
{
    long        struct_size;        // sizeof(PluginPrefPage)
    HINSTANCE   hModule;            // plugin HINSTANCE
    LPCWSTR     lpTemplate;         // dialog resource template
    DLGPROC     lpDialogFunc;       // dialog window proc
    LPCWSTR     lpDisplayText;      // preference item display text
    long        nCategory;          // type of dialog (see PREFPAGE_CATEGORY_* below)
    HINSTANCE   hModuleParent;      // handle to parent item HINSTANCE (set to NULL for default behavior)
    long		groupID;			// value to determine display order of pref item
    long		createParam;		// value passed to CreateDialog
	HICON		hIcon;				// icon to display with item in prefs (plugin can delete icon after call)

};

// PluginPrefPage::nCategory values
#define PREFPAGE_CATEGORY_GENERAL				0x000	// use this value if unsure
#define PREFPAGE_CATEGORY_SYSTEM				0x100
#define PREFPAGE_CATEGORY_UI					0x200 
#define PREFPAGE_CATEGORY_PLAYER				PREFPAGE_CATEGORY_UI // legacy

#define PREFPAGE_CATEGORY_ENCODER				0x300
#define PREFPAGE_CATEGORY_LIBRARY				0x400

#define PREFPAGE_CATEGORY_SKINS					0x500
#define PREFPAGE_CATEGORY_SKINBROWSER			0x600

#define PREFPAGE_CATEGORY_PLUGINS				0x700
#define PREFPAGE_CATEGORY_PLUGINTYPES			0x800

#define PREFPAGE_CATEGORY_GRACENOTE				0x900
#define PREFPAGE_CATEGORY_ENCODEFORMAT			0x1000	// use this value for encoder configuration pages
#define PREFPAGE_CATEGORY_PLAYBACK				0x1100


//----ResInfo------------------------------------------------------------------

struct ResInfo
{
    long        struct_size;        // sizeof(ResInfo)
    HINSTANCE   hModule;            // plugin HINSTANCE
    LPCWSTR     resID;              // resource id of resource
    long        langid;             // requested lang id of resource (0 for current player lang)
    long        encoding;           // 0-default (based on version flag), 1-UTF8, 2-active code page
};


//----AccelInfo----------------------------------------------------------------

struct AccelInfo
{
    long        struct_size;        // sizeof(AccelInfo)
    short       modifiers;          // zero or more of the AI_* values below
    short       key;                // Specifies the accelerator key. This member can be either a virtual-key code or a character code.
	                                //     If virtual-key code, modifiers must include AI_VIRTKEY
	                                //     (See Windows SDK for virtual-key definitions)
};

// AccelInfo:modifiers values
#define AI_VIRTKEY  0x01
#define AI_SHIFT    0x04
#define AI_CONTROL  0x08
#define AI_ALT      0x10


//----opGetSupportedExtensions flags-------------------------------------------
#define EXTS_ALL						0x0
#define EXTS_REGISTERED					0x1
#define EXTS_AUDIO						0x2
#define EXTS_VIDEO						0x4
#define EXTS_PLAYLIST					0x8
#define EXTS_PLAYLIST_READABLE			0x10
#define EXTS_PLAYLIST_WRITEABLE			0x20

//----Common Dialog flags------------------------------------------------------

// flags for opOpenFolderDlg
#define ODF_SHOWTYPEFILTER				0x1000
#define	ODF_SHOWSUBFOLDERCHECK			0x2000
#define	ODF_LOADSUBFOLDERS				0x4000
#define	ODF_NOEXTENSIONFILTERS			0x8000

#define ODF_DEFAULT_OPENFOLDER			0x0
#define ODF_DEFAULT_LOADFOLDER			(ODF_SHOWTYPEFILTER|ODF_SHOWSUBFOLDERCHECK|ODF_LOADSUBFOLDERS)

// flags for opOpenFilesDlg
#define ODF_SINGLESELCTION				0x10000
#define ODF_SHOWOPENURL					0x20000
#define ODF_SHOWADDUNIQUE				0x40000

#define ODF_DEFAULT_OPENFILES			(ODF_SHOWOPENURL|ODF_SHOWADDUNIQUE|ODF_FILTER_ALL|ODF_SETUSERFOLDER)

// common flags
#define ODF_SETUSERFOLDER				0x10

#define ODF_FILTER_AUDIO				0x100000
#define ODF_FILTER_VIDEO				0x200000
#define ODF_FILTER_PLAYLISTS			0x400000
#define ODF_FILTER_SUPPORTED			0x800000
#define ODF_FILTER_EXTENSIONS			0x1000000
#define ODF_FILTER_ALLFILES				0x2000000

#define ODF_FILTER_ALL					(ODF_FILTER_AUDIO|ODF_FILTER_VIDEO|ODF_FILTER_PLAYLISTS|ODF_FILTER_SUPPORTED|ODF_FILTER_EXTENSIONS|ODF_FILTER_ALLFILES)

// data fields set in returned IQCDMediaList
#define	ODF_RET_EXTFILTER				L"ExtFilter"	// list of extensions to filter to (':' delimited) (opGetFolderDlg)
#define ODF_RET_LOADSUBFOLDERS			L"Recurse"		// "0" for No, "1" for Yes (opGetFolderDlg)
#define ODF_RET_UNIQUEONLY				L"Unique"		// "0" for No, "1" for Yes (opGetFilesDlg)


//----opSetContextMenuContext flags--------------------------------------------
// use combinations of below (eg: CONTEXTMENU_SHOW|CONTEXTMENU_ON_ANY|CONTEXTMENU_AUDIO)

#define CONTEXTMENU_DEFAULT              0x0

#define CONTEXTMENU_ENABLE               0x10    // enable item when conditions (set by below) are true
#define CONTEXTMENU_SHOW                 0x20    // show item when conditions (set by below) are true

#define CONTEXTMENU_ON_SINGLE            0x100   // when selection contains one of selected type
#define CONTEXTMENU_ON_MULTIPLE          0x200   // when selection contains more than one of selected type
#define CONTEXTMENU_ON_ANY               0x400   // when selection contains at least one of selected type
#define CONTEXTMENU_ON_ALL               0x800   // when selection consists of all of selected type

#define CONTEXTMENU_PLAYLIST             0x1000  // if neither defined, CONTEXTMENU_PLAYLIST assumed
#define CONTEXTMENU_ENCODELIST           0x2000

#define CONTEXTMENU_CDAUDIO              0x10000
#define CONTEXTMENU_AUDIOFILE            0x20000
#define CONTEXTMENU_VIDEOFILE            0x40000
#define CONTEXTMENU_AUDIOSTREAM          0x80000
#define CONTEXTMENU_VIDEOSTREAM          0x100000

#define CONTEXTMENU_ANYFILE              (CONTEXTMENU_AUDIOFILE|CONTEXTMENU_VIDEOFILE)
#define CONTEXTMENU_ANYSTREAM            (CONTEXTMENU_AUDIOSTREAM|CONTEXTMENU_VIDEOSTREAM)
#define CONTEXTMENU_ANYTYPE              (CONTEXTMENU_CDAUDIO|CONTEXTMENU_ANYFILE|CONTEXTMENU_ANYSTREAM)


//----opAttachCustomCtrl msgs--------------------------------------------------
// Messages sent to custom control hMsgWnd
//
#define WM_CUSTOMCTRL_GETTOOLTIP		(WM_APP+100)		// wParam = (WCHAR*)buffer, lParam = sizeof buffer in WCHARs
#define WM_CUSTOMCTRL_LBUTTONDOWN		(WM_APP+200)		// wParam = 0, lParam = 0
#define WM_CUSTOMCTRL_RBUTTONDOWN		(WM_APP+201)		// wParam = 0, lParam = 0
#define WM_CUSTOMCTRL_LBUTTONUP			(WM_APP+202)		// wParam = 0, lParam = 0
#define WM_CUSTOMCTRL_RBUTTONUP			(WM_APP+203)		// wParam = 0, lParam = 0
#define WM_CUSTOMCTRL_LBUTTONDBLCLK		(WM_APP+204)		// wParam = 0, lParam = 0


//-----------------------------------------------------------------------------
// When subclassing the parent window, a plugin can watch for these messages
// to react to events going on between plugins and player
// DO NOT SEND THESE MESSAGES - can only watch for them

// Plugin to Player Notifiers
#define WM_PN_PLAYBACKPROGRESS      (WM_USER+100) // playback progress updated
#define WM_PN_PLAYSTARTED           (WM_USER+101) // playback has started
#define WM_PN_PLAYSTOPPED           (WM_USER+102) // playback has stopped by user
#define WM_PN_PLAYPAUSED            (WM_USER+103) // playback has been paused
#define WM_PN_PLAYDONE              (WM_USER+104) // playback has finished (track completed)

#define WM_PN_MEDIAEJECTED          (WM_USER+105) // a CD was ejected (CDRom drive letter= 'A' + lParam)
#define WM_PN_MEDIAINSERTED         (WM_USER+106) // a CD was inserted (CDRom drive letter= 'A' + lParam)
#define WM_PN_INFOCHANGED           (WM_USER+107) // track information was updated (lParam = (LPCSTR)medianame)
#define WM_PN_TRACKCHANGED          (WM_USER+109) // current track playing has changed (relevant from CD plugin) (lParam = (LPCSTR)medianame)

#define WM_PN_ENCODEPROGRESS        (WM_USER+113) // encoding progress updated
#define WM_PN_ENCODESTARTED         (WM_USER+114) // encoding has started
#define WM_PN_ENCODESTOPPED         (WM_USER+115) // encoding has stopped by user
#define WM_PN_ENCODEPAUSED          (WM_USER+116) // encoding has been paused
#define WM_PN_ENCODEDONE            (WM_USER+117) // encoding has finished (track completed)

// Player to Plugin Notifiers
#define WM_PN_PLAYLISTCHANGED       (WM_USER+200) // playlist has changed in some way (add, delete, sort, shuffle, drag-n-drop, etc...)
#define WM_PN_DIALOGSAVE            (WM_USER+300) // sent to all dialogs set in Preferences. Indicates preferences should be saved (closing)

#define WM_PN_NOTIFY_SKINCHANGE		(WM_USER+400)
#define WM_PN_NOTIFY_SKINCOLOR		(WM_USER+401)

// For intercepting main menu display
// (so you can get handle, modify, and display your own)
#define WM_SHOWMAINMENU             (WM_USER+20)

// For intercepting commands for embedded window controls
#define WM_EMBEDDEDWINDOW           (WM_USER+26)
// WM_EMBEDDEDWINDOW wParam's
#define EMBEDDEDWINDOW_NORMALSIZE   0x100000
#define EMBEDDEDWINDOW_DOUBLESIZE   0x200000

// send to border window to cause resize
#define WM_SIZEBORDERWINDOW         (WM_USER+1) // wParam = POINT* for pos of window, lParam = POINT* for client area size
#define WM_MAXIMIZEBORDERWINDOW		(WM_USER+2) // wParam = TRUE for maximize, FALSE for restore

//-----------------------------------------------------------------------------
// To shutdown player, send this command
#define WM_SHUTDOWN                 (WM_USER+5)

//-----------------------------------------------------------------------------
// opSetStatusMessage textflags
#define TEXT_DEFAULT                0x0         // message scrolls by in status window
#define TEXT_TOOLTIP                0x1         // message acts as tooltip in status window
#define TEXT_URGENT                 0x2         // forces message to appear even if no status window (using msg box)
#define TEXT_HOLD                   0x4         // tooltip message stays up (no fade out)
#define TEXT_UNICODE                0x10        // buffer contains a unicode string (multibyte string otherwise)
#define TEXT_NONSYSTEM              0x1000000   // string should be displayed in non-system status area if available


#endif //QCDMODDEFS_H