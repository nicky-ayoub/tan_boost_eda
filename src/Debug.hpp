// $Id: Debug.h,v 1.16 2002/11/21 17:53:11 khtan Exp khtan $
/* Reference : A Handy Debugger Class by Maurice Fox, C/C++ Users Journal Apr 2001
   Copyright (C) Maurice J. Fox 2000 - 2003 mauricef@ix.netcom.com
   Copyright (C) Kwee Heong Tan 2002 - 2003 tan.k.h@juno.com
   Permission is granted to use this code without restriction as
   long as this copyright notice appears in all source files.
*/
/* ----------------  Original source documentation by Maurice J Fox -------
  Debug, a class for debugging : 

  This package is inspired by the similar DBUG package developed
  for C and UNIX by Fred Fish, but does not use any of its code.
  
  It provides for tracing, debugging, interval timing, and output 
  control that can be controlled at run time through option strings.  
  Additionally, it provides optional function timing in connection 
  with tracing.

  Tracing:

  Tracing is printing a line of data at entry to a function and at
  exit from the function.  To get tracing, simply construct a Debug
  object at or near entry to the function.  Pass the name of the 
  as an argument string to the constructor.  Tracing output will be
  printed if tracing is enabled and either of two conditions is true:
  The set of remembered function names is empty, or the string supplied
  matches a remembered function name supplied as a keyword to the T:
  option.
  
  Having tracing on causes tracing and debugging output indented by
  two spaces for each level of function call up to a fixed limit,
  currently 12.  Upon execution of the object constructor, a line of
  data goes to the output, looking something like this:
          => fred
  When you reach a return statement in the function, or fall
  off the end of the code, the destructor for the object just
  created runs, and inserts a line of data into the output,
  looking something like this:
          <= fred
  So, no special effort is needed for exit tracing.  The package 
  also takes care of tracking the level of function calls.

  Debugging:
  
  Debugging is printing a line of data to the specified output
  destination.  To get debugging, use the overloaded Dbg member
  function or the Dbx member function on a previously constructed Debug 
  object.  See the function prototypes below for the available choices.  
  In each variant of Dbg, the first argument is treated as the keyword 
  that controls debugging.  The line that is printed contains indentation 
  (if tracing is in effect), the keyword followed by a colon, then the 
  requested output.  The line is printed if debugging is enabled and either 
  of two conditions is true:  The set of remembered keywords is empty, or
  the keyword supplied matches a remembered keyword supplied through
  the D: option.

  The package provides a small set of generally useful Dbg functions.  If
  these don't meet your needs, you can use the given functions as a
  pattern to write your own.  Another alternative is to use a single
  output stream, as discussed below.

  Interval timing:

  Interval timing allows you to measure the elapsed time between
  any two points in the program.  The functions TimeStart and
  TimeEnd are non-member functions, which means that you need
  not create a Debug object to invoke them.  (You may invoke them 
  through a Debug object if you like, however.)  It also means that
  the start and end points of the time interval(s) being measured
  need not be in the same function.  Interval timing is controlled
  by keywords, much the same fashion as tracing or debugging.  
  Each keyword is timed independently, so the intervals being
  timed can overlap.  
  
  You can use one TimeStart call to mark a single point, then a
  number of TimeEnd calls with the same keyword to get a series
  of elapsed times from that single start point.  If you call 
  TimeEnd without a preceding call to TimeStart with the same
  keyword you will get the same result as if you had called
  TimeStart immediately followed by TimeEnd.  This won't break
  anything, and might even be useful.

  Function Timing:
  This is an optional capability associated with tracing.  You select 
  this at compile time by the choice of constructor used for the Debug 
  object at the beginning of the function.  If you use the one with 
  only one parameter, you don't get timing.  If you use the one with 
  two parameters you get timing.  The second parameter is a bool, 
  which you can think of as "raw timing."  If it is true, you get the 
  time since the Epoch (or whatever your OS uses) on entry, and the 
  elapsed time of the function on exit.  This time is purportedly in 
  seconds and milliseconds, but its precision depends upon the clock 
  tick rate on your system.  If the second parameter to the constructor 
  is false, you get user friendly time in the style of asctime(), 
  which is what we use.  

  So:  If you use Debug D("fred"); you get
        => fred
        <= fred

        If you use Debug D("fred", true); you get
        => fred 901238947.234
        <= fred         0.877

        If you use Debug D("fred", false); you get
        => fred Sat May 13 15:39:23 2000
        <= fred Sat May 13 15:39:24 2000

  Option strings:

  Option strings allow run-time control of the behavior of the 
  package.  If you want to see any output, you need at least one
  option string, because the run-time default behavior is to do
  nothing at all.

  An option string is a sequence of non-blank characters of the
  form [anything]Option[Option . . .].
  Option is a sequence of the form X:[keyword_list]
  X here represents a non-blank character other than comma or colon.
  keyword_list represents an optional sequence of "words" separated
  by commas.  Each "word" is a sequence of non-blank characters 
  other than comma or colon.  The first option can be preceded by
  a sequence of non-blank characters not containing a colon, which
  the package ignores.  The reason for doing this is given 
  under "Passing option strings to the package."

  For example, the following is a syntactically valid option string:
  D:fred,wilma,logT:O:debug.out

  So is this one:
  DEBUGD:fred,wilma,logT:O:debug.out

  The options are D: T: and O:  D: is followed by three keywords:
  fred wilma and log.  T: is followed by no keywords, and O: is 
  followed by one keyword: debug.out

  Options currently recognized:

  T:  Enables tracing.  Keywords that follow this option are
  saved and matched against the function names provided to object
  constructors.  If a match is found, the function is traced at
  entry and exit.  If no keywords are in effect, ALL functions are
  traced at entry and exit.  If you give this option with keywords
  several times, the keywords specified are added to the list of 
  keywords recognized.

  t:  Disables tracing.  This causes tracing to be turned off.  Also
  the list of remembered keywords is cleared.  Any keywords that 
  follow this option are ignored, but cause no error.  Since the 
  option string is scanned from left to right, you could give the 
  string T:main,f1 at one point in the program.  Only the 
  functions main and f1 would be traced.  Later on, if you gave 
  the string t:T:, it would first have the effect of disabling 
  tracing and clearing the keywords, then enabling tracing for 
  all functions.

  D:  Enables debugging.  Keywords that follow the option are saved
  and matched against the keywords provided to calls to the member
  function Dbg.  If a match is found, the line is printed to the
  output.  If no keywords are in effect, ALL calls to Dbg result in
  lines being printed.  As with T:, giving this option several times
  with keywords causes the keywords to accumulate.

  d:  Disables debugging.  This causes debugging to be turned off,
  and clears the list of remembered keywords.  The same logic described
  above for t: applies, only to debugging.

  M:  Enables interval timing.  Keywords that follow the option 
  are saved and matched against the keywords provided to calls 
  to the member functions TimeStart and TimeEnd.  If a match is 
  found, a line is printed to the output.  The function TimeStart
  starts timing the keyword and prints the raw time.  The function
  TimeEnd stops timing the keyword and prints the elapsed time 
  since the last call to TimeStart for that keyword.  If you call
  TimeStart for the same keyword several times without intervening
  calls to TimeEnd, the package uses the latest one called as the 
  start of the timing interval.  If no keywords are in effect, ALL
  calls to TimeStart and TimeEnd result in lines being printed.  
  As with T:, giving this option several times with keywords causes 
  the keywords to accumulate.

  m:  Disables interval timing.  This causes debugging to be turned off,
  and clears the list of remembered keywords.  The same logic described
  above for t: applies, only to interval timing.

  O:  Output control.  If there is no keyword following, this option
  has no effect.  A keyword that follows is treated as an output
  specifier, as follows:
  -  Specifies standard output (cout)
  -- Specifies buffered standard error (clog)
  Anything else is used as the filename for an open call.  The
  package does not report the success or failure of open calls, but
  hey, it's a debugging tool, not production software.  It opens the 
  file for appending, so you could switch from one file to another and
  back again without losing data.  This capability also allows you
  to do such things as limit the size of log files and the like.  If
  you specify several keywords (useless but harmless), only the last
  one seen actually has any effect.

  It is not necessary to supply an O: option to get output.  By default,
  the package is completely inactive, but its output is preset to clog.
  If you just choose one of D: T: or M: you will get output to clog.
  
  Passing option strings to the package:

  Debug gets its option strings through the non-member function
  Debug::specify(), which takes as an argument a pointer to a 
  C-style string.  The use envisioned is that one could put Debug
  objects all over the place, turn off tracing and debugging through
  the specify() function and scan the command line before creating
  the first Debug object.  It's up to you, but here's a skeleton
  example:

  int main(int argc, char *argv[]) {
      for(int i = 1;i < argc; i++) {
        . . .
        if(!strncmp("-DEBUG",argv[i],6)) Debug::specify(argv[i]);
        . . .
      }
      Debug D("main");
      . . .
      D.Dbg("A","Value of x is ",x);
      etc . . . 

  Now, while in development, you could run the program with a line
  like -DEBUGD:T: on the command line, which would trace and debug 
  everything.  By default, output goes to buffered standard error (clog).
  A line like -DEBUGD:A,B,CT:O:debug.out would trace everything, debug 
  only those lines with keywords A, B, or C, and put the data into the 
  file debug.out.

  Another possibility is to use the package for logging.  Look at this:

  int main(int argc, char *argv[]) {
      Debug::specify("D:logt:"); // No tracing, logging with keyword log
      for(int i = 1;i < argc; i++) {
        . . .
        if(!strncmp("-DEBUG",argv[i],6)) Debug::specify(argv[i]);
        . . .
      }
      Debug D("main");
      . . .
      D.Dbg("A","Value of x is ",x);
      D.Dbg("log","Blah blah blah ",x);
      etc . . . 
   
   One user suggested using an environment variable to pass in
   an option string.  That could be useful in a GUI development or
   other circumstance where getting data from the command line is
   not feasible.

   You can compile a standalone test/demo program by compiling
   Debug.cpp all alone with the symbol TESTING defined.  Feed it
   option strings on the command line and watch what happens.

   Normally, the presence of the debug code will have negligible
   effect on the performance of the application.  If you really
   feel a need to eliminate debugging in production code, you can 
   compile a new no-op version by defining the symbol DEBUG_OFF 
   both for Debug.cpp and your own code.  Usually you're just 
   kidding yourself when you do that, though.

   Concerning performance, this version no longer flushes its output
   after each line.  The endl; at the end of each output statement
   has been replaced with "\n";.  This improved the speed somewhat for 
   all output choices.  The big improvement over previous versions
   was replacing clog with cout.  This made a very noticeable speed
   improvement when writing lots of debugging data to a file and a 
   smaller improvement to the other two destinations.
   
   The non-member function Sync() allows you to flush the output stream
   at will.  This can be useful when several processes are writing to
   the same file, as can happen when a process forks a child in a UNIX
   environment.  It's true that multiple processes append to the same
   in the correct sequence, that's only true of the physical transfer
   of the buffer to the file.  The system process transfers the buffer
   when it gets full, which might not display what you want to see.  
   So, in a UNIX environment, you may want to do something like this:

   D.Sync();
   pid = fork();

   or

   Debug::Sync();
   pid = fork();

   OPTIONAL CAPABILITIES:

   Multiple keywords:  
   
   This allows you to specify several keywords, separated by commas,
   (no spaces!) for a Dbg, Dbx, TimeStart, or TimeEnd function call.
   Then, if any one of the keywords is in effect, (or no keywords,
   as usual) you get the requested effect.  It could have some 
   performance impact if you use a lot of keywords, so you select this
   when compiling the package.  Define the symbol DEBUG_MULT_KWDS when
   compiling Debug.cpp to get this behavior.  So, if you use this capability
   you can write something like this:

        D.Dbg("fork,cache,file","Value of x is",x);

   and get debugging output when any of the keywords fork, cache, or file
   has been specified, or when no keyword has been specified for 
   debugging.  If you write such a line when using the version that does
   not recognize multiple keywords, you won't get any output if any
   keyword is specified, because the code that parses option strings
   will not create a keyword with embedded commas, so that "keyword" 
   would never be matched.  (But you would have figured that out for 
   yourself!)

   Don't get cute and write multiple keywords with consecutive commas,
   leading commas, or trailing commas.  You'll probably crash your 
   program or get some other undesired behavior.  (Undesired, unless
   you get a kick out of seeing things blow up.)

   DOS Disk Drives

   If you want to use Debug in a DOS-like environment and need to name
   a disk drive for an output file, you have a problem because the colon
   used to identify options clashes with the use of the colon in the
   name of a disk drive.  To make a version that uses a semicolon to
   identify options instead of a colon, compile with the symbol
   DEBUG_DOS_FORMAT defined.  All of the foregoing documentation still
   applies, except that where it reads colon, mentally substitute
   semicolon.  Then you can use an option string like

        D;T;O;D:\test\debug.out

   and get the output to the DOS file D:\test\debug.out.  This would not
   work in a UNIX environment, where semicolons separate commands.

   Single Output Stream.

   You can send all output through the same stream if you wish.  To
   do this, compile with the symbol DEBUG_1STREAM defined.  
   Internally, this works by logically sending all output to the clog
   stream, changing the actual destination by using the rdbuf() member
   function.  Consequently, any clog calls made directly within your
   program will go wherever you have sent the output.

   Why would I want to do this? you ask.  The main reason is to put
   arbitrary data into the debugging output stream without writing your
   own Dbg calls.  You can write output to clog with a line like

   clog << "Checkpoint" << n << "reached, X is now " << x << 
    " Y is now " << Y << " Z is now " << z << endl;

   and it will go wherever you have selected using the O: option.  
   This could be a lot simpler than writing your own overloaded Dbg
   function as suggested above.

   Because output now goes through clog, you can write something else
   to cerr, and it will go to unbuffered standard error, not to the
   debugging output.  Of course, if you have chosen to have output
   go to clog, they will both go to the same destination.  If you do
   not use this option, whatever you write to clog or cerr will go to
   the same destination, intermingled with debugging output only if
   you have specified O:--.

   Except for the documented effects of these three options, they are
   all completely independent.  This has been extensively tested.
*/
/* ----------------  Modifications by Kwee Heong Tan ----------------------
1) Added support for very old STL, circa 1996 with preprocessor symbol STL96
2) Added support for compiling between VC6 and Comeau with __COMO__
3) Removed DEBUG_DOS_FORMAT, setting Debug::separator to '?'.
   This is so that usage is the same between Windows and Unix.
4) Refactored output code by adding a pDebugStream that is set up once.
5) Added support for emacs outline with preprocessor define EMACS_OUTLINE.
   This is turned on by default.
   I use emacs-outline mode a lot and prepending the Enter trace with
   the outline prefix allows flexible display of information.
   An example of a trace with EMACS_OUTLINE turned on.
      To show just headings 1 and 2 :
         | * => backtraceGraph      1: vertex label== = U3:nand ...
         | **  => discover_vertex        1: vertex label== = U3:nand ...
         | **  => discover_vertex        1: vertex label== = U2:nand ...
         | **  => discover_vertex        1: vertex label== = J:in ...
         | **  => discover_vertex        1: vertex label== = K:in ...
         | **  => discover_vertex        1: vertex label== = M:in ...
         | * => implicateVertex      1: vertex label== = J:in ...
         | **  => setAllVertexOutputs        1: vertex label== = J:in ...
         | **  => discover_vertex        1: vertex label== = J:in ...
         | **  => discover_vertex        1: vertex label== = U2:nand ...
      To show all text :
         | * => backtraceGraph      1: vertex label== = U3:nand
         | **  => discover_vertex        1: vertex label== = U3:nand
         |     <= discover_vertex
         | **  => discover_vertex        1: vertex label== = U2:nand
         |     <= discover_vertex
         | **  => discover_vertex        1: vertex label== = J:in
         | ***   => HasXs          1: vertex label== = J:in
         |           1: edgeLabel== = X
         |       <= HasXs
         | ***   => getEnablingSignal          1: dResult== = ONE
         |       <= getEnablingSignal
         |     <= discover_vertex
         | **  => discover_vertex        1: vertex label== = K:in
         | ***   => HasXs          1: vertex label== = K:in
         |           1: edgeLabel== = X
         |       <= HasXs
         | ***   => getEnablingSignal          1: dResult== = ONE
         |       <= getEnablingSignal
         |     <= discover_vertex
         | **  => discover_vertex        1: vertex label== = M:in
         | ***   => HasXs          1: vertex label== = M:in
         |           1: edgeLabel== = X
         |       <= HasXs
         | ***   => getEnablingSignal          1: dResult== = ONE
         |       <= getEnablingSignal
         |     <= discover_vertex
         |   <= backtraceGraph
         | * => implicateVertex      1: vertex label== = J:in
         | **  => setAllVertexOutputs        1: vertex label== = J:in
         |         1: dSignal== = ONE
         |         2: before : _e[]["label"]== = X
         |         2: after : _e[]["label"]== = ONE
         |     <= setAllVertexOutputs
         | **  => discover_vertex        1: vertex label== = J:in
         | ***   => EvaluateZeroInput          1: vertex label== = J:in
         |           1: signal== = ONE
         |           1: sResult== = ONE
         |       <= EvaluateZeroInput
         |         1: outSignal== = ONE
         |     <= discover_vertex
         | **  => discover_vertex        1: vertex label== = U2:nand
         | ***   => EvaluateMultipleInputs          1: func= = nand
         |           1: inputs== =  ONE , X ,
         |           1: result== = X
         |       <= EvaluateMultipleInputs
         | ***   => EvaluateZeroInput          1: vertex label== = U2:nand
         |           1: signal== = _D
         |           1: signal== = X
         |           1: sResult== = _D
         |       <= EvaluateZeroInput
         |         1: multi-inputs :  = _FirstInconsistentOutputFound set to true
         |     <= discover_vertex
         |       1: dfv return with :  = entire tree searched
         |       1: firstOutputFound== = 0
         |       1: firstNonDPassable== = 0
         |       1: firstInconsistentOutput== = 0
         |   <= implicateVertex
         | 
   EMACS_OUTLINE support consists of
      - changing the Debug constructor to accept a new boolean to allow CR/LF
        to be selectively written. This allows the first .Dbg to be appended
        to the Enter heading, assisting in reading the trace output.
      - writing out the EMACS OUTLINE prefix "*" appropriately.
6) Removed various and sundry Dbg and Dbx calls, replacing with 2 Dbg member
   function templates. Dbx calls were removed because it required the user to
   remember which types use Dbx and which types use Dbg.
   Dbg member function template for T* automatically detects the NULL pointer.
   Added two specializations for char* case.
7) By templating Debug code, added a Debug.hpp for ease of compilation, and 
   use on VC6 projects.
 */
