// DEPRECATED
//
// users should generate reaper_plugin_functions.h directly from Reaper.
// any comments etc that we want in the header, should go in reascript.cpp/WriteAPIHeader().



/***************************************
*** REAPER Plug-in API
**
** Copyright (C) 2006-2009, Cockos Incorporated
**
**    This software is provided 'as-is', without any express or implied
**    warranty.  In no event will the authors be held liable for any damages
**    arising from the use of this software.
**
**    Permission is granted to anyone to use this software for any purpose,
**    including commercial applications, and to alter it and redistribute it
**    freely, subject to the following restrictions:
**
**    1. The origin of this software must not be misrepresented; you must not
**       claim that you wrote the original software. If you use this software
**       in a product, an acknowledgment in the product documentation would be
**       appreciated but is not required.
**    2. Altered source versions must be plainly marked as such, and must not be
**       misrepresented as being the original software.
**    3. This notice may not be removed or altered from any source distribution.
**
** Notes: the C++ interfaces used require MSVC on win32, or at least the MSVC-compatible C++ ABI. Sorry, mingw users :(
**
**
** The functions in this file can be retreived a few ways, including calling the reaper_plugin_info_t passed on load's GetFunc(""),
** or using our VST API extensions.
**
** Code should not assume that all functions exist, and either gracefully handle missing functions, or fail to load if a missing function is essential.
*/


#ifndef _REAPER_PLUGIN_FUNCTIONS_H_
#define _REAPER_PLUGIN_FUNCTIONS_H_

#include "reaper_plugin.h"


#ifndef REAPER_PLUGIN_DECLARE_APIFUNCS
#define REAPER_PLUGIN_DECLARE_APIFUNCS extern
#endif




// Creates a PCM_source from a ISimpleMediaDecoder
// (if fn is non-null, it will open the file in dec)
REAPER_PLUGIN_DECLARE_APIFUNCS PCM_source *(*PCM_Source_CreateFromSimple)(ISimpleMediaDecoder *dec, const char *fn);


// resampling
REAPER_PLUGIN_DECLARE_APIFUNCS REAPER_Resample_Interface *(*Resampler_Create)();

// to enumerate the resample modes, start with mode=0 and go up
// returns NULL when none left
REAPER_PLUGIN_DECLARE_APIFUNCS const char *(*Resample_EnumModes)(int mode); 


REAPER_PLUGIN_DECLARE_APIFUNCS IReaperPitchShift *(*ReaperGetPitchShiftAPI)(int version); // version must be REAPER_PITCHSHIFT_API_VER
REAPER_PLUGIN_DECLARE_APIFUNCS bool (*EnumPitchShiftModes)(int idx, char **out); // returns FALSE when done, sets out to NULL if a mode is currently unsupported
REAPER_PLUGIN_DECLARE_APIFUNCS const char *(*EnumPitchShiftSubModes)(int idx, int submode);

REAPER_PLUGIN_DECLARE_APIFUNCS REAPER_PeakGet_Interface *(*PeakGet_Create)(const char *fn, int srate, int nch);
REAPER_PLUGIN_DECLARE_APIFUNCS REAPER_PeakBuild_Interface *(*PeakBuild_Create)(PCM_source *src, const char *fn, int srate, int nch);
REAPER_PLUGIN_DECLARE_APIFUNCS void (*GetPeakFileName)(const char *fn, char *buf, int bufmax); // get the peak file name for a given file (can be either filename.reapeaks, or a hashed filename in another path)
REAPER_PLUGIN_DECLARE_APIFUNCS void (*ClearPeakCache)(); // resets the global peak caches

REAPER_PLUGIN_DECLARE_APIFUNCS void* (*GetPeaksBitmap)(PCM_source_peaktransfer_t* pks, double maxamp, int w, int h, LICE_IBitmap* bmp); // see note in reaper_plugin.h about PCM_source_peaktransfer_t::samplerate

// looks at srcBlock data to populate pksBlock.peaks and (if non-NULL) pksBlock.peaks_minvals
REAPER_PLUGIN_DECLARE_APIFUNCS int (*CalculatePeaks)(PCM_source_transfer_t* srcBlock, PCM_source_peaktransfer_t* pksBlock);

REAPER_PLUGIN_DECLARE_APIFUNCS  int (*Audio_RegHardwareHook)(bool isAdd, audio_hook_register_t *reg); // return >0 on success


REAPER_PLUGIN_DECLARE_APIFUNCS int (*Audio_IsRunning)(); // is audio running at all? threadsafe
REAPER_PLUGIN_DECLARE_APIFUNCS int (*Audio_IsPreBuffer)(); // is in pre-buffer? threadsafe
REAPER_PLUGIN_DECLARE_APIFUNCS int (*IsInRealTimeAudio)(); // are we in a realtime audio thread (between OnAudioBuffer calls, not in some worker/anticipative FX thread)? threadsafe

REAPER_PLUGIN_DECLARE_APIFUNCS int (*PlayPreview)(preview_register_t *preview); // return nonzero on success
REAPER_PLUGIN_DECLARE_APIFUNCS int (*StopPreview)(preview_register_t *preview); // return nonzero on success
REAPER_PLUGIN_DECLARE_APIFUNCS int (*PlayTrackPreview2)(void *proj,preview_register_t *preview); // return nonzero on success, in these, m_out_chan is a track index (0-n)
REAPER_PLUGIN_DECLARE_APIFUNCS int (*StopTrackPreview2)(void *proj,preview_register_t *preview); // return nonzero on success

//deprecated
REAPER_PLUGIN_DECLARE_APIFUNCS int (*PlayTrackPreview)(preview_register_t *preview); // return nonzero on success, in these, m_out_chan is a track index (0-n)
REAPER_PLUGIN_DECLARE_APIFUNCS int (*StopTrackPreview)(preview_register_t *preview); // return nonzero on success


REAPER_PLUGIN_DECLARE_APIFUNCS  INT_PTR (*GetColorTheme)(int idx, int defval); // deprecated?
REAPER_PLUGIN_DECLARE_APIFUNCS void *(*GetColorThemeStruct)(int *sz);// returns the whole color theme (icontheme.h) and the size
REAPER_PLUGIN_DECLARE_APIFUNCS void *(*GetIconThemeStruct)(int *sz);// returns the whole icon theme (icontheme.h) and the size
REAPER_PLUGIN_DECLARE_APIFUNCS void *(*GetIconThemePointer)(const char *name); // returns a named icontheme entry
REAPER_PLUGIN_DECLARE_APIFUNCS int (*GSC_mainwnd)(int t); // this is just like win32 GetSysColor() but can have overrides.

REAPER_PLUGIN_DECLARE_APIFUNCS void (*screenset_register)(char *id, screensetCallbackFunc callbackFunc, void *param);
REAPER_PLUGIN_DECLARE_APIFUNCS void (*screenset_unregister)(char *id);
REAPER_PLUGIN_DECLARE_APIFUNCS void (*screenset_unregisterByParam)(void *param);



  // note: these can only reliably create midi access for devices not already opened in prefs/MIDI. 
  // CreateMIDIInput/CreateMIDIOutput are suitable for control surfaces etc.
REAPER_PLUGIN_DECLARE_APIFUNCS midi_Input *(*CreateMIDIInput)(int dev);
REAPER_PLUGIN_DECLARE_APIFUNCS midi_Output *(*CreateMIDIOutput)(int dev, bool streamMode, int *msoffset100); 
  // msoffset used if streamMode is set, point to a persistent variable that can change and reflects added delay to output in 100ths of a millisecond.

  //note: device indices don't directly map to Windows, instead they go through a caching layer.  
REAPER_PLUGIN_DECLARE_APIFUNCS int (*GetMaxMidiInputs)(); // returns max "dev" for midi inputs/outputs
REAPER_PLUGIN_DECLARE_APIFUNCS int (*GetMaxMidiOutputs)();

REAPER_PLUGIN_DECLARE_APIFUNCS int (*GetNumMIDIInputs)(); // returns actual number of midi inputs (slower)
REAPER_PLUGIN_DECLARE_APIFUNCS int (*GetNumMIDIOutputs)(); // returns actual number of midi outputs (slower)

REAPER_PLUGIN_DECLARE_APIFUNCS bool (*GetMIDIOutputName)(int dev, char *nameout, int nameoutlen); // returns true if device present
REAPER_PLUGIN_DECLARE_APIFUNCS bool (*GetMIDIInputName)(int dev, char *nameout, int nameoutlen); // returns true if device present

    
  // if you wish to access MIDI inputs/outputs that are opened via Prefs/MIDI, you may do so, but ONLY if in the audio thread,
  // specifically in a hook installed by Audio_RegHardwareHook, or if in a VST/etc and IsInRealTimeAudio() returns TRUE.
  // The API:
REAPER_PLUGIN_DECLARE_APIFUNCS midi_Input *(*GetMidiInput)(int idx);
REAPER_PLUGIN_DECLARE_APIFUNCS midi_Output *(*GetMidiOutput)(int idx);

