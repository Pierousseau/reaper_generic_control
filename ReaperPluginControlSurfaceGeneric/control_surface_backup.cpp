/*
** reaper_csurf
** MCU support - Modified for generic controller surfaces such as Korg NanoKontrol 2 support by : Pierre Rousseau (May 2017)
** Copyright (C) 2006-2008 Cockos Incorporated
** License: LGPL.
*/


#include "control_surface_interface.h"
#include "helpers.h"
#include <WDL/ptrlist.h>


class ControlSurfaceGeneric;
static WDL_PtrList<ControlSurfaceGeneric> m_generic_control_list;
static bool g_csurf_mcpmode;
static int m_flipmode;
static int m_allmcus_bank_offset;


class ControlSurfaceGeneric : public IReaperControlSurface
{
  bool m_is_mcuex;
  int m_midi_in_dev, m_midi_out_dev;
  int m_offset, m_size;
  midi_Output *m_midiout;
  midi_Input *m_midiin;

  int m_vol_lastpos[256];
  int m_pan_lastpos[256];
  char m_mackie_lasttime[10];
  int m_mackie_lasttime_mode;
  int m_mackie_arrow_states;
  int m_mackie_modifiers;
  int m_button_states;
  int m_buttonstate_lastrun;

  unsigned char m_generic_control_jogstate;
  unsigned short int m_generic_control_keystate;
  unsigned short int m_generic_control_automode;

  char m_fader_touchstate[256];
  unsigned int m_pan_lasttouch[256];

  WDL_String descspace;
  char configtmp[1024];

  double m_mcu_meterpos[8];
  DWORD m_mcu_timedisp_lastforce, m_mcu_meter_lastrun;
  unsigned int m_state_lastrun;
  unsigned int m_frameupd_lastrun;

  void MCUReset()
  {
    memset(m_mackie_lasttime, 0, sizeof(m_mackie_lasttime));
    memset(m_fader_touchstate, 0, sizeof(m_fader_touchstate));
    memset(m_pan_lasttouch, 0, sizeof(m_pan_lasttouch));
    m_mackie_lasttime_mode = -1;
    m_mackie_modifiers = 0;
    m_state_lastrun = 0;
    m_generic_control_jogstate = 0;
    m_generic_control_automode = 40400;
    m_generic_control_keystate = 0;


    memset(m_vol_lastpos, 0xff, sizeof(m_vol_lastpos));
    memset(m_pan_lastpos, 0xff, sizeof(m_pan_lastpos));
    if (m_midiout) m_midiout->Send(0x90, 0x5e, 0, -1);
    if (m_midiout) m_midiout->Send(0x90, 0x5f, 0, -1);


    if (m_midiout)
    {
      UpdateMackieDisplay(0, " Generic Controller.....     ***REAPER***     .....Generic Controller  ", 72 * 2);

      int x;
      for (x = 0; x < 8; x++)
      {
        struct
        {
          MIDI_event_t evt;
          char data[9];
        }
        poo;
        poo.evt.frame_offset = 0;
        poo.evt.size = 9;
        poo.evt.midi_message[0] = 0xF0;
        poo.evt.midi_message[1] = 0x00;
        poo.evt.midi_message[2] = 0x00;
        poo.evt.midi_message[3] = 0x66;
        poo.evt.midi_message[4] = m_is_mcuex ? 0x15 : 0x14;
        poo.evt.midi_message[5] = 0x20;
        poo.evt.midi_message[6] = 0x00 + x;
        poo.evt.midi_message[7] = 0x03;
        poo.evt.midi_message[8] = 0xF7;
        Sleep(5);
        m_midiout->SendMsg(&poo.evt, -1);
      }
      Sleep(5);
      for (x = 0; x < 8; x++)
      {
        m_midiout->Send(0xD0, (x << 4) | 0xF, 0, -1);
      }
    }

  }


  void UpdateMackieDisplay(int pos, const char *text, int pad)
  {
    struct
    {
      MIDI_event_t evt;
      char data[512];
    }
    poo;
    poo.evt.frame_offset = 0;
    poo.evt.size = 0;
    poo.evt.midi_message[poo.evt.size++] = 0xF0;
    poo.evt.midi_message[poo.evt.size++] = 0x00;
    poo.evt.midi_message[poo.evt.size++] = 0x00;
    poo.evt.midi_message[poo.evt.size++] = 0x66;
    poo.evt.midi_message[poo.evt.size++] = m_is_mcuex ? 0x15 : 0x14;
    poo.evt.midi_message[poo.evt.size++] = 0x12;

    poo.evt.midi_message[poo.evt.size++] = pos;
    int l = strlen(text);
    if (pad<l)l = pad;
    if (l > 200)l = 200;

    int cnt = 0;
    while (cnt < l)
    {
      poo.evt.midi_message[poo.evt.size++] = *text++;
      cnt++;
    }
    while (cnt++<pad)  poo.evt.midi_message[poo.evt.size++] = ' ';
    poo.evt.midi_message[poo.evt.size++] = 0xF7;
    Sleep(5);
    m_midiout->SendMsg(&poo.evt, -1);
  }

