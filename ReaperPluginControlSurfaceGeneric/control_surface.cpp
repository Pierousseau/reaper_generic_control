/*
** Based on reaper_csurf
** Copyright (C) 2006-2008 Cockos Incorporated
** License: LGPL.
** 
** MCU support - Modified for generic controller surfaces such as Korg NanoKontrol 2 support by : Pierre Rousseau (May 2017)
*/


#include "control_surface_interface.h"
#include "helpers.h"
#include <WDL/ptrlist.h>

#include <filesystem>
#include <functional>
#include <fstream>
#include <list>
#include <map>
#include <string>

#ifdef max
#undef min
#undef max
#endif

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

class ControlSurfaceGeneric;

class SurfacePreset
{
public:
  SurfacePreset(const std::string & filename)
    : _filename(filename)
    , _file_name_hash((int)std::hash<std::string>()(filename))
  {
    parseName();
    initFunctions();
  }

  int fileHash() const
  {
    return _file_name_hash;
  }

  std::string name() const
  {
    return _name;
  }

  void parseName()
  {
    std::ifstream f(_filename);
    rapidjson::IStreamWrapper isw(f);
    rapidjson::Document doc;
    doc.ParseStream<rapidjson::kParseCommentsFlag | rapidjson::kParseNanAndInfFlag>(isw);
    if (doc.HasMember("SurfaceName"))
      _name = doc["SurfaceName"].GetString();
  }

