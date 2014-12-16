/* Copyright (C) Kwee Heong Tan 2002 - 2003
   Permission is granted to use this code without restriction as
   long as this copyright notice appears in all source files.
*/
#if defined(WIN32) && !defined( __COMO__)
   #pragma warning(disable:4786 4503 4101)
#endif

#include <iostream>
using std::cout;

#include "atpg.hpp"
#include "CmdLine.h"
void processCmdLine(CmdLine& cL,int argc,char** argv){
  cL.addStandaloneSwitch("-i","initialize graph");
  cL.addStandaloneSwitch("-atpg","run atpg algorithm");
  cL.addStandaloneSwitch("-h","print this help message");
  cL.addParameterSwitch("-t","undefined","run test");
  cL.addParameterSwitch("-r","undefined","read dot file path");
  cL.addParameterSwitch("-w","undefined","write dot file path");
  cL.addParameterSwitch("-x","T?D?O?-","debug option, default to trace and debug to stdout");
  cL.process(argc,argv);
}
int main(int argc,char** argv){
  /** Outline of source file organization
   SUBSYSTEM                  | SOURCE FILES
   ---------------------------+-----------------------------------------                         
   Command line               | CmdLine.h, CmdLine.cpp
   Program tracing            | Debug.hpp
   Multi-valued logic         | DLogic.hpp
   Graph visualization and IO | modifications to Graphviz.hpp
   Driver program             | atpg.cpp
   EDA algorithm - basic DFT  | atpg.hpp, DFrontier.hpp

   INCLUSION TREE ( -+-> means "includes" )
     -atpg.cpp -+->CmdLine.h
                |->atpg.hpp -+->Debug.hpp -----+->Debug.cpp --+->Debug.h
                                  |->DFrontier.hpp
                                  |->DLogic.hpp  ---+->DLogic.h
                                  |                 |->DLogic.cpp
                                  |->BGL headers
                                  |->STL headers
     -CmdLine.cpp -+->CmdLine.h

  Template code have the prefix .hpp for two reasons. Firstly, Boost code uses .hpp, so
  I follow the convention. Secondly, the VC6 IDE will separately compile a .cpp 
  regardless of whether it is in the Source or Header folders. Using .hpp prevents this
  over-eager compilation.

   */
  cout << "$Id$\n";
  SupportGraph sG;
  CmdLine cLine;
  processCmdLine(cLine,argc,argv);
  if("set"==cLine.switchValue("-h")){
    cLine.printSwitches();
  }else{
    const string& inputFile=cLine.switchValue("-r");
    if("undefined"!=inputFile){
      SupportGraph::DotFileType dotFileType=sG.getDotFileType(inputFile);
      if (SupportGraph::Digraph==dotFileType){
        cout << "Digraph file type\n";
        RunGraph<GraphvizDigraph> tGraph(inputFile);
        cout << tGraph.getVersion() << "\n";
        tGraph.setDebug(cLine.switchValue("-x"));
        if("set"==cLine.switchValue("-i")) tGraph.initializeGraph();
        if("undefined"!=cLine.switchValue("-t")) tGraph.test(cLine.switchValue("-t"));
        if("set"==cLine.switchValue("-atpg")) tGraph.runATPG();
        if("undefined"!=cLine.switchValue("-w")) tGraph.writeGraph(cLine.switchValue("-w"));
      }else if(SupportGraph::Graph==dotFileType){
        cout << "Graph file type not supported. Skipping\n";
      }else{
        cout << "Unknown file type. Skipping\n";
      }
    }
  }
  return 0;
}