  void OnMIDIEvent(MIDI_event_t *evt)
  {
#if 0
    char buf[512];
    sprintf(buf, "message %02x, %02x, %02x\n", evt->midi_message[0], evt->midi_message[1], evt->midi_message[2]);
    OutputDebugString(buf);
#endif

    unsigned char onResetMsg[] = { 0xf0,0x00,0x00,0x66,0x14,0x01,0x58,0x59,0x5a, };
    onResetMsg[4] = m_is_mcuex ? 0x15 : 0x14;

    if (evt->midi_message[0] == 0xf0 && evt->size >= sizeof(onResetMsg) && !memcmp(evt->midi_message, onResetMsg, sizeof(onResetMsg)))
    {
      // on reset
      MCUReset();
      TrackList_UpdateAllExternalSurfaces();
      return;
    }
    if ((evt->midi_message[0] & 0xf0) == 0xe0) // volume fader move
    {
      int tid = evt->midi_message[0] & 0xf;
      if (tid == 8) tid = 0; // master offset, master=0
      else tid += 1 + m_offset + m_allmcus_bank_offset;

      MediaTrack *tr = CSurf_TrackFromID(tid, g_csurf_mcpmode);

      if (tr)
      {
        if (m_flipmode)
        {
          CSurf_SetSurfacePan(tr, CSurf_OnPanChange(tr, int14ToPan(evt->midi_message[2], evt->midi_message[1]), false), NULL);
        }
        else
          CSurf_SetSurfaceVolume(tr, CSurf_OnVolumeChange(tr, int14ToVol(evt->midi_message[2], evt->midi_message[1]), false), NULL);
      }
    }
    else if ((evt->midi_message[0] & 0xf0) == 0xb0) // pan, jog wheel movement
    {
      if (evt->midi_message[1] >= 0x10 && evt->midi_message[1] < 0x18) // pan
      {
        int tid = evt->midi_message[1] - 0x10;

        m_pan_lasttouch[tid & 7] = timeGetTime();

        if (tid == 8) tid = 0; // adjust for master
        else tid += 1 + m_offset + m_allmcus_bank_offset;
        MediaTrack *tr = CSurf_TrackFromID(tid, g_csurf_mcpmode);
        if (tr)
        {
          double adj = (evt->midi_message[2] & 0x3f) / 31.0;
          if (evt->midi_message[2] & 0x40) adj = -adj;
          if (m_flipmode)
          {
            CSurf_SetSurfaceVolume(tr, CSurf_OnVolumeChange(tr, adj*11.0, true), NULL);
          }
          else
          {
            CSurf_SetSurfacePan(tr, CSurf_OnPanChange(tr, adj, true), NULL);
          }
        }
      }
      else if (evt->midi_message[1] == 0x3c) // jog wheel
      {
        if (m_generic_control_jogstate == 1)
        {
          if (evt->midi_message[2] == 0x41) SendMessage(g_hwnd, WM_COMMAND, 40112, 0); // V-Out Zoom
          else  if (evt->midi_message[2] == 0x01) SendMessage(g_hwnd, WM_COMMAND, 40111, 0); // V-In Zoom
        }
        else if (m_generic_control_jogstate == 2)
        {
          if (evt->midi_message[2] == 0x41) SendMessage(g_hwnd, WM_COMMAND, 40138, 0); // scroll-UP
          else  if (evt->midi_message[2] == 0x01) SendMessage(g_hwnd, WM_COMMAND, 40139, 0); // scroll-DN
        }
        else if (m_generic_control_jogstate == 3)
        {
          if (evt->midi_message[2] == 0x41) SendMessage(g_hwnd, WM_COMMAND, 40140, 0); // scroll-L
          else  if (evt->midi_message[2] == 0x01) SendMessage(g_hwnd, WM_COMMAND, 40141, 0); // scroll-R
        }
        else if (m_generic_control_jogstate == 4)
        {
          if (evt->midi_message[2] == 0x41) SendMessage(g_hwnd, WM_COMMAND, 1011, 0); // H-Out Zoom
          else  if (evt->midi_message[2] == 0x01) SendMessage(g_hwnd, WM_COMMAND, 1012, 0); // H-In Zoom
        }
        else if ((m_generic_control_jogstate == 5) && (m_generic_control_keystate & 2))
        {
          if (evt->midi_message[2] == 0x41) SendMessage(g_hwnd, WM_COMMAND, 40102, 0); // set time sel. left
          else  if (evt->midi_message[2] == 0x01) SendMessage(g_hwnd, WM_COMMAND, 40103, 0); // set time sel. right
        }
        else if (evt->midi_message[2] == 0x41)  CSurf_OnRew(0);
        else if (evt->midi_message[2] == 0x01)  CSurf_OnFwd(0);
      }

    }


    else if ((evt->midi_message[0] & 0xf0) == 0x90) // button pushes
    {
      int allow_passthru = 0;
      if (evt->midi_message[2] >= 0x40)
      {
        if (evt->midi_message[1] >= 0x2e && evt->midi_message[1] <= 0x31)
        {
          int maxfaderpos = 0;
          int movesize = 8;
          int x;
          for (x = 0; x < m_generic_control_list.GetSize(); x++)
          {
            ControlSurfaceGeneric *item = m_generic_control_list.Get(x);
            if (item)
            {
              if (item->m_offset + 8 > maxfaderpos)
                maxfaderpos = item->m_offset + 8;
            }
          }

          if (evt->midi_message[1] & 1) // increase by X
          {
            int msize = CSurf_NumTracks(g_csurf_mcpmode);
            if (movesize>1)
            {
              if (m_allmcus_bank_offset + maxfaderpos >= msize) return;
            }

            m_allmcus_bank_offset += movesize;

            if (m_allmcus_bank_offset >= msize) m_allmcus_bank_offset = msize - 1;
          }
          else
          {
            m_allmcus_bank_offset -= movesize;
            if (m_allmcus_bank_offset<0)m_allmcus_bank_offset = 0;
          }
          // update all of the sliders
          TrackList_UpdateAllExternalSurfaces();

          for (x = 0; x < m_generic_control_list.GetSize(); x++)
          {
            ControlSurfaceGeneric *item = m_generic_control_list.Get(x);
            if (item && !item->m_is_mcuex && item->m_midiout)
            {
              item->m_midiout->Send(0xB0, 0x40 + 11, '0' + (((m_allmcus_bank_offset + 1) / 10) % 10), -1);
              item->m_midiout->Send(0xB0, 0x40 + 10, '0' + ((m_allmcus_bank_offset + 1) % 10), -1);
            }
          }
        }

        else if (evt->midi_message[1] >= 0x20 && evt->midi_message[1] < 0x28) // pan knob push
        {
          int trackid = evt->midi_message[1] - 0x20;
          m_pan_lasttouch[trackid] = timeGetTime();

          trackid += 1 + m_allmcus_bank_offset + m_offset;


          MediaTrack *tr = CSurf_TrackFromID(trackid, g_csurf_mcpmode);
          if (tr)
          {
            if (m_flipmode)
            {
              CSurf_SetSurfaceVolume(tr, CSurf_OnVolumeChange(tr, 1.0, false), NULL);
            }
            else
            {
              CSurf_SetSurfacePan(tr, CSurf_OnPanChange(tr, 0.0, false), NULL);
            }
          }
        }
        else if (evt->midi_message[1] >= 0x00 && evt->midi_message[1] < 0x08) // rec arm push
        {
          int tid = evt->midi_message[1];
          tid += 1 + m_allmcus_bank_offset + m_offset;
          MediaTrack *tr = CSurf_TrackFromID(tid, g_csurf_mcpmode);
          if (tr)
            CSurf_OnRecArmChange(tr, -1);
        }
        else if (evt->midi_message[1] >= 0x08 && evt->midi_message[1] < 0x18) // mute/solopush
        {
          int tid = evt->midi_message[1] - 0x08;
          int ismute = (tid & 8);
          tid &= 7;
          tid += 1 + m_allmcus_bank_offset + m_offset;

          MediaTrack *tr = CSurf_TrackFromID(tid, g_csurf_mcpmode);
          if (tr)
          {
            if (ismute)
              CSurf_SetSurfaceMute(tr, CSurf_OnMuteChange(tr, -1), NULL);
            else
              CSurf_SetSurfaceSolo(tr, CSurf_OnSoloChange(tr, -1), NULL);
          }
        }
        else if (evt->midi_message[1] >= 0x18 && evt->midi_message[1] <= 0x1f) // sel push
        {
          int tid = evt->midi_message[1] - 0x18;
          tid &= 7;
          tid += 1 + m_allmcus_bank_offset + m_offset;
          MediaTrack *tr = CSurf_TrackFromID(tid, g_csurf_mcpmode);
          if (tr) CSurf_OnSelectedChange(tr, -1); // this will automatically update the surface
        }

        /*else if (evt->midi_message[1] == 0x33) // edit
        Note: same code generated as Shift+F4... boo! */

        else if (evt->midi_message[1] == 0x56) // loop
        {
          if ((m_generic_control_keystate & 2) && (GetSetRepeat(-1) == 0))
          {
            SendMessage(g_hwnd, WM_COMMAND, 40031, 0);//zoom time selection
          }
          SendMessage(g_hwnd, WM_COMMAND, 1068, 0);// loop
        }
        else if (evt->midi_message[1] == 0x53)// shift
        {
          (m_generic_control_keystate & 1) ? (m_generic_control_keystate ^= 1) : (m_generic_control_keystate |= 1);
        }
        if (evt->midi_message[1] == 0x2a)// PAN CH-PARAM
        {
          (m_generic_control_keystate & 8) ? m_generic_control_keystate ^= 8 : m_generic_control_keystate |= 8;
          if (m_generic_control_keystate ^ 8)
          {
            if (m_generic_control_keystate & 128) (m_generic_control_keystate ^= 128);
            if (m_generic_control_keystate & 64) (m_generic_control_keystate ^= 64);
            if (m_generic_control_keystate & 32) (m_generic_control_keystate ^= 32);
            if (m_generic_control_keystate & 16) (m_generic_control_keystate ^= 16);
          }
          if (m_midiout) m_midiout->Send(0x90, 0x2a, (m_generic_control_keystate & 8) ? 0x01 : 0, -1);
        }
        else if (evt->midi_message[1] == 0x2d) // send
        {
          SendMessage(g_hwnd, WM_COMMAND, 40293, 0); //track i/o
        }
        else if (evt->midi_message[1] == 0x50) // s-send
        {
          SendMessage(g_hwnd, WM_COMMAND, 40251, 0); // routing matrix
        }
        else if (evt->midi_message[1] == 0x2b) // s-plugin
        {
          SendMessage(g_hwnd, WM_COMMAND, 40291, 0);// track FX
        }
        else if (evt->midi_message[1] == 0x4c) // effect
        {
          (SendMessage(g_hwnd, WM_COMMAND, 40549, 0)); //show mixer inserts
          (SendMessage(g_hwnd, WM_COMMAND, 40557, 0)); //show mixer sends
        }
        else if (evt->midi_message[1] == 0x34) // name/value
        {
          SendMessage(g_hwnd, WM_COMMAND, m_generic_control_automode, 0);	// cycle automation modes
          (m_generic_control_automode <= 40403) ? (m_generic_control_automode++) : (m_generic_control_automode = 40400);
        }
        else if (evt->midi_message[1] == 0x28)// display ^
        {
          SendMessage(g_hwnd, WM_COMMAND, 1041, 0);// cycle track folder state
        }
        else if (evt->midi_message[1] == 0x29)// display v
        {
          SendMessage(g_hwnd, WM_COMMAND, 1042, 0);// cycle folder collapsed state
        }
        else if (evt->midi_message[1] == 0x2c) // eq-all
        {
          SendMessage(g_hwnd, WM_COMMAND, 40016, 0); //preferences
        }
        else if (evt->midi_message[1] == 0x5b) // <<
        {
          if (m_generic_control_keystate>15)
          {
            (m_generic_control_keystate & 1) ? SendMessage(g_hwnd, WM_COMMAND, 40376, 0) : SendMessage(g_hwnd, WM_COMMAND, 40416, 0);
          }	//prev.transient/select+move to prev.item
          else if (m_generic_control_keystate & 1)
          {
            (GetSetRepeat(-1) & 1) ? SendMessage(g_hwnd, WM_COMMAND, 40632, 0) : SendMessage(g_hwnd, WM_COMMAND, 40042, 0);
          }	//start of loop & proj.start
          else
          {
            SendMessage(g_hwnd, WM_COMMAND, 40646, 0);//rew next grid
          }
          if (m_generic_control_keystate & 1) (m_generic_control_keystate ^= 1);
        }
        else if (evt->midi_message[1] == 0x5c) // >>
        {
          if (m_generic_control_keystate>15)
          {
            SendMessage(g_hwnd, WM_COMMAND, (m_generic_control_keystate & 1) ? 40375 : 40417, 0);
          }	//next transient & sel.move.to next item
          else if (m_generic_control_keystate & 1)
          {
            SendMessage(g_hwnd, WM_COMMAND, (GetSetRepeat(-1) & 1) ? 40633 : 40043, 0);
          }	//end of loop & end of project
          else
          {
            SendMessage(g_hwnd, WM_COMMAND, 40647, 0); // fwd to next grid pos./loop-end
          }
          if (m_generic_control_keystate & 1) (m_generic_control_keystate ^= 1);
        }
        else if (evt->midi_message[1] == 0x59) // write
        {
          SendMessage(g_hwnd, WM_COMMAND, 40157, 0); //write marker
        }
        else if (evt->midi_message[1] == 0x58) // marker <
        {
          SendMessage(g_hwnd, WM_COMMAND, 40172, 0); // prev marker
        }
        else if (evt->midi_message[1] == 0x5a)// marker >
        {
          SendMessage(g_hwnd, WM_COMMAND, 40173, 0);// next marker
        }
        else if (evt->midi_message[1] == 0x46)// undo
        {
          SendMessage(g_hwnd, WM_COMMAND, 40029, 0); //undo
        }
        else if (evt->midi_message[1] == 0x47) // shift-undo
        {
          if (m_generic_control_keystate & 128)
          {
            SendMessage(g_hwnd, WM_COMMAND, 40640, 0);// remove item FX
          }
          if (m_generic_control_keystate & 64)
          {
            SendMessage(g_hwnd, WM_COMMAND, 40653, 0);// reset item pitch
          }
          if (m_generic_control_keystate & 32)
          {
            SendMessage(g_hwnd, WM_COMMAND, 40652, 0);// reset item rate
          }
          if (m_generic_control_keystate & 16)
          {
            SendMessage(g_hwnd, WM_COMMAND, 40415, 0);// reset selected envelope points to zero/centre
          }
          SendMessage(g_hwnd, WM_COMMAND, 40030, 0);//redo
        }
        else if (evt->midi_message[1] == 0x48) // save
        {
          (m_generic_control_keystate & 128) ? (SendMessage(g_hwnd, WM_COMMAND, 40392, 0)) : (SendMessage(g_hwnd, WM_COMMAND, 40026, 0));
          //save project or save track-template
        }
        else if (evt->midi_message[1] == 0x49) // shift-save
        {
          (m_generic_control_keystate & 128) ? (SendMessage(g_hwnd, WM_COMMAND, 40394, 0)) : (SendMessage(g_hwnd, WM_COMMAND, 40022, 0));
          //saves-as project or save project-template
        }
        else if (evt->midi_message[1] == 0x36) // f1
        {
          if (m_generic_control_keystate & 8)
          {
            (m_generic_control_keystate |= 128);
            if (m_midiout) m_midiout->Send(0x90, 0x2a, 0x7f, -1);
          }
          else SendMessage(g_hwnd, WM_COMMAND, 40001, 0); //add track
        }
        else if (evt->midi_message[1] == 0x4d) // shift-f1
        {
          SendMessage(g_hwnd, WM_COMMAND, 46000, 0); // add template-track
        }
        else if (evt->midi_message[1] == 0x37) // f2
        {
          if (m_generic_control_keystate & 8)
          {
            (m_generic_control_keystate |= 64);
            if (m_midiout) m_midiout->Send(0x90, 0x2a, 0x7f, -1);
          }
          else SendMessage(g_hwnd, WM_COMMAND, 40279, 0); // docker toggle
          if (m_generic_control_keystate & 1) (m_generic_control_keystate ^= 1);
        }
        else if (evt->midi_message[1] == 0x4e) // fshift-f2
        {
          SendMessage(g_hwnd, WM_COMMAND, 40078, 0); // mixer toggle
        }
        else if (evt->midi_message[1] == 0x38) // f3
        {
          if (m_generic_control_keystate & 8)
          {
            (m_generic_control_keystate |= 32);
            if (m_midiout) m_midiout->Send(0x90, 0x2a, 0x7f, -1);
          }
          else
          {
            SendMessage(g_hwnd, WM_COMMAND, (m_generic_control_keystate & 1) ? 40344 : 40298, 0);// toggle fx bypass track/all
          }
          if (m_generic_control_keystate & 1) (m_generic_control_keystate ^= 1);
        }
        else if (evt->midi_message[1] == 0x39) // f4
        {
          if (m_generic_control_keystate & 8)
          {
            (m_generic_control_keystate |= 16);
            if (m_midiout) m_midiout->Send(0x90, 0x2a, 0x7f, -1);
          }
          else {
            ((m_generic_control_keystate & 4) ? m_generic_control_keystate ^= 4 : m_generic_control_keystate |= 4);
            (m_generic_control_keystate & 4) ? SendMessage(g_hwnd, WM_COMMAND, 40490, 0) : SendMessage(g_hwnd, WM_COMMAND, 40491, 0);
          }
        }
        else if (evt->midi_message[1] == 0x33) // shift-f4 or Edit
        {
          SendMessage(g_hwnd, WM_COMMAND, 40495, 0); // rec-mon cycle
        }
        else if (evt->midi_message[1] == 0x3a) // f5
        {
          if (m_generic_control_keystate & 128)
          {
            SendMessage(g_hwnd, WM_COMMAND, (m_generic_control_keystate & 1) ? 40530 : 40529, 0);// sel./toggle items
          }
          else if (m_generic_control_keystate & 64)
          {
            SendMessage(g_hwnd, WM_COMMAND, (m_generic_control_keystate & 1) ? 40207 : 40206, 0);// pitch up/down cent
          }
          else if (m_generic_control_keystate & 32)
          {
            SendMessage(g_hwnd, WM_COMMAND, (m_generic_control_keystate & 1) ? 40520 : 40519, 0);// itemrate up/down 10cents
          }
          else if (m_generic_control_keystate & 16)
          {
            SendMessage(g_hwnd, WM_COMMAND, (m_generic_control_keystate & 1) ? 40052 : 40406, 0);// view vol env/toggle vol env active
          }
          else
          {
            SendMessage(g_hwnd, WM_COMMAND, (m_generic_control_keystate & 1) ? 40061 : 40196, 0);//split items at edit-play cursor/time sel.
          }
          if (m_generic_control_keystate & 1) (m_generic_control_keystate ^= 1);
        }
        else if (evt->midi_message[1] == 0x3b) // f6
        {
          if (m_generic_control_keystate & 128)
          {
            SendMessage(g_hwnd, WM_COMMAND, (m_generic_control_keystate & 1) ? 40639 : 40638, 0);// show item-take FX chain/duplicate item/take
          }
          else if (m_generic_control_keystate & 64)
          {
            SendMessage(g_hwnd, WM_COMMAND, (m_generic_control_keystate & 1) ? 40205 : 40204, 0);// pitch up/down semitone
          }
          else if (m_generic_control_keystate & 32)
          {
            SendMessage(g_hwnd, WM_COMMAND, (m_generic_control_keystate & 1) ? 40518 : 40517, 0);// itemrate up/down semitone
          }
          else if (m_generic_control_keystate & 16)
          {
            SendMessage(g_hwnd, WM_COMMAND, (m_generic_control_keystate & 1) ? 40053 : 40407, 0);//view pan env/toggle pan env. active
          }
          else
          {
            SendMessage(g_hwnd, WM_COMMAND, (m_generic_control_keystate & 1) ? 40202 : 40109, 0);// open pri./sec. editor
          }
          if (m_generic_control_keystate & 1) (m_generic_control_keystate ^= 1);
        }
        else if (evt->midi_message[1] == 0x3c) // f7
        {
          if (m_generic_control_keystate & 128)
          {
            SendMessage(g_hwnd, WM_COMMAND, (m_generic_control_keystate & 1) ? 40209 : 40361, 0);// apply track FX to item (mono O/P)/apply track fx to item (stereo O/P)
          }
          else if (m_generic_control_keystate & 64)
          {
            SendMessage(g_hwnd, WM_COMMAND, (m_generic_control_keystate & 1) ? 40516 : 40515, 0);// pitch up/down octave
          }
          else if (m_generic_control_keystate & 32)
          {
            SendMessage(g_hwnd, WM_COMMAND, (m_generic_control_keystate & 1) ? 40525 : 40524, 0);// playrate up/down 10cents
          }
          else if (m_generic_control_keystate & 16)
          {
            SendMessage(g_hwnd, WM_COMMAND, (m_generic_control_keystate & 1) ? 40050 : 40408, 0);// view pre-fx vol/pan envs
          }
          else
          {
            SendMessage(g_hwnd, WM_COMMAND, (m_generic_control_keystate & 1) ? 40642 : 40643, 0);// explode takes in order/place
          }
          if (m_generic_control_keystate & 1) (m_generic_control_keystate ^= 1);
        }
        else if (evt->midi_message[1] == 0x3d) // f8
        {
          if (m_generic_control_keystate & 128)
          {
            SendMessage(g_hwnd, WM_COMMAND, (m_generic_control_keystate & 1) ? 40011 : 40009, 0);// view item properties/source properties
          }
          else if (m_generic_control_keystate & 64)
          {
            SendMessage(g_hwnd, WM_COMMAND, (m_generic_control_keystate & 1) ? 40637 : 40377, 0);// view virt.midi-keyb./all input to vkb
          }
          else if (m_generic_control_keystate & 32)
          {
            SendMessage(g_hwnd, WM_COMMAND, (m_generic_control_keystate & 1) ? 40523 : 40522, 0);// transport playrate up/down semitone
          }
          else if (m_generic_control_keystate & 16)
          {
            SendMessage(g_hwnd, WM_COMMAND, (m_generic_control_keystate & 1) ? 40051 : 40409, 0);//view pre-fx pan env/toggle pre-fx vol.pan active
          }
          else
          {
            SendMessage(g_hwnd, WM_COMMAND, (m_generic_control_keystate & 1) ? 40011 : 40009, 0);// view item properties/source properties
          }
          if (m_generic_control_keystate & 1) (m_generic_control_keystate ^= 1);
        }
        else if (evt->midi_message[1] == 0x64) // zoom
        {
          (m_generic_control_keystate & 2) ? m_generic_control_keystate ^= 2 : m_generic_control_keystate |= 2;
          if (m_generic_control_keystate ^ 2) (m_generic_control_jogstate = 0);
          if (m_midiout) m_midiout->Send(0x90, 0x64, (m_generic_control_keystate & 2) ? 0x01 : 0, -1);
        }
        else if (evt->midi_message[1] == 0x60) // ^
        {
          if (m_generic_control_keystate & 2)
          {
            (m_generic_control_jogstate = 1);
            if (m_midiout) m_midiout->Send(0x90, 0x64, 0x7f, -1);
          }
          else if (m_generic_control_keystate & 128)		//mode f1
          {
            SendMessage(g_hwnd, WM_COMMAND, 40117, 0);	//Move items up one track
          }
          else SendMessage(g_hwnd, WM_COMMAND, (m_generic_control_keystate>15) ? 40418 : 40286, 0); //sel.&move to item in prev track
        }
        else if (evt->midi_message[1] == 0x61) // v
        {
          if (m_generic_control_keystate & 2)
          {
            (m_generic_control_jogstate = 2);
            if (m_midiout) m_midiout->Send(0x90, 0x64, 0x7f, -1); // mode led
          }
          else if (m_generic_control_keystate & 128)
          {
            SendMessage(g_hwnd, WM_COMMAND, 40118, 0);	//Move items down one track
          }
          else SendMessage(g_hwnd, WM_COMMAND, (m_generic_control_keystate>16) ? 40419 : 40285, 0);
        }		//sel.&move-to item in next track / next track
        else if (evt->midi_message[1] == 0x62) // <
        {
          if (m_generic_control_keystate & 2)
          {
            (m_generic_control_jogstate = 3);
            if (m_midiout) m_midiout->Send(0x90, 0x64, 0x7f, -1);
          }
          else SendMessage(g_hwnd, WM_COMMAND, (m_generic_control_keystate & 128) ? 40120 : 40416, 0);
        }		//Move items L/sel.move-to prev item	
        else if (evt->midi_message[1] == 0x63) // >
        {
          if (m_generic_control_keystate & 2)
          {
            (m_generic_control_jogstate = 4);
            if (m_midiout) m_midiout->Send(0x90, 0x64, 0x7f, -1);
          }
          else SendMessage(g_hwnd, WM_COMMAND, (m_generic_control_keystate & 128) ? 40119 : 40417, 0);
        }		//Move items R/sel.move-to next item
        else if (evt->midi_message[1] == 0x5f) // rec
        {
          SendMessage(g_hwnd, WM_COMMAND, 1013, 0);
        }
        else if (evt->midi_message[1] == 0x5e) // play
        {
          SendMessage(g_hwnd, WM_COMMAND, 1007, 0);
        }
        else if (evt->midi_message[1] == 0x5d) // stop
        {
          SendMessage(g_hwnd, WM_COMMAND, 1016, 0);
        }
        else if (evt->midi_message[1] == 0x65) // scrub button
        {
          (m_generic_control_jogstate != 5) ? m_generic_control_jogstate = 5 : m_generic_control_jogstate = 0;
          if (m_midiout) m_midiout->Send(0x90, 0x65, (m_generic_control_jogstate == 5) ? 0x7f : 0, -1);
        }

        else if (evt->midi_message[1] == 0x32) // flip button
        {
          m_flipmode = !m_flipmode;
          if (m_midiout) m_midiout->Send(0x90, 0x32, m_flipmode ? 1 : 0, -1);
          CSurf_ResetAllCachedVolPanStates();
          TrackList_UpdateAllExternalSurfaces();
        }
        else
        {
          allow_passthru = 1;
        }
      }
      else if (evt->midi_message[1] >= 0x68 && evt->midi_message[1] < 0x71) // touch state
      {
        m_fader_touchstate[evt->midi_message[1] - 0x68] = evt->midi_message[2]>0x40;
      }
      else if (allow_passthru && evt->midi_message[2] >= 0x40)
      {
        int a = evt->midi_message[1];
        MIDI_event_t evt = { 0,3,{ 0xbf - (m_mackie_modifiers & 15),a,0 } };
        kbd_OnMidiEvent(&evt, -1);
      }
    }
  }


public:

