/*
** reaper_csurf
** Copyright (C) 2006-2008 Cockos Incorporated
** License: LGPL.
*/


#include "csurf.h"

extern reaper_csurf_reg_t csurf_bcf_reg,csurf_faderport_reg,csurf_hui_reg,
     csurf_mcu_reg,csurf_mcuex_reg,csurf_tranzport_reg,csurf_alphatrack_reg,csurf_01X_reg;

REAPER_PLUGIN_HINSTANCE g_hInst; // used for dialogs, if any
HWND g_hwnd;


double (*DB2SLIDER)(double x);
double (*SLIDER2DB)(double y);
int (*GetNumMIDIInputs)(); 
int (*GetNumMIDIOutputs)();
midi_Input *(*CreateMIDIInput)(int dev);
midi_Output *(*CreateMIDIOutput)(int dev, bool streamMode, int *msoffset100); 
bool (*GetMIDIOutputName)(int dev, char *nameout, int nameoutlen);
bool (*GetMIDIInputName)(int dev, char *nameout, int nameoutlen);

void * (*projectconfig_var_addr)(void*proj, int idx);


int (*CSurf_TrackToID)(MediaTrack *track, bool mcpView);
MediaTrack *(*CSurf_TrackFromID)(int idx, bool mcpView);
int (*CSurf_NumTracks)(bool mcpView);

    // these will be called from app when something changes
void (*CSurf_SetTrackListChange)();
void (*CSurf_SetSurfaceVolume)(MediaTrack *trackid, double volume, IReaperControlSurface *ignoresurf);
void (*CSurf_SetSurfacePan)(MediaTrack *trackid, double pan, IReaperControlSurface *ignoresurf);
void (*CSurf_SetSurfaceMute)(MediaTrack *trackid, bool mute, IReaperControlSurface *ignoresurf);
void (*CSurf_SetSurfaceSelected)(MediaTrack *trackid, bool selected, IReaperControlSurface *ignoresurf);
void (*CSurf_SetSurfaceSolo)(MediaTrack *trackid, bool solo, IReaperControlSurface *ignoresurf);
void (*CSurf_SetSurfaceRecArm)(MediaTrack *trackid, bool recarm, IReaperControlSurface *ignoresurf);
bool (*CSurf_GetTouchState)(MediaTrack *trackid, int isPan);
void (*CSurf_SetAutoMode)(int mode, IReaperControlSurface *ignoresurf);

void (*CSurf_SetPlayState)(bool play, bool pause, bool rec, IReaperControlSurface *ignoresurf);
void (*CSurf_SetRepeatState)(bool rep, IReaperControlSurface *ignoresurf);

// these are called by our surfaces, and actually update the project
double (*CSurf_OnVolumeChange)(MediaTrack *trackid, double volume, bool relative);
double (*CSurf_OnPanChange)(MediaTrack *trackid, double pan, bool relative);
bool (*CSurf_OnMuteChange)(MediaTrack *trackid, int mute);
bool (*CSurf_OnSelectedChange)(MediaTrack *trackid, int selected);
bool (*CSurf_OnSoloChange)(MediaTrack *trackid, int solo);
bool (*CSurf_OnFXChange)(MediaTrack *trackid, int en);
bool (*CSurf_OnRecArmChange)(MediaTrack *trackid, int recarm);
void (*CSurf_OnPlay)();
void (*CSurf_OnStop)();
void (*CSurf_OnFwd)(int seekplay);
void (*CSurf_OnRew)(int seekplay);
void (*CSurf_OnRecord)();
void (*CSurf_GoStart)();
void (*CSurf_GoEnd)();
void (*CSurf_OnArrow)(int whichdir, bool wantzoom);
void (*CSurf_OnTrackSelection)(MediaTrack *trackid);
void (*CSurf_ResetAllCachedVolPanStates)();
void (*CSurf_ScrubAmt)(double amt);

