// -*- mode: c++; c-file-style: "linux"; c-basic-offset: 2; indent-tabs-mode: nil -*-
//  Copyright (C) 2004-2015 Andrej Vodopivec <andrej.vodopivec@gmail.com>
//            (C) 2013 Doug Ilijev <doug.ilijev@gmail.com>
//            (C) 2014-2015 Gunter Königsmann <wxMaxima@physikbuch.de>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#ifndef WXMAXIMA_H
#define WXMAXIMA_H

#include "wxMaximaFrame.h"
#include "MathParser.h"

#include <wx/socket.h>
#include <wx/config.h>
#include <wx/process.h>
#include <wx/regex.h>
#include <wx/html/htmlwin.h>
#include <wx/dnd.h>

#if defined (__WXMSW__)
 #include <wx/msw/helpchm.h>
#endif

#include <wx/html/helpctrl.h>

#define SOCKET_SIZE 1024
#define DOCUMENT_VERSION_MAJOR 1
/*! The part of the .wxmx format version number that appears after the dot.
  
  - Updated to version 1.1 after user selectable animation-speeds were introduced:
    Old wxMaxima versions play them back in the default speed instead but still
    open the file.
  - Bumped to version 1.2 after saving highlighting was introduced: Older versions
    of wxMaxima will ignore the highlighting on opening .wxmx files.
  - Bumped to version 1.3 after sub-subsections were introduced:
    Old wxMaxima versions interpret them as subsections but still open the file.
  - Bumped to version 1.4 when we started allowing to embed .jpg images in a .wxmx
    file. wxMaxima versions between 13.04 and 15.08 replace these images by a 
    "image cannot be loaded" marker but will correctly display the rest of the file.
 */
#define DOCUMENT_VERSION_MINOR 4


#ifndef __WXGTK__
class MyAboutDialog : public wxDialog
{
public:
  MyAboutDialog(wxWindow *parent, int id, const wxString title, wxString description);
  ~MyAboutDialog() {};
  void OnLinkClicked(wxHtmlLinkEvent& event);
  DECLARE_EVENT_TABLE()
};
#endif

/* The top-level window and the main application logic

 */
class wxMaxima : public wxMaximaFrame
{
public:
  void CleanUp();                                  //!< shuts down server and client on exit
  //! An enum of individual IDs for all timers this class handles
  enum TimerIDs
  {
    //! The keyboard was inactive long enough that we can attempt an auto-save.
    KEYBOARD_INACTIVITY_TIMER_ID, 
    //! The time between two auto-saves has elapsed.
    AUTO_SAVE_TIMER_ID,
    //! We look if we got new data from maxima's stdout.
    MAXIMA_STDOUT_POLL_ID
  };

  /*! A timer that determines when to do the next autosave;

    The actual autosave is triggered if both this timer is expired and the keyboard
    has been inactive for >10s so the autosave won't cause the application to shortly
    stop responding due to saving the file while the user is typing a word.

    This timer is used in one-shot mode so in the unikely case that saving needs more
    time than this timer to expire the user still got a chance to do something against
    it between two expirys. 
   */
  wxTimer m_autoSaveTimer;
  //! Has m_autoSaveTimer expired since the last save?
  bool m_autoSaveIntervalExpired;
  //! Is triggered when a timer this class is responsible for requires
  void OnTimerEvent(wxTimerEvent& event);
  //! A timer that polls for output from the maxima process.
  wxTimer m_maximaStdoutPollTimer;

  /*! The interval between auto-saves (in milliseconds). 

    Values <10000 mean: Auto-save is off.
  */
  long int m_autoSaveInterval;
  