/*
    You should call the above GetMidi*put() before you use them, to verify the device is still open.    
    Do NOT call midi_Input::SwapBufs(), but you can call GetReadBuf() to peek in the MIDI input.
    Do NOT call midi_Output::BeginBlock()/EndBlock() in this mode, just Send()/SendMsg().

*/




/*
** More API functions for use:

Generally speaking plug-ins should check to make sure they get non-NULL versions of these -- if a function is not available,
the plug-in should either refuse to load, or handle the missing function gracefully.

*/

// formats tpos (which is time in seconds) as hh:mm:ss.sss
REAPER_PLUGIN_DECLARE_APIFUNCS void (*format_timestr)(double tpos, char *buf, int buflen); 

// time formatting mode overrides: -1=proj default. 
// 0=time
// 1=measures:beats + time
// 2=measures:beats
// 3=seconds
// 4=samples
// 5=h:m:s:f
REAPER_PLUGIN_DECLARE_APIFUNCS void (*format_timestr_pos)(double tpos, char *buf, int buflen, int modeoverride); // modeoverride=-1 for project setting
REAPER_PLUGIN_DECLARE_APIFUNCS double (*parse_timestr_pos)(const char *buf, int modeoverride); // parses time string , modeoverride see above
REAPER_PLUGIN_DECLARE_APIFUNCS void (*format_timestr_len)(double tpos, char *buf, int buflen, double offset, int modeoverride); // offset is start of where the "length" will be calculated from
REAPER_PLUGIN_DECLARE_APIFUNCS double (*parse_timestr_len)(const char *buf, double offset, int modeoverride);

// parse hh:mm:ss.sss time string, return time in seconds (or 0.0 on error)
REAPER_PLUGIN_DECLARE_APIFUNCS double (*parse_timestr)(const char *buf); 


REAPER_PLUGIN_DECLARE_APIFUNCS void *(*GetItemProjectContext)(MediaItem *item);



// TIME MAP (QN<->time<->beats) - note the "beats" definition is somewhat deprecated, best to use QN if possible.

// convert a time into beats. 
// if measures is non-NULL, measures will be set to the measure count, return value will be beats since measure.
// if cml is non-NULL, will be set to current measure length in beats (i.e. time signature numerator)
// if fullbeats is non-NULL, and measures is non-NULL, fullbeats will get the full beat count (same value returned if measures is NULL).
// if cdenom is non-NULL, will be set to the current time signature denominator.
REAPER_PLUGIN_DECLARE_APIFUNCS double (*TimeMap2_timeToBeats)(ReaProject *proj, double tpos, int *measures, int *cml, double *fullbeats, int *cdenom);

REAPER_PLUGIN_DECLARE_APIFUNCS double (*TimeMap2_beatsToTime)(ReaProject *proj, double tpos, int *measures); // convert a beat position (or optionally a beats+measures if measures is non-NULL) to time.

REAPER_PLUGIN_DECLARE_APIFUNCS double (*TimeMap2_QNToTime)(ReaProject *proj, double qn); // converts project QN position to time.
REAPER_PLUGIN_DECLARE_APIFUNCS double (*TimeMap2_timeToQN)(ReaProject *proj, double tpos); // converts project time position to QN position.
REAPER_PLUGIN_DECLARE_APIFUNCS double (*TimeMap2_GetDividedBpmAtTime)(ReaProject *proj, double time); // get the effective BPM at the time (seconds) position (i.e. 2x in /8 signatures)
REAPER_PLUGIN_DECLARE_APIFUNCS double (*TimeMap2_GetNextChangeTime)(ReaProject* proj, double time); // when does the next time map (tempo or time sig) change occur

// deprecated
REAPER_PLUGIN_DECLARE_APIFUNCS double (*TimeMap_QNToTime)(double qn); // converts project QN position to time.
REAPER_PLUGIN_DECLARE_APIFUNCS double (*TimeMap_timeToQN)(double qn); // converts project QN position to time.
REAPER_PLUGIN_DECLARE_APIFUNCS double (*TimeMap_GetDividedBpmAtTime)(double time); // get the effective BPM at the time (seconds) position (i.e. 2x in /8 signatures)

// resolves a filename "in" by using project settings etc.  if no file found out will be a copy of in.
REAPER_PLUGIN_DECLARE_APIFUNCS  void (*resolve_fn)(const char *in, char *out, int outlen); 
// makes filename relative to the current project, if any
REAPER_PLUGIN_DECLARE_APIFUNCS  void (*relative_fn)(const char *in, char *out, int outlen); 

// get reaper.ini full filename
REAPER_PLUGIN_DECLARE_APIFUNCS const char* (*get_ini_file)(); 

// get a pointer to a configuration variable that corresponds to the named value in reaper.ini
// most are int *, but some are double * or char * (string)...


// must not use project-config values for much after calling -- use the projectconfig_* for this.
REAPER_PLUGIN_DECLARE_APIFUNCS void * (*get_config_var)(const char *name, int *szout); 

// only for per-project stuff, allows fast lookup later (only valid for double and int types, not for strings/buffers/etc)
REAPER_PLUGIN_DECLARE_APIFUNCS int (*projectconfig_var_getoffs)(const char *name, int *szout);
REAPER_PLUGIN_DECLARE_APIFUNCS void * (*projectconfig_var_addr)(ReaProject *proj, int idx);

// Create a PCM_source from a filename
REAPER_PLUGIN_DECLARE_APIFUNCS PCM_source *(*PCM_Source_CreateFromFile)(const char *filename);

  // allows you to create a PCM_soucre from filename, and override MIDI files being imported as midi-events.
REAPER_PLUGIN_DECLARE_APIFUNCS PCM_source *(*PCM_Source_CreateFromFileEx)(const char *filename, bool forcenoMidiImp); 

// Create a PCM_source from a "type" (use this if you're going to load its state via LoadState and a ProjectStateContext
// valid types include "WAVE", "MIDI", or whatever plug-ins define as well.
REAPER_PLUGIN_DECLARE_APIFUNCS PCM_source *(*PCM_Source_CreateFromType)(const char *sourcetype);

// Gets basic time signature (beats per minute, numerator of time signature in "bpi")
// this does not reflect tempo envelopes but is purely what is set in the project settings.
REAPER_PLUGIN_DECLARE_APIFUNCS void (*GetProjectTimeSignature2)(void *proj, double *bpm, double *bpi);

// deprecated
REAPER_PLUGIN_DECLARE_APIFUNCS void (*GetProjectTimeSignature)(double *bpm, double *bpi);

// gets the main window, shouldn't really be necessary to use though since the main API func now includes hwnd_main
REAPER_PLUGIN_DECLARE_APIFUNCS HWND (*GetMainHwnd)(); 
 
// inserts a file at the edit cursor.
// mode: 0=add to current track, 1=add new track, 3=add to selected items as takes
// returns >0 on success.
REAPER_PLUGIN_DECLARE_APIFUNCS int (*InsertMedia)(char *file, int mode); 


// Get output channel name. returns NULL if exceeded channel count. if audio isnt open, NULL, too.
REAPER_PLUGIN_DECLARE_APIFUNCS const char *(*GetOutputChannelName)(int idx); 

// Get input channel name. returns NULL if exceeded channel count. if audio isnt open, NULL, too.
REAPER_PLUGIN_DECLARE_APIFUNCS const char *(*GetInputChannelName)(int idx); 


// Gets the project media path (either same as the RPP, or if the RPP has a path set, that path)
REAPER_PLUGIN_DECLARE_APIFUNCS void (*GetProjectPath)(char *buf, int bufsz);
REAPER_PLUGIN_DECLARE_APIFUNCS void (*GetProjectPathEx)(void *proj, char *buf, int bufsz);


// tests an extension (i.e. "wav" or "mid") to see if it's a media extension.
// if wantOthers is set to TRUE, then "RPP", "TXT" and other project-type formats will also pass.
REAPER_PLUGIN_DECLARE_APIFUNCS bool (*IsMediaExtension)(const char *ext, bool wantOthers);

// this returns a double-NULL terminated list of importable media files, suitable for passing to GetOpenFileName() etc
// includes *.* (All Files), too...
REAPER_PLUGIN_DECLARE_APIFUNCS char *(*plugin_getFilterList)();

  // this returns a double-NULL terminated list of importable project files, suitable for passing to GetOpenFileName() etc
  // includes *.* (All Files), too...
REAPER_PLUGIN_DECLARE_APIFUNCS char *(*plugin_getImportableProjectFilterList)();


// Create a MIDI file PCM_sink.
// currently cfg and cfgl are ignored.
// bpm is BPM, div is ticks/QN to use.
//
// Notes: 
//   if div==0, div is reset to the preference setting for tick/qn precision (default 960)
//   if div>=0 and bpm<0, then midi file is tempo mapped to project.
//   if div<0, then it represents a SMPTE framerate (see midi spec)
REAPER_PLUGIN_DECLARE_APIFUNCS PCM_sink *(*PCM_Sink_CreateMIDIFileEx)(void *proj,const char *filename, const char *cfg, int cfgl, double bpm, int div); 
REAPER_PLUGIN_DECLARE_APIFUNCS PCM_sink *(*PCM_Sink_CreateMIDIFile)(const char *filename, const char *cfg, int cfgl, double bpm, int div); 