  ControlSurfaceGeneric(bool ismcuex, int offset, int size, int indev, int outdev, int *errStats)
  {
    m_generic_control_list.Add(this);

    m_is_mcuex = ismcuex;
    m_offset = offset;
    m_size = size;
    m_midi_in_dev = indev;
    m_midi_out_dev = outdev;


    // init locals
    int x;
    for (x = 0; x < sizeof(m_mcu_meterpos) / sizeof(m_mcu_meterpos[0]); x++)
      m_mcu_meterpos[x] = -100000.0;
    m_mcu_timedisp_lastforce = 0;
    m_mcu_meter_lastrun = 0;
    memset(m_fader_touchstate, 0, sizeof(m_fader_touchstate));
    memset(m_pan_lasttouch, 0, sizeof(m_pan_lasttouch));


    //create midi hardware access
    m_midiin = m_midi_in_dev >= 0 ? CreateMIDIInput(m_midi_in_dev) : NULL;
    m_midiout = m_midi_out_dev >= 0 ? CreateThreadedMIDIOutput(CreateMIDIOutput(m_midi_out_dev, false, NULL)) : NULL;

    if (errStats)
    {
      if (m_midi_in_dev >= 0 && !m_midiin) *errStats |= 1;
      if (m_midi_out_dev >= 0 && !m_midiout) *errStats |= 2;
    }

    MCUReset();

    if (m_midiin)
      m_midiin->start();

  }

