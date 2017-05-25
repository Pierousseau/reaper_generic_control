
#pragma once

#include <jmde/reaper_plugin.h>
#include <WDL/db2val.h>
#include <WDL/wdlstring.h>

#include <stdio.h>
#include "resource.h"

extern REAPER_PLUGIN_HINSTANCE g_hInst; // used for dialogs
extern HWND g_hwnd;
/* 
** Calls back to REAPER (all validated on load)
*/
extern double (*DB2SLIDER)(double x);
extern double (*SLIDER2DB)(double y);
extern int (*GetNumMIDIInputs)(); 
extern int (*GetNumMIDIOutputs)();
extern midi_Input *(*CreateMIDIInput)(int dev);
extern midi_Output *(*CreateMIDIOutput)(int dev, bool streamMode, int *msoffset100); 
extern bool (*GetMIDIOutputName)(int dev, char *nameout, int nameoutlen);
extern bool (*GetMIDIInputName)(int dev, char *nameout, int nameoutlen);

extern int (*CSurf_TrackToID)(MediaTrack *track, bool mcpView);
extern MediaTrack *(*CSurf_TrackFromID)(int idx, bool mcpView);
extern int (*CSurf_NumTracks)(bool mcpView);

    // these will be called from app when something changes
extern void (*CSurf_SetTrackListChange)();
extern void (*CSurf_SetSurfaceVolume)(MediaTrack *trackid, double volume, IReaperControlSurface *ignoresurf);
extern void (*CSurf_SetSurfacePan)(MediaTrack *trackid, double pan, IReaperControlSurface *ignoresurf);
extern void (*CSurf_SetSurfaceMute)(MediaTrack *trackid, bool mute, IReaperControlSurface *ignoresurf);
extern void (*CSurf_SetSurfaceSelected)(MediaTrack *trackid, bool selected, IReaperControlSurface *ignoresurf);
extern void (*CSurf_SetSurfaceSolo)(MediaTrack *trackid, bool solo, IReaperControlSurface *ignoresurf);
extern void (*CSurf_SetSurfaceRecArm)(MediaTrack *trackid, bool recarm, IReaperControlSurface *ignoresurf);
extern bool (*CSurf_GetTouchState)(MediaTrack *trackid, int isPan);
extern void (*CSurf_SetAutoMode)(int mode, IReaperControlSurface *ignoresurf);

extern void (*CSurf_SetPlayState)(bool play, bool pause, bool rec, IReaperControlSurface *ignoresurf);
extern void (*CSurf_SetRepeatState)(bool rep, IReaperControlSurface *ignoresurf);

// these are called by our surfaces, and actually update the project
extern double (*CSurf_OnVolumeChange)(MediaTrack *trackid, double volume, bool relative);
extern double (*CSurf_OnPanChange)(MediaTrack *trackid, double pan, bool relative);
extern bool (*CSurf_OnMuteChange)(MediaTrack *trackid, int mute);
extern bool (*CSurf_OnSelectedChange)(MediaTrack *trackid, int selected);
extern bool (*CSurf_OnSoloChange)(MediaTrack *trackid, int solo);
extern bool (*CSurf_OnFXChange)(MediaTrack *trackid, int en);
extern bool (*CSurf_OnRecArmChange)(MediaTrack *trackid, int recarm);
extern void (*CSurf_OnPlay)();
extern void (*CSurf_OnStop)();
extern void (*CSurf_OnFwd)(int seekplay);
extern void (*CSurf_OnRew)(int seekplay);
extern void (*CSurf_OnRecord)();
extern void (*CSurf_GoStart)();
extern void (*CSurf_GoEnd)();
extern void (*CSurf_OnArrow)(int whichdir, bool wantzoom);
extern void (*CSurf_OnTrackSelection)(MediaTrack *trackid);
extern void (*CSurf_ResetAllCachedVolPanStates)();
extern void (*CSurf_ScrubAmt)(double amt);

extern void (*kbd_OnMidiEvent)(MIDI_event_t *evt, int dev_index);

extern const char *(*GetTrackInfo)(INT_PTR track, int *flags); 

