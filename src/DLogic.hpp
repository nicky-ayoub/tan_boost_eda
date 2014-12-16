/* Copyright (C) Kwee Heong Tan 2002 - 2003
   Permission is granted to use this code without restriction as
   long as this copyright notice appears in all source files.
*/
//---------------------------------------------------------------------- 
//
//  Filename: $Id: DLogic.h,v 1.3 2002/11/21 18:09:02 khtan Exp $
//
//  Description:
//
//---------------------------------------------------------------------- 
#ifndef _DLogic_H
#define _DLogic_H
     
// The OSTREAM and ISTREAM typedefs are used due to a problem that occurs
// when compiling enum classes using MS Visual C++ 5.0; even with the 
// "using namespace std" declaration, lines containing ostream and istream 
// are flagged as ambiguous by VC++.
     
#include <string>
#include <iostream>
     
using namespace std;
     
typedef std::ostream  OSTREAM;
typedef std::istream  ISTREAM;
     
     
//---------------------------------------------------------------------- 
#ifdef ENUMS_USE_EXCEPTION_HANDLING
    #define DLogic_THROW_DECL    throw (const char *) 
    #define DLogic_THROW_EXEC    throw m_errorMsg;
#else
    #define DLogic_THROW_DECL
    #define DLogic_THROW_EXEC    DLogic::RangeError()
#endif
//---------------------------------------------------------------------- 
class DLogic{
public:  
    static const DLogic  ZERO;
    static const DLogic  ONE;
    static const DLogic  D;
    static const DLogic  _D;
    static const DLogic  X;
    static const DLogic  _UNDEFINED;
    // khtan added
    static const DLogic DLogicAnd[5][5];
    static const DLogic DLogicOr[5][5];
    static const DLogic DLogicNot[5];
public:
    DLogic(){
        m_defPtr = _UNDEFINED.m_defPtr;
    }
    DLogic (const DLogic&  objref ){
        m_defPtr = objref.m_defPtr;
    }
    explicit DLogic ( const string&  estr );
    explicit DLogic ( const int  ival );
    // GetInt is used instead of "operator int()" to enforce explicit 
    // conversion from DLogic to int.
    short GetInt() const DLogic_THROW_DECL {
        if (m_defPtr == _UNDEFINED.m_defPtr){
            DLogic_THROW_EXEC;
        }
        return m_defPtr->m_value;
    }
    bool valid() const {
        return (m_defPtr != _UNDEFINED.m_defPtr);
    }
    DLogic& operator= ( const DLogic&  objref ){
      // Q - no need to check for assignment to self?
        m_defPtr = objref.m_defPtr;
        return *this;
    }
    bool operator== ( const DLogic&  objref ) const DLogic_THROW_DECL {
        if ( (m_defPtr == _UNDEFINED.m_defPtr) || (objref.m_defPtr == _UNDEFINED.m_defPtr) ){
            DLogic_THROW_EXEC;
        }
        return m_defPtr == objref.m_defPtr;
    }
    bool operator!= ( const DLogic&  objref ) const DLogic_THROW_DECL {
        if ( (m_defPtr == _UNDEFINED.m_defPtr) || (objref.m_defPtr == _UNDEFINED.m_defPtr) ){
            DLogic_THROW_EXEC;
        }
        return m_defPtr != objref.m_defPtr;
    }
    bool operator< ( const DLogic&  objref ) const DLogic_THROW_DECL {
        if ( (m_defPtr == _UNDEFINED.m_defPtr) || (objref.m_defPtr == _UNDEFINED.m_defPtr) ){
            DLogic_THROW_EXEC;
        }
        return m_defPtr < objref.m_defPtr;
    }
    bool operator<= ( const DLogic&  objref ) const DLogic_THROW_DECL {
        if ( (m_defPtr == _UNDEFINED.m_defPtr) || (objref.m_defPtr == _UNDEFINED.m_defPtr) ){
            DLogic_THROW_EXEC;
        }
        return m_defPtr <= objref.m_defPtr;
    }
    bool operator> ( const DLogic&  objref ) const DLogic_THROW_DECL {
        if ( (m_defPtr == _UNDEFINED.m_defPtr) || (objref.m_defPtr == _UNDEFINED.m_defPtr) ){
            DLogic_THROW_EXEC;
        }
        return m_defPtr > objref.m_defPtr;
    }
    bool operator>= (const DLogic&  objref ) const DLogic_THROW_DECL {
        if ( (m_defPtr == _UNDEFINED.m_defPtr) || (objref.m_defPtr == _UNDEFINED.m_defPtr) ){
            DLogic_THROW_EXEC;
        }
        return m_defPtr >= objref.m_defPtr;
    }
    DLogic operator++();
    DLogic operator++(int);
    DLogic operator--();
    DLogic operator--(int);
    const char* GetLabel() const;
    string GetString() const;
    friend OSTREAM& operator<< ( OSTREAM& ostr, const DLogic&  objref );
    friend ISTREAM& operator>> ( ISTREAM& ostr, DLogic&  objref );
    // khtan added
    const DLogic& operator&&(const DLogic &rhs) const{
      return DLogic::DLogicAnd[m_defPtr->m_value][rhs.m_defPtr->m_value];
    }
    const DLogic& operator||(const DLogic &rhs) const{
      return DLogic::DLogicOr[m_defPtr->m_value][rhs.m_defPtr->m_value];
    }
    const DLogic& operator!() const{
      return DLogic::DLogicNot[m_defPtr->m_value];
    }
public: 
    static DLogic first(){ return ZERO; }
    static DLogic last(){ return X; }
    static int count(){ return m_defArrCount - 1; }
private:  
    struct EnumDefStruct{
        short   m_value;
        string  m_name;
        EnumDefStruct(const int value,const char* name):m_value(value), m_name(name){}
    };
    // The class constructor with a EnumDefStruct* argument should only 
    // be used to construct the static constants that make up the enum 
    // values; thus it's declared private.
    DLogic (EnumDefStruct*  defPtr ){ m_defPtr = defPtr; }
    static void RangeError();
private:
    EnumDefStruct*  m_defPtr;
private:
    static short          m_defArrCount; 
    static EnumDefStruct  m_defArr[];
    static const char*    m_errorMsg;
};
    #include <functional>

template<typename T>
struct DLogicNotFunctor : std::unary_function<T,T>{
  const T& operator()(const T& rhs) const{
    return (! rhs);
  }
};
template<typename T>
struct DLogicAndFunctor : std::binary_function<T,T,T>{
  const T& operator()(const T& lhs,const T& rhs) const{
    return ( lhs && rhs );
  }
};
template<typename T>
struct DLogicOrFunctor : std::binary_function<T,T,T>{
  const T& operator()(const T& lhs, const T& rhs) const{
    return (lhs || rhs);
  }
};
 
#endif