  wxMaxima(wxWindow *parent, int id, const wxString title,
           const wxPoint pos, const wxSize size = wxDefaultSize);
  ~wxMaxima();
  void ShowTip(bool force);
  wxString GetHelpFile();
  void ShowMaximaHelp(wxString keyword = wxEmptyString);
  void ShowWxMaximaHelp();
  void InitSession();
  void SetOpenFile(wxString file)
  {
    m_openFile = file;
  }
  void SetBatchMode(bool batch = false)
  {
    m_batchmode = batch;
  }
  void StripComments(wxString& s);
  void SendMaxima(wxString s, bool history = false);
  void OpenFile(wxString file,
                wxString command = wxEmptyString); //!< Open a file
  bool DocumentSaved() { return m_fileSaved; }
  void LoadImage(wxString file) { m_console->OpenHCaret(file, GC_TYPE_IMAGE); }
private:
  //! On opening a new file we only need a new maxima process if the old one ever evaluated cells.
  bool m_hasEvaluatedCells;
  //! Searches for maxima's output prompts
  wxRegEx m_outputPromptRegEx;
  //! The number of output cells the current command has produced so far.
  int m_outputCellsFromCurrentCommand;
  //! The maximum number of lines per command we will display 
  int m_maxOutputCellsPerCommand;
  //! The number of consecutive unsucessfull attempts to connect to the maxima server
  int m_unsuccessfullConnectionAttempts;
  //! The current working directory maxima's file I/O is relative to.
  wxString m_CWD;
  //! Are we in batch mode?
  bool m_batchmode;
  //! Can we display the "ready" prompt right now?
  bool m_ready;
  /*! A human-readable presentation of eventual unmatched-parenthesis type errors

    If text doesn't contain any error this function returns wxEmptyString
   */
  wxString GetUnmatchedParenthesisState(wxString text);
protected:
  //! Is called on start and whenever the configuration changes
  void ConfigChanged();
  //! Called when the "Scroll to currently evaluated" button is pressed.
  void OnFollow(wxCommandEvent& event);
  void ShowCHMHelp(wxString helpfile,wxString keyword);
  /*! Launches the HTML help browser

    \param helpfile The name of the file the help browser has to be launched with
    \param otherhelpfile We offer help for maxima and wxMaxima in separate manuals.
                         This parameter contains the filename of the manual we aren't
                         using currently so the help browser can open a tab containing
                         this file.
  */
  void ShowHTMLHelp(wxString helpfile,wxString otherhelpfile,wxString keyword);
  void CheckForUpdates(bool reportUpToDate = false);
  void OnRecentDocument(wxCommandEvent& event);
  void OnIdle(wxIdleEvent& event);
  void MenuCommand(wxString cmd);                  //
  void FileMenu(wxCommandEvent& event);            //
  void MaximaMenu(wxCommandEvent& event);          //
  void AlgebraMenu(wxCommandEvent& event);         //
  void EquationsMenu(wxCommandEvent& event);       //
  void CalculusMenu(wxCommandEvent& event);        //!< event handling for menus
  void SimplifyMenu(wxCommandEvent& event);        //
  void PlotMenu(wxCommandEvent& event);            //
  void NumericalMenu(wxCommandEvent& event);       //
  void HelpMenu(wxCommandEvent& event);            //
  void EditMenu(wxCommandEvent& event);            //
  void Interrupt(wxCommandEvent& event);           //
  /* Make the menu item, toolbars and panes visible that should be visible right now.

     \todo Didn't update the stats pane. I assume this was a bug.
   */
  void UpdateMenus(wxUpdateUIEvent& event);
  void UpdateToolBar(wxUpdateUIEvent& event);      //
  void UpdateSlider(wxUpdateUIEvent& event);       //
  /*! Toggle the visibility of a pane
    \param event The event that triggered calling this function.
   */
  void ShowPane(wxCommandEvent& event);            
  void OnProcessEvent(wxProcessEvent& event);      //
  void PopupMenu(wxCommandEvent& event);           //
  void StatsMenu(wxCommandEvent& event);           //

  //! Is triggered when the "Find" button in the search dialog is pressed
  void OnFind(wxFindDialogEvent& event);
  //! Is triggered when the "Close" button in the search dialog is pressed
  void OnFindClose(wxFindDialogEvent& event);
  //! Is triggered when the "Replace" button in the search dialog is pressed
  void OnReplace(wxFindDialogEvent& event);
  //! Is triggered when the "Replace All" button in the search dialog is pressed
  void OnReplaceAll(wxFindDialogEvent& event);
  
