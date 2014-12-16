/* Copyright (C) Kwee Heong Tan 2002 - 2003
   Permission is granted to use this code without restriction as
   long as this copyright notice appears in all source files.
*/
// $Id$
// standard inclusions
#include <boost/config.hpp>
#include <functional>
#include <numeric>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <deque>
// boost inclusions, except config.hpp
#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/pending/queue.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/reverse_graph.hpp>
// local inclusions
#include "Debug.hpp"
#include "DFrontier.hpp"
#include "DLogic.hpp"
// std namespace usage
using namespace std;
// boost namespace usage
using namespace boost;

// extended BGL functionality
//using boost::dfv;
//using boost::dfs;

/** Further areas of investigation
1) Multiple faults
With some changes, the algorithm can be used to simulate multiple faults.
The backtrace algorithm remains unchanged since although the set of input 
vertices may be larger, the suggested signals for each input remains unchanged.
In fact, it is possible for one input to cover more than one fault.

The termination condition has to be changed because we currently finish after
detecting one output. The only criteria to terminate would be an empty D Frontier.
2) Reconvergent signals
As the book [5] indicates, the basic ATPG algorithm does not work in the
case of reconvergent fanout. A modified algorithm called PODEM ( path-oriented
decision making ) introduces a retry step that addresses this.
 */
// ------------------------------------------------------------
// Problem areas
// ------------------------------------------------------------

// ------------------------------------------------------------
// Global Typedefs
// ------------------------------------------------------------
typedef boost::tuple<bool,bool,bool> tripleBool;

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
string SupportGraph::_DotFileStringType[N]={"Unknown","Digraph","Graph"};