void (*kbd_OnMidiEvent)(MIDI_event_t *evt, int dev_index);
void (*TrackList_UpdateAllExternalSurfaces)();
int (*GetMasterMuteSoloFlags)();
void (*ClearAllRecArmed)();
void (*SetTrackAutomationMode)(MediaTrack *tr, int mode);
int (*GetTrackAutomationMode)(MediaTrack *tr);
void (*SoloAllTracks)(int solo); // solo=2 for SIP
void (*MuteAllTracks)(bool mute);
void (*BypassFxAllTracks)(int bypass); // -1 = bypass all if not all bypassed, otherwise unbypass all
const char *(*GetTrackInfo)(INT_PTR track, int *flags); 
void (*SetTrackSelected)(MediaTrack *tr, bool sel);
void (*UpdateTimeline)(void);
int (*GetPlayState)();
double (*GetPlayPosition)();
double (*GetCursorPosition)();
int (*GetSetRepeat)(int val);

void (*format_timestr_pos)(double tpos, char *buf, int buflen, int modeoverride); // modeoverride=-1 for proj
void (*SetAutomationMode)(int mode, bool onlySel); // sets all or selected tracks
void (*Main_UpdateLoopInfo)(int ignoremask);

double (*TimeMap2_timeToBeats)(void *proj, double tpos, int *measures, int *cml, double *fullbeats, int *cdenom);
double (*Track_GetPeakInfo)(MediaTrack *tr, int chidx);
void (*mkvolpanstr)(char *str, double vol, double pan);
void (*mkvolstr)(char *str, double vol);
void (*mkpanstr)(char *str, double pan);

bool (*GetTrackUIVolPan)(MediaTrack *tr, double *vol, double *pan);



void (*MoveEditCursor)(double adjamt, bool dosel);
void (*adjustZoom)(double amt, int forceset, bool doupd, int centermode); // forceset=0, doupd=true, centermode=-1 for default
double (*GetHZoomLevel)(); // returns pixels/second


int (*TrackFX_GetCount)(MediaTrack *tr);
int (*TrackFX_GetNumParams)(MediaTrack *tr, int fx);
bool (*TrackFX_GetFXName)(MediaTrack *tr, int fx, char *buf, int buflen);
double (*TrackFX_GetParam)(MediaTrack *tr, int fx, int param, double *minval, double *maxval);
bool (*TrackFX_SetParam)(MediaTrack *tr, int fx, int param, double val);
bool (*TrackFX_GetParamName)(MediaTrack *tr, int fx, int param, char *buf, int buflen);
bool (*TrackFX_FormatParamValue)(MediaTrack *tr, int fx, int param, double val, char *buf, int buflen);
GUID *(*GetTrackGUID)(MediaTrack *tr);


int *g_config_csurf_rate,*g_config_zoommode;

int __g_projectconfig_timemode2, __g_projectconfig_timemode;
int __g_projectconfig_measoffs;
int __g_projectconfig_timeoffs; // double