  ~ControlSurfaceGeneric()
  {
    m_generic_control_list.Delete(m_generic_control_list.Find(this));
    if (m_midiout)
    {

#if 1 // reset MCU to stock!, fucko enable this in dist builds, maybe?
      struct
      {
        MIDI_event_t evt;
        char data[5];
      }
      poo;
      poo.evt.frame_offset = 0;
      poo.evt.size = 8;
      poo.evt.midi_message[0] = 0xF0;
      poo.evt.midi_message[1] = 0x00;
      poo.evt.midi_message[2] = 0x00;
      poo.evt.midi_message[3] = 0x66;
      poo.evt.midi_message[4] = m_is_mcuex ? 0x15 : 0x14;
      poo.evt.midi_message[5] = 0x08;
      poo.evt.midi_message[6] = 0x00;
      poo.evt.midi_message[7] = 0xF7;
      Sleep(5);
      m_midiout->SendMsg(&poo.evt, -1);
      Sleep(5);

#elif 0
      char bla[11] = { "          " };
      int x;
      for (x = 0; x < sizeof(bla) - 1; x++)
        m_midiout->Send(0xB0, 0x40 + x, bla[x], -1);
      UpdateMackieDisplay(0, "", 56 * 2);
#endif


    }
    delete m_midiout;
    delete m_midiin;
  }