#ifndef DEBUG_CLASS_DEFINED
#define DEBUG_CLASS_DEFINED 1
#if defined(WIN32) && !defined(__COMO__)
   #pragma warning(disable:4002 4786)
#endif
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <assert.h>

#define EMACS_OUTLINE // turn on support for emacs outline headers in trace

   #include <utility> // STL96: includes function.h, must be before string
   #include <string>
   #include <vector>
#ifdef STL96
   #include <iostream.h>
   #include <fstream.h>
   #define STD
#else
   #include <iostream>
   #include <fstream>
   #define STD std::
#endif

#define DECLARE_DEBUG(name) Debug D(name)

#ifdef DEBUG_OFF
class Debug {
public:
  Debug(const char* f,bool rawtimeMode=false,bool bEndLine=false){}
    ~Debug() {}
    template<class T> void Dbg(const STD string& keyword, const STD string& description, T* pT){}
    template<class T> void Dbg(const STD string& keyword, const STD string& description, const T& tValue){}
                      void Dbg(const STD string& keyword, const STD string& description, char* pC) {}
    template<class T> void Dbg(const STD string& keyword, const T* pT){}
    template<class T> void Dbg(const STD string& keyword, const T& tValue){}
                      void Dbg(const STD string& keyword, char* pC){}
    static void specify(const char *){}
    static void TimeStart(const char *, const char *){}
    static void TimeEnd(const char *, const char *){}
    static void Sync(){}

private:
    Debug() {}
};

