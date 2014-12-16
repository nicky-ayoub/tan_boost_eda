/* Copyright (C) Kwee Heong Tan 2002 - 2003
   Permission is granted to use this code without restriction as
   long as this copyright notice appears in all source files.
*/
// $Id: CmdLine.h,v 1.14 2002/11/28 18:21:11 khtan Exp khtan $
#if defined(WIN32) && !defined( __COMO__)
#pragma warning(disable:4786) // only need this here, included elsewhere
#endif
#include <string>
#include <map>
#include <vector>
#ifdef STL96
   #include <iostream.h>
   #define STD
#else
   #include <iostream>
   #define STD std::
#endif

class CmdLine{
/*  USAGE DOCUMENTATION 
General
--------
The CmdLine class is used to specify the options a program or class wishes
to handle, and then process the {argc,argv} arguments of a standard main
call.

Parameter types
---------------
There are 2 types of parameter switches - standalone and parameterized.
   A standalone switch takes no values, eg -h or -help
   A parameterized switch takes a value, eg -s J:in

Since the switch is just a string value, you are not restricted to the <dash><letter>
syntax, eg -h. For example, --help or ==help or **help are all acceptable.

Setup
-----
The user decides which parameter types and string values to represent them.
The member functions addStandaloneSwitch() and addParameterSwitch() are used
to set them. For a parameterized switch, a string initialValue can be specified
as a default.

The member function process(argc,argv) is used to go over the argv array
to check whether the switches are set, and in the case of parameterized
switches, to take in the parameter value.

Query
------
In use, the member function switchValue() returns the values "set" or "unset"
for standalone switches, and the parameter value or its initial value, 
for parameterized switches.

The NumberOfRemainingParameters is used to detect any parameters in argv
that were not processed. This allows CmdLine to ignore switches it is not
interested in and still allows the user to process them later, via the
RemainingParamter(int index).

Additional utilities
---------------------
printSwitches and dump are self explanatory.

 */
  friend STD ostream& operator<<(STD ostream &s,const CmdLine& c);
  public :
    CmdLine();
    void addParameterSwitch(const STD string flag,const STD string initialValue,const STD string help);
    void addStandaloneSwitch(const STD string flag,const STD string help);
    const STD string& switchValue(const STD string flag);
    const STD string& switchHelp(const STD string flag);
    void process(int argc,char** argv);
    int NumberOfRemainingParameters();
    const char* RemainingParameter(int index);
    void printSwitches(STD ostream& s=STD cout);
    void dump(STD ostream& s=STD cout) const;
    void dump(STD ostream& s=STD cout);
 private:
    typedef STD map< STD string, STD string, STD less<STD string> > mapType;
    mapType m;
    mapType h;
    STD vector< char* > l;
    bool debug;
};