  void parseControls(midi_Output * midi_out)
  {
    std::ifstream f(_filename);
    rapidjson::IStreamWrapper isw(f);
    rapidjson::Document doc;
    doc.ParseStream<rapidjson::kParseCommentsFlag | rapidjson::kParseNanAndInfFlag>(isw);

    if (doc.HasMember("Transport"))
    {
      auto & transport = doc["Transport"];
      if (transport.HasMember("Rewind"))
        _controls[transport["Rewind"].GetInt()] = [](int) { CSurf_GoStart(); };
      if (transport.HasMember("Forward"))
        _controls[transport["Forward"].GetInt()] = [](int) { CSurf_GoEnd(); };
      if (transport.HasMember("Stop"))
        _controls[transport["Stop"].GetInt()] = [](int) { CSurf_OnStop(); };
      if (transport.HasMember("Play"))
      {
        int code = transport["Play"].GetInt();
        _controls[code] = [](int value) { CSurf_OnPlay(); };
        if (midi_out)
          setPlay = [midi_out, code](bool val) { midi_out->Send(0xB0, code, val ? 0x7f : 0, -1); };
      }
      if (transport.HasMember("Record"))
      {
        int code = transport["Record"].GetInt();
        _controls[code] = [](int value) { CSurf_OnRecord(); };
        if (midi_out)
          setRec = [midi_out, code](bool val) { midi_out->Send(0xB0, code, val ? 0x7f : 0, -1); };
      }
      if (transport.HasMember("Cycle"))
      {
        int code = transport["Cycle"].GetInt();
        _controls[code] = [](int value) { GetSetRepeat(value); };
        if (midi_out)
          setCycle = [midi_out, code](bool val) { midi_out->Send(0xB0, code, val ? 0x7f : 0, -1); };
      }

      if (transport.HasMember("Marker Set"))
        _controls[transport["Marker Set"].GetInt()] = [](int value) { if (value == 127) SendMessage(g_hwnd, WM_COMMAND, 40157, 0); };
      if (transport.HasMember("Marker Previous"))
        _controls[transport["Marker Previous"].GetInt()] = [](int value) { if (value == 127) SendMessage(g_hwnd, WM_COMMAND, 40172, 0); };
      if (transport.HasMember("Marker Next"))
        _controls[transport["Marker Next"].GetInt()] = [](int value) { if (value == 127) SendMessage(g_hwnd, WM_COMMAND, 40173, 0); };
    }

    std::map<MediaTrack *, int> solo_map;
    std::map<MediaTrack *, int> mute_map;
    std::map<MediaTrack *, int> rec_map;
    if (doc.HasMember("Master"))
    {
      auto & master = doc["Master"];
      MediaTrack * media_track = CSurf_TrackFromID(0, false);

      if (master.HasMember("Solo"))
      {
        int code = master["Solo"].GetInt();
        _controls[code] = [media_track](int value) { CSurf_OnSoloChange(media_track, value); };
        solo_map[media_track] = code;
      }
      if (master.HasMember("Mute"))
      {
        int code = master["Mute"].GetInt();
        _controls[code] = [media_track](int value) { CSurf_OnMuteChange(media_track, value); };
        mute_map[media_track] = code;
      }
      if (master.HasMember("Record"))
      {
        int code = master["Record"].GetInt();
        _controls[code] = [media_track](int value) { CSurf_OnRecArmChange(media_track, value); };
        rec_map[media_track] = code;
      }

      if (master.HasMember("Volume"))
        _controls[master["Volume"].GetInt()] = [media_track](int value) { CSurf_OnVolumeChange(media_track, charToVol(value), false); };
      if (master.HasMember("Pan"))
        _controls[master["Pan"].GetInt()] = [media_track](int value) { CSurf_OnPanChange(media_track, charToPan(value), false); };
    }

    if (doc.HasMember("Tracks"))
    {
      int track_id = 1;
      for (const auto & track : doc["Tracks"].GetArray())
      {
        MediaTrack * media_track = CSurf_TrackFromID(track_id, false);

        if (track.HasMember("Solo"))
        {
          int code = track["Solo"].GetInt();
          _controls[code] = [media_track](int value) { CSurf_OnSoloChange(media_track, value); };
          solo_map[media_track] = code;
        }
        if (track.HasMember("Mute"))
        {
          int code = track["Mute"].GetInt();
          _controls[code] = [media_track](int value) { CSurf_OnMuteChange(media_track, value); };
          mute_map[media_track] = code;
        }
        if (track.HasMember("Record"))
        {
          int code = track["Record"].GetInt();
          _controls[code] = [media_track](int value) { CSurf_OnRecArmChange(media_track, value); };
          rec_map[media_track] = code;
        }

        if (track.HasMember("Volume"))
          _controls[track["Volume"].GetInt()] = [media_track](int value) { CSurf_OnVolumeChange(media_track, charToVol(value), false); };
        if (track.HasMember("Pan"))
          _controls[track["Pan"].GetInt()] = [media_track](int value) { CSurf_OnPanChange(media_track, charToPan(value), false); };

        track_id++;
      }
    }

    if (midi_out)
    {
      if (!solo_map.empty())
        setTrackSolo = [midi_out, solo_map](MediaTrack * track, bool val) { std::map<MediaTrack *, int>::const_iterator finder = solo_map.find(track); if (finder != solo_map.end()) midi_out->Send(0xB0, finder->second, val ? 0x7f : 0, -1); };
      if (!mute_map.empty())
        setTrackMute = [midi_out, mute_map](MediaTrack * track, bool val) { std::map<MediaTrack *, int>::const_iterator finder = mute_map.find(track); if (finder != mute_map.end()) midi_out->Send(0xB0, finder->second, val ? 0x7f : 0, -1); };
      if (!rec_map.empty())
        setTrackRecord = [midi_out, rec_map](MediaTrack * track, bool val) { std::map<MediaTrack *, int>::const_iterator finder = rec_map.find(track); if (finder != rec_map.end()) midi_out->Send(0xB0, finder->second, val ? 0x7f : 0, -1); };
      
      resetTracks = [midi_out, solo_map, mute_map, rec_map] () { for (const auto & control : solo_map) midi_out->Send(0xB0, control.second, 0, -1);
                                                                 for (const auto & control : mute_map) midi_out->Send(0xB0, control.second, 0, -1);
                                                                 for (const auto & control :  rec_map) midi_out->Send(0xB0, control.second, 0, -1); };
    }
  }