  const char *GetTypeString() { return "generic_control"; }
  const char *GetDescString()
  {
    descspace.Set("Generic Controller");
    char tmp[512];
    sprintf(tmp, " (dev %d,%d)", m_midi_in_dev, m_midi_out_dev);
    descspace.Append(tmp);
    return descspace.Get();
  }
  const char *GetConfigString() // string of configuration data
  {
    sprintf(configtmp, "%d %d %d %d", m_offset, m_size, m_midi_in_dev, m_midi_out_dev);
    return configtmp;
  }

  void CloseNoReset()
  {
    delete m_midiout;
    delete m_midiin;
    m_midiout = 0;
    m_midiin = 0;
  }

  void Run()
  {
    DWORD now = timeGetTime();

    if (now >= m_frameupd_lastrun + (1000 / max((*g_config_csurf_rate), 1)) || now < m_frameupd_lastrun - 250)
    {
      m_frameupd_lastrun = now;

      if (m_midiout)
      {
        if (!m_is_mcuex)
        {
          double pp = (GetPlayState() & 1) ? GetPlayPosition() : GetCursorPosition();
          unsigned char bla[10];
          memset(bla, 0, sizeof(bla));
        }
        if (GetPlayState() & 1)
        {
          int x;
#define VU_BOTTOM 70
          double decay = 0.0;
          if (m_mcu_meter_lastrun)
          {
            decay = VU_BOTTOM * (double)(now - m_mcu_meter_lastrun) / (1.4*1000.0);            // they claim 1.8s for falloff but we'll underestimate
          }
          m_mcu_meter_lastrun = now;
          for (x = 0; x < 8; x++)
          {
            int idx = m_offset + m_allmcus_bank_offset + x + 1;
            MediaTrack *t;
            if ((t = CSurf_TrackFromID(idx, g_csurf_mcpmode)))
            {
              double pp = VAL2DB((Track_GetPeakInfo(t, 0) + Track_GetPeakInfo(t, 1)) * 0.5);

              if (m_mcu_meterpos[x] > -VU_BOTTOM * 2) m_mcu_meterpos[x] -= decay;

              if (pp < m_mcu_meterpos[x]) continue;
              m_mcu_meterpos[x] = pp;
              int v = 0xd; // 0xe turns on clip indicator, 0xf turns it off
              if (pp < 0.0)
              {
                if (pp < -VU_BOTTOM)
                  v = 0x0;
                else v = (int)((pp + VU_BOTTOM)*13.0 / VU_BOTTOM);
              }

              m_midiout->Send(0xD0, (x << 4) | v, 0, -1);
            }
          }
        }

      }
    }

    if (m_midiin)
    {
      m_midiin->SwapBufs(timeGetTime());
      int l = 0;
      MIDI_eventlist *list = m_midiin->GetReadBuf();
      MIDI_event_t *evts;
      while ((evts = list->EnumItems(&l))) OnMIDIEvent(evts);
    }
  }

