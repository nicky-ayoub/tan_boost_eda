/* Reference : A Handy Debugger Class by Maurice Fox, C/C++ Users Journal Apr 2001
   Copyright (C) Maurice J. Fox 2000 - 2003
   Copyright (C) Kwee Heong Tan 2002 - 2003
   Permission is granted to use this code without restriction as
   long as this copyright notice appears in all source files.
*/
// $Id: Debug.cpp,v 1.16 2002/11/21 17:52:42 khtan Exp khtan $
#include "Debug.hpp"
#include <algorithm>
#include <string.h>
// Added enhancements turned on with preprocessor defines

#ifdef STL96
   #define CLEAR(name) name.erase(name.begin(),name.end());
   extern "C" {
      int ftime(struct timeb *timeptr);
   }
#else
   using namespace std;
   #define CLEAR(name) name.clear()
#endif

#ifdef TESTING
static void f1(void);
static void f2(void);
int main( int argc, char * argv[] ) {
    Debug D("main");
    float pi = (float)3.14;
    int   i = 1999;
    char String[] = "This is a testing string ";

    for(int j = 1; j < argc; j++) {
      Debug::specify(argv[j]);
      D.Dbg("argv[j]",argv[j]);
      D.Dbg("float", pi);
      D.Dbg("integer", i);
      D.Dbg("string", String, i);
      f1();
    }
    return 0;
}

static void f1() {
    Debug D("f1");
    D.Dbg("string","f1's string");
    D.Sync();
    D.Dbg("string","string one ", "string two");
    f2();
}

static void f2() {
    Debug D("f2");
    D.Dbg("string","f2's string");
    D.Dbg("integer","This is an unsigned hex one ", static_cast<unsigned long>(0XDEADBEEF));
    D.Dbg("integer","This is a regular integer ", static_cast<int>(0XDEADBEEF));
    //    D.TimeEnd("bogus,once","This has no preceding TimeStart call");
    //    D.TimeEnd("once","This is in f2");
    D.Dbg("bogus,fred,fork,a,string,integer","a string");
    Debug::Sync();
    //sleep(1);
}
#endif // TESTING

#ifndef DEBUG_OFF
    // Static members
int Debug::level = 0;
bool Debug::Debugging = false;
bool Debug::Tracing = false;
bool Debug::Timing = false;
STD ofstream* Debug::pFile=NULL;
STD ostream* Debug::pDebugStream=&clog; // default on startup, use clog

char Debug::separator = '?';

char Debug::mybuff[200];
string Debug::filename;

vector<string> Debug::keywords;
vector<string> Debug::functions;
vector<string> Debug::timekeys;
vector<pair<string, struct timeb> > Debug::timers;

Debug::Debug(const char* f,bool rawtimeMode,bool bEndLine) :
function_name(f), timing(rawtimeMode),raw_time(rawtimeMode),going_in(rawtimeMode)
{
    level++;
    if(tracing(function_name)) 
        Enter(bEndLine);
    going_in=false;
}
Debug::~Debug() {
        if(tracing(function_name)) Exit();
        level--;
}

bool Debug::tracing(const string & f) {
    if(!Tracing) return false;
    
    return (functions.empty() || 
    find(functions.begin(),functions.end(),f) != functions.end());
}

bool Debug::debugging(const string & k) {
    if(!Debugging) return false;

#if defined DEBUG_MULT_KWDS
    if( keywords.empty()) return true;
    string::size_type start, end;
    string kw;
    start = 0;
    do {// This is the beginning of a do-while loop
        // Look for a comma delimiting an internal keyword
        // We are assuming well-formed multiple keywords - 
        // no leading, trailing, or multiple commas.
        end = k.find(',', start);
        if( end == string::npos ) {
            // Assign the rest of k to kw.  This could be all of k
            // when start is still 0.
            kw = k.substr(start);
        } else {
            // Assign the substring just found to kw, adjust start point 
            kw = k.substr(start,end-start);
            start = end + 1;
        }
        if(find(keywords.begin(),keywords.end(),kw) != keywords.end()) {
            return true;
        }
    } while( end != string::npos );
    return false;
#else
    return (keywords.empty() || 
    find(keywords.begin(),keywords.end(),k) != keywords.end());
#endif
}