  void midiIn(int code, int value)
  {
    ControlMap::const_iterator finder = _controls.find(code);
    if (finder != _controls.end())
      finder->second(value);
  }

  void initFunctions()
  {
    setPlay         = [](bool) {};
    setRec          = [](bool) {};
    setCycle        = [](bool) {};   
    setTrackSolo    = [](MediaTrack*, bool) {};
    setTrackMute    = [](MediaTrack*, bool) {};
    setTrackRecord  = [](MediaTrack*, bool) {};
    setTrackVolume  = [](MediaTrack*, double) {};
    setTrackPan     = [](MediaTrack*, double) {};
    resetTracks     = []() {};
  }

  std::function<void(bool)> setPlay;
  std::function<void(bool)> setRec;
  std::function<void(bool)> setCycle;

  std::function<void(MediaTrack*, bool)> setTrackSolo;
  std::function<void(MediaTrack*, bool)> setTrackMute;
  std::function<void(MediaTrack*, bool)> setTrackRecord;
  std::function<void(MediaTrack*, double)>  setTrackVolume;
  std::function<void(MediaTrack*, double)>  setTrackPan;
  std::function<void()> resetTracks;

private:
  std::string _name;
  std::string _filename;
  int _file_name_hash;

  typedef std::map<int, std::function<void(int)>> ControlMap;

  ControlMap _controls;
};


class SurfacePresets : public std::map<int, std::shared_ptr<SurfacePreset>>
{
public:
  SurfacePresets()
  {
    init();
  }

  void init()
  {
    clear();
    std::string presets_dir_path = _getExecutableDirPath() + "plugins\\reaper_plugin_control_surface_generic_presets";
    for (auto & p : std::experimental::filesystem::v1::directory_iterator(presets_dir_path))
      insert(std::make_shared<SurfacePreset>(p.path().string()));
  }

protected:
  void insert(std::shared_ptr<SurfacePreset> preset)
  {
    if (preset)
      std::map<int, std::shared_ptr<SurfacePreset>>::insert(std::make_pair(preset->fileHash(), preset));
  }

  std::string _getExecutableDirPath()
  {
    char buffer[_MAX_PATH];
    DWORD length = GetModuleFileNameA(0, buffer, sizeof(buffer));
    bool status = ((length > 0) && (length < sizeof(buffer)));
    if (status == false)
      return std::string();
    std::string path = buffer;
    std::string::size_type pos = path.find_last_of("\\/");
    return path.substr(0, pos) + "\\";
  }
};



static SurfacePresets surface_presets;



class ControlSurfaceGeneric : public IReaperControlSurface
{
public:
  ControlSurfaceGeneric(int indev, int outdev, int preset, int *errStats)
    : _midi_in_dev(indev)
    , _midi_out_dev(outdev)
    , _preset(preset)
  {
    _midi_in = _midi_in_dev >= 0 ? CreateMIDIInput(_midi_in_dev) : NULL;
    _midi_out = _midi_out_dev >= 0 ? CreateThreadedMIDIOutput(CreateMIDIOutput(_midi_out_dev, false, NULL)) : NULL;

    _active_preset = _preset >= 0 ? surface_presets[_preset] : nullptr;

    if (errStats)
    {
      if (_midi_in_dev >= 0 && !_midi_in) *errStats |= 1;
      if (_midi_out_dev >= 0 && !_midi_out) *errStats |= 2;
    }

    _surfaceInit();

    if (_midi_in)
      _midi_in->start();
  }

  ~ControlSurfaceGeneric()
  {
    CloseNoReset();
  }

  const char *GetTypeString()
  {
    return "GenericController";
  }

  const char *GetDescString()
  {
    _desc_string.Set("Generic Controller");
    char tmp[512];
    sprintf(tmp, " (dev %d,%d - preset %s)", _midi_in_dev, _midi_out_dev, _active_preset ? _active_preset->name().c_str() : "None");
    _desc_string.Append(tmp);
    return _desc_string.Get();
  }