  void SetTrackListChange()
  {
    if (m_midiout)
    {
      int x;
      for (x = 0; x < 8; x++)
      {
        MediaTrack *t = CSurf_TrackFromID(x + m_offset + m_allmcus_bank_offset + 1, g_csurf_mcpmode);
        if (!t || t == CSurf_TrackFromID(0, false))
        {
          // clear item
          int panint = m_flipmode ? panToInt14(0.0) : volToInt14(0.0);
          unsigned char volch = m_flipmode ? volToChar(0.0) : panToChar(0.0);

          m_midiout->Send(0xe0 + (x & 0xf), panint & 0x7f, (panint >> 7) & 0x7f, -1);
          m_midiout->Send(0xb0, 0x30 + (x & 0xf), 1 + ((volch * 11) >> 7), -1);
          m_vol_lastpos[x] = panint;


          m_midiout->Send(0x90, 0x10 + (x & 7), 0, -1); // reset mute
          m_midiout->Send(0x90, 0x18 + (x & 7), 0, -1); // reset selected

          m_midiout->Send(0x90, 0x08 + (x & 7), 0, -1); //reset solo
          m_midiout->Send(0x90, 0x0 + (x & 7), 0, -1); // reset recarm

          char buf[7] = { 0, };
          UpdateMackieDisplay(x * 7, buf, 7); // clear display

          struct
          {
            MIDI_event_t evt;
            char data[9];
          }
          poo;
          poo.evt.frame_offset = 0;
          poo.evt.size = 9;
          poo.evt.midi_message[0] = 0xF0;
          poo.evt.midi_message[1] = 0x00;
          poo.evt.midi_message[2] = 0x00;
          poo.evt.midi_message[3] = 0x66;
          poo.evt.midi_message[4] = m_is_mcuex ? 0x15 : 0x14;
          poo.evt.midi_message[5] = 0x20;
          poo.evt.midi_message[6] = 0x00 + x;
          poo.evt.midi_message[7] = 0x03;
          poo.evt.midi_message[8] = 0xF7;
          Sleep(5);
          m_midiout->SendMsg(&poo.evt, -1);
          Sleep(5);
          m_midiout->Send(0xD0, (x << 4) | 0xF, 0, -1);
        }
      }
    }
  }
#define FIXID(id) int id=CSurf_TrackToID(trackid,g_csurf_mcpmode); int oid=id; \
  if (id>0) { id -= m_offset+m_allmcus_bank_offset+1; if (id==8) id=-1; } else if (id==0) id=8; 