// Enumerate available PCM sinks. returns 0 when finished (start with id=0, etc). descstr gets set to description of type.
REAPER_PLUGIN_DECLARE_APIFUNCS unsigned int (*PCM_Sink_Enum)(int id, char **descstr);

// returns "wav" or "mid" etc
REAPER_PLUGIN_DECLARE_APIFUNCS const char *(*PCM_Sink_GetExtension)(const void *data, int data_size); 

// show configuration for this type. 
REAPER_PLUGIN_DECLARE_APIFUNCS HWND (*PCM_Sink_ShowConfig)(const void *cfg, int cfg_l, HWND hwndParent);

/* to get the configuration from a child HWND, use the following code:
  int sz=0;
  SendMessage(hwnd,WM_USER+1024,(WPARAM)&sz,0);
  if (sz>0)
  {
    void *buf=malloc(sz);
    SendMessage(hwnd,WM_USER+1024,0,(LPARAM)buf);
    // buf/sz is now the cfg, cfg_l
  }
*/

//
// Create a PCM sink (configured with cfg/cfgl). 
// In general, the first 4 bytes of "cfg" represent the sink type (i.e. (REAPER_FOURCC('W','A','V','E'), 
// any additional data is type-specific configuration.
// if buildpeaks is TRUE then the appropriate .reapeaks file is created on the fly.
// Currently nch must be 1 or 2 for most sinks.

REAPER_PLUGIN_DECLARE_APIFUNCS PCM_sink *(*PCM_Sink_Create)(const char *filename, const char *cfg, int cfgl, int nch, int srate, bool buildpeaks);
REAPER_PLUGIN_DECLARE_APIFUNCS PCM_sink *(*PCM_Sink_CreateEx)(void *proj, const char *filename, const char *cfg, int cfgl, int nch, int srate, bool buildpeaks);


// update the entire edit area of the main window
REAPER_PLUGIN_DECLARE_APIFUNCS void (*UpdateTimeline)();
REAPER_PLUGIN_DECLARE_APIFUNCS void (*UpdateItemInProject)(MediaItem* item);

// gets number of tracks in project
REAPER_PLUGIN_DECLARE_APIFUNCS int (*GetNumTracks)();

// gets track info (returns name).
// track index, -1=master, 0..n, or cast a MediaTrack * to int
// if flags is non-NULL, will be set to:
// &1=folder
// &2=selected
// &4=has fx enabled
// &8=muted
// &16=soloed
// &32=SIP'd (with &16)
// &64=rec armed
REAPER_PLUGIN_DECLARE_APIFUNCS const char *(*GetTrackInfo)(INT_PTR track, int *flags);  
REAPER_PLUGIN_DECLARE_APIFUNCS GUID *(*GetTrackGUID)(MediaTrack *tr);
// see also GetSetMediaTrackInfo (below)

// process a MIDI event/list through the standard keyboard assignable key system. For the most part dev_index should be -1
REAPER_PLUGIN_DECLARE_APIFUNCS void (*kbd_OnMidiEvent)(MIDI_event_t *evt, int dev_index); // can be called from anywhere (threadsafe)
REAPER_PLUGIN_DECLARE_APIFUNCS void (*kbd_OnMidiList)(MIDI_eventlist *list, int dev_index); // can be called from anywhere (threadsafe)

// enumerate actions for a particular section
// returns 0 on not found, otherwise command ID
REAPER_PLUGIN_DECLARE_APIFUNCS int (*kbd_enumerateActions)(KbdSectionInfo *section, int idx, const char **nameOut); 

// call this on executing any action, will return TRUE if it handled it.
// this is so that if your window gets a WM_COMMAND, you can call this before your handlers to see if it's been overridden or a custom action.
// val, valhw, and relmode should point to integers representing the MIDI parameter. If *valhw >= 0 then the midi parameter 
// is ((val<<7)|valhw) (which means 0..16383) and absolute. Otherwise, if *valhw is -1, val is 0..127, and relmode can be 0 (Absolute), or 1-3 for the
// different MIDI CC relative modes. Note that the hook function MAY return false but have modified these parameters.
REAPER_PLUGIN_DECLARE_APIFUNCS bool (*kbd_RunCommandThroughHooks)(KbdSectionInfo *section, int *actionCommandID,int *val, int *valhw, int *relmode, HWND hwnd); //actioncommandID may get modified

// processes a midi event through the actionlist directly (optioanlly for only a single section) (only call from the main thread!). 
// if hwndCtx is set it is used as the destination for the message
REAPER_PLUGIN_DECLARE_APIFUNCS bool (*kbd_processMidiEventActionEx)(MIDI_event_t *evt, KbdSectionInfo *section, HWND hwndCtx);

// process hmenu as a top-level actions menu (with an ID_CUSTOMACTION_LAST_ACTION=2999, and any custom actions added after the last separator)
REAPER_PLUGIN_DECLARE_APIFUNCS void (*kbd_ProcessActionsMenu)(HMENU menu, KbdSectionInfo *section); 


// reprocess a menu, setting key assignments to what their command IDs are mapped to.
// processes the menu recurisvely.
REAPER_PLUGIN_DECLARE_APIFUNCS void  (*kbd_reprocessMenu)(HMENU menu, KbdSectionInfo *section);

// pass in the HWND to receive commands, a MSG of a key command,  and a valid section, and 
// kbd_translateAccelerator() will process it looking for any keys bound to it, and send the messages off.
// returns 1 if processed, 0 if no key binding found.
REAPER_PLUGIN_DECLARE_APIFUNCS int (*kbd_translateAccelerator)(HWND hwnd, MSG *msg, KbdSectionInfo *section);

// formats the name of the key defined by ACCEL (ignoring the cmd value) into a string. 
REAPER_PLUGIN_DECLARE_APIFUNCS void (*kbd_formatKeyName)(ACCEL *ac, char *s);

// get the string of a key assigned to command "cmd" in a section.
// this function is poorly named (as it doesn't return the command's name -- kbd_getTextFromCmd() does that)
REAPER_PLUGIN_DECLARE_APIFUNCS void (*kbd_getCommandName)(int cmd, char *s, KbdSectionInfo *section);

// gets the name of a command "cmd" in a section. 
REAPER_PLUGIN_DECLARE_APIFUNCS const char *(*kbd_getTextFromCmd)(DWORD cmd, KbdSectionInfo *section);

// convert to/from a 0...1000 scale to dB. Faders in REAPER are 0..1000 and this converts according to the curves in the preferences.
REAPER_PLUGIN_DECLARE_APIFUNCS double (*DB2SLIDER)(double x);
REAPER_PLUGIN_DECLARE_APIFUNCS double (*SLIDER2DB)(double y);

// make vol/pan strings. make sure str points to at least 128 bytes of buffer.
REAPER_PLUGIN_DECLARE_APIFUNCS void (*mkvolpanstr)(char *str, double vol, double pan);
REAPER_PLUGIN_DECLARE_APIFUNCS void (*mkvolstr)(char *str, double vol);
REAPER_PLUGIN_DECLARE_APIFUNCS void (*mkpanstr)(char *str, double pan);

// parse a pan string
REAPER_PLUGIN_DECLARE_APIFUNCS double (*parsepanstr)(char *str);

// track index 0 always = master track
// note these indices in mcpView can be tracks in any order (mixer view), 
// if !mcpView, 1->n = tracks (note the 1-based which differs from "GetTrackInfo"'s indices which are 0 based
REAPER_PLUGIN_DECLARE_APIFUNCS int (*CSurf_TrackToID)(MediaTrack *track, bool mcpView); 
REAPER_PLUGIN_DECLARE_APIFUNCS MediaTrack *(*CSurf_TrackFromID)(int idx, bool mcpView);
REAPER_PLUGIN_DECLARE_APIFUNCS int (*CSurf_NumTracks)(bool mcpView);
 
    // these will be called from app when something changes
REAPER_PLUGIN_DECLARE_APIFUNCS void (*CSurf_SetTrackListChange)();
REAPER_PLUGIN_DECLARE_APIFUNCS void (*CSurf_SetSurfaceVolume)(MediaTrack *trackid, double volume, IReaperControlSurface *ignoresurf);
REAPER_PLUGIN_DECLARE_APIFUNCS void (*CSurf_SetSurfacePan)(MediaTrack *trackid, double pan, IReaperControlSurface *ignoresurf);
REAPER_PLUGIN_DECLARE_APIFUNCS void (*CSurf_SetSurfaceMute)(MediaTrack *trackid, bool mute, IReaperControlSurface *ignoresurf);
REAPER_PLUGIN_DECLARE_APIFUNCS void (*CSurf_SetSurfaceSelected)(MediaTrack *trackid, bool selected, IReaperControlSurface *ignoresurf);
REAPER_PLUGIN_DECLARE_APIFUNCS void (*CSurf_SetSurfaceSolo)(MediaTrack *trackid, bool solo, IReaperControlSurface *ignoresurf);
REAPER_PLUGIN_DECLARE_APIFUNCS void (*CSurf_SetSurfaceRecArm)(MediaTrack *trackid, bool recarm, IReaperControlSurface *ignoresurf);
REAPER_PLUGIN_DECLARE_APIFUNCS bool (*CSurf_GetTouchState)(MediaTrack *trackid, int isPan);
REAPER_PLUGIN_DECLARE_APIFUNCS void (*CSurf_SetAutoMode)(int mode, IReaperControlSurface *ignoresurf);