  const char *GetConfigString() // string of configuration data
  {
    sprintf(_cfg_string, "%d %d %d", _midi_in_dev, _midi_out_dev, _preset);
    return _cfg_string;
  }

  virtual void CloseNoReset() override
  {
    if (_midi_in)
      delete _midi_in;
    if (_midi_out)
      delete _midi_out;
    _midi_out = 0;
    _midi_in = 0;
  }

  virtual void Run() override
  {
    if (_midi_in)
    {
      _midi_in->SwapBufs(timeGetTime());
      int l = 0;
      MIDI_eventlist *list = _midi_in->GetReadBuf();
      MIDI_event_t *evts;
      while ((evts = list->EnumItems(&l)))
        _midiEvent(evts);
    }
  }

  virtual void SetTrackListChange() override
  {
    if (_active_preset)
      _active_preset->parseControls(_midi_out);
  }

  virtual void SetSurfaceVolume(MediaTrack *trackid, double volume) override
  {
    if (_midi_out && _active_preset)
      _active_preset->setTrackVolume(trackid, volume);
  }

  virtual void SetSurfacePan(MediaTrack *trackid, double pan) override
  {
    if (_midi_out && _active_preset)
      _active_preset->setTrackPan(trackid, pan);
  }

  virtual void SetSurfaceMute(MediaTrack *trackid, bool mute) override
  {
    if (_midi_out && _active_preset)
      _active_preset->setTrackMute(trackid, mute);
  }

  virtual void SetSurfaceSolo(MediaTrack *trackid, bool solo) override
  {
    if (_midi_out && _active_preset)
      _active_preset->setTrackSolo(trackid, solo);
  }

  virtual void SetSurfaceRecArm(MediaTrack *trackid, bool recarm) override
  {
    if (_midi_out && _active_preset)
      _active_preset->setTrackRecord(trackid, recarm);
  }
  
  virtual void SetPlayState(bool play, bool pause, bool rec) override
  {
    if (_midi_out && _active_preset)
    {
      _active_preset->setPlay(play && ! pause);
      _active_preset->setRec(rec);
    }
  }

  virtual void SetRepeatState(bool rep) override
  {
    if (_midi_out && _active_preset)
      _active_preset->setCycle(rep);
  }

private:
  void _surfaceInit()
  {
    if (_midi_out && _active_preset)
    {
      _active_preset->setPlay(false);
      _active_preset->setRec(false);
      _active_preset->setCycle(false);

      _active_preset->resetTracks();
    }
  }

  void _midiEvent(MIDI_event_t *evt)
  {
    if (!_active_preset)
      return;

    int code = evt->midi_message[1];
    int value = evt->midi_message[2];
    _active_preset->midiIn(code, value);
  }

private:
  int _midi_in_dev;
  int _midi_out_dev;
  int _preset;

  midi_Output * _midi_out;
  midi_Input * _midi_in;
  std::shared_ptr<SurfacePreset> _active_preset;

  WDL_String _desc_string;
  char _cfg_string[1024];
};

static void parseParameters(const char *str, int params[3])
{
  params[0] = params[1] = params[2] = -1;

  const char *p = str;
  if (p)
  {
    int x = 0;
    while (x < 3)
    {
      while (*p == ' ') p++;
      if ((*p < '0' || *p > '9') && *p != '-') break;
      params[x++] = atoi(p);
      while (*p && *p != ' ') p++;
    }
  }
}

static IReaperControlSurface *createFunc(const char *type_string, const char *configString, int *errStats)
{
  int params[3];
  parseParameters(configString, params);

  return new ControlSurfaceGeneric(params[0], params[1], params[2], errStats);
}

