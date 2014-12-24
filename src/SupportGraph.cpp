/* Copyright (C) Kwee Heong Tan 2002 - 2003
   Permission is granted to use this code without restriction as
   long as this copyright notice appears in all source files.
*/
// $Id$
// standard inclusions
// std namespace usage
#include "SupportGraph.hpp"

std::string SupportGraph::_DotFileStringType[N]={"Unknown","Digraph","Graph"};

SupportGraph::DotFileType SupportGraph::getDotFileType(const std::string& path){
  /* Can't inquire about a Graphviz's type until we read it in, and
     can read the graph in until we declare its type.
     So read the .dot file and see what's in the first line
   */
  std::ifstream inDotFile(path.c_str(),std::ios::in);
  std::string tokenRead;
  while(inDotFile>>tokenRead){ // read first line only
    //cout << "tokenRead==" << tokenRead << "\n";
    inDotFile.close();
    if(std::string::npos!=tokenRead.find("digraph")){
      return Digraph;
    }else if(std::string::npos!=tokenRead.find("graph")){
      return Graph;
    }else{
      return Unknown;
    }
  }
  return Unknown;
};
