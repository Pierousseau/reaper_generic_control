/*
** A generator for MIDI controller surface preset files, to be used in reaper_plugin_control_surface_generic Reaper plugin
** Copyright (c) 2017 Pierre Rousseau
** https://github.com/Pierousseau/reaper_generic_control
** License: LGPL.
**
** Code in the present file uses RapidJson, but does not use the Reaper SDK in any way.
*/


#include <Windows.h>
#include <mmsystem.h>
#include <conio.h>

#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <vector>

#ifdef max
#undef min
#undef max
#endif

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>


std::string getExecutableDirPath()
{
#ifdef _WIN32
  char buffer[_MAX_PATH];
  DWORD length = GetModuleFileNameA(0, buffer, sizeof(buffer));
  bool status = ((length > 0) && (length < sizeof(buffer)));
  if (status == false)
    return std::string();
  std::string path = buffer;
  std::string::size_type pos = path.find_last_of("\\/");
  return path.substr(0, pos) + "\\";
#endif
}

void CALLBACK midiInCallback(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

class MidiInputMapper
{
public:
  MidiInputMapper()
    : _h_midi_device(NULL)
    , _midi_port(0)
    , _status(MIDI_IGNORE)
    , _input_id(0)
  {
  }

  ~MidiInputMapper()
  {
    if (_h_midi_device)
      midiInClose(_h_midi_device);
    _h_midi_device = NULL;
  }

  bool init()
  {
    unsigned int count = midiInGetNumDevs();
    if (count == 0)
    {
      std::cout << "No MIDI Devices found" << std::endl;
      return false;
    }

    std::cout << "Found MIDI Devices :" << std::endl;
    for (unsigned int i = 0; i < count; ++i)
    {
      MIDIINCAPS caps;
      midiInGetDevCaps(i, &caps, sizeof(MIDIINCAPS));
      std::cout << "  " << i << ": " << caps.szPname << std::endl;
    }
    std::cout << std::endl;

    MMRESULT rv;
    rv = midiInOpen(&_h_midi_device, _midi_port, (DWORD_PTR)&midiInCallback, (DWORD_PTR)this, CALLBACK_FUNCTION);
    if (rv == MMSYSERR_NOERROR)
      return true;

    std::cout << "Failed to open MIDI port." << std::endl;
    return false;
  }

  void midiIn(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
  {
    if (_status != MIDI_GRAB)
      return;

    if (wMsg == MIM_DATA)
    {
      _input_id = (dwParam1 >> 8) & 0xFF;
      _status = MIDI_GRABBED;
    }
  }

  bool getInput(const std::string & caption, const std::string & label)
  {
    std::cout << "Please " << caption << " now (or Esc to cancel)" << std::endl;

    midiInStart(_h_midi_device);
    _status = MIDI_GRAB;

    while (true)
    {
      if (_status != MIDI_GRAB)
        break;

      if (!_kbhit())
      {
        Sleep(100);
        continue;
      }

      if (_getch() == VK_ESCAPE)
        break;
    }
    midiInStop(_h_midi_device);

    if (_status == MIDI_GRABBED)
    {
      std::cout << label << " has code " << (int)_input_id << ". Confim ? (Y/N)" << std::endl;
      char answer = _getch();
      if (answer == 'y' || answer == 'Y')
      {
        return true;
      }
      return getInput(caption, label);
    }

    _status = MIDI_IGNORE;
    return false;
  }

  void learnButton(const std::string & button)
  {
    bool ok = getInput("press the " + button + " button", button + " button");
    if (ok)
      _buttons[button] = _input_id;
  }

  void learnTrackButton(const std::string & button, int track)
  {
    bool ok = getInput("press the " + button + " button for track " + std::to_string(track), button + "[" + std::to_string(track) + "] button");
    if (ok)
      _tracks[track][button] = _input_id;
  }

  void learnTrackKnob(const std::string & knob, int track)
  {
    bool ok = getInput("move the " + knob + " knob for track " + std::to_string(track), knob + "[" + std::to_string(track) + "] knob");
    if (ok)
      _tracks[track][knob] = _input_id;
  }

  void learnMasterButton(const std::string & button)
  {
    bool ok = getInput("press the " + button + " button for master", "master " + button + " button");
    if (ok)
      _master[button] = _input_id;
  }

  void learnMasterKnob(const std::string & knob)
  {
    bool ok = getInput("move the " + knob + " knob for master", "master " + knob + " knob");
    if (ok)
      _master[knob] = _input_id;
  }

  void learn()
  {
    std::cout << "What is the name of your control surface ?" << std::endl;
    std::cin >> _surface_name;

    std::cout << "Does your control surface have transport buttons ? (Y/N)" << std::endl;
    char answer = _getch();
    if (answer == 'y' || answer == 'Y')
    {
      learnButton("Rewind");
      learnButton("Forward");
      learnButton("Stop");
      learnButton("Play");
      learnButton("Record");
      learnButton("Cycle");

      learnButton("Marker Set");
      learnButton("Marker Previous");
      learnButton("Marker Next");
    }

    std::cout << "Does your control surface have master track controls ? (Y/N)" << std::endl;
    answer = _getch();
    if (answer == 'y' || answer == 'Y')
    {
      learnMasterButton("Solo");
      learnMasterButton("Mute");
      learnMasterButton("Record");
      learnMasterKnob("Volume");
      learnMasterKnob("Pan");
    }

    std::cout << std::endl << "How many tracks can your control surface handle ?" << std::endl;
    int track_count;
    std::cin >> track_count;
    if (track_count > 0)
      _tracks.resize(track_count);
    for (int i = 0 ; i < track_count ; i++)
    {
      learnTrackButton("Solo", i);
      learnTrackButton("Mute", i);
      learnTrackButton("Record", i);
      learnTrackKnob("Volume", i);
      learnTrackKnob("Pan", i);
    }
  }

  void dump()
  {
    std::cout << "Here are your inputs for control surface \"" << _surface_name << "\":" << std::endl;
    for (auto & button : _buttons)
      std::cout << button.first << " has code " << button.second << std::endl;

    for (auto & button : _master)
      std::cout << "Master " << button.first << " has code " << button.second << std::endl;

    int i = 0;
    for (auto & track : _tracks)
    {
      for (auto & button : track)
        std::cout << button.first << "[" << i << "] has code " << button.second << std::endl;
      i++;
    }
    std::cout << std::endl;
  }

  rapidjson::Value jsonString(const std::string & input, rapidjson::MemoryPoolAllocator< > & alloc)
  {
    if (input.empty())
      return rapidjson::Value("");
    return rapidjson::Value(input.c_str(), alloc);
  }

  rapidjson::Value serializeTransport(rapidjson::MemoryPoolAllocator< > & alloc)
  {
    rapidjson::Value result(rapidjson::kObjectType);
    for (const auto & button : _buttons)
      result.AddMember(jsonString(button.first, alloc), button.second, alloc);
    return result;
  }

  rapidjson::Value serializeTracks(rapidjson::MemoryPoolAllocator< > & alloc)
  {
    rapidjson::Value result(rapidjson::kArrayType);
    for (const auto & track : _tracks)
    {
      rapidjson::Value track_json(rapidjson::kObjectType);
      for (const auto & button : track)
        track_json.AddMember(jsonString(button.first, alloc), button.second, alloc);
      result.PushBack(track_json, alloc);
    }
    return result;
  }

  rapidjson::Value serializeMaster(rapidjson::MemoryPoolAllocator< > & alloc)
  {
    rapidjson::Value result(rapidjson::kObjectType);
    for (const auto & button : _master)
      result.AddMember(jsonString(button.first, alloc), button.second, alloc);
    return result;
  }

  void serialize()
  {
    std::string default_filename, input_filename, dummy;
    default_filename = getExecutableDirPath() + _surface_name + ".json";
    std::cout << "Ok, I'll save this mapping now. Please enter output file name (leave blank for \"" << default_filename << "\")" << std::endl;
    getline(std::cin, dummy);
    getline(std::cin, input_filename);
    if (input_filename.empty())
      input_filename = default_filename;
    
    rapidjson::Document doc;
    rapidjson::MemoryPoolAllocator< > & alloc = doc.GetAllocator();
    doc.SetObject();
    doc.AddMember("SurfaceName", jsonString(_surface_name, alloc), alloc);
    if (!_buttons.empty())
      doc.AddMember("Transport", serializeTransport(alloc), alloc);
    if (!_master.empty())
      doc.AddMember("Master", serializeMaster(alloc), alloc);
    if (!_tracks.empty())
      doc.AddMember("Tracks", serializeTracks(alloc), alloc);
    
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);

    std::ofstream output_file;
    output_file.open(input_filename);
    if (output_file.is_open())
      output_file << buffer.GetString();
    output_file.close();
  }

private:
  enum InputStatus { MIDI_IGNORE , MIDI_GRAB, MIDI_GRABBED };
  typedef std::map<std::string, int> ControlMap;

  HMIDIIN _h_midi_device = NULL;
  DWORD _midi_port = 0;

  InputStatus _status;
  unsigned char _input_id;

  std::string _surface_name;
  ControlMap _buttons;
  ControlMap _master;
  std::vector<ControlMap> _tracks;
};


void CALLBACK midiInCallback(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
  MidiInputMapper *input_mapper = (MidiInputMapper*)dwInstance;
  input_mapper->midiIn(hMidiIn, wMsg, dwParam1, dwParam2);
}


int main(int argc, char* argv[])
{
  MidiInputMapper input_mapper;
  if (input_mapper.init() == false)
    return -1;

  do
  {
    input_mapper.learn();

    std::cout << std::endl;
    input_mapper.dump();

    std::cout << "Is this correct ? (Y/N)" << std::endl;
    char answer = _getch();
    if (answer == 'y' || answer == 'Y')
      break;
  } while (1);

  input_mapper.serialize();

  std::cout << "Done, thanks (press any key to quit program)." << std::endl;
  _getch();

  return 0;
}