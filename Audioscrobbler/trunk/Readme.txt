Audioscrobbler plug-in for QMP resources:
http://www.audioscrobbler.net/development/protocol/
http://curl.haxx.se/libcurl/c/



Plug-in written and maintained by:
Toke Noer (toke at noer "dot" it)


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