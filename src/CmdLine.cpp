/* Copyright (C) Kwee Heong Tan 2002 - 2003
   Permission is granted to use this code without restriction as
   long as this copyright notice appears in all source files.
*/
// $Id: CmdLine.cpp,v 1.16 2002/11/28 18:24:45 khtan Exp khtan $
#include "CmdLine.hpp" // include this first so that pragma warning will take effect
using namespace std;

CmdLine::CmdLine(){
  debug=false;
}
void CmdLine::addParameterSwitch(const string flag,const string initialValue,const string help){
  m.insert(mapType::value_type(flag,initialValue));
  h.insert(mapType::value_type(flag,help));
}
void CmdLine::addStandaloneSwitch(const string flag,const string help){
  addParameterSwitch(flag,"unset",help);
}
const string& CmdLine::switchValue(const string flag){
  return m[flag];
}
const string& CmdLine::switchHelp(const string flag){
  return h[flag];
}
void CmdLine::process(int argc,char** argv){
        if(debug==true) cout << "Processing input arguments\n";
        for(int i=1;i<argc;i++){ // C++ skip command name itself, in argv[0]
            if(debug==true){
              cout << "\targv[" << i << "] " << argv[i] << "\n";
            }
            string flag=argv[i];
            mapType::iterator pos=m.find(flag);
            
            if(pos==m.end()){
              l.push_back(argv[i]);
            }else if ("unset"==(*pos).second){
              m.erase(pos);
              m.insert(mapType::value_type(flag,"set"));
            }else{
              m.erase(pos);
              m.insert(mapType::value_type(flag,argv[++i]));
            }
        }
}
int CmdLine::NumberOfRemainingParameters(){
  return l.size();
}
const char * CmdLine::RemainingParameter(int index){
  return l[index];
  // return l.at(index);
}
void CmdLine::printSwitches(ostream& s){
  mapType::const_iterator mI;
  s.setf(ios::left);
  for( mI=m.begin();mI!=m.end();++mI){
    s << "\t";
    s.width(10);
    #ifdef STL96
        s << (*mI).first;
        s.width(20);
        s << switchHelp((*mI).first) << "\n";
    #else
	s << mI->first;
	s.width(20);
	s << switchHelp(mI->first) << "\n";
    #endif
  }
  s.unsetf(ios::left);
}
void CmdLine::dump(ostream& s) const{
    const_cast<CmdLine *>(this)->dump(s);
}
void CmdLine::dump(ostream& s){
  s << "State of switches\n";
  mapType::const_iterator mI;
  s.setf(ios::left);
  for( mI=m.begin();mI!=m.end();++mI){
    s << "\t";
    s.width(10);
    #ifdef STL96
       s << (*mI).first;
       s.width(20);
       s << (*mI).second << "\t:"<< switchHelp((*mI).first) << "\n";
    #else
       s << mI->first;
       s.width(20);
       s << mI->second << "\t:"<< switchHelp(mI->first) << "\n";
    #endif
  }
  s.unsetf(ios::left);
  s << "Remaining parameters\n";
  for(int i=0;i<NumberOfRemainingParameters();++i){
    s << "\t" << RemainingParameter(i) << "\n";
  }
}

ostream& operator<<(ostream &s,const CmdLine& c){
  c.dump(s);
  return s;
}