SupportGraph::DotFileType SupportGraph::getDotFileType(const string& path){
  /* Can't inquire about a Graphviz's type until we read it in, and
     can read the graph in until we declare its type.
     So read the .dot file and see what's in the first line
   */
  ifstream inDotFile(path.c_str(),ios::in);
  string tokenRead;
  while(inDotFile>>tokenRead){ // read first line only
    // cout << "tokenRead==" << tokenRead << "\n";
    inDotFile.close();
    if(string::npos!=tokenRead.find("digraph")){
      return Digraph;
    }else if(string::npos!=tokenRead.find("graph")){
      return Graph;
    }else{
      return Unknown;
    }
  }
  return Unknown;
};
// ------------------------------------------------------------
// class VertexSignalPair
// ------------------------------------------------------------
template<typename VertexType>
class VertexSignalPair{
/* Class to hold a vertex and a suggested DLogic signal to apply to it.
   By defining operator< only in terms of the Vertex, we can easily
   create sets that are vertex-unique.
   NB - compiler defaults of destructor, copy constructor, operator= sufficient
 */
public:
  VertexSignalPair(VertexType v,DLogic s) : _v(v), _s(s) {};
  VertexType getVertex() { return _v; }
  VertexType getVertex() const { return _v; }
  DLogic getSignal() { return _s; }
  DLogic getSignal() const { return _s; }
  bool operator<(const VertexSignalPair<VertexType>& rhs) const{
    return (this->getVertex() < rhs.getVertex());
  }
  friend ostream& operator<<(ostream& ostr,const VertexSignalPair<VertexType>& objref){
    ostr << "(" << objref.getVertex() << ":"<< objref.getSignal() << ")";
    return ostr;
  };
private:
  VertexSignalPair(){};
  VertexType _v;
  DLogic _s;
};
// ------------------------------------------------------------
// function dumpDFrontier
// function dumpSetVS
// function putDescendantsInDFrontier
// function putDescendantsInPContainer
// ------------------------------------------------------------
template<class VertexType,class VertexAttrMapType>
void dumpDFrontier(std::deque<VertexType>& dF,VertexAttrMapType& vMap){
  typedef std::deque<VertexType> DFrontierType;
  Debug D("dumpDFrontier");
  ostringstream outputString;
  if(0==dF.size()){
    outputString << "empty";
  }else{
    for(typename DFrontierType::iterator iD=dF.begin();iD!=dF.end();++iD){
      outputString << vMap[*iD]["label"]+",";
    }
  }
  D.Dbg("1","DFrontier==",outputString.str());
};
template<class VertexType,class VertexAttrMapType>
void dumpSetVS(std::set<VertexSignalPair<VertexType> > & sVS,VertexAttrMapType& vMap){
  typedef set<VertexSignalPair<VertexType> > SetVertexSignalPairType;
  Debug D("dumpSetVS");
  ostringstream outputString;
  if(0==sVS.size()){
    outputString << "empty";
  }else{
    for(typename SetVertexSignalPairType::iterator iD=sVS.begin();iD!=sVS.end();++iD){
      outputString << vMap[iD->getVertex()]["label"]+"+";
      outputString << iD->getSignal();
      outputString << ",";
    }
  }
  D.Dbg("1","sVS==",outputString.str());
};
template<class VertexType, class GraphType> 
void putDescendantsInDFrontier(VertexType& v,GraphType& g,std::deque<VertexType>& dF){
  typedef typename boost::property_map<GraphType,boost::vertex_attribute_t>::type VertexAttrMapType;
  typedef typename boost::graph_traits<GraphType>::adjacency_iterator AdjacencyIteratorType;
  typedef std::deque<VertexType> DFrontierType;
    Debug D("putDescendantsInDFrontier");
    VertexAttrMapType vMap=boost::get(boost::vertex_attribute,g);
    AdjacencyIteratorType startAI, endAI;
    for(tie(startAI,endAI)=adjacent_vertices(v,g);startAI!=endAI;++startAI){
      typename DFrontierType::iterator iVertexFound=std::find(dF.begin(),dF.end(),*startAI);
      if(dF.end()==iVertexFound){
        dF.push_back(*startAI);
      }//if *startAI not already in DFrontier
    }//for adjacent_vertices
    dumpDFrontier(dF,vMap);
};
template<class VertexType, class GraphType> 
void putDescendantsInPContainer(VertexType& v,GraphType& g,std::deque<VertexType>& cV){
  typedef typename boost::property_map<GraphType,boost::vertex_attribute_t>::type VertexAttrMapType;
  typedef typename boost::graph_traits<GraphType>::adjacency_iterator AdjacencyIteratorType;
    Debug D("putDescendantsInPContainer");
    VertexAttrMapType vMap=boost::get(vertex_attribute,g);
    D.Dbg("1","source vertex==",vMap[v]["label"]);
    AdjacencyIteratorType startAI, endAI;
    for(tie(startAI,endAI)=adjacent_vertices(v,g);startAI!=endAI;++startAI){
        D.Dbg("1","descendant vertex ==",vMap[*startAI]["label"]);
        cV.push_back(*startAI);
    }//for adjacent_vertices
};
// ------------------------------------------------------------
// struct isDTypeFunctor
// struct isXTypeFunctor
// function DPassable
// function OutputConsistent
// function EvaluateOutputs
// function EvaluateSingleInput
// function EvaluateMultipleInputs
// function setOutputEdges
// ------------------------------------------------------------
struct isDTypeFunctor{
  bool operator()(const DLogic& lhs) const{
    return (DLogic::D==lhs || DLogic::_D==lhs);
  }
};
struct isXTypeFunctor{
  bool operator()(const DLogic& lhs) const{
    return (DLogic::X==lhs);
  }
};
template<typename ContainerType>
bool DPassable(ContainerType& vDLogic, DLogic result){
/* Verifies that if D/_D is in the input set, the result
   contains a D/_D.

   This is used to indicate whether the D/_D is being
   sensitised from input to output, or stopped.

   If there is an X in the input, it is too early to tell, return is trivally true.
   If there is no D/_D in the input, the return is trivially true.
   If there is a D/_D in the input, the result is true only if the result is D/_D.
 */
  typename ContainerType::iterator target=std::find_if(vDLogic.begin(),vDLogic.end(),isXTypeFunctor());
  bool bReturn=false;
  if(vDLogic.end()==target){ // container has no Xs
    target=std::find_if(vDLogic.begin(),vDLogic.end(),isDTypeFunctor());
    if(vDLogic.end()!=target){ // container contains a D/_D
      bReturn=(DLogic::D==result||DLogic::_D==result);
    }else{
      bReturn=true;
    }
  }else{
    bReturn=true;
  }
  return bReturn;
};
bool OutputConsistent(DLogic sA, DLogic sB){
/* Verifies that the sA can replace sB without violating consistency rules.
   where sA == signal evaluated for update
         sB == signal already on output edges

   For example, if sA=1 and sB=0, there is inconsistency.
                if sB=X, any value of sA is fine
                if sB=D or _D, we are seeing a stuck-at-fault, so skip as well
 */
    if(DLogic::ONE==sB || DLogic::ZERO==sB ){
      return (sA==sB);
    }else{
      return true;
    }
};
template<class Vertex,class GraphType> 
DLogic EvaluateOutputs(Vertex& v, GraphType& g){
  /* The functionality can be described as follows :
     RealSignals, R=={ONE,ZERO,D,_D}
     Since the vertex has 0 inputs, we are talking about ensuring that all
     out_edges have the same signal value. This can then be used to
     set the remaining out_edges
     Set of out_edges | Evaluated result
     -----------------|-----------------
     R,X,X,X          |  R
     R,R,X,X          |  R
     R1,R2,X,X        |  assert error ( R1 != R2 )
     X,X,X,X          |  X

     By convention, 
     D==good/bad==0/1 and _D==1/0
     Should _D and 1 be considered compatible?
  */ 
  typedef typename boost::property_map<GraphType,boost::vertex_attribute_t>::type VertexAttrMapType;
  typedef typename boost::property_map<GraphType,boost::edge_attribute_t>::type EdgeAttrMapType;
  typedef typename boost::graph_traits<GraphType>::out_edge_iterator OutEdgeIteratorType;
  Debug D("EvaluateOutputs");
  VertexAttrMapType vMap=boost::get(boost::vertex_attribute,g);
  EdgeAttrMapType eMap=boost::get(boost::edge_attribute,g);
  string VertexLabel=vMap[v]["label"];
  D.Dbg("1","vertex label==",VertexLabel);
  OutEdgeIteratorType startEI,endEI;
  set<string> RealSignalSet;
  for(  tie(startEI,endEI)=out_edges(v,g);startEI!=endEI;++startEI){
    string& signal=eMap[*startEI]["label"];
    D.Dbg("1","signal==",signal);
    if("X"!=signal){
      RealSignalSet.insert(signal);
    }//if
  }//for
  string sResult="X";
  switch(RealSignalSet.size()){
  case 0: // bec no driving signal found
    break;
  case 1: // one consistent driving signal found
    sResult=*(RealSignalSet.begin()); 
    break;
  default : // more than one incompatible driving signal
    cout << "Error! More than one incompatible signal found on out_edges of " << VertexLabel << "\n";
    break;
  }//switch
  D.Dbg("1","sResult==",sResult);
  return DLogic(sResult);
};
DLogic EvaluateSingleInput(const string& func,const string& input){
  Debug D("EvaluateSingleInput");
  D.Dbg("1","func=",func);
  D.Dbg("1","input=",input);

  DLogic result(input);

  if("inv"==func || "not"==func){
    result = (! result);
  }
  // D.Dbg("1","result==",result.GetString());
    D.Dbg("1","result==",result);
  return result;
};
DLogic EvaluateMultipleInputs(const string& func,vector<DLogic>& v){
  Debug D("EvaluateMultipleInputs");
  static DLogicAndFunctor<DLogic> DLogicAnd;
  static DLogicOrFunctor<DLogic> DLogicOr;
  D.Dbg("1","func=",func);
  ostringstream outputString;
  std::copy(v.begin(),v.end(),ostream_iterator<DLogic>(outputString,","));
  D.Dbg("1","inputs==",outputString.str());

  DLogic result=DLogic::X;
  if("nand"==func){
     result= ! std::accumulate(v.begin(),v.end(),DLogic::ONE,DLogicAnd);
  }else if("nor"==func){
     result= ! std::accumulate(v.begin(),v.end(),DLogic::ZERO,DLogicOr);
  }else if("and"==func){
     result=std::accumulate(v.begin(),v.end(),DLogic::ONE,DLogicAnd);
  }else if("or"==func){
     result=std::accumulate(v.begin(),v.end(),DLogic::ZERO,DLogicOr);
  }
  D.Dbg("1","result==",result.GetString());
  return result;
};
template<class VertexType,class GraphType>
void setOutputEdges(VertexType& v, GraphType& g, DLogic d){
  typedef typename boost::graph_traits<GraphType>::out_edge_iterator OutEdgeIteratorType;
  typedef typename boost::property_map<GraphType,boost::edge_attribute_t>::type EdgeAttrMapType;
  Debug D("setOutputEdges");
  EdgeAttrMapType eMap=boost::get(edge_attribute,g);
  OutEdgeIteratorType startEI,endEI;
  string signal=d.GetString();
  for(  tie(startEI,endEI)=out_edges(v,g);startEI!=endEI;++startEI){
    string& edgeLabel=eMap[*startEI]["label"];
    if("X"==edgeLabel){
      eMap[*startEI]["label"]=signal;
    }else{
      cout << "Warning! Output edge should be X\n";
    }
  }
};
// ------------------------------------------------------------
// class XEdgeVisitor
// ------------------------------------------------------------
template<typename GraphType>
class XEdgeVisitor : public boost::default_dfs_visitor{
/* Set an un-initialized edge to "X"
   Combined with DFS traversal, sets all un-initialized edges to "X"
   NB - compiler defaults of destructor, copy constructor sufficient
 */
public :
  typedef typename boost::property_map<GraphType,boost::vertex_attribute_t>::type VertexAttrMapType;
  typedef typename boost::property_map<GraphType,boost::edge_attribute_t>::type EdgeAttrMapType;
  XEdgeVisitor(GraphType& g){ _EdgeAttrMap=boost::get(edge_attribute,g); }
  template <class Edge, GraphType> void examine_edge(Edge e, const GraphType &){
    if(""==_EdgeAttrMap[e]["label"])
      _EdgeAttrMap[e]["label"]="X";
  }
private:
  XEdgeVisitor();
  XEdgeVisitor& operator=(const XEdgeVisitor&);
  EdgeAttrMapType _EdgeAttrMap;
};
// ------------------------------------------------------------
// class GetSignalFunctor
// ------------------------------------------------------------
template<typename GraphType>
class GetSignalFunctor : public unary_function<typename boost::graph_traits<GraphType>::edge_descriptor,DLogic>{
/* functor to return signal value of an edge
   NB - compiler defaults of desctructor, copy constructor sufficient
 */
public:
  typedef typename boost::property_map<GraphType,boost::edge_attribute_t>::type EdgeAttrMapType;
  typedef typename boost::graph_traits<GraphType>::edge_descriptor EdgeType;
  GetSignalFunctor(GraphType& g){ _E=boost::get(edge_attribute,g);};
  DLogic operator()(const EdgeType& e) const {
    string& signal=_E[e]["label"];
    return DLogic(signal);
  };
private:
  GetSignalFunctor();
  GetSignalFunctor& operator=(const GetSignalFunctor&);
  EdgeAttrMapType _E;
};
// ------------------------------------------------------------
// class BacktraceVisitor
// ------------------------------------------------------------
template<typename GraphType>
class BacktraceVisitor : public boost::default_dfs_visitor{
/* Given a vertex in the DFrontier, we traceback until we find 
   input nodes. These are kept in a container of unique vertices,
   actually SetVertexSignalPair.
   Q : How do we know whether the input nodes have already been visited?
       There are no Xs on the input node.
   NB - compiler defaults of desctructor, copy constructor sufficient
*/
public:
  typedef typename boost::property_map<GraphType,boost::vertex_attribute_t>::const_type ConstVertexAttrMapType;
  typedef typename boost::property_map<GraphType,boost::edge_attribute_t>::type EdgeAttrMapType;
  typedef typename boost::graph_traits<GraphType>::in_edge_iterator InEdgeIteratorType;
  typedef typename boost::graph_traits<GraphType>::out_edge_iterator OutEdgeIteratorType;
  typedef typename boost::graph_traits<GraphType>::vertex_descriptor VertexType;
  typedef set<VertexSignalPair<VertexType> > SetVertexSignalPairType;