static WDL_DLGRET dlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_INITDIALOG:
    {
      int params[3];
      parseParameters((const char *)lParam, params);

      int n = GetNumMIDIInputs();
      int x = SendDlgItemMessage(hwndDlg, IDC_COMBO2, CB_ADDSTRING, 0, (LPARAM)"None");
      SendDlgItemMessage(hwndDlg, IDC_COMBO2, CB_SETITEMDATA, x, -1);
      for (x = 0; x < n; x++)
      {
        char buf[512];
        if (GetMIDIInputName(x, buf, sizeof(buf)))
        {
          int a = SendDlgItemMessage(hwndDlg, IDC_COMBO2, CB_ADDSTRING, 0, (LPARAM)buf);
          SendDlgItemMessage(hwndDlg, IDC_COMBO2, CB_SETITEMDATA, a, x);
          if (x == params[0])
            SendDlgItemMessage(hwndDlg, IDC_COMBO2, CB_SETCURSEL, a, 0);
        }
      }

      x = SendDlgItemMessage(hwndDlg, IDC_COMBO3, CB_ADDSTRING, 0, (LPARAM)"None");
      SendDlgItemMessage(hwndDlg, IDC_COMBO3, CB_SETITEMDATA, x, -1);
      n = GetNumMIDIOutputs();
      for (x = 0; x < n; x++)
      {
        char buf[512];
        if (GetMIDIOutputName(x, buf, sizeof(buf)))
        {
          int a = SendDlgItemMessage(hwndDlg, IDC_COMBO3, CB_ADDSTRING, 0, (LPARAM)buf);
          SendDlgItemMessage(hwndDlg, IDC_COMBO3, CB_SETITEMDATA, a, x);
          if (x == params[1])
            SendDlgItemMessage(hwndDlg, IDC_COMBO3, CB_SETCURSEL, a, 0);
        }
      }

      x = SendDlgItemMessage(hwndDlg, IDC_COMBO4, CB_ADDSTRING, 0, (LPARAM)"None");
      SendDlgItemMessage(hwndDlg, IDC_COMBO4, CB_SETITEMDATA, x, -1);
      for (const auto & preset : surface_presets)
      {
        int x = preset.first;
        int a = SendDlgItemMessage(hwndDlg, IDC_COMBO4, CB_ADDSTRING, 0, (LPARAM)preset.second->name().c_str());
        SendDlgItemMessage(hwndDlg, IDC_COMBO4, CB_SETITEMDATA, a, x);
        if (x == params[2])
          SendDlgItemMessage(hwndDlg, IDC_COMBO4, CB_SETCURSEL, a, 0);
      }
    }
    break;

    case WM_USER + 1024:
    {
      if (wParam > 1 && lParam)
      {
        char tmp[512];

        int midi_in = -1, midi_out = -1, preset = -1;
        int r = SendDlgItemMessage(hwndDlg, IDC_COMBO2, CB_GETCURSEL, 0, 0);
        if (r != CB_ERR)
          midi_in = SendDlgItemMessage(hwndDlg, IDC_COMBO2, CB_GETITEMDATA, r, 0);

        r = SendDlgItemMessage(hwndDlg, IDC_COMBO3, CB_GETCURSEL, 0, 0);
        if (r != CB_ERR)
          midi_out = SendDlgItemMessage(hwndDlg, IDC_COMBO3, CB_GETITEMDATA, r, 0);

        r = SendDlgItemMessage(hwndDlg, IDC_COMBO4, CB_GETCURSEL, 0, 0);
        if (r != CB_ERR)
          preset = SendDlgItemMessage(hwndDlg, IDC_COMBO4, CB_GETITEMDATA, r, 0);

        sprintf(tmp, "%d %d %d", midi_in, midi_out, preset);
        lstrcpyn((char *)lParam, tmp, wParam);
      }
    }
    break;
  }
  return 0;
}

static HWND configFunc(const char *type_string, HWND parent, const char *initConfigString)
{
  return CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_SURFACEEDIT_MCU), parent, dlgProc, (LPARAM)initConfigString);
}

reaper_csurf_reg_t generic_surface_control_reg =
{
  "GenericController",
  "Generic Surface Controller",
  createFunc,
  configFunc,
};
