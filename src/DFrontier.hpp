/* Copyright (C) Kwee Heong Tan 2002 - 2003
   Permission is granted to use this code without restriction as
   long as this copyright notice appears in all source files.
*/
#ifndef __DFrontier__
#define __DFrontier__
#include <vector>
#include <string>
using std::vector;
using std::string;

template <typename GraphType>
class DFrontier{
typedef typename boost::graph_traits<GraphType>::vertex_descriptor VertexType;
public:
   DFrontier() : _reachedPO(false) {}
   const string getVersion(){ return _Version; };
   bool empty(){ return _stackVertex.empty(); }
   bool reachedPO() { return _reachedPO; }
   void setReachedPO() { _reachedPO=true;}
   void addObjective(VertexType& v){ // optimization heuristic: weight Vertex based on distance from PI
       _stackVertex.push_back(v);
   }
   VertexType getObjective(){ 
      VertexType v=_stackVertex.front();
      _stackVertex.pop_back(); 
      return v;
   };
private:
   static string _Version;
   vector<VertexType> _stackVertex;
   bool _reachedPO;
};
template<typename G>
string DFrontier<G>::_Version="$Id: DFrontier.hpp,v 1.2 2002/11/21 18:09:12 khtan Exp $";
#endif // __DFrontier__