  BacktraceVisitor(const GraphType& g,EdgeAttrMapType& eM,SetVertexSignalPairType& setVS) : _EdgeAttrMap(eM), _SetVS(setVS){
  /* BacktraceVisitor is used with GraphType==<reverse_graph<G>>, so the GraphType edge attr map has to be passed in */
  }
  template<class Vertex, GraphType> bool HasXs(Vertex v,const GraphType& g){
    Debug D("HasXs");
    string VertexLabel=boost::get(boost::vertex_attribute,g,v)["label"];
    D.Dbg("1","vertex label==",VertexLabel);

    InEdgeIteratorType startEI,endEI;
    bool bReturn=false;
    for(tie(startEI,endEI)=in_edges(v,g);startEI!=endEI;++startEI){
      string& edgeLabel=_EdgeAttrMap[*startEI]["label"];
      D.Dbg("1","edgeLabel==",edgeLabel);       
      if("X"==edgeLabel){
        bReturn=true;
        break;
      }
     }
    return bReturn;
  }
  template <class Vertex,  GraphType> DLogic suggestEnablingSignal(Vertex v, const GraphType & g){
    /* Looks at source vertices of all incoming edges and collect
       their func values. Return enabling signal, based on func values if
       NAND,NOR,AND and OR found. Otherwise, no simple algorithm exists, so we
       return DLogic::ONE
       Refactor this function if more models have to be supported.
    */
    Debug D("getEnablingSignal");
    set<string> setFunc;
    // process source vertices, putting func in set
    InEdgeIteratorType startEI,endEI;
    Vertex vSource;
    for(tie(startEI,endEI)=in_edges(v,g);startEI!=endEI;++startEI){
      vSource=source(*startEI,g);
      NodeHelper hSource(boost::get(vertex_attribute,g,vSource)["label"]);
      setFunc.insert(hSource.getFunc());
    }//for

    DLogic dResult=DLogic::ONE; // default
    string& sFunc=const_cast<string&>(*(setFunc.begin()));
    switch(setFunc.size()) {
    case 0: 
      cout << "Warning: vertex drives no function model.\n";
      break;
    case 1:
      if("nand"==sFunc || "and"==sFunc){
        dResult=DLogic::ONE;
      }else if("nor"==sFunc || "or"==sFunc){
        dResult=DLogic::ZERO;
      }
      break;
    default:
      cout << "Warning: vertex drives multiple function models. Using default enabling signal.\n";
      break;
    }//switch
    D.Dbg("1","dResult==",dResult.GetString());
    return dResult;
  }
  template <class Vertex, GraphType> void discover_vertex(Vertex v, const GraphType & g){
    Debug D("discover_vertex");
    string VertexLabel=boost::get(boost::vertex_attribute,g,v)["label"];
    D.Dbg("1","vertex label==",VertexLabel);
    NodeHelper VertexHelper(VertexLabel);
    const string& VertexFunc=VertexHelper.getFunc();
    if("in"==VertexFunc){
      if(true==HasXs(v,g)){ // cannot tweak here
        DLogic driveSignal=suggestEnablingSignal(v,g);
        _SetVS.insert(VertexSignalPair<Vertex>(v,driveSignal));
      }
    }//if input vertices
  }
private:
  BacktraceVisitor();
  BacktraceVisitor& operator=(const BacktraceVisitor&);
  EdgeAttrMapType& _EdgeAttrMap;
  SetVertexSignalPairType& _SetVS; // set of vertex-signal
};
// ------------------------------------------------------------
// class NodeHelper
// ------------------------------------------------------------
class NodeHelper{
  /* Class implements format policy of Vertex label
     label == name:func, with default func==noop
     using defaults for constructor and copy constructor
     NB - compiler defaults of destructor, copy constructor, operator= sufficient
   */
public:
  NodeHelper(const string& label){
    string::size_type dotLocation=label.find_first_of(":");
    if(string::npos!=dotLocation){
      setName(label.substr(1,dotLocation));
      setFunc(label.substr(dotLocation+1));
    }else{
      setName(label);
      setFunc("noop");
    }
  }
  void setName(const string& name){ _name=name; }
  void setFunc(const string& func){ _func=func; }
  const string& getName(){return _name;}
  const string& getFunc(){return _func;}
private:
  string _name;
  string _func;
};
// ------------------------------------------------------------
// class RunGraph
// ------------------------------------------------------------
template<class GraphType>
class RunGraph{
/* class to implement Podem algorithm
*/
public:
      typedef typename boost::property_map<GraphType,boost::vertex_attribute_t>::type VertexAttrMapType;
	  typedef typename boost::property_map<GraphType,boost::edge_attribute_t>::type EdgeAttrMapType;
      typedef typename boost::graph_traits<GraphType>::vertex_descriptor VertexType;
      typedef typename boost::graph_traits<GraphType>::vertex_iterator VertexIteratorType;
      typedef typename boost::graph_traits<GraphType>::edge_iterator EdgeIteratorType;
      typedef typename boost::graph_traits<GraphType>::in_edge_iterator InEdgeIteratorType;
      typedef typename boost::graph_traits<GraphType>::out_edge_iterator OutEdgeIteratorType;
      typedef std::deque<VertexType> DFrontierType;
      typedef std::deque<VertexType> PropagateContainerType;
      typedef set<VertexSignalPair<VertexType> > SetVertexSignalPairType;
      // Needs boost::               Y                    N                            Y                    N
  BOOST_STATIC_ASSERT((boost::is_same<GraphType,GraphvizGraph>::value || boost::is_same<GraphType,GraphvizDigraph>::value));
      RunGraph(const string& path);
      ~RunGraph();
      void setDebug(const string& dString);
      void initializeGraph();
      void backtraceVertex(const VertexType& v);
      // propagate code
      bool processOutput(VertexType v,GraphType& g,PropagateContainerType& cv,DLogic outS,DLogic currentS);
      tripleBool propagateChange(VertexType v, GraphType& g,PropagateContainerType& cv);
      tripleBool propagateVertex(VertexType v, DLogic driveSignal);
      // driver code
      void runATPG();
      void test(const string& startLabel);

