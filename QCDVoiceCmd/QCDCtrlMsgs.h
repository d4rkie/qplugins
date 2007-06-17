/*
** These are WM_COMMAND messages that you can use to control
** specifics of QCD.
** 
** To send these, use:
** SendMessage(hwndPlayer, WM_COMMAND, command_name, 0);
*/

// open window messages
#define QCD_COMMAND_ABOUT			40001	// about
#define QCD_COMMAND_ONTOP			40002	// toggle 'always on top'
#define QCD_COMMAND_PREF			40004	// preferences
#define QCD_COMMAND_PLUGINS			40006	// plugin browser
#define QCD_COMMAND_SKINS			40026	// skin browser
#define QCD_COMMAND_OPENFILE		40007	// open file/stream
#define QCD_COMMAND_OPENFOLDER		40034	// open folder
#define QCD_COMMAND_INFOBROWSER		40017	// browser window
#define QCD_COMMAND_EDITINFO		40025	// edit info
#define QCD_COMMAND_EXTENDEDINFO	40005	// extended info
#define QCD_COMMAND_SEGMENTS		40024	// cd segments
#define QCD_COMMAND_HELP			42001	// help

// skin control messages
#define QCD_COMMAND_NEXTSKIN		40027	// switch to next skin mode
#define QCD_COMMAND_SKINBASE		40031	// switch to default QCD skin
#define QCD_COMMAND_SKIN1			41036	// switch to skin mode 1 to 9
#define QCD_COMMAND_SKIN2			41037	
#define QCD_COMMAND_SKIN3			41038
#define QCD_COMMAND_SKIN4			41039
#define QCD_COMMAND_SKIN5			41040
#define QCD_COMMAND_SKIN6			41041
#define QCD_COMMAND_SKIN7			41042
#define QCD_COMMAND_SKIN8			41043
#define QCD_COMMAND_SKIN9			41044
#define QCD_COMMAND_SKINREFRESH		41045	// refresh current skin

// playback control messages
#define QCD_COMMAND_REPEATALL		40008	// toggle repeat all
#define QCD_COMMAND_REPEATTRACK		40009	// toggle repeat track
#define QCD_COMMAND_SHUFFLE			40010	// toggle shuffle play
#define QCD_COMMAND_TRKBWD			40012	// previous track
#define QCD_COMMAND_TRKFWD			40013	// next track
#define QCD_COMMAND_STOP			40014	// stop
#define QCD_COMMAND_PAUSE			40015	// pause
#define QCD_COMMAND_PLAY			40016	// play (Presses Play Button - to play index see QCD_COMMAND_PLAYINDEX)
#define QCD_COMMAND_FWD5			40032	// seek forward 5 seconds
#define QCD_COMMAND_BWD5			40033	// seek back 5 seconds

// visual effects control messages
#define QCD_COMMAND_NEXTVISPLUGIN	40022	// switch to next visual plugin
#define QCD_COMMAND_EXTERNALVIS		40036	// open external visual effects window
#define QCD_COMMAND_FULLSCREENVIS	41046	// launch fullscreen visual effects window
#define QCD_COMMAND_INTERNALVIS		41051	// set visuals to internal (within skin)
#define QCD_COMMAND_NOVIS			41052	// turn off visuals

// volume control messages
#define QCD_COMMAND_VOLUP			41026	// Volume goes up 1%
#define QCD_COMMAND_VOLDOWN			41027	// Volume goes down 1%

// playlist control messages
#define QCD_COMMAND_SAVEPLAYLIST	40023	// open 'save playlist as' window
#define QCD_COMMAND_MARKALL			40018	// selects all tracks in playlist
#define QCD_COMMAND_DELETEMARKED	40019	// delete all selected tracks
#define QCD_COMMAND_DELALL			40020	// delete all tracks
#define QCD_COMMAND_CLEARMARKS		40021	// deselect all tracks
#define QCD_COMMAND_DELUNMARKED		42000	// delete all unselected tracks
											// NOTE: to delete individual indeces, see opDeleteIndex in QCDModDefs.h

#define QCD_COMMAND_EJECT			40011	// replace playlist
#define QCD_COMMAND_EJECT_CDROM		12000	// eject cdrom X: usage (QCD_COMMAND_EJECT_CDROM + driveIndex) (eg: drive 'A' = 0, drive 'D' = 4)
#define QCD_COMMAND_EJECT_OPENALL	12060	// eject all cdroms
#define QCD_COMMAND_EJECT_CLOSEALL	12070	// close all cdroms

#define QCD_COMMAND_PLAYINDEX		20000	// play index: usage (QCD_COMMAND_PLAYINDEX + index)
#define QCD_COMMAND_LOADCDTRACKS	10000	// load cd tracks: usage (QCD_COMMAND_LOADCDTRACKS + (drive num * 2) [+ 1 to erase pl])
#define QCD_COMMAND_REQUERYCD		11000	// requery info for cd: usage (QCD_COMMAND_REQUERYCD + drive num)

#define QCD_COMMAND_SORT_NUMBER				40100	// sort by playlist number
#define QCD_COMMAND_SORT_TRACKNAME			40101	// sort by trackname
#define QCD_COMMAND_SORT_FILENAME			40102	// sort by filename
#define QCD_COMMAND_SORT_PATH				40103	// sort by path / filename
#define QCD_COMMAND_SORT_ARTISTTRACKNAME	40106	// sort by artist / track name
#define QCD_COMMAND_SORT_LENGTH				40108	// sort by track length
#define QCD_COMMAND_SORT_TRACKNUMBER		40109	// sort by track number 
#define QCD_COMMAND_SORT_REVERSEPLAYLIST	40104	// reverse playlist
#define QCD_COMMAND_SORT_RENUMBER			40105	// renumber playlist
#define QCD_COMMAND_SORT_RANDOMIZE			40107	// randomize playlist

// timer control messages
#define QCD_COMMAND_TRACKELAPSED	40200	// set timer to elapsed time for current track
#define QCD_COMMAND_TRACKREMAINING	40201	// set timer to remaining time for current track
#define QCD_COMMAND_PLELAPSED		40202	// set timer to elapsed time for playlist
#define QCD_COMMAND_PLREMAINING		40203	// set timer to remaining time for playlist
#define QCD_COMMAND_SHOWHOURS		40204	// toggle whether timer shows time in hours

// EQ control messages
#define QCD_COMMAND_EQ_PRESETS		42002
#define QCD_COMMAND_EQ_NEXTPRESET	42015
#define QCD_COMMAND_EQ_PREVPRESET	42016