#else

class Debug {
public:
#if defined(EMACS_OUTLINE)
    Debug(const char* f,bool rawtimeMode=false,bool bEndLine=false);
#else
    Debug(const char* f,bool rawtimeMode=false,bool bEndLine=true);
#endif
    ~Debug();
    void Dbg(const STD string& keyword, const STD string& description, char* pC);
    template<class T> void Dbg(const STD string& keyword, const STD string& description, const T* pT){
      if(!debugging(keyword)) return;
      indent();
      if(NULL==pT){
        (*pDebugStream) << keyword << ": "<< description << " = NULL\n";
      }else{
        (*pDebugStream) << keyword << ": "<< description << " = " << (*pT) << "\n";
      }
    }
    template<class T> void Dbg(const STD string& keyword, const STD string& description, const T& tValue){
      if(!debugging(keyword)) return;
      indent();
      (*pDebugStream) << keyword << ": "<< description << " = " << tValue << "\n";
    }
    void Dbg(const STD string& keyword, char* pC);
    template<class T> void Dbg(const STD string& keyword, const T* pT){
      // cout <<"-print T*\n";
      if(!debugging(keyword)) return;
      indent();
      if(NULL==pT){
        (*pDebugStream) << keyword << ": NULL\n";
      }else{
        (*pDebugStream) << keyword << ": " << (*pT) << "\n";
      }
    }
    template<class T> void Dbg(const STD string& keyword, const T& tValue){
      // cout <<"-print T&\n";
      if(!debugging(keyword)) return;
      indent();
      (*pDebugStream) << keyword << ": "<< (const T) tValue << "\n";
    }
    static void specify( const char * );
    static void TimeStart(const STD string &, const char *);
    static void TimeEnd(const STD string &, const char *);
    static void Sync();
private:
    // Constants
    enum {limit = 12};// indentation limit for tracing