REAPER_PLUGIN_DECLARE_APIFUNCS void (*CSurf_SetPlayState)(bool play, bool pause, bool rec, IReaperControlSurface *ignoresurf);
REAPER_PLUGIN_DECLARE_APIFUNCS void (*CSurf_SetRepeatState)(bool rep, IReaperControlSurface *ignoresurf);

// these are called by our surfaces, and actually update the project
REAPER_PLUGIN_DECLARE_APIFUNCS double (*CSurf_OnVolumeChange)(MediaTrack *trackid, double volume, bool relative);
REAPER_PLUGIN_DECLARE_APIFUNCS double (*CSurf_OnPanChange)(MediaTrack *trackid, double pan, bool relative);

// these are like the non-extended versions but allow you to disable selected-track ganging
REAPER_PLUGIN_DECLARE_APIFUNCS double (*CSurf_OnVolumeChangeEx)(MediaTrack *trackid, double volume, bool relative, bool allowGang);
REAPER_PLUGIN_DECLARE_APIFUNCS double (*CSurf_OnPanChangeEx)(MediaTrack *trackid, double pan, bool relative, bool allowGang);

REAPER_PLUGIN_DECLARE_APIFUNCS bool (*CSurf_OnMuteChange)(MediaTrack *trackid, int mute);
REAPER_PLUGIN_DECLARE_APIFUNCS bool (*CSurf_OnSelectedChange)(MediaTrack *trackid, int selected);
REAPER_PLUGIN_DECLARE_APIFUNCS bool (*CSurf_OnSoloChange)(MediaTrack *trackid, int solo);
REAPER_PLUGIN_DECLARE_APIFUNCS bool (*CSurf_OnFXChange)(MediaTrack *trackid, int en);
REAPER_PLUGIN_DECLARE_APIFUNCS bool (*CSurf_OnRecArmChange)(MediaTrack *trackid, int recarm);
REAPER_PLUGIN_DECLARE_APIFUNCS void (*CSurf_OnPlay)();
REAPER_PLUGIN_DECLARE_APIFUNCS void (*CSurf_OnStop)();
REAPER_PLUGIN_DECLARE_APIFUNCS void (*CSurf_OnFwd)(int seekplay);
REAPER_PLUGIN_DECLARE_APIFUNCS void (*CSurf_OnRew)(int seekplay);
REAPER_PLUGIN_DECLARE_APIFUNCS void (*CSurf_OnRecord)();
REAPER_PLUGIN_DECLARE_APIFUNCS void (*CSurf_GoStart)();
REAPER_PLUGIN_DECLARE_APIFUNCS void (*CSurf_GoEnd)();
REAPER_PLUGIN_DECLARE_APIFUNCS void (*CSurf_OnArrow)(int whichdir, bool wantzoom);
REAPER_PLUGIN_DECLARE_APIFUNCS void (*CSurf_OnTrackSelection)(MediaTrack *trackid);
REAPER_PLUGIN_DECLARE_APIFUNCS void (*CSurf_ResetAllCachedVolPanStates)();

REAPER_PLUGIN_DECLARE_APIFUNCS void (*CSurf_FlushUndo)(bool force); // call this to force flushing of the undo states after using CSurf_On*Change()

REAPER_PLUGIN_DECLARE_APIFUNCS int (*GetMasterMuteSoloFlags)(); // &1=master mute, &2=master solo. This is deprecated as you can just query the master track as well.
REAPER_PLUGIN_DECLARE_APIFUNCS void (*TrackList_UpdateAllExternalSurfaces)();

REAPER_PLUGIN_DECLARE_APIFUNCS void (*ClearAllRecArmed)();
REAPER_PLUGIN_DECLARE_APIFUNCS void (*SetTrackAutomationMode)(MediaTrack *tr, int mode);
REAPER_PLUGIN_DECLARE_APIFUNCS int (*GetTrackAutomationMode)(MediaTrack *tr);
REAPER_PLUGIN_DECLARE_APIFUNCS void (*TrackList_AdjustWindows)(bool isMajor);
REAPER_PLUGIN_DECLARE_APIFUNCS void (*SoloAllTracks)(int solo); // solo=2 for SIP
REAPER_PLUGIN_DECLARE_APIFUNCS void (*MuteAllTracks)(bool mute);
REAPER_PLUGIN_DECLARE_APIFUNCS void (*BypassFxAllTracks)(int bypass); // -1 = bypass all if not all bypassed, otherwise unbypass all
REAPER_PLUGIN_DECLARE_APIFUNCS void (*SetTrackSelected)(MediaTrack *tr, bool sel);

REAPER_PLUGIN_DECLARE_APIFUNCS int (*GetPlayState)(); // &1=playing, &2=pause, &=4 is recording
REAPER_PLUGIN_DECLARE_APIFUNCS double (*GetPlayPosition)(); // returns latency-compensated actual-what-you-hear position
REAPER_PLUGIN_DECLARE_APIFUNCS double (*GetPlayPosition2)(); // returns position of next audio block being processed
REAPER_PLUGIN_DECLARE_APIFUNCS double (*GetCursorPosition)(); // edit cursor position
REAPER_PLUGIN_DECLARE_APIFUNCS int (*GetPlayStateEx)(void *proj); // &1=playing, &2=pause, &=4 is recording
REAPER_PLUGIN_DECLARE_APIFUNCS double (*GetPlayPositionEx)(void *proj); // returns latency-compensated actual-what-you-hear position
REAPER_PLUGIN_DECLARE_APIFUNCS double (*GetPlayPosition2Ex)(void *proj); // returns position of next audio block being processed
REAPER_PLUGIN_DECLARE_APIFUNCS double (*GetCursorPositionEx)(void *proj); // edit cursor position

REAPER_PLUGIN_DECLARE_APIFUNCS void (*OnPlayButton)(); // direct way to simulate play button hit
REAPER_PLUGIN_DECLARE_APIFUNCS void (*OnPauseButton)(); // direct way to simulate pause button hit
REAPER_PLUGIN_DECLARE_APIFUNCS void (*OnStopButton)(); // direct way to simulate stop button hit
REAPER_PLUGIN_DECLARE_APIFUNCS void (*OnPlayButtonEx)(void *proj); // direct way to simulate play button hit
REAPER_PLUGIN_DECLARE_APIFUNCS void (*OnPauseButtonEx)(void *proj); // direct way to simulate pause button hit
REAPER_PLUGIN_DECLARE_APIFUNCS void (*OnStopButtonEx)(void *proj); // direct way to simulate stop button hit


REAPER_PLUGIN_DECLARE_APIFUNCS void (*SetAutomationMode)(int mode, bool onlySel); // sets all or selected tracks to mode. 
REAPER_PLUGIN_DECLARE_APIFUNCS void (*Main_UpdateLoopInfo)(int ignoremask);
REAPER_PLUGIN_DECLARE_APIFUNCS double (*Track_GetPeakInfo)(MediaTrack *tr, int chidx);

REAPER_PLUGIN_DECLARE_APIFUNCS bool (*GetTrackUIVolPan)(MediaTrack *tr, double *vol, double *pan);

REAPER_PLUGIN_DECLARE_APIFUNCS void (*CSurf_ScrubAmt)(double amt);
REAPER_PLUGIN_DECLARE_APIFUNCS void (*MoveEditCursor)(double adjamt, bool dosel);
REAPER_PLUGIN_DECLARE_APIFUNCS void (*adjustZoom)(double amt, int forceset, bool doupd, int centermode); // forceset=0, doupd=true, centermode=-1 for default
REAPER_PLUGIN_DECLARE_APIFUNCS double (*GetHZoomLevel)(); // returns pixels/second

REAPER_PLUGIN_DECLARE_APIFUNCS int (*TrackFX_GetCount)(MediaTrack *tr);
REAPER_PLUGIN_DECLARE_APIFUNCS int (*TrackFX_GetNumParams)(MediaTrack *tr, int fx);
REAPER_PLUGIN_DECLARE_APIFUNCS bool (*TrackFX_GetFXName)(MediaTrack *tr, int fx, char *buf, int buflen);
REAPER_PLUGIN_DECLARE_APIFUNCS double (*TrackFX_GetParam)(MediaTrack *tr, int fx, int param, double *minval, double *maxval);
REAPER_PLUGIN_DECLARE_APIFUNCS bool (*TrackFX_SetParam)(MediaTrack *tr, int fx, int param, double val);
REAPER_PLUGIN_DECLARE_APIFUNCS bool (*TrackFX_GetParamName)(MediaTrack *tr, int fx, int param, char *buf, int buflen);
REAPER_PLUGIN_DECLARE_APIFUNCS bool (*TrackFX_FormatParamValue)(MediaTrack *tr, int fx, int param, double val, char *buf, int buflen);