extern "C"
{

REAPER_PLUGIN_DLL_EXPORT int REAPER_PLUGIN_ENTRYPOINT(REAPER_PLUGIN_HINSTANCE hInstance, reaper_plugin_info_t *rec)
{
  g_hInst=hInstance;

  if (!rec || rec->caller_version != REAPER_PLUGIN_VERSION || !rec->GetFunc)
      return 0;

  g_hwnd = rec->hwnd_main;
  int errcnt=0;
#define IMPAPI(x) if (!((*((void **)&(x)) = (void *)rec->GetFunc(#x)))) errcnt++;

  IMPAPI(DB2SLIDER)
  IMPAPI(SLIDER2DB)
  IMPAPI(GetNumMIDIInputs)
  IMPAPI(GetNumMIDIOutputs)
  IMPAPI(CreateMIDIInput)
  IMPAPI(CreateMIDIOutput)
  IMPAPI(GetMIDIOutputName)
  IMPAPI(GetMIDIInputName)
  IMPAPI(CSurf_TrackToID)
  IMPAPI(CSurf_TrackFromID)
  IMPAPI(CSurf_NumTracks)
  IMPAPI(CSurf_SetTrackListChange)
  IMPAPI(CSurf_SetSurfaceVolume)
  IMPAPI(CSurf_SetSurfacePan)
  IMPAPI(CSurf_SetSurfaceMute)
  IMPAPI(CSurf_SetSurfaceSelected)
  IMPAPI(CSurf_SetSurfaceSolo)
  IMPAPI(CSurf_SetSurfaceRecArm)
  IMPAPI(CSurf_GetTouchState)
  IMPAPI(CSurf_SetAutoMode)
  IMPAPI(CSurf_SetPlayState)
  IMPAPI(CSurf_SetRepeatState)
  IMPAPI(CSurf_OnVolumeChange)
  IMPAPI(CSurf_OnPanChange)
  IMPAPI(CSurf_OnMuteChange)
  IMPAPI(CSurf_OnSelectedChange)
  IMPAPI(CSurf_OnSoloChange)
  IMPAPI(CSurf_OnFXChange)
  IMPAPI(CSurf_OnRecArmChange)
  IMPAPI(CSurf_OnPlay)
  IMPAPI(CSurf_OnStop)
  IMPAPI(CSurf_OnFwd)
  IMPAPI(CSurf_OnRew)
  IMPAPI(CSurf_OnRecord)
  IMPAPI(CSurf_GoStart)
  IMPAPI(CSurf_GoEnd)
  IMPAPI(CSurf_OnArrow)
  IMPAPI(CSurf_OnTrackSelection)
  IMPAPI(CSurf_ResetAllCachedVolPanStates)
  IMPAPI(CSurf_ScrubAmt)
  IMPAPI(TrackList_UpdateAllExternalSurfaces)
  IMPAPI(kbd_OnMidiEvent)
  IMPAPI(GetMasterMuteSoloFlags)
  IMPAPI(ClearAllRecArmed)
  IMPAPI(SetTrackAutomationMode)
  IMPAPI(GetTrackAutomationMode)
  IMPAPI(SoloAllTracks)
  IMPAPI(MuteAllTracks)
  IMPAPI(BypassFxAllTracks)
  IMPAPI(GetTrackInfo)
  IMPAPI(SetTrackSelected)
  IMPAPI(SetAutomationMode)
  IMPAPI(UpdateTimeline)
  IMPAPI(Main_UpdateLoopInfo)
  IMPAPI(GetPlayState)
  IMPAPI(GetPlayPosition)
  IMPAPI(GetCursorPosition)
  IMPAPI(format_timestr_pos)
  IMPAPI(TimeMap2_timeToBeats)
  IMPAPI(Track_GetPeakInfo)
  IMPAPI(GetTrackUIVolPan)
  IMPAPI(GetSetRepeat)
  IMPAPI(mkvolpanstr)
  IMPAPI(mkvolstr)
  IMPAPI(mkpanstr)
  IMPAPI(MoveEditCursor)
  IMPAPI(adjustZoom)
  IMPAPI(GetHZoomLevel)

  IMPAPI(TrackFX_GetCount)
  IMPAPI(TrackFX_GetNumParams)
  IMPAPI(TrackFX_GetParam)
  IMPAPI(TrackFX_SetParam)
  IMPAPI(TrackFX_GetParamName)
  IMPAPI(TrackFX_FormatParamValue)
  IMPAPI(TrackFX_GetFXName)
  
  IMPAPI(GetTrackGUID)
  
  void * (*get_config_var)(const char *name, int *szout); 
  int (*projectconfig_var_getoffs)(const char *name, int *szout);
  IMPAPI(get_config_var);
  IMPAPI(projectconfig_var_getoffs);
  IMPAPI(projectconfig_var_addr);
  if (errcnt) return 0;

  int sztmp;
#define IMPVAR(x,nm) if (!((*(void **)&(x)) = get_config_var(nm,&sztmp)) || sztmp != sizeof(*x)) errcnt++;
#define IMPVARP(x,nm,type) if (!((x) = projectconfig_var_getoffs(nm,&sztmp)) || sztmp != sizeof(type)) errcnt++;
  IMPVAR(g_config_csurf_rate,"csurfrate")
  IMPVAR(g_config_zoommode,"zoommode")

  IMPVARP(__g_projectconfig_timemode,"projtimemode",int)
  IMPVARP(__g_projectconfig_timemode2,"projtimemode2",int)
  IMPVARP(__g_projectconfig_timeoffs,"projtimeoffs",double);
  IMPVARP(__g_projectconfig_measoffs,"projmeasoffs",int);


  if (errcnt) return 0;


  rec->Register("csurf",&csurf_bcf_reg);
  rec->Register("csurf",&csurf_faderport_reg);
  rec->Register("csurf",&csurf_hui_reg);
  rec->Register("csurf",&csurf_mcu_reg);
  rec->Register("csurf",&csurf_mcuex_reg);
  rec->Register("csurf",&csurf_tranzport_reg);
  rec->Register("csurf",&csurf_alphatrack_reg);
  rec->Register("csurf",&csurf_01X_reg);

  return 1;

}

};





