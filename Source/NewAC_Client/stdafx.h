// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#ifndef __STDAFX_H
#define __STDAFX_H
#if _MSC_VER > 1000
#pragma once
#endif
#ifdef __BORLANDC__
#define _countof(x) sizeof(x) / sizeof(x[0])
#endif

#define STRICT
#pragma warning(disable: 4100)  // unreferenced formal parameter
//#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
//#define _SECURE_SCL 1
#define _SECURE_SCL 0
// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif

//#ifndef _WIN32_WINDOWS	// Allow use of features specific to Windows 98 or later.
//#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
//#endif

#ifndef _WIN32_IE		// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif
//#define _SECURE_SCL 1
//#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

//#include <Wingdi.h>
#include <windowsx.h>
#include <commctrl.h>
//#include <richedit.h>
#include <Commdlg.h>
#include <prsht.h>
#include <tchar.h>
#include <shlobj.h>
//#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <map>
//#include "SciLexer.h"
//#include "Scintilla.h"

//#include <algorithm>
//#include <process.h>
//#define NDEBUG //directive ("no debugging") in the source code before the #include <assert.h> directive, the effect is to comment out the assert statement.
//#include <cstring>
//#include <cstdio>
#include <cassert>
#include <exception>
//#include <strstream.h>
#include <sstream>
using namespace std;

enum { CONSOLEAPP = 1, WINDOWAPP, DLLAPP, STATICLIBAPP, WXWIDGETSAPP };
enum { iSource = 1, iHeader, iResource, iOthers };
enum { CPLUSPLUS = 1, CLANGUAGE };
enum { SDIAPP = 1, MDIAPP, DLGAPP };
enum { WXDLL = 1, WXSTATIC, WXMONOLITHIC };

typedef struct tagWizdata
{
    int iType; //Console =1 Windows = 2
    int iLanguage; //C++ = 1, C = 2
    BOOL bEmpty;
    int WinSubType; //SDI, MDI, DLGBASE
    int WxSubType;  // WXDLL = 1, WXSTATIC, WXMONOLITHIC};
} WIZDATA, * LPWIZDATA;


class AppData
{
public:
    AppData();
    void RebuildDirectories(void);
    string CPPEXT;
    string CEXT;
    string RESEXT;
    string PROJECTEXT;
    string EXEEXT;
    string HEADEREXT;
    string TEXTEXT;
    string DLLEXT;
    string LIBEXT;
    BOOL bManager;
    BOOL bMessages;
    BOOL bAutoHideMessages;
    RECT rc;
    RECT rec;
    HINSTANCE hInst;
    int cxClient;      //initialized at WM_SIZE
    int cyClient;
    int iStatusHeight; //height of status bar
    int iToolHeight;   //height of Rebar window
    //initialized at WM_SIZE
    int g_tabsize;
    int splitsize;//width of splitter windows
    string szProjectFile; //path and filename of project file
    string CurTitle;  //hols the name without path of file in editor window
    string szExecutableFile;

    string szCompilerPath;// = TEXT("c:\\Borland\\bcc55\\bin\\bcc32.exe"); //Path to compilerstatic
    string szResourcePath;// = TEXT("c:\\Borland\\bcc55\\bin\\brcc32.exe");// Path to resource compiler
    string szLinkerPath;// = TEXT("c:\\Borland\\bcc55\\bin\\ilink32.exe"); // path to linker
    string psdkPath;//     = TEXT("c:\\Borland\\bcc55\\lib\\psdk");        //lib path
    string IncludePath;//  = TEXT("c:\\Borland\\bcc55\\include");           //include path
    string LibPath;//      = TEXT("c:\\Borland\\bcc55\\lib");
    string CompilerArgs; //these are the strings passed to the compiler, linker and resource compiler
    string ResourceArgs;
    string LinkerArgs;
    string g_CurrentDir; // for holding the current directory or project file directory
    string addlibraries; //place to add aditional libraries to the linker
    string szProjectName; //name of project without path
    BOOL bCancelWizard; //TRUE if user cancel the Wizard for creating a project
    BOOL bProjectModified; //if you add files to a project this global become true
    BOOL bModify;
    BOOL g_bCompilerStatus; //compiler strings have been or not initialized
    BOOL bColorsModify;
    int iErrors;
    int iWarnings;
    BOOL brunning;
    BOOL bCompiled;
    char g_szAppName[80]; //Title name for main Window
    HWND hActiveEditor; //Main Edit Window
    HWND hwndTV; //Lower TreeView Control Window (Handles the project files)
    HWND hTabWnd; //Tab Control window
    HWND hwndOpenTV;  //Upper TreeView Control handle the open files (allocated memory)
    HWND hTabDebug;//Compiler Error Messages Tab Window
    HWND hDebug; //Compiler Error Messages readonly Editor window
    HWND hwndProgressDlg;
    HWND hListView;
    HWND hComboFunctions;
    HWND hwndMain; //Main Window process the main window messages
    HWND hWndStatusbar;
    HWND hTreeSplitter;  //Horizontal Splitter that divides the TreeView Controls
    HWND hHorzSplitter; //horizontal splitter that divides tab controls
    HWND hVertSplitter; //Vertical splitter window that divides the TreeView Controls and Tab Control
    HWND hwndSelected;
    vector <string> VectorSource; //holds source files names(*.cpp or *.c) //holds source files names(*.cpp or *.c)
    vector <string> VectorHeader; // holds headers files (*.h) //
    vector <string> VectorOther;  // holds others files actually only text files (*.txt)
    vector <string> VectorResource;
    map<string, HWND > Mem_map;
    map <string, char> file_map;
    map <string, string> dir_map;
    map <string, int> proto_map;
    HTREEITEM hRoot;
    HTREEITEM hSource;
    HTREEITEM hHeader;
    HTREEITEM hResource;
    HTREEITEM hOther;//root and childs of hwndTV Control
};
//using std::goodbit;



// C RunTime Header Files
//#include <cstdlib>
//#include <process.h>
//#include <malloc.h>
//#include <memory>
//#include <tchar.h>


// TODO: reference additional headers your program requires here

#endif