REAPER_PLUGIN_DECLARE_APIFUNCS void (*SetEditCurPos2)(void *proj, double time, bool moveview, bool seekplay);
REAPER_PLUGIN_DECLARE_APIFUNCS int (*GetSetRepeatEx)(void *proj, int val); // -1 == query, 0=clear, 1=set, >1=toggle . returns new value
REAPER_PLUGIN_DECLARE_APIFUNCS const char *(*GetExePath)(); // returns path of REAPER.exe (not including EXE), i.e. "C:\\Program Files\\REAPER"

//deprecated
REAPER_PLUGIN_DECLARE_APIFUNCS int (*GetSetRepeat)(int val); // -1 == query, 0=clear, 1=set, >1=toggle . returns new value
REAPER_PLUGIN_DECLARE_APIFUNCS void (*SetEditCurPos)(double time, bool moveview, bool seekplay);

 // use this like:
 // int x=0;
 // bool isrgn;
 // double pos, rgnend;
 // char *name;
 // int number;
 // while ((x=EnumProjectMarkers(x,&isrgn,&pos,&rgnend,&name,&number))
 // {
 //    look_at_values; rgnend only valid if isrgn=true.
 // }
REAPER_PLUGIN_DECLARE_APIFUNCS int (*EnumProjectMarkers)(int idx, bool *isrgn, double *pos, double *rgnend, char **name, int *markrgnindexnumber);

// rgnend ignored if !isRgn
REAPER_PLUGIN_DECLARE_APIFUNCS bool (*SetProjectMarker)(int markrgnindexnumber, bool isrgn, double pos, double rgnend, const char* name);

REAPER_PLUGIN_DECLARE_APIFUNCS int (*EnumProjectMarkers2)(void *proj,int idx, bool *isrgn, double *pos, double *rgnend, char **name, int *markrgnindexnumber);

// rgnend ignored if !isRgn
REAPER_PLUGIN_DECLARE_APIFUNCS bool (*SetProjectMarker2)(void *proj,int markrgnindexnumber, bool isrgn, double pos, double rgnend, const char* name);

  // whichstates can be any combination of UNDO_STATE_*

REAPER_PLUGIN_DECLARE_APIFUNCS void (*Undo_OnStateChange2)(void *proj, const char *descchange); // limited state change to items
REAPER_PLUGIN_DECLARE_APIFUNCS void (*Undo_OnStateChangeEx2)(void *proj, const char *descchange, int whichStates, int trackparm); // trackparm=-1 by default, or if updating one fx chain, you can specify track index

REAPER_PLUGIN_DECLARE_APIFUNCS int (*Undo_DoRedo2)(void *proj); // nonzero if success
REAPER_PLUGIN_DECLARE_APIFUNCS int (*Undo_DoUndo2)(void *proj); // nonzero if success
REAPER_PLUGIN_DECLARE_APIFUNCS const char *(*Undo_CanUndo2)(void *proj); // returns string of last action, if able, NULL if not
REAPER_PLUGIN_DECLARE_APIFUNCS const char *(*Undo_CanRedo2)(void *proj); // returns string of next action, if able, NULL if not
REAPER_PLUGIN_DECLARE_APIFUNCS void (*Undo_BeginBlock2)(void *proj); // call to start a new "block"
REAPER_PLUGIN_DECLARE_APIFUNCS void (*Undo_EndBlock2)(void *proj,const char *descchange, int extraflags); // call to end the block, with extra flags if any, and a description

// deprecated
REAPER_PLUGIN_DECLARE_APIFUNCS void (*Undo_OnStateChange)(const char *descchange); // limited state change to items
REAPER_PLUGIN_DECLARE_APIFUNCS void (*Undo_OnStateChangeEx)(const char *descchange, int whichStates, int trackparm); // trackparm=-1 by default, or if updating one fx chain, you can specify track index
REAPER_PLUGIN_DECLARE_APIFUNCS void (*Undo_BeginBlock)(); // call to start a new "block"
REAPER_PLUGIN_DECLARE_APIFUNCS void (*Undo_EndBlock)(const char *descchange, int extraflags); // call to end the block, with extra flags if any, and a description


REAPER_PLUGIN_DECLARE_APIFUNCS void (*GetSet_LoopTimeRange2)(void *proj,bool isSet, bool isLoop, double *start, double *end, bool allowautoseek);
REAPER_PLUGIN_DECLARE_APIFUNCS void (*GetSet_LoopTimeRange)(bool isSet, bool isLoop, double *start, double *end, bool allowautoseek);
REAPER_PLUGIN_DECLARE_APIFUNCS void (*GetSet_ArrangeView2)(void *proj,bool isSet, int screen_x_start, int screen_x_end, double* start_time, double* end_time);

REAPER_PLUGIN_DECLARE_APIFUNCS void *(*plugin_getapi)(const char *name);


// additional notes on plugin_register / rec->Register
// register("API_blafunc",funcaddress) then other plug-ins can use GetFunc("blafunc"),
// (use -API_* to unregister)

// another thing you can register is "hookcommand", which you pass a function:
      // NON_API: bool runCommand(int command, int flag);
      //              register("hookcommand",runCommand);
      // which returns TRUE to eat (process) the command. flag is usually 0 but can sometimes have useful info depending on the message
      // note: it's OK to call Main_OnCommand() within your runCommand, however you MUST check for recursion if doing so!
      // in fact, any use of this hook should benefit from a simple reentrancy test...

// you can also register command IDs for main window actions, 
// register with "command_id", parameter is a unique string with only A-Z, 0-9, 
// returns command ID for main actions (or 0 if not supported/out of actions)
REAPER_PLUGIN_DECLARE_APIFUNCS int (*plugin_register)(const char *name, void *infostruct); // like rec->Register


REAPER_PLUGIN_DECLARE_APIFUNCS void (*Main_OnCommand)(int command, int flag);
REAPER_PLUGIN_DECLARE_APIFUNCS void (*Main_OnCommandEx)(int command, int flag, void *proj);

  // opens a project. will prompt the user to save, etc.
  // if you pass a .RTrackTemplate file then it adds that to the project instead.
REAPER_PLUGIN_DECLARE_APIFUNCS void (*Main_openProject)(char *name);


  // gets peak info from a source (for hi-res peak generation)
REAPER_PLUGIN_DECLARE_APIFUNCS void (*HiresPeaksFromSource)(PCM_source *src, PCM_source_peaktransfer_t *block);

// gets context menus. submenu 0:trackctl, 1:mediaitems, 2:ruler, 3:empty track area
REAPER_PLUGIN_DECLARE_APIFUNCS HMENU (*GetContextMenu)(int idx); 


REAPER_PLUGIN_DECLARE_APIFUNCS void (*genGuid)(GUID *g);
REAPER_PLUGIN_DECLARE_APIFUNCS void (*guidToString)(GUID *g, char *dest); // dest should be at least 64 chars long to be safe
REAPER_PLUGIN_DECLARE_APIFUNCS void (*stringToGuid)(const char *str, GUID *g);


  // category is <0 for receives, 0=sends, >0 for hardware outputs
  // sendidx is 0..n (NULL on any required parameter to stop)
  //
  // parameter names:
  // "P_DESTTRACK" (read only, returns MediaTrack *, destination track, only applies for sends/recvs)
  // "P_SRCTRACK" (read only, returns MediaTrack *, source track, only applies for sends/recvs)
  // "B_MUTE" (returns bool *, read/write)
  // "B_PHASE" (returns bool *, read/write - true to flip phase)
  // "B_MONO" (returns bool *, read/write)
  // "D_VOL" (returns double *, read/write - 1.0 = +0dB etc)
  // "D_PAN" (returns double *, read/write - -1..+1)
  // "D_PANLAW" (returns double *,read/write - 1.0=+0.0db, 0.5=-6dB, -1.0 = projdef etc)
  // "I_SENDMODE" (returns int *, read/write - 0=post-fader, 1=pre-fx, 2=post-fx(depr), 3=post-fx)
  // "I_SRCCHAN" (returns int *, read/write - index,&1024=mono, -1 for none)
  // "I_DSTCHAN" (returns int *, read/write - index, &1024=mono, otherwise stereo pair, hwout:&512=rearoute)
  // "I_MIDIFLAGS" (returns int *, read/write - low 5 bits=source channel 0=all, 1-16, next 5 bits=dest channel, 0=orig, 1-16=chan)
  // set setNewValue to non-NULL to set the value
REAPER_PLUGIN_DECLARE_APIFUNCS void *(*GetSetTrackSendInfo)(MediaTrack *tr, int category, int sendidx, const char *parmname, void *setNewValue);


// item access functions. note that you will need to call Undo_OnStateChange(), UpdateTimeline() etc..
  
 // returns TRUE if item deleted. 
REAPER_PLUGIN_DECLARE_APIFUNCS bool (*DeleteTrackMediaItem)(MediaTrack *tr, MediaItem *it);

// gets number of media items on this track
REAPER_PLUGIN_DECLARE_APIFUNCS int (*GetTrackNumMediaItems)(MediaTrack *tr);

