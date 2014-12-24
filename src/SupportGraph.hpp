/* Copyright (C) Kwee Heong Tan 2002 - 2003
   Permission is granted to use this code without restriction as
   long as this copyright notice appears in all source files.
*/
// $Id$
// standard inclusions
// std namespace usage
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
using namespace std;
// boost namespace usage

// ------------------------------------------------------------
// class SupportGraph
// ------------------------------------------------------------
class SupportGraph{
/* Utility class to hold special functions.
   - check whether a .dot file is a graph or digraph.
 */
public:
  enum DotFileType{Unknown,Digraph,Graph,N};
  static DotFileType getDotFileType(const string& path);
  static string _DotFileStringType[N];
  SupportGraph(){};
private:
  SupportGraph(const SupportGraph&);
  SupportGraph& operator=(const SupportGraph&);
};