extern int (*GetMasterMuteSoloFlags)();
extern void (*TrackList_UpdateAllExternalSurfaces)();

extern void (*MoveEditCursor)(double adjamt, bool dosel);
extern void (*adjustZoom)(double amt, int forceset, bool doupd, int centermode); // 0,true,-1 are defaults
extern double (*GetHZoomLevel)(); // returns pixels/second

extern void (*ClearAllRecArmed)();
extern void (*SetTrackAutomationMode)(MediaTrack *tr, int mode);
extern int (*GetTrackAutomationMode)(MediaTrack *tr);
extern void (*SoloAllTracks)(int solo); // solo=2 for SIP
extern void (*MuteAllTracks)(bool mute);
extern void (*BypassFxAllTracks)(int bypass); // -1 = bypass all if not all bypassed, otherwise unbypass all
extern void (*SetTrackSelected)(MediaTrack *tr, bool sel);
extern int (*GetPlayState)();
extern double (*GetPlayPosition)();
extern double (*GetCursorPosition)();
extern void (*format_timestr_pos)(double tpos, char *buf, int buflen, int modeoverride); // modeoverride=-1 for proj
extern void (*UpdateTimeline)(void);

extern int (*GetSetRepeat)(int val);

extern void (*SetAutomationMode)(int mode, bool onlySel);
extern void (*Main_UpdateLoopInfo)(int ignoremask);

extern double (*TimeMap2_timeToBeats)(void *proj, double tpos, int *measures, int *cml, double *fullbeats, int *cdenom);

extern void * (*projectconfig_var_addr)(void *proj, int idx);

extern double (*Track_GetPeakInfo)(MediaTrack *tr, int chidx);
extern bool (*GetTrackUIVolPan)(MediaTrack *tr, double *vol, double *pan);
extern void (*mkvolpanstr)(char *str, double vol, double pan);
extern void (*mkvolstr)(char *str, double vol);
extern void (*mkpanstr)(char *str, double pan);

extern int (*TrackFX_GetCount)(MediaTrack *tr);
extern int (*TrackFX_GetNumParams)(MediaTrack *tr, int fx);
extern double (*TrackFX_GetParam)(MediaTrack *tr, int fx, int param, double *minval, double *maxval);
extern bool (*TrackFX_SetParam)(MediaTrack *tr, int fx, int param, double val);
extern bool (*TrackFX_GetParamName)(MediaTrack *tr, int fx, int param, char *buf, int buflen);
extern bool (*TrackFX_FormatParamValue)(MediaTrack *tr, int fx, int param, double val, char *buf, int buflen);
extern bool (*TrackFX_GetFXName)(MediaTrack *tr, int fx, char *buf, int buflen);
extern GUID *(*GetTrackGUID)(MediaTrack *tr);

extern int *g_config_csurf_rate,*g_config_zoommode;

extern int __g_projectconfig_timemode2, __g_projectconfig_timemode;
extern int __g_projectconfig_timeoffs;
extern int __g_projectconfig_measoffs;

/*
** REAPER command message defines
*/

#define IDC_REPEAT                      1068
#define ID_FILE_SAVEAS                  40022
#define ID_FILE_NEWPROJECT              40023
#define ID_FILE_OPENPROJECT             40025
#define ID_FILE_SAVEPROJECT             40026
#define IDC_EDIT_UNDO                   40029
#define IDC_EDIT_REDO                   40030
#define ID_MARKER_PREV                  40172
#define ID_MARKER_NEXT                  40173
#define ID_INSERT_MARKERRGN             40174
#define ID_INSERT_MARKER                40157
#define ID_LOOP_SETSTART                40222
#define ID_LOOP_SETEND                  40223
#define ID_METRONOME                    40364
#define ID_GOTO_MARKER1                 40161
#define ID_SET_MARKER1                  40657

// Reaper track automation modes
enum AutoMode {
  AUTO_MODE_TRIM,
  AUTO_MODE_READ,
  AUTO_MODE_TOUCH,
  AUTO_MODE_WRITE,
  AUTO_MODE_LATCH,
};

midi_Output *CreateThreadedMIDIOutput(midi_Output *output); // returns null on null