// gets a media item from the track. note that they are in order by the start position
// and if you change a position of an item (below), the index could change.
// also the index could change across undo/redos, even if the order doesn't.
REAPER_PLUGIN_DECLARE_APIFUNCS MediaItem *(*GetTrackMediaItem)(MediaTrack *tr, int itemidx);



// get (or set) different media item parameters:
// "P_TRACK" : MediaTrack * (read only)
// "B_MUTE" : bool * to "muted" state
// "B_LOOPSRC" : bool * to "loop source"
// "B_ALLTAKESPLAY" : bool * to "all takes play"
// "B_UISEL" : bool * to "ui selected"
// "C_BEATATTACHMODE" : char * to one char of "beat attached mode", -1=def, 0=time, 1=allbeats, 2=beatsposonly
// "C_LOCK" : char * to one char of "lock flags" (&1 is locked, currently)
// "C_FADEINSHAPE" : char * to fadein shape, 0=linear, ...
// "C_FADEOUTSHAPE" : char * to fadeout shape
// "D_VOL" : double * of item volume (volume bar)
// "D_POSITION" : double * of item position (seconds)
// "D_LENGTH" : double * of item length (seconds)
// "D_SNAPOFFSET" : double * of item snap offset (seconds)
// "D_FADEINLEN" : double * of item fade in length (manual, seconds)
// "D_FADEOUTLEN" : double * of item fade out length (manual, seconds)
// "D_FADEINLEN_AUTO" : double * of item autofade in length (seconds, -1 for "no autofade set")
// "D_FADEOUTLEN_AUTO" : double * of item autofade out length (seconds, -1 for "no autofade set")
// "I_GROUPID" : int * to group ID (0 = no group)
// "I_LASTY" : int * to last y position in track (readonly) 
// "I_LASTH" : int * to last height in track (readonly)
// "I_CUSTOMCOLOR" : int * : custom color, windows standard color order (i.e. RGB(r,g,b)|0x100000). if you do not |0x100000, then it will not be used (though will store the color anyway)
// "I_CURTAKE" : int * to active take
// "F_FREEMODE_Y" : float * to free mode y position (0..1)
// "F_FREEMODE_H" : float * to free mode height (0..1)
REAPER_PLUGIN_DECLARE_APIFUNCS void *(*GetSetMediaItemInfo)(MediaItem *item, const char *parmname, void *setNewValue);
REAPER_PLUGIN_DECLARE_APIFUNCS double (*GetMediaItemInfo_Value)(MediaItem* item, const char* parmname);
REAPER_PLUGIN_DECLARE_APIFUNCS bool (*SetMediaItemInfo_Value)(MediaItem* item, const char* parmname, double newvalue);
REAPER_PLUGIN_DECLARE_APIFUNCS MediaTrack* (*GetMediaItem_Track)(MediaItem* item);



// get number of takes in this item
REAPER_PLUGIN_DECLARE_APIFUNCS int (*GetMediaItemNumTakes)(MediaItem *item);

// get a take from an item, or -1 for the "active" take
REAPER_PLUGIN_DECLARE_APIFUNCS MediaItem_Take *(*GetMediaItemTake)(MediaItem *item, int tk);

// get (or set) different take parameters
// "P_TRACK" : pointer to MediaTrack (read-only)
// "P_ITEM" : pointer to MediaItem (read-only)
// "P_NAME" : char * to take name
// "P_SOURCE" : PCM_source *. Note that if setting this, you should first retrieve the old source, set the new, THEN delete the old.
// "D_STARTOFFS" : double *, start offset in take of item
// "D_VOL" : double *, take volume
// "D_PAN" : double *, take pan
// "D_PANLAW" : double *, take pan law (-1.0=default, 0.5=-6dB, 1.0=+0dB, etc)
// "D_PLAYRATE" : double *, take play rate (1.0=normal, 2.0=doublespeed, etc)
// "D_PITCH" : double *, take pitch adjust (in semitones, 0.0=normal, +12 = one octave up, etc)
// "B_PPITCH", bool *, "preserve pitch when changing rate"
// "I_CHANMODE", int *, channel mode (0=normal, 1=revstereo, 2=downmix, 3=l, 4=r)
// "I_PITCHMODE", int *, pitch shifter mode, -1=proj default, otherwise high word=shifter low word = parameter
// "GUID" : GUID * : 16-byte GUID, can query or update
REAPER_PLUGIN_DECLARE_APIFUNCS void *(*GetSetMediaItemTakeInfo)(MediaItem_Take *tk, const char *parmname, void *setNewValue);
REAPER_PLUGIN_DECLARE_APIFUNCS double (*GetMediaItemTakeInfo_Value)(MediaItem_Take *tk, const char *parmname);
REAPER_PLUGIN_DECLARE_APIFUNCS bool (*SetMediaItemTakeInfo_Value)(MediaItem_Take *tk, const char *parmname, double newvalue);
REAPER_PLUGIN_DECLARE_APIFUNCS bool (*GetSetMediaItemTakeInfo_String)(MediaItem_Take* tk, const char* parmname, char* string, bool setnetvalue);
REAPER_PLUGIN_DECLARE_APIFUNCS MediaTrack* (*GetMediaItemTake_Track)(MediaItem_Take* tk);
REAPER_PLUGIN_DECLARE_APIFUNCS MediaItem* (*GetMediaItemTake_Item)(MediaItem_Take* tk);
REAPER_PLUGIN_DECLARE_APIFUNCS PCM_source* (*GetMediaItemTake_Source)(MediaItem_Take* tk);

REAPER_PLUGIN_DECLARE_APIFUNCS MediaItem *(*AddMediaItemToTrack)(MediaTrack *tr); // creates a new media item.
REAPER_PLUGIN_DECLARE_APIFUNCS MediaItem_Take *(*AddTakeToMediaItem)(MediaItem *item); // creates a new take in an item
REAPER_PLUGIN_DECLARE_APIFUNCS bool (*MoveMediaItemToTrack)(MediaItem *item, MediaTrack *desttr); // returns TRUE if move succeeded


// get (or set) track parameters.
// "P_PARTRACK" : MediaTrack * : parent track (read-only)
// "P_NAME" : char * : track name (on master returns NULL)
// "B_MUTE" : bool * : mute flag
// "B_PHASE" : bool * : invert track phase
// "IP_TRACKNUMBER : int : track number (returns zero if not found, -1 for master track) (read-only, returns the int directly)
// "I_SOLO" : int * : 0=not soloed, 1=solo, 2=soloed in place
// "I_FXEN" : int * : 0=fx bypassed, nonzero = fx active
// "I_RECARM" : int * : 0=not record armed, 1=record armed
// "I_RECINPUT" : int * : record input. 0..n = mono hardware input, 512+n = rearoute input, 1024 set for stereo input pair. 4096 set for MIDI input, if set, then low 5 bits represent channel (0=all, 1-16=only chan), then next 5 bits represent physical input (31=all, 30=VKB)
// "I_RECMODE" : int * : record mode (0=input, 1=stereo out, 2=none, 3=stereo out w/latcomp, 4=midi output, 5=mono out, 6=mono out w/ lat comp, 7=midi overdub, 8=midi replace
// "I_RECMON" : int * : record monitor (0=off, 1=normal, 2=not when playing (tapestyle))
// "I_RECMONITEMS" : int * : monitor items while recording (0=off, 1=on)
// "I_AUTOMODE" : int * : track automation mode (0=trim/off, 1=read, 2=touch, 3=write, 4=latch
// "I_NCHAN" : int * : number of track channels, must be 2-64, even
// "I_SELECTED" : int * : track selected? 0 or 1
// "I_WNDH" : int * : current TCP window height (Read-only)
// "I_ISFOLDER" : int * : folder status (1=folder, 0=normal, 2=last track in folder)
// "I_FOLDERCOMPACT" : int * : folder compacting (only valid on folders), 0=normal, 1=small, 2=tiny children
// "I_MIDIHWOUT" : int * : track midi hardware output index (<0 for disabled, low 5 bits are which channels (0=all, 1-16), next 5 bits are output device index (0-31))
// "I_PERFFLAGS" : int * : track perf flags (&1=no media buffering, &2=no anticipative FX)
// "I_CUSTOMCOLOR" : int * : custom color, windows standard color order (i.e. RGB(r,g,b)|0x100000). if you do not |0x100000, then it will not be used (though will store the color anyway)
// "I_HEIGHTOVERRIDE" : int * : custom height override for TCP window. 0 for none, otherwise size in pixels
// "D_VOL" : double * : trim volume of track (0 (-inf)..1 (+0dB) .. 2 (+6dB) etc ..)
// "D_PAN" : double * : trim pan of track (-1..1)
// "D_PANLAW" : double * : pan law of track. <0 for "project default", 1.0 for +0dB, etc
// "B_SHOWINMIXER" : bool * : show track panel in mixer -- do not use on master
// "B_SHOWINTCP" : bool * : show track panel in tcp -- do not use on master
// "B_MAINSEND" : bool * : track sends audio to parent
// "B_FREEMODE" : bool * : track free-mode enabled (requires UpdateTimeline() after changing etc)
// "C_BEATATTACHMODE" : char * : char * to one char of "beat attached mode", -1=def, 0=time, 1=allbeats, 2=beatsposonly
// "F_MCP_FXSEND_SCALE" : float * : scale of fx+send area in MCP (0.0=smallest allowed, 1=max allowed)
// "F_MCP_SENDRGN_SCALE" : float * : scale of send area as proportion of the fx+send total area (0=min allow, 1=max)
// "GUID" : GUID * : 16-byte GUID, can query or update (do not use on master though)
REAPER_PLUGIN_DECLARE_APIFUNCS void *(*GetSetMediaTrackInfo)(MediaTrack *tr, const char *parmname, void *setNewValue);
REAPER_PLUGIN_DECLARE_APIFUNCS double (*GetMediaTrackInfo_Value)(MediaTrack* tr, const char* parmname);
REAPER_PLUGIN_DECLARE_APIFUNCS bool (*SetMediaTrackInfo_Value)(MediaTrack* tr, const char* parmname, double newvalue);
REAPER_PLUGIN_DECLARE_APIFUNCS bool (*GetSetMediaTrackInfo_String)(MediaTrack* tr, const char* parmname, char* string, bool setnetvalue);