#ifndef _WIN32 // MAC resources
#include "../../WDL/swell/swell-dlggen.h"
#include "res.rc_mac_dlg"
#undef BEGIN
#undef END
#include "../../WDL/swell/swell-menugen.h"
#include "res.rc_mac_menu"
#endif


#ifndef _WIN32 // let OS X use this threading step

#include "../../WDL/mutex.h"
#include "../../WDL/ptrlist.h"



class threadedMIDIOutput : public midi_Output
{
public:
  threadedMIDIOutput(midi_Output *out) 
  { 
    m_output=out;
    m_quit=false;
    DWORD id;
    m_hThread=CreateThread(NULL,0,threadProc,this,0,&id);
  }
  virtual ~threadedMIDIOutput() 
  {
    if (m_hThread)
    {
      m_quit=true;
      WaitForSingleObject(m_hThread,INFINITE);
      CloseHandle(m_hThread);
      m_hThread=0;
      Sleep(30);
    }

    delete m_output;
    m_empty.Empty(true);
    m_full.Empty(true);
  }

  virtual void SendMsg(MIDI_event_t *msg, int frame_offset) // frame_offset can be <0 for "instant" if supported
  {
    if (!msg) return;

    WDL_HeapBuf *b=NULL;
    if (m_empty.GetSize())
    {
      m_mutex.Enter();
      b=m_empty.Get(m_empty.GetSize()-1);
      m_empty.Delete(m_empty.GetSize()-1);
      m_mutex.Leave();
    }
    if (!b && m_empty.GetSize()+m_full.GetSize()<500)
      b=new WDL_HeapBuf(256);

    if (b)
    {
      int sz=msg->size;
      if (sz<3)sz=3;
      int len = msg->midi_message + sz - (unsigned char *)msg;
      memcpy(b->Resize(len,false),msg,len);
      m_mutex.Enter();
      m_full.Add(b);
      m_mutex.Leave();
    }
  }

  virtual void Send(unsigned char status, unsigned char d1, unsigned char d2, int frame_offset) // frame_offset can be <0 for "instant" if supported
  {
    MIDI_event_t evt={0,3,status,d1,d2};
    SendMsg(&evt,frame_offset);
  }

  ///////////

  static DWORD WINAPI threadProc(LPVOID p)
  {
    WDL_HeapBuf *lastbuf=NULL;
    threadedMIDIOutput *_this=(threadedMIDIOutput*)p;
    unsigned int scnt=0;
    for (;;)
    {
      if (_this->m_full.GetSize()||lastbuf)
      {
        _this->m_mutex.Enter();
        if (lastbuf) _this->m_empty.Add(lastbuf);
        lastbuf=_this->m_full.Get(0);
        _this->m_full.Delete(0);
        _this->m_mutex.Leave();

        if (lastbuf) _this->m_output->SendMsg((MIDI_event_t*)lastbuf->Get(),-1);
        scnt=0;
      }
      else 
      {
        Sleep(1);
        if (_this->m_quit&&scnt++>3) break; //only quit once all messages have been sent
      }
    }
    delete lastbuf;
    return 0;
  }

  WDL_Mutex m_mutex;
  WDL_PtrList<WDL_HeapBuf> m_full,m_empty;

  HANDLE m_hThread;
  bool m_quit;
  midi_Output *m_output;
};




midi_Output *CreateThreadedMIDIOutput(midi_Output *output)
{
  if (!output) return output;
  return new threadedMIDIOutput(output);
}

#else

// windows doesnt need it since we have threaded midi outputs now
midi_Output *CreateThreadedMIDIOutput(midi_Output *output)
{
  return output;
}

#endif