bool Debug::IntTiming(const string & t) {
    if(!Timing) return false;

#if defined DEBUG_MULT_KWDS
    if( timekeys.empty()) return true;
    string::size_type start, end;
    string kw;
    start = 0;
    do {// This is the beginning of a do-while loop
        // Look for a comma delimiting an internal keyword
        // We are assuming well-formed multiple keywords - 
        // no leading, trailing, or multiple commas.
        end = t.find(',', start);
        if( end == string::npos ) {
            // Assign the rest of t to kw.  This could be all of t
            // when start is still 0.
            kw = t.substr(start);
        } else {
            // Assign the substring just found to kw, adjust start point 
            kw = t.substr(start,end-start);
            start = end + 1;
        }
        if(find(timekeys.begin(),timekeys.end(),kw) != timekeys.end()) {
            return true;
        }
    } while( end != string::npos );
    return false;
#else
    return(timekeys.empty() ||
    find(timekeys.begin(),timekeys.end(),t) != timekeys.end());
#endif
}
void Debug::Enter(bool bEndLine){
  string sBuffer;
  for(int i = 1; i < (level < limit ? level : limit) ; i++ ){
    sBuffer+="  ";
  }
#if defined(EMACS_OUTLINE) // not handling level > limit case
  sBuffer.replace(0,level-1,level-1,'*');
#endif//EMACS_OUTLINE

  (*pDebugStream) << sBuffer;
  (*pDebugStream) << "=> " << function_name << time_stuff();
  if(bEndLine) (*pDebugStream) << "\n";
}
void Debug::Exit(){
    for(int i = 1; i < (level < limit ? level : limit) ; i++ )
    (*pDebugStream) << "  ";

    (*pDebugStream) << "<= " << function_name << time_stuff() << "\n";
}
string Debug::time_stuff() {
    if(!timing) return "";

    if(raw_time) {
        struct timeb Tb;
        ftime(&Tb);
        if(going_in) {
            sprintf(mybuff, " %ld.%03d",Tb.time, Tb.millitm);
            Start = Tb;
        } else {
            long int secs, millis;
            millis = Tb.millitm - Start.millitm;
            secs = Tb.time - Start.time;
            if(millis < 0) {
                millis += 1000;
                -- secs;
            }
            sprintf(mybuff, "         %ld.%03ld", secs, millis);
        }

    } else { // Cooked time
        time_t Clock;
        time(&Clock);
        sprintf(mybuff," %s", asctime(localtime(&Clock)));
        mybuff[strlen(mybuff)-1] = '\0';
    }
    return mybuff;
}

void Debug::indent() {
    if( Tracing ) {
        for( int i = 0; i <= (level < (limit + 1) ? level : (limit + 1)); i++ ) 
        (*pDebugStream) << "  ";
    }
}
void Debug::Dbg(const std::string& keyword, const std::string& description, char* pC){
  // cout << "-print char*\n";
  if(!debugging(keyword)) return;
  indent();
  if(NULL==pC){
    (*pDebugStream) << keyword << ": "<< description << " = NULL\n";
  }else{
    (*pDebugStream) << keyword << ": "<< description << " = " << pC << "\n";
  }
};
void Debug::Dbg(const std::string& keyword, char* pC){
  // cout << "-print char*\n";
  if(!debugging(keyword)) return;
  indent();
  if(NULL==pC){
    (*pDebugStream) << keyword << ": NULL\n";
  }else{
    (*pDebugStream) << keyword << ": "<< pC << "\n";
  }
};

void Debug::TimeStart(const string &t, const char *s) {
    if(!IntTiming(t)) return;
    
    unsigned int i = 0;
    for(i = 0; i < timers.size(); i++) {
        if(timers[i].first == t) break;
    }
    timeb tb;
    ftime(&tb);
    if( i == timers.size()) {  // Not found, so put one in.
        timers.push_back(make_pair(t,tb));
    } else {
        timers[i].second = tb;
    }
    sprintf(mybuff," %ld.%03d", tb.time, tb.millitm);

    indent();
    (*pDebugStream) << t << ": " << s << mybuff << "\n";
}
    
void Debug::TimeEnd(const string &t, const char *s) {
    if(!IntTiming(t)) return;
    
    unsigned int i = 0;
    for(i = 0; i < timers.size(); i++) {
        if(timers[i].first == t) break;
    }
    timeb tb;
    ftime(&tb);
    if( i == timers.size()) { // Not found!
        // This puts an entry in the timers vector
        // it won't be very informative, but it keeps
        // things going OK.
        timers.push_back(make_pair(t,tb));
    }
    // i now has the index of our timer in the vector

    int secs, millis;
    millis = tb.millitm - timers[i].second.millitm;
    secs = tb.time - timers[i].second.time;
    if(millis < 0) {
        millis += 1000;
        --secs;
    }
    sprintf(mybuff, " %d.%03d", secs, millis);

    indent();
    (*pDebugStream) << t << ": " << s << mybuff << "\n";
}