      VertexType endVertex(){ return *vertices(_g).second; }; // syntatic sugar for making vertex checks clearer. Q, should keep value or call vertices each time?
      VertexType findVertexWithLabel(const string& label);
      void writeGraph(const string& path);
      void seedDFrontier(DFrontierType& dF);
      void updateDFrontier(DFrontierType& dF);
      void printFinishStats(bool DFrontierEmpty,bool FirstOutputFound, bool FirstNonDPassable, bool FirstInconsistentOutput);
      const char* getVersion(){ return _Version; };
      static char* _Version;
private:
      RunGraph();
      RunGraph(const RunGraph&);
      RunGraph& operator=(const RunGraph&);
      GraphType _g;
      VertexAttrMapType _v;
      EdgeAttrMapType _e;
      DFrontierType _df;
      reverse_graph<GraphType>* _pRG;
      SetVertexSignalPairType _setVS;
};
template<typename G>
char* RunGraph<G>::_Version="$Id$";
template<typename G>
RunGraph<G>::RunGraph(const string& path){
  cout << "Reading " << path << "\n";
  read_graphviz(path.c_str(),_g);
  _v=boost::get(vertex_attribute,_g);
  _e=boost::get(edge_attribute,_g);
  _df.clear();
  _setVS.clear();
  _pRG=new reverse_graph<G>(_g);
};
template<typename G>
RunGraph<G>::~RunGraph(){
  delete _pRG;
};
template<typename G>
void RunGraph<G>::writeGraph(const string& path){
  cout << "Writing " << path << "\n";
  write_graphviz(path.c_str(),_g);
};
template<typename G>
typename RunGraph<G>::VertexType RunGraph<G>::findVertexWithLabel(const string& label){
   VertexIteratorType viStart,viEnd;
   for(tie(viStart,viEnd)=vertices(_g);viStart!=viEnd;++viStart){
      if(label==_v[*viStart]["label"]){
        return *viStart;
      }
   }
   return *viEnd;
};
template<typename G>
void RunGraph<G>::seedDFrontier(DFrontierType& dF){
/* Search graph for edges with D/_D and put their target vertices
   into DFrontier. Checks before inserting vertices so that DFrontier
   has no duplicate vertices.
   Note: Tradeoff performance to search for all faults in graph, so that
   we can study multiple faults. Alternate implementation could return when
   first fault found.
*/
   Debug D("seedDFrontier");
   EdgeIteratorType firstEI,lastEI;
   VertexType vTarget;
   for(tie(firstEI,lastEI)=edges(_g);firstEI!=lastEI;++firstEI){
     if(("D"==_e[*firstEI]["label"])||("_D"==_e[*firstEI]["label"])){
      vTarget=target(*firstEI,_g);
      typename DFrontierType::iterator iVertexFound=std::find(dF.begin(),dF.end(),vTarget);
      if(dF.end()==iVertexFound){
        dF.push_back(vTarget);
      }//if vTarget not in DFrontier
     }//if edge has fault injected
   }//for all edges
   dumpDFrontier(dF,_v);
};
template<typename G>
void RunGraph<G>::updateDFrontier(DFrontierType& dF){
/* If there are no more undriven inputs of the front element, remove it */
  Debug D("updateDFrontier");
  assert(0!=dF.size()); // function should not be called if DFrontier is empty
  InEdgeIteratorType firstEI,lastEI;
  set<string> InputSignalSet;
  VertexType vTarget=*(dF.begin());
  for(tie(firstEI,lastEI)=in_edges(vTarget,_g);firstEI!=lastEI;++firstEI){
    string& signal=_e[*firstEI]["label"];
    D.Dbg("1","signal==",signal);
    if("X"==signal){
      InputSignalSet.insert(signal);
    }
  }//for all edges
  if(0==InputSignalSet.size()){
    D.Dbg("1","updateDFrontier: ","removing vertex");
    dF.pop_front();
  }
  dumpDFrontier(dF,_v);
};
template<typename G>
void RunGraph<G>::printFinishStats(bool DFrontierEmpty,bool FirstOutputFound, bool FirstNonDPassable, bool FirstInconsistentOutput){
  if(true==FirstNonDPassable||true==FirstInconsistentOutput){
    cout << "Bad run\n";
  }else{
    cout << "Good run\n";
  }
  cout << boolalpha;
  cout << "\t1) DFrontier empty :\t" << DFrontierEmpty << "\n";
  cout << "\t2) First output found :\t" << FirstOutputFound << "\n";
  cout << "\t3) First non D passable found :\t" << FirstNonDPassable << "\n";
  cout << "\t4) First inconsistent output found :\t" << FirstInconsistentOutput << "\n";
};
template<typename G>
void RunGraph<G>::setDebug(const string& dString){
  Debug::specify(dString.c_str());
};
template<typename G>
void RunGraph<G>::initializeGraph(){
  Debug D("initializeGraph");
  XEdgeVisitor<G> initV(_g);
  depth_first_search(_g,visitor(initV));
  cout << "\n";
};
template<typename G>
void RunGraph<G>::backtraceVertex(const VertexType& v){
  Debug D("backtraceGraph");
  string& vertexLabel=_v[v]["label"];
  D.Dbg("1","vertex label==",vertexLabel);

  typedef std::vector<default_color_type> ColorMapType;
  ColorMapType GraphColorMap(num_vertices(_g));
  BacktraceVisitor<reverse_graph<G> > backtraceVisitor(_g,_e,_setVS);
  depth_first_visit(*_pRG,v,backtraceVisitor,&GraphColorMap[0]);
};
template<typename G>
tripleBool RunGraph<G>::propagateVertex(VertexType v, DLogic driveSignal){
    Debug D("propagateVertex");
    string VertexLabel=_v[v]["label"];
    D.Dbg("1","vertex label==",VertexLabel);
    deque<VertexType> cV;
    cV.push_back(v);
    setOutputEdges(v,_g,driveSignal);
    VertexType vOrigin;
    bool bOutputFound=false,bInconsistentOutput=false,bNonDPassable=false;
    while(0!=cV.size()){
      ostringstream outputString;
      for(typename deque<VertexType>::iterator it=cV.begin();it!=cV.end();++it){
        outputString << _v[*it]["label"]+",";
      }
      D.Dbg("1","cV==",outputString.str());
      vOrigin=*cV.begin();
      tie(bOutputFound,bInconsistentOutput,bNonDPassable)=propagateChange(vOrigin,_g,cV);
      cV.pop_front();
      if(bOutputFound||bInconsistentOutput||bNonDPassable) break;
    }
    return tripleBool(bOutputFound,bInconsistentOutput,bNonDPassable);
};
template<typename G>
bool RunGraph<G>::processOutput(VertexType v,G& g,PropagateContainerType& cv,DLogic outS,DLogic currentS){
  Debug D("processOutput");
  bool bInconsistentOutput=false;
  if(false==OutputConsistent(outS,currentS)){
    bInconsistentOutput=true;
  }else{
    if(DLogic::X!=outS){
      setOutputEdges(v,g,outS);
      putDescendantsInPContainer(v,g,cv);
      if(DLogic::D==outS||DLogic::_D==outS) putDescendantsInDFrontier(v,g,_df);
    }
  }
  D.Dbg("1","bInconsistentOutput==",bInconsistentOutput);
  return bInconsistentOutput;
};
template<typename G>
tripleBool RunGraph<G>::propagateChange(VertexType v, G& g,PropagateContainerType& cv) {
    Debug D("propagateChange");
    string VertexLabel=_v[v]["label"];
    D.Dbg("1","vertex label==",VertexLabel);
    NodeHelper VertexHelper(VertexLabel);
    const string& VertexFunc=VertexHelper.getFunc();
    DLogic outSignal,currentSignal;

    bool bOutputFound=false;
    bool bInconsistentOutput=false;
    bool bNonDPassable=false;

    if("noop"!=VertexFunc){
      if("out"==VertexFunc){
        bOutputFound=true;
      }else if("in"==VertexFunc) {
        outSignal=EvaluateOutputs(v,g);
        D.Dbg("1","outSignal==",outSignal.GetString());
        processOutput(v,g,cv,outSignal,DLogic::X);
      }else{
        vector<DLogic> vSignals;
        int totalDegree=degree(v,g); // Bug in in_degree
        int numOutputs=out_degree(v,g);
        int numInputs=totalDegree-numOutputs;
        
        InEdgeIteratorType startEI,endEI;
        switch(numInputs){
        case 0 :
          break;
        case 1 : // single input, see Note 1
          tie(startEI,endEI)=in_edges(v,g);
          outSignal=EvaluateSingleInput(VertexFunc,_e[*startEI]["label"]);
          currentSignal=EvaluateOutputs(v,g);
          bInconsistentOutput=processOutput(v,g,cv,outSignal,currentSignal);
          break;
        default : // multi-inputs
          tie(startEI,endEI)=in_edges(v,g);
          // see podem.oln Note1 : VC6 error if std:: left out of transform
          // see podem.oln Note2 : VC6 error if std:: left out of back_inserter
          std::transform(startEI,endEI,std::back_inserter(vSignals),GetSignalFunctor<G>(_g));
          outSignal=EvaluateMultipleInputs(VertexFunc,vSignals);
          currentSignal=EvaluateOutputs(v,g);
          if(false==DPassable(vSignals,outSignal)){
            bNonDPassable=true;
            D.Dbg("1","multi-inputs : ","bNonDPassable set to true");
          }
          bInconsistentOutput=processOutput(v,g,cv,outSignal,currentSignal);
          break;
        }//switch(numInputs)
      }
    }//if("noop"!=VertexFunc
    return tripleBool(bOutputFound,bInconsistentOutput,bNonDPassable);
};
/** Main call tree for runATPG

    runATPG --+--> initializeGraph
              +--> seedDFrontier
              +--> backtraceVertex --+--> BacktraceVisitor
              |                      +--> depth_first_visit
              +--> propagateVertex --+--> propagateChange --+--> EvaluateOutputs
              |                                             +--> EvaluateSingleInput
              |                                             +--> GetSignalFunctor
              |                                             +--> EvaluateMultipleInputs
              |                                             +--> processOutput
              +--> printFinishStats

 */