REAPER_PLUGIN_DECLARE_APIFUNCS void (*DeleteTrack)(MediaTrack *tr); // deletes a track
REAPER_PLUGIN_DECLARE_APIFUNCS void (*InsertTrackAtIndex)(int idx, bool wantDefaults); // inserts a track at "idx", of course this will be clamped to 0..GetNumTracks(). wantDefaults=TRUE for default envelopes/FX, otherwise no enabled fx/env

REAPER_PLUGIN_DECLARE_APIFUNCS MediaTrack* (*GetLastTouchedTrack)();  

REAPER_PLUGIN_DECLARE_APIFUNCS void* (*GetSelectedTrackEnvelope)(ReaProject* proj); // get the currently selected track envelope as an opaque pointer (can be passed to GetSetObjectState), returns 0 if no envelope is selected

REAPER_PLUGIN_DECLARE_APIFUNCS char* (*GetSetObjectState)(void* obj, char* str);  // get or set the state of a {track,item,envelope} as an xml/rpp chunk, str=0 to get the chunk string returned (must call FreeHeapPtr when done), str!=0 to set the state (returns zero)

REAPER_PLUGIN_DECLARE_APIFUNCS void (*FreeHeapPtr)(void* ptr); // free heap memory returned from a Reaper API function

REAPER_PLUGIN_DECLARE_APIFUNCS double (*Master_GetPlayRateAtTime)(double time_s, void *__proj);

// shows the action list for a section (NULL=main), and if  callerWnd is set then it sets the destination of any executed action.
// if caller==NULL and callerWnd!=NULL, then reset the context (if open) -- call ShowActionList(NULL,hwnd) on WM_DESTROY if you 
// have ever called ShowActionList(section,hwnd)
REAPER_PLUGIN_DECLARE_APIFUNCS void (*ShowActionList)(KbdSectionInfo* caller, HWND callerWnd); 



// dock API
// add your window to the dock. name is the title, pos is the relative position (MIDI editors are 9000, media explorer 5000, video 5001, mixer 1000, etc.)
// allowShow will let it activate it no other dock window open.
REAPER_PLUGIN_DECLARE_APIFUNCS void (*DockWindowAdd)(HWND hwnd, char *name, int pos, bool allowShow);

// DockWindowRemove removes your window from the dock. If you need to make it undocked you will need to restore the window style.
// YOU _MMMUUUSTTTT__ call DockWindowRemove(hwnd) in your WM_DESTROY. It is safe to call this even if you never added it to the dock.
REAPER_PLUGIN_DECLARE_APIFUNCS void (*DockWindowRemove)(HWND hwnd);

// active your window in the dock.
REAPER_PLUGIN_DECLARE_APIFUNCS void (*DockWindowActivate)(HWND hwnd);

//
// call this with amt=1 to add a request for "always run fx" to be on. call amt=-1 when done to remove your request.
REAPER_PLUGIN_DECLARE_APIFUNCS void (*PluginWantsAlwaysRunFx)(int amt);


// __mergesort is a stable sorting function with an API similar to qsort(). 
// HOWEVER, it requires some temporary space, equal to the size of the data being sorted, so you can pass it as the last parameter,
// or NULL and it will allocate and free space internally.
REAPER_PLUGIN_DECLARE_APIFUNCS void (*__mergesort)(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *), void *tmpspace);

// WDL_fft_8 performs a forward or inverse complex FFT. len must be 16,32,64,128,256,512,1024,2048,4096,8192,16384 or 32768.
// the buf must point to len*2 doubles. if isInverse is set then it is an inverse transform.
// no scaling is done so an FFT followed by an iFFT will result in the values being scaled up by len.
// the output is permuted, so a function to quickly access the permuted elements is provided (WDL_fft_permute).
REAPER_PLUGIN_DECLARE_APIFUNCS void (*WDL_fft_8)(double *buf, int len, int isInverse);

REAPER_PLUGIN_DECLARE_APIFUNCS int (*WDL_fft_permute)(int fftsize, int idx); // permute "idx" for an fft of "fftsize"

// WDL_fft_complexmul_8 multiplies two blocks of complex numbers (dest, src) and stores them in dest.
REAPER_PLUGIN_DECLARE_APIFUNCS void (*WDL_fft_complexmul_8)(double *dest, double *src, int length); 

// WDL_fft_complexmul_8 multiplies two blocks of complex numbers (src1, src2) and stores them in dest.
REAPER_PLUGIN_DECLARE_APIFUNCS void (*WDL_fft_complexmul2_8)(double *dest, double *src, double *src2, int length); 

// WDL_fft_complexmul_8 multiplies two blocks of complex numbers (src1, src2) and adds them to dest.
REAPER_PLUGIN_DECLARE_APIFUNCS void (*WDL_fft_complexmul3_8)(double *dest, double *src, double *src2, int length); 


REAPER_PLUGIN_DECLARE_APIFUNCS void (*EnsureNotCompletelyOffscreen)(RECT *r); // call with a saved window rect for your window and it'll correct any positioning info.

REAPER_PLUGIN_DECLARE_APIFUNCS HWND (*GetTooltipWindow)(); // gets a tooltip window, in case you want to ask it for font information. Can return NULL.

REAPER_PLUGIN_DECLARE_APIFUNCS const char* (*GetTrackMIDINoteNameEx)(void *proj, void *track, int note, int chan);
REAPER_PLUGIN_DECLARE_APIFUNCS const char* (*SetTrackMIDINoteNameEx)(void *proj, void *track, int note, int chan, const char* name);
REAPER_PLUGIN_DECLARE_APIFUNCS const char* (*HasTrackMIDIProgramsEx)(void *proj, void *track);  // returns name of track plugin that is supplying MIDI programs, or NULL if there is none
REAPER_PLUGIN_DECLARE_APIFUNCS bool (*EnumTrackMIDIProgramNamesEx)(void *proj, void *track, int programNumber, char* programName, int maxnamelen); // returns false if there are no plugins on the track that support MIDI programs, or if all programs have been enumerated


REAPER_PLUGIN_DECLARE_APIFUNCS const char* (*GetTrackMIDINoteName)(int track, int note, int chan);
REAPER_PLUGIN_DECLARE_APIFUNCS const char* (*SetTrackMIDINoteName)(int track, int note, int chan, const char* name);
REAPER_PLUGIN_DECLARE_APIFUNCS const char* (*HasTrackMIDIPrograms)(int track);  // returns name of track plugin that is supplying MIDI programs, or NULL if there is none
REAPER_PLUGIN_DECLARE_APIFUNCS bool (*EnumTrackMIDIProgramNames)(int track, int programNumber, char* programName, int maxnamelen); // returns false if there are no plugins on the track that support MIDI programs, or if all programs have been enumerated
                                                     

REAPER_PLUGIN_DECLARE_APIFUNCS void *(*EnumProjects)(int idx, char *projfn, int projfnlen); // idx=-1 for "current" project, projfn can be NULL if not interested in filename
REAPER_PLUGIN_DECLARE_APIFUNCS void (*SelectProjectInstance)(ReaProject *proj);

////////// LICE
// LICE is part of WDL and is an image compositing library.  It is also provided by REAPER so (our) plug-ins don't need to actually
// include the code.

#ifdef REAPER_PLUGIN_FUNCTIONS_WANT_LICE // disabled by default as to not conflict with any LICE people may have.

class LICE_IBitmap;
// create a new bitmap. this is like calling new LICE_MemBitmap (mode=0) or new LICE_SysBitmap (mode=1).
REAPER_PLUGIN_DECLARE_APIFUNCS LICE_IBitmap *(*LICE_CreateBitmap)(int mode, int w, int h); 