  void SetSurfaceVolume(MediaTrack *trackid, double volume)
  {
    FIXID(id)
      if (m_midiout && id >= 0 && id < 256 && id < m_size)
      {
        if (m_flipmode)
        {
          unsigned char volch = volToChar(volume);
          if (id<8)
            m_midiout->Send(0xb0, 0x30 + (id & 0xf), 1 + ((volch * 11) >> 7), -1);
        }
        else
        {
          int volint = volToInt14(volume);

          if (m_vol_lastpos[id] != volint)
          {
            m_vol_lastpos[id] = volint;
            m_midiout->Send(0xe0 + (id & 0xf), volint & 0x7f, (volint >> 7) & 0x7f, -1);
          }
        }
      }
  }

  void SetSurfacePan(MediaTrack *trackid, double pan)
  {
    FIXID(id)
      if (m_midiout && id >= 0 && id < 256 && id < m_size)
      {
        unsigned char panch = panToChar(pan);
        if (m_pan_lastpos[id] != panch)
        {
          m_pan_lastpos[id] = panch;

          if (m_flipmode)
          {
            int panint = panToInt14(pan);
            if (m_vol_lastpos[id] != panint)
            {
              m_vol_lastpos[id] = panint;
              m_midiout->Send(0xe0 + (id & 0xf), panint & 0x7f, (panint >> 7) & 0x7f, -1);
            }
          }
          else
          {
            if (id<8)
              m_midiout->Send(0xb0, 0x30 + (id & 0xf), 1 + ((panch * 11) >> 7), -1);
          }
        }
      }
  }

  void SetSurfaceMute(MediaTrack *trackid, bool mute)
  {
    FIXID(id)
      if (m_midiout && id >= 0 && id < 256 && id < m_size)
      {
        if (id<8)
          m_midiout->Send(0x90, 0x10 + (id & 7), mute ? 0x7f : 0, -1);
      }
  }

  void SetSurfaceSelected(MediaTrack *trackid, bool selected)
  {
    FIXID(id)
      if (m_midiout && id >= 0 && id < 256 && id < m_size)
      {
        if (id<8)
          m_midiout->Send(0x90, 0x18 + (id & 7), selected ? 0x7f : 0, -1);
      }
  }

  void SetSurfaceSolo(MediaTrack *trackid, bool solo)
  {
    FIXID(id)
      if (m_midiout && id >= 0 && id < 256 && id < m_size)
      {
        if (id < 8)
          m_midiout->Send(0x90, 0x08 + (id & 7), solo ? 1 : 0, -1); //blink
        else if (id == 8)
          m_midiout->Send(0x90, 0x73, solo ? 1 : 0, -1);
      }
  }

  void SetSurfaceRecArm(MediaTrack *trackid, bool recarm)
  {
    FIXID(id)
      if (m_midiout && id >= 0 && id < 256 && id < m_size)
      {
        if (id < 8)
        {
          m_midiout->Send(0x90, 0x0 + (id & 7), recarm ? 0x7f : 0, -1);
        }
      }
  }

  void SetPlayState(bool play, bool pause, bool rec)
  {
    if (m_midiout)
    {
      m_midiout->Send(0x90, 0x5f, rec ? 0x7f : 0, -1);
      m_midiout->Send(0x90, 0x5e, play ? 0x7f : 0, -1);
      m_midiout->Send(0x90, 0x5d, pause ? 0x7f : 0, -1);
    }
  }

  void SetRepeatState(bool rep)
  {
    if (m_midiout)
    {
      m_midiout->Send(0x90, 0x56, rep ? 0x7f : 0, -1);
    }
  }

  void SetTrackTitle(MediaTrack *trackid, const char *title)
  {
    FIXID(id)
      if (m_midiout && id >= 0 && id < 8)
      {
        char buf[7];
        memcpy(buf, title, 6);
        buf[6] = 0;
        UpdateMackieDisplay(id * 7, buf, 7);
      }
  }