/** Algorithm descrition for runATPG

The overall control logic is implemented in the member function testATPG.

In step (0), a fault is either a D or a _D on some edge. A D means that a 
good circuit shows a ONE, while the faulty circuit shows a ZERO. This is 
called a stuck-at-zero fault. Similarly, a _D represents a stuck-at-one fault.

As the simulation proceeds, the fault will be propagated from one node to
another until it hopefully reaches an output node. This means that the fault
signal D or _D will pass along the path until it reaches the output node.

At any time, the nodes whose inputs are D or _D are collectively called
the D Frontier. During simulation, this D Frontier fans out until they 
encounter an output node. The D Frontier is implemented in the class
DFrontier.

Initially, the only vertex in the D Frontier is the vertex whose input is the
fault to be simulated. This is implemented in the function seedDFrontier.
  
As the simulation progresses, more vertices are added to the D Frontier
as the D or _D gets passed along to connected vertices. Also, vertices 
are removed from the D Frontier when their D or _D inputs are evaluated,
and outputs updated.

In step (1), the backtracing is easy to implement with BGL's reverse_graph.
In this implementation, I chose to skip the D or _D edge itself and only
backtrace the remaining unknown edges back to their inputs. This is done
using BGL's depth_first_visit with a custom BacktraceVisitor.

When an input vertex is found, we select a logic value, either ONE or ZERO
to simulate it. A suggested logic value is obtained by examining all the
vertices driven by this input, and using the enabling values of the vertices'
functionality. For example, if an input is connected to a nand gate, the
enabling value of a nand is ONE. If an input is connected to a nor gate,
the enabling value is ZERO.

In step (2), t  prevents this.
he simulation is implemented with propagateVertex call. The
propagateVertex uses a container of vertices to keep track of pending
changes. This is because a vertex may be updated several times depending
on when inputs arrive. The standard DFS or BFS is designed to visit each
vertex once, and so will not propagate all the inputs correctly.

Step (3) is again implemented in propagateVertex. 
 
 */