// simple bindings for LICE_IBitmap::*(), in case the ABI differs or your LICE_IBitmap definition is different than ours.
REAPER_PLUGIN_DECLARE_APIFUNCS HDC (*LICE__GetDC)(LICE_IBitmap *bm);
REAPER_PLUGIN_DECLARE_APIFUNCS int (*LICE__GetWidth)(LICE_IBitmap *bm);
REAPER_PLUGIN_DECLARE_APIFUNCS int (*LICE__GetHeight)(LICE_IBitmap *bm);
REAPER_PLUGIN_DECLARE_APIFUNCS int (*LICE__GetRowSpan)(LICE_IBitmap *bm);
REAPER_PLUGIN_DECLARE_APIFUNCS void (*LICE__Destroy)(LICE_IBitmap *bm);
REAPER_PLUGIN_DECLARE_APIFUNCS void *(*LICE__GetBits)(LICE_IBitmap *bm);
REAPER_PLUGIN_DECLARE_APIFUNCS bool (*LICE__IsFlipped)(LICE_IBitmap *bm);
REAPER_PLUGIN_DECLARE_APIFUNCS bool (*LICE__resize)(LICE_IBitmap *bm, int w, int h);

REAPER_PLUGIN_DECLARE_APIFUNCS bool (*WDL_VirtualWnd_ScaledBlitBG)(LICE_IBitmap *dest, WDL_VirtualWnd_BGCfg *src,int destx, int desty, int destw, int desth,int clipx, int clipy, int clipw, int cliph,float alpha, int mode);

// load a PNG from a file or resource. if bmp = NULL a new LICE_MemBitmap will be created
REAPER_PLUGIN_DECLARE_APIFUNCS LICE_IBitmap *(*LICE_LoadPNG)(const char *filename, LICE_IBitmap *bmp);
REAPER_PLUGIN_DECLARE_APIFUNCS LICE_IBitmap *(*LICE_LoadPNGFromResource)(HINSTANCE hInst, int resid, LICE_IBitmap *bmp);

// ok these you should look at lice.h for (minimal) documentation :/
REAPER_PLUGIN_DECLARE_APIFUNCS void (*LICE_ScaledBlit)(LICE_IBitmap *dest, LICE_IBitmap *src, int dstx, int dsty, int dstw, int dsth, float srcx, float srcy, float srcw, float srch, float alpha, int mode);

REAPER_PLUGIN_DECLARE_APIFUNCS void (*LICE_RotatedBlit)(LICE_IBitmap *dest, LICE_IBitmap *src, int dstx, int dsty, int dstw, int dsth, float srcx, float srcy, float srcw, float srch, float angle, bool cliptosourcerect, float alpha, int mode,float rotxcent, float rotycent); // these coordinates are offset from the center of the image, in source pixel coordinates

REAPER_PLUGIN_DECLARE_APIFUNCS void (*LICE_Blur)(LICE_IBitmap *dest, LICE_IBitmap *src, int dstx, int dsty, int srcx, int srcy, int srcw, int srch);

REAPER_PLUGIN_DECLARE_APIFUNCS void (*LICE_PutPixel)(LICE_IBitmap *bm, int x, int y, LICE_pixel color, float alpha, int mode);
REAPER_PLUGIN_DECLARE_APIFUNCS LICE_pixel (*LICE_GetPixel)(LICE_IBitmap *bm, int x, int y);

REAPER_PLUGIN_DECLARE_APIFUNCS void (*LICE_GradRect)(LICE_IBitmap *dest, int dstx, int dsty, int dstw, int dsth, float ir, float ig, float ib, float ia,float drdx, float dgdx, float dbdx, float dadx, float drdy, float dgdy, float dbdy, float dady, int mode);

REAPER_PLUGIN_DECLARE_APIFUNCS void (*LICE_ClearRect)(LICE_IBitmap *dest, int x, int y, int w, int h, LICE_pixel mask, LICE_pixel orbits);
REAPER_PLUGIN_DECLARE_APIFUNCS void (*LICE_FillRect)(LICE_IBitmap *dest, int x, int y, int w, int h, LICE_pixel color, float alpha, int mode);
REAPER_PLUGIN_DECLARE_APIFUNCS void (*LICE_DrawRect)(LICE_IBitmap *dest, int x, int y, int w, int h, LICE_pixel color, float alpha, int mode);
REAPER_PLUGIN_DECLARE_APIFUNCS void (*LICE_BorderedRect)(LICE_IBitmap *dest, int x, int y, int w, int h, LICE_pixel bgcolor, LICE_pixel fgcolor, float alpha, int mode);

REAPER_PLUGIN_DECLARE_APIFUNCS void (*LICE_Clear)(LICE_IBitmap *dest, LICE_pixel color);
REAPER_PLUGIN_DECLARE_APIFUNCS void (*LICE_Blit)(LICE_IBitmap *dest, LICE_IBitmap *src, int dstx, int dsty, int srcx, int srcy, int srcw, int srch, float alpha, int mode);

REAPER_PLUGIN_DECLARE_APIFUNCS void (*LICE_MultiplyAddRect)(LICE_IBitmap *dest, int x, int y, int w, int h, float rsc, float gsc, float bsc, float asc,float radd, float gadd, float badd, float aadd);

REAPER_PLUGIN_DECLARE_APIFUNCS void (*LICE_SimpleFill)(LICE_IBitmap *dest, int x, int y, LICE_pixel newcolor, LICE_pixel comparemask, LICE_pixel keepmask);

REAPER_PLUGIN_DECLARE_APIFUNCS void (*LICE_DrawChar)(LICE_IBitmap *bm, int x, int y, char c, LICE_pixel color, float alpha, int mode);
REAPER_PLUGIN_DECLARE_APIFUNCS void (*LICE_DrawText)(LICE_IBitmap *bm, int x, int y, const char *string, LICE_pixel color, float alpha, int mode);
REAPER_PLUGIN_DECLARE_APIFUNCS void (*LICE_MeasureText)(const char *string, int *w, int *h);

REAPER_PLUGIN_DECLARE_APIFUNCS void (*LICE_Line)(LICE_IBitmap *dest, int x1, int y1, int x2, int y2, LICE_pixel color, float alpha, int mode, bool aa);

REAPER_PLUGIN_DECLARE_APIFUNCS void (*LICE_FillTriangle)(LICE_IBitmap *dest, int x1, int y1, int x2, int y2, int x3, int y3, LICE_pixel color, float alpha, int mode);

REAPER_PLUGIN_DECLARE_APIFUNCS void (*LICE_DrawGlyph)(LICE_IBitmap* dest, int x, int y, LICE_pixel color, LICE_pixel_chan* alphas, int glyph_w, int glyph_h, float alpha, int mode);

// Returns false if the line is entirely offscreen.
REAPER_PLUGIN_DECLARE_APIFUNCS bool (*LICE_ClipLine)(int* pX1, int* pY1, int* pX2, int* pY2, int xLo, int yLo, int xHi, int yHi);

REAPER_PLUGIN_DECLARE_APIFUNCS void (*LICE_Arc)(LICE_IBitmap* dest, float cx, float cy, float r, float minAngle, float maxAngle, LICE_pixel color, float alpha, int mode, bool aa);
REAPER_PLUGIN_DECLARE_APIFUNCS void (*LICE_Circle)(LICE_IBitmap* dest, float cx, float cy, float r, LICE_pixel color, float alpha, int mode, bool aa);
REAPER_PLUGIN_DECLARE_APIFUNCS void (*LICE_FillCircle)(LICE_IBitmap* dest, float cx, float cy, float r, LICE_pixel color, float alpha, int mode, bool aa);

REAPER_PLUGIN_DECLARE_APIFUNCS void (*LICE_RoundRect)(LICE_IBitmap *drawbm, float xpos, float ypos, float w, float h, int cornerradius,LICE_pixel col, float alpha, int mode, bool aa);

REAPER_PLUGIN_DECLARE_APIFUNCS LICE_IFont* (*LICE_CreateFont)(); 
REAPER_PLUGIN_DECLARE_APIFUNCS void (*LICE__DestroyFont)(LICE_IFont* font);
REAPER_PLUGIN_DECLARE_APIFUNCS void (*LICE__SetFromHFont)(LICE_IFont* font, HFONT hfont, int flags); // font must REMAIN valid, unless LICE_FONT_FLAG_PRECALCALL is set
REAPER_PLUGIN_DECLARE_APIFUNCS LICE_pixel (*LICE__SetTextColor)(LICE_IFont* font, LICE_pixel color);
REAPER_PLUGIN_DECLARE_APIFUNCS LICE_pixel (*LICE__SetBkColor)(LICE_IFont* font, LICE_pixel color);
REAPER_PLUGIN_DECLARE_APIFUNCS int (*LICE__DrawText)(LICE_IFont* font, LICE_IBitmap *bm, const char *str, int strcnt, RECT *rect, UINT dtFlags);

REAPER_PLUGIN_DECLARE_APIFUNCS int (*ShowMessageBox)(const char* msg, const char* title, int type);  // type 0=OK, 2=OKCANCEL, 2=ABORTRETRYIGNORE, 3=YESNOCANCEL, 4=YESNO, 5=RETRYCANCEL : ret 1=OK, 2=CANCEL, 3=ABORT, 4=RETRY, 5=IGNORE, 6=YES, 7=NO

REAPER_PLUGIN_DECLARE_APIFUNCS int (*APITest)(); 

#endif




#endif//_REAPER_PLUGIN_FUNCTIONS_H_
