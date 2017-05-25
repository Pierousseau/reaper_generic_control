#ifdef _WIN32
#include <windows.h>
#else
#include "../../WDL/swell/swell.h"
#endif

#include <stdio.h>
#include <math.h>

#include "resource.h"

#include "../reaper_plugin.h"

#include "../../WDL/queue.h"
#include "../../WDL/jnetlib/jnetlib.h"
#include "../../WDL/jnetlib/httpget.h"

int g_registered_command=0;


REAPER_PLUGIN_HINSTANCE g_hInst;
int (*InsertMedia)(char *file, int mode); // mode: 0=add to current track, 1=add new track
HWND (*GetMainHwnd)();
void (*GetProjectPath)(char *buf, int bufsz);
gaccel_register_t acreg=
{
  {FALT|FVIRTKEY,'5',0},
  "Insert random NINJAM loop"
};



HWND g_parent;

WDL_DLGRET doInsertProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  static JNL_HTTPGet *m_get;
  static WDL_Queue m_buf;
  switch (uMsg)
  {
    case WM_INITDIALOG:
      m_get=new JNL_HTTPGet;
      m_get->connect("http://autosong.ninjam.com/autoloop/auto.php");
      m_buf.Clear();

      SetTimer(hwndDlg,1,20,NULL);
    return 0;
    case WM_TIMER:
      if (wParam==1)
      {
        int rv=m_get->run();
        if (rv>=0)
        {
          int av=m_get->bytes_available();
          if (av>0)
          {
            m_get->get_bytes((char*)m_buf.Add(NULL,av),av);
          }
        }

        if (rv==0)  return 0;

        KillTimer(hwndDlg,1);
        char *hdr=0;
        if (rv<0||m_buf.GetSize()<1||!(hdr=m_get->getheader("ex-fn"))||!*hdr||strstr(hdr,"/") || strstr(hdr,"\\"))
          MessageBox(hwndDlg,"Error getting a loop, internet ok?","Err",MB_OK);
        else
        {
          char buf[2048];
          GetProjectPath(buf,sizeof(buf)-1024);
          strcat(buf,"\\");
          lstrcpyn(buf+strlen(buf),hdr,512);
          FILE *fp=fopen(buf,"rb");
          if (fp)
          {
            fclose(fp);
            MessageBox(hwndDlg,"Error writing loop, file already exists!","Err",MB_OK);
          }
          else
          {
            fp=fopen(buf,"wb");
            if (!fp)
            {
              MessageBox(hwndDlg,"Error writing loop, could not create flie!","Err",MB_OK);
            }
            else
            {
              fwrite(m_buf.Get(),1,m_buf.GetSize(),fp);
              fclose(fp);

              InsertMedia(buf,0);
            }
          }
          // save to disk, insert loop
        }
        EndDialog(hwndDlg,0);

      }
    return 0;
    case WM_COMMAND:
      if (LOWORD(wParam)==IDCANCEL)
      {
        EndDialog(hwndDlg,0);
      }
    return 0;
    case WM_DESTROY:
      m_buf.Clear();
      delete m_get;
    return 0;
  }
  return 0;
}


void DoInsertPoo()
{
  static int icnt;
  if (!icnt)
  {
    JNL::open_socketlib();
    icnt++;
  }
  DialogBox(g_hInst,MAKEINTRESOURCE(IDD_CFG),g_parent,doInsertProc);
}

bool hookCommandProc(int command, int flag)
{
  if (g_registered_command && command == g_registered_command)
  {
    DoInsertPoo();
    return true;
  }
  return false;
}

extern "C"
{

REAPER_PLUGIN_DLL_EXPORT int REAPER_PLUGIN_ENTRYPOINT(REAPER_PLUGIN_HINSTANCE hInstance, reaper_plugin_info_t *rec)
{
  g_hInst=hInstance;
  if (rec)
  {
    if (rec->caller_version != REAPER_PLUGIN_VERSION || !rec->GetFunc)
      return 0;

    *((void **)&InsertMedia) = rec->GetFunc("InsertMedia");
    *((void **)&GetMainHwnd) = rec->GetFunc("GetMainHwnd");
    *((void **)&GetProjectPath) = rec->GetFunc("GetProjectPath");
    

    
    if (!InsertMedia || !GetMainHwnd||!GetProjectPath) return 0;
    acreg.accel.cmd = g_registered_command = rec->Register("command_id",(void*)"NINJAMLOOP");

    if (!g_registered_command) return 0; // failed getting a command id, fail!

    rec->Register("gaccel",&acreg);
    rec->Register("hookcommand",(void*)hookCommandProc);


    g_parent = GetMainHwnd();

    HMENU hMenu = GetSubMenu(GetMenu(GetMainHwnd()),
#ifdef _WIN32
							 5
#else // OS X has one extra menu
							 6
#endif
	  );
    MENUITEMINFO mi={sizeof(MENUITEMINFO),};
    mi.fMask = MIIM_TYPE | MIIM_ID;
    mi.fType = MFT_STRING;
    mi.dwTypeData = "Insert random NINJAM loop";
    mi.wID = g_registered_command;
    InsertMenuItem(hMenu, 6, TRUE, &mi); 

    // our plugin registered, return success

    return 1;
  }
  else
  {
    return 0;
  }
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