/** Termination conditions
While the book [5] does not describe this in detail, there are two situations
in which the algorithm will terminate.

1) It is possible that a fault in the circuit is not observable.
   The algorithm terminates when the D Frontier is empty.

   A circuit may be non observable structurally, ie there are no output nodes
   traceable from the fault. This means we will never reach an output node.

   A circuit may also be non observable because the specific fault cannot
   be propagated. This means that before we reach an output node, the fault
   propagation is blocked because of a controlling input somewhere along
   the path. An example is the sample input 2.dot, which differs from 
   the demo circuit Not2.dot only in the seed fault is _D instead of D.

2) If a fault is observable, we should be able to see the path of D propagation
   to the output node.

As part of my investigation, the conditions for termination are kept tracked
of. These are :
   - an output node is found
   - the D path is blocked
   - an inconsistent output is found.  
   - DFrontier being empty

The first 3 are the tripleBool returned from propagateChange, while an empty
D Frontier can be queried from the DFrontier data structure.

These states are printed out at the end of the run, in printFinishStats
so that better understand of the current algorithm and its behavior on
different circuits can be studied.
 
 */
template<typename G>
void RunGraph<G>::runATPG(){
  Debug D("runATPG");

  bool bFirstOutputFound=false,bFirstNonDPassable=false,bFirstInconsistentOutput=false;
  bool bStopRun=false; // for testing purpose, set to false or remove later
  VertexType vObjective;
  initializeGraph();
  seedDFrontier(_df);
  //  while(!(_df.empty() || bFirstOutputFound || bFirstNonDPassable || bFirstInconsistentOutput || bStopRun)) {
  while(!(_df.empty() || bFirstOutputFound || bStopRun)) {
    vObjective=*(_df.begin());
    backtraceVertex(vObjective);
    dumpSetVS(_setVS,_v);    
    for(typename SetVertexSignalPairType::iterator it=_setVS.begin();it!=_setVS.end();++it){
        tie(bFirstOutputFound,bFirstInconsistentOutput,bFirstNonDPassable)=propagateVertex(it->getVertex(),it->getSignal());
        if(bFirstOutputFound||bFirstNonDPassable||bFirstInconsistentOutput) break;
    }
    _setVS.clear();
    updateDFrontier(_df); // remove vObjective if no more undriven inputs
    // writeGraph("debug.dot");
    bStopRun=false;
  };//while - done with PODEM
  printFinishStats(_df.empty(),bFirstOutputFound,bFirstNonDPassable,bFirstInconsistentOutput);
};
template<typename G>
void RunGraph<G>::test(const string& startLabel){
  Debug D("test - new");
};
// ------------------------------------------------------------
// Collection of tests - should not be compiled unless we need it
// ------------------------------------------------------------
#if 0
template<typename G>
void RunGraph<G>::test(const string& startLabel){
  Debug D("test - hasX");

  BacktraceVisitor<G> forwardVisitor(_g,_v,_e,_setVS);
  D.Dbg("1","depth_first_search: ","forwardVisitor");
  depth_first_search(_g,visitor(forwardVisitor));
  cout <<"\n";
  BacktraceVisitor< reverse_graph<G> > reverseVisitor(*_pRG,_v,_e,_setVS);
  D.Dbg("1","depth_first_search: ","reverseVisitor");
  depth_first_search(*_pRG,visitor(reverseVisitor)); // reverse graph takes reverseVisitor of type reverse_graph<G>
  cout <<"\n";
};
#endif
#if 0
template<typename G>
void RunGraph<G>::test(const string& startLabel){
  // use zeroinput.dot as input
  Debug D("test - reverse_graph");
  cout << "No of vertices==" << num_vertices(*_pRG) << "\n";
#if defined(REVERSE_PROBLEM)
  DfsVisitor<G> forwardVisitor(_g);
#else
  DfsVisitor<G> forwardVisitor(_g,_v,_e);
#endif
  depth_first_search(_g,visitor(forwardVisitor));
  cout <<"\n";
  depth_first_search(*_pRG,visitor(forwardVisitor)); // reverse graph takes forwardVisitor of type G
  cout <<"\n";

#if defined(REVERSE_PROBLEM)
  DfsVisitor< reverse_graph<G> > reverseVisitor(*_pRG);
#else
  DfsVisitor< reverse_graph<G> > reverseVisitor(*_pRG,_v,_e);
#endif
  depth_first_search(*_pRG,visitor(reverseVisitor)); // reverse graph takes reverseVisitor of type reverse_graph<G>
  cout <<"\n";

};
#endif
#if 0
template<typename G>
void RunGraph<G>::test(const string& startLabel){
  Debug D("test - EvaluateMultipleInputs");
  //  DLogic result=EvaluateSingleInput("not","_D");
  DLogic result;
  vector<DLogic> vA;
  for(DLogic dA=DLogic::first();dA.valid();++dA){
    for(DLogic dB=DLogic::first();dB.valid();++dB){
      vA.clear();
      vA.push_back(dA);
      vA.push_back(dB);
      result=EvaluateMultipleInputs("nand",vA);
    }
  }
};
#endif// all inactive tests
// ------------------------------------------------------------
// Notes
// ------------------------------------------------------------
/*
Note 1 : limitation of single input
   The current model supported is the NOT, which will allow D/_D to
   pass always. 
   Thus, DPassable() is not called.

*/ 
