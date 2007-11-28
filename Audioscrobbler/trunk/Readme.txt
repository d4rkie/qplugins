Audioscrobbler plug-in for QMP resources:
http://www.audioscrobbler.net/development/protocol/
http://curl.haxx.se/libcurl/c/



Plug-in written and maintained by:
Toke Noer (toke at noer "dot" it)



//////////////////////////////////////////////////////////////////////////////
// Changelog
//////////////////////////////////////////////////////////////////////////////

// Changelog 1.1.2:
// + Added: Proxy support. Audioscrobbler will use the qmp proxy settings.
// * Change: Logging to file moved to separate thread. (Better performance for qmp main thread)
// * Change: Log path now set to audioscrobbler.dll folder under non multiuser setup.
// * Fix: Audioscrobbler could get stuck not sending info, if handshake failed.
// * Fix: Special characters like ", &, ' etc. wasn't handled properly in cache file.
// * Fix: Crash if cache file lacked version information.
// * Fix: Logging issue when server responds with something unknown to AS.
// * Security fix: Bad handling of unknown server response.
// * Profile guided optimized for speed.

// Changelog 1.0.0:
//   * Fix: Pause times weren't summed
//   * Better handling of failure when plug-in loads

// Changelog 0.1.8:
//   * Win9x support
//   + Added timestamp to log file
//   * Fix: Only first track on CD was scrobbled
//   * Fix: Numerious log fixes
//   * Fix: Player crash if log file can't be opened (f.x with two logs writing to same location)
//   * Security fix: More secure handling of data from audioscrobbler

// Changelog 0.1.7:
//   * Fix: Memory leak
//   * Fix: CD's wasn't scrobbled
//   * Fix: Crash if username had special characters
//   * Fix: Trying to send too often when connection is bad
//   * Fix: Offline mode didn't work, if the player had handshaken already.
//   * Updated CURL Lib (memory leak fix)