  void SanitizeSocketBuffer(char *buffer, int length);  //!< fix early nulls
  void ServerEvent(wxSocketEvent& event);          //!< server event: maxima connection
  /*! Is triggered on Input or disconnect from maxima

    The data we get from maxima is split into small packets we append to m_currentOutput 
    until we got a full line we can display.
   */
  void ClientEvent(wxSocketEvent& event);

  void ConsoleAppend(wxString s, int type);        //!< append maxima output to console
  void DoConsoleAppend(wxString s, int type,       //
                       bool newLine = true, bool bigSkip = true);
  void DoRawConsoleAppend(wxString s, int type);   //

  /*! Spawn the "configure" menu.

    \todo Inform maxima about the new default plot window size.
  */
  void EditInputMenu(wxCommandEvent& event);
  void EvaluateEvent(wxCommandEvent& event);       //
  void InsertMenu(wxCommandEvent& event);          //
  void PrintMenu(wxCommandEvent& event);
  void SliderEvent(wxScrollEvent& event);
  //! Issued on double click on a history item
  void HistoryDClick(wxCommandEvent& event);
  //! Issued on double click on a table of contents item
  void StructureDClick(wxCommandEvent& event);
  void OnInspectorEvent(wxCommandEvent& ev);
  void DumpProcessOutput();

  //! Try to evaluate the next command for maxima that is in the evaluation queue
  void TryEvaluateNextInQueue();
  //! Trigger execution of the evaluation queue
  void TriggerEvaluation();
  void TryUpdateInspector();

  wxString ExtractFirstExpression(wxString entry);
  wxString GetDefaultEntry();
  bool StartServer();                              //!< starts the server
  /*!< starts maxima (uses getCommand) or restarts it if needed

    Normally a restart is only needed if
      - maxima isn't currently in the process of starting up or
      - maxima is runnning and has never evaluated any program 
        so a restart won't change anything
    \param force true means to restart maxima unconditionally.
   */
  bool StartMaxima(bool force = false);
  void OnClose(wxCloseEvent& event);               //!< close wxMaxima window
  wxString GetCommand(bool params = true);         //!< returns the command to start maxima
                                                   //    (uses guessConfiguration)

  //! Polls the stderr and stdout of maxima for input.
  void ReadStdErr();
  /*! Determines the process id of maxima from its initial output

    This function does several things:
     - it sets m_pid to the process id of maxima
     - it discards all data until this point
     - and it prepares the worksheet for editing.

     \param data The string ReadFirstPrompt() does read its data from. 
                  After leaving this function data is empty again.
   */
  void ReadFirstPrompt(wxString &data);
  /* Reads text that isn't enclosed between xml tags.

     Some commands provide status messages before the math output or the command has finished.
     This function makes wxMaxima output them directly as they arrive.

     After processing the lines not enclosed in xml tags they are removed from data.
   */
  void ReadMiscText(wxString &data);
  /* Reads the input prompt from Maxima.

     After processing the input prompt it is removed from data.
   */
  void ReadPrompt(wxString &data);
  /* Reads the math cell's contents from Maxima.
     
     Math cells are enclosed between the tags \<mth\> and \</mth\>. 
     This function removes the from data after appending them
     to the console.
   */
  void ReadMath(wxString &data);
  /*! read lisp errors

    Lisp errors typically don't provide a prompt prefix/suffix.

    After processing the error it is removed from data.

    \todo Add detection for lisp error prefixes for more lisps.
   */
  void ReadLispError(wxString &data);
  /*! Reads autocompletion templates we get on definition of a function or variable

    After processing the templates they are removed from data.
   */
  void ReadLoadSymbols(wxString &data);
#ifndef __WXMSW__
  //!< reads the output the maxima command sends to stdout
  void ReadProcessOutput();                        
#endif
  //!< Does this file contain anything worth saving?
  bool SaveNecessary();