void Debug::Sync() {
    pDebugStream->flush();
}
    
/***********************************
* Debug::specify()
* Extracts options and keywords from a Debug specification string
* and deals with them "appropriately."
* 
* PSEUDOCODE:
* look for an option
* while(there is an option)
*   look for next option or end of string
*   look for a keyword in the substring thus found
*   while(where is a keyword)
*       depending upon the option found
*       save the keyword OR
*       extract and save some value OR
*       ignore the keyword
*       look for a keyword
*   end while
* end while
*
* Option strings have the form
* L:keyword,keyword,L:keyword,keyword,
* L:L:keyword,keyword
* L:L:,keyword
* etc.  Terminating commas are optional
* Leading commas are harmless.
**************************************/
void Debug::specify(const char * opt) {
  string s = opt;

  // Find options
  unsigned int j = s.find_first_of(separator,0);
  while(j < s.length()) {  // Option found
    // j points to the separator at the end of this option

    unsigned int k = s.find_first_of(separator,j+1);
    // k now points to the separator at the end of the next option
    // or to outer space
        
    string Option;
    if(j == 0) {
      Option = "0";
      Option += s[0];
    } else {
      Option = string(s,j-1,2);
    }
        
    // Find keywords
    unsigned int l = s.find_first_of(',',j+1);

    // Deal with the case of no comma, like F:keywordD: or
    // F:keyword
    if((l >= k) && (j < k-2)) {
      if( k > s.length() ) // No trailing option
        l = s.length();
      else l = k-1;    // Trailing option is found
    }
    while(l < k) {  // keyword found
      string kw;
      if(j+1 < l ) {
        kw = string(s,j+1,l-j-1);
      } else {
        kw = "";
      }

      if( l < s.length() ) {
            
        j = l;  // Advance to virtual ( or real ) comma.
        unsigned int m = s.find_first_of(',',l+1);

        // Again, what if there's no trailing comma in a
        // sequence of keywords like F:abcd,efgh,ijklD:
        // or F:a,b,c,d
        if((m >= k) && (j < k-1)) {
          if( (m > s.length()) && ( k > s.length()))
            m = s.length();
          else m = k-1;
        }
        l = m;
      } else l = k;  // Bail out of the loop

      // This is where we deal with the keywords and options
      switch(Option[0]) {
      case 'D':    // Turn debugging on
        if(kw.length()) keywords.push_back(kw);
        break;

      case 'O':   // Changing output, maybe
#ifdef STL96
          if(!( (! kw.length()) || kw==filename)) {
#else
          if(kw.length() && kw != filename) {
#endif
            filename = kw;
            if( kw == "-"){
              pDebugStream=&cout;
            }else{ 
              if(kw == "--") {
                pDebugStream=&clog;
              }else{
                if(NULL!=pFile){
                  delete pFile;
                  pFile=NULL;
                }
                pFile=new(ofstream);
                pFile->open(filename.c_str(),ios::app);
                pDebugStream=pFile;
              }
            }
          }
          break;

        case 'T':    // Turn tracing on
          if(kw.length()) functions.push_back(kw);
          break;

        case 'M':    // Turn interval timing on
          if(kw.length()) timekeys.push_back(kw);
          break;

        default:       // Ignore other options silently
          break;
        }
      } // end of loop on finding keywords
      // This is where we complete dealing with options
      switch(Option[0]) {
      case 'D':    // Turn debugging on
        Debugging = true;
        break;

      case 'd':       // Turn debugging off
        CLEAR(keywords);
        Debugging = false;
        break;

      case 'O':   // Changing output, maybe - khtan : no longer needed
        break;

      case 'T':    // Turn tracing on
        Tracing = true;
        break;

      case 't':       // Turn tracing off
	    CLEAR(functions);
        Tracing = false;
        break;
            
      case 'M':       // Turn interval timing on
        Timing = true;
        break;

      case 'm':       // Turn interval timing off
	    CLEAR(timekeys);
        CLEAR(timers);
        Timing = false;
        break;

      default:       // Ignore other options silently
        break;
      }
      j = k; // Advance to next option
    } // end of loop on finding options
}
#endif // ndef DEBUG_OFF