  bool GetTouchState(MediaTrack *trackid, int isPan)
  {
    FIXID(id)
      if (!m_flipmode != !isPan)
      {
        if (id >= 0 && id < 8)
        {
          DWORD now = timeGetTime();
          if (m_pan_lasttouch[id] == 1 || (now<m_pan_lasttouch[id] + 3000 && now >= m_pan_lasttouch[id] - 1000)) // fake touch, go for 3s after last movement
          {
            return true;
          }
        }
        return false;
      }
    if (id >= 0 && id < 9)
      return !!m_fader_touchstate[id];

    return false;
  }

  void ResetCachedVolPanStates()
  {
    memset(m_vol_lastpos, 0xff, sizeof(m_vol_lastpos));
    memset(m_pan_lastpos, 0xff, sizeof(m_pan_lastpos));
  }

  void OnTrackSelection(MediaTrack *trackid)
  {
    int tid = CSurf_TrackToID(trackid, g_csurf_mcpmode);
    // if no normal MCU's here, then slave it
    int x;
    int movesize = 8;
    for (x = 0; x < m_generic_control_list.GetSize(); x++)
    {
      ControlSurfaceGeneric *item = m_generic_control_list.Get(x);
      if (item)
      {
        if (item->m_offset + 8 > movesize)
          movesize = item->m_offset + 8;
      }
    }

    int newpos = tid - 1;
    if (newpos >= 0 && (newpos < m_allmcus_bank_offset || newpos >= m_allmcus_bank_offset + movesize))
    {
      int no = newpos - (newpos % movesize);

      if (no != m_allmcus_bank_offset)
      {
        m_allmcus_bank_offset = no;
        // update all of the sliders
        TrackList_UpdateAllExternalSurfaces();
        for (x = 0; x < m_generic_control_list.GetSize(); x++)
        {
          ControlSurfaceGeneric *item = m_generic_control_list.Get(x);
          if (item && !item->m_is_mcuex && item->m_midiout)
          {
            item->m_midiout->Send(0xB0, 0x40 + 11, '0' + (((m_allmcus_bank_offset + 1) / 10) % 10), -1);
            item->m_midiout->Send(0xB0, 0x40 + 10, '0' + ((m_allmcus_bank_offset + 1) % 10), -1);
          }
        }
      }
    }
  }

};

static void parseParms(const char *str, int parms[4])
{
  parms[0] = 0;
  parms[1] = 9;
  parms[2] = parms[3] = -1;

  const char *p = str;
  if (p)
  {
    int x = 0;
    while (x<4)
    {
      while (*p == ' ') p++;
      if ((*p < '0' || *p > '9') && *p != '-') break;
      parms[x++] = atoi(p);
      while (*p && *p != ' ') p++;
    }
  }
}

static IReaperControlSurface *createFunc(const char *type_string, const char *configString, int *errStats)
{
  int parms[4];
  parseParms(configString, parms);

  return new ControlSurfaceGeneric(!strcmp(type_string, "MCUEX"), parms[0], parms[1], parms[2], parms[3], errStats);
}


static WDL_DLGRET dlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
  case WM_INITDIALOG:
  {
    int parms[4];
    parseParms((const char *)lParam, parms);

    int n = GetNumMIDIInputs();
    int x = SendDlgItemMessage(hwndDlg, IDC_COMBO2, CB_ADDSTRING, 0, (LPARAM)"None");
    SendDlgItemMessage(hwndDlg, IDC_COMBO2, CB_SETITEMDATA, x, -1);
    x = SendDlgItemMessage(hwndDlg, IDC_COMBO3, CB_ADDSTRING, 0, (LPARAM)"None");
    SendDlgItemMessage(hwndDlg, IDC_COMBO3, CB_SETITEMDATA, x, -1);
    for (x = 0; x < n; x++)
    {
      char buf[512];
      if (GetMIDIInputName(x, buf, sizeof(buf)))
      {
        int a = SendDlgItemMessage(hwndDlg, IDC_COMBO2, CB_ADDSTRING, 0, (LPARAM)buf);
        SendDlgItemMessage(hwndDlg, IDC_COMBO2, CB_SETITEMDATA, a, x);
        if (x == parms[2]) SendDlgItemMessage(hwndDlg, IDC_COMBO2, CB_SETCURSEL, a, 0);
      }
    }
    n = GetNumMIDIOutputs();
    for (x = 0; x < n; x++)
    {
      char buf[512];
      if (GetMIDIOutputName(x, buf, sizeof(buf)))
      {
        int a = SendDlgItemMessage(hwndDlg, IDC_COMBO3, CB_ADDSTRING, 0, (LPARAM)buf);
        SendDlgItemMessage(hwndDlg, IDC_COMBO3, CB_SETITEMDATA, a, x);
        if (x == parms[3]) SendDlgItemMessage(hwndDlg, IDC_COMBO3, CB_SETCURSEL, a, 0);
      }
    }
    SetDlgItemInt(hwndDlg, IDC_EDIT1, parms[0], TRUE);
    SetDlgItemInt(hwndDlg, IDC_EDIT2, parms[1], FALSE);
  }
  break;
  case WM_USER + 1024:
    if (wParam > 1 && lParam)
    {
      char tmp[512];

      int indev = -1, outdev = -1, offs = 0, size = 9;
      int r = SendDlgItemMessage(hwndDlg, IDC_COMBO2, CB_GETCURSEL, 0, 0);
      if (r != CB_ERR) indev = SendDlgItemMessage(hwndDlg, IDC_COMBO2, CB_GETITEMDATA, r, 0);
      r = SendDlgItemMessage(hwndDlg, IDC_COMBO3, CB_GETCURSEL, 0, 0);
      if (r != CB_ERR) outdev = SendDlgItemMessage(hwndDlg, IDC_COMBO3, CB_GETITEMDATA, r, 0);

      BOOL t;
      r = GetDlgItemInt(hwndDlg, IDC_EDIT1, &t, TRUE);
      if (t) offs = r;
      r = GetDlgItemInt(hwndDlg, IDC_EDIT2, &t, FALSE);
      if (t)
      {
        if (r<1)r = 1;
        else if (r>256)r = 256;
        size = r;
      }

      sprintf(tmp, "%d %d %d %d", offs, size, indev, outdev);
      lstrcpyn((char *)lParam, tmp, wParam);
    }
    break;
  }
  return 0;
}

static HWND configFunc(const char *type_string, HWND parent, const char *initConfigString)
{
  return CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_SURFACEEDIT_MCU), parent, dlgProc, (LPARAM)initConfigString);
}


reaper_csurf_reg_t control_surface_generic_reg =
{
  "Generic Controller",
  "Generic Controller",
  createFunc,
  configFunc,
};