  /*!
    This method is called once when maxima starts. It loads wxmathml.lisp
    and sets some option variables.

    \todo Set pngcairo to be the default terminal as soon as the mac platform 
    supports it.
 */
  void SetupVariables();
  void KillMaxima();                 //!< kills the maxima process
  /*! Update the title

    Updates the "saved" status, as well, but does only do anything if saved has
    changed or force is true.
    \param saved The new "saved" status
    \param force Force update if the "saved" status hasn't changed.
   */
  void ResetTitle(bool saved,bool force = false);
  void FirstOutput(wxString s);

  // Opens a wxm file
  bool OpenWXMFile(wxString file, MathCtrl *document, bool clearDocument = true);
  //! Opens a wxmx file
  bool OpenWXMXFile(wxString file, MathCtrl *document, bool clearDocument = true);
  //! Loads a wxmx description
  GroupCell* CreateTreeFromXMLNode(wxXmlNode *xmlcells, wxString wxmxfilename = wxEmptyString);
  /*! Saves the current file

    \param forceSave true means: Always ask for a file name before saving.
   */
  bool SaveFile(bool forceSave = false);
  int SaveDocumentP();
  //! Set the current working directory file I/O from maxima is relative to.
  void SetCWD(wxString file);
  //! Get the current working directory file I/O from maxima is relative to.
  wxString GetCWD()
    {
      return m_CWD;
    }
  wxSocketBase *m_client;
  wxSocketServer *m_server;
  bool m_isConnected;
  bool m_isRunning;
  bool m_first;
  //! The process id of maxima. Is determined by ReadFirstPrompt.
  long m_pid;
  wxProcess *m_process;
  // The stdout of the maxima process
  wxInputStream *m_input;
  // The stderr of the maxima process
  wxInputStream *m_error;
  int m_port;
  wxString m_currentOutput;
  //! The marker for the start of a input prompt
  wxString m_promptPrefix;
  //! The marker for the end of a input prompt
  wxString m_promptSuffix;
  //! The marker for the start of a list of autocompletion templates
  wxString m_symbolsPrefix;
  //! The marker for the end of a list of autocompletion templates
  wxString m_symbolsSuffix;
  wxString m_firstPrompt;
  bool m_dispReadOut;               //!< what is displayed in statusbar
  bool m_inLispMode;                //!< don't add ; in lisp mode
  wxString m_lastPrompt;
  wxString m_lastPath;
  MathParser m_MParser;
  wxPrintData* m_printData;
  bool m_closing;
  wxString m_openFile;
  wxString m_currentFile;
  bool m_fileSaved;
  bool m_variablesOK;
  wxString m_chmhelpFile;
  bool m_htmlHelpInitialized;
  wxString m_maximaVersion;
  wxString m_lispVersion;
#if defined (__WXMSW__)
  wxCHMHelpController m_chmhelpCtrl;
#endif
  wxHtmlHelpController m_htmlhelpCtrl;
  wxFindReplaceData m_findData;
  wxRegEx m_funRegEx;
  wxRegEx m_varRegEx;
  wxRegEx m_blankStatementRegEx;
#if wxUSE_DRAG_AND_DROP
  friend class MyDropTarget;
#endif
  DECLARE_EVENT_TABLE()
};

#if wxUSE_DRAG_AND_DROP

class MyDropTarget : public wxFileDropTarget
{
public:
  MyDropTarget(wxMaxima * wxmax) { m_wxmax = wxmax; }
  bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& files);
private:
  wxMaxima *m_wxmax;
};

#endif

class MyApp : public wxApp
{
public:
  virtual bool OnInit();
  wxLocale m_locale;
  /*! Create a new window

    \param file The file name
    \param batchmode Do we want to execute the file and save it, but halt on error?
   */
  void NewWindow(wxString file = wxEmptyString,bool batchmode=false);
  //! Is called by atExit and tries to close down the maxima process if wxMaxima has crashed.
  static void Cleanup_Static();
  //! A pointer to the currently running wxMaxima instance
  static wxMaxima *m_frame;
#if defined (__WXMAC__)
  wxWindowList topLevelWindows;
  void OnFileMenu(wxCommandEvent &ev);
  virtual void MacNewFile();
  virtual void MacOpenFile(const wxString& file);
  
#endif
};

DECLARE_APP(MyApp)

#endif // WXMAXIMA_H