    // Static members
    static int level; // Counts the depth of function calling
    static bool Debugging; // Controls printing in Dbg calls
    static bool Tracing;   // Controls printing in ctor and dtor calls
    static bool Timing;    // Controls interval timing

    static char separator;
    static STD string filename;
    static STD ofstream* pFile;
    static STD ostream* pDebugStream;
    static bool use_cout;
    static bool use_clog;

    static STD vector<STD string> keywords;
    static STD vector<STD string> functions;
    static STD vector<STD string> timekeys;
    static STD vector<STD pair<STD string, struct timeb> > timers;
    static char mybuff[200];

    // Internal functions
    bool tracing(const STD string &); // Tells whether to do tracing or not
    bool debugging(const STD string &); // Tells whether to do debugging or not.
                    // Both of these look at the flag and at the vector.
    static void indent();  // Indents the output line
    void Enter(bool); // Gets us into a function with proper indents, printing the name of the function.
    void Exit();    // Gets us out of a function with proper indents.
    Debug();        // No default ctor
    STD string time_stuff();
    Debug(const Debug &);   // No copy ctor
    static bool IntTiming(const STD string &);  // Tells whether to do interval timing

    // object data;
    STD string function_name;
    bool timing;
    bool raw_time;
    bool going_in;
    struct timeb Start;
};

#endif
#endif


