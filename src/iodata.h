/*----------------------------------------------------------------------+
|                                                                       |
|   Copyright (C) 2010 Nokia Corporation.                               |
|                                                                       |
|   Author: Ilya Dogolazky <ilya.dogolazky@nokia.com>                   |
|                                                                       |
|   This file is part of Iodata                                         |
|                                                                       |
|   Iodata is free software; you can redistribute it and/or modify      |
|   it under the terms of the GNU Lesser General Public License         |
|   version 2.1 as published by the Free Software Foundation.           |
|                                                                       |
|   Iodata is distributed in the hope that it will be useful, but       |
|   WITHOUT ANY WARRANTY;  without even the implied warranty  of        |
|   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                |
|   See the GNU Lesser General Public License  for more details.        |
|                                                                       |
|   You should have received a copy of the GNU  Lesser General Public   |
|  License along with Iodata. If not, see http://www.gnu.org/licenses/  |
|                                                                       |
+----------------------------------------------------------------------*/
#ifndef MAEMO_IODATA_IODATA_H
#define MAEMO_IODATA_IODATA_H

#include <stdint.h>

#include <string>
#include <vector>
#include <set>
#include <map>
#include <utility>
#include <exception>

// to be replaced with "static_assert" in C++0x
#define compile_time_assertion(x) do{switch(0){case(x):case(0):;}}while(0)
#define type_is_unsigned(t) ((t)(-1)>0)
namespace iodata
{
  using namespace std ;

  struct item ;
  struct record ;
  struct array ;
  struct bitmask ;
  // struct dictionary ;
  struct integer ;
  struct bytes ;

  typedef uint64_t bitmask_t ;
  typedef int32_t integer_t ;

  struct bit_codec ;
  struct ordered_bitmask_t ;
  struct parser ;


  struct item
  {
    // virtual void throw_unless_record() { throw "it not a record" ; }
    virtual void plain_output(ostream &, const string &prefix) const { throw (string)"oops, "+__PRETTY_FUNCTION__+"("+prefix+") ain't implemented" ; }
    virtual ~item() { }
    virtual bool is_leaf() = 0 ;
    virtual const char *class_name() const = 0 ;

    const record *rec() const ;
    const array *arr() const ;
    const string &str() const ;
    integer_t value() const ;
    bitmask_t decode(const bit_codec *c) const ;

    const item *get(const string &key) const ; // element of a record
    const item *get(unsigned int i) const ; // array element
    unsigned size() const ;
  } ;

  ostream &operator<<(ostream &os, const record &) ;

  struct integer : public item
  {
    integer_t x ;
    integer(integer_t xx) : x(xx) { } ;
    void plain_output(ostream &os, const string &prefix) const ;
    bool is_leaf() { return true ; }
    static const char *static_class_name() { return "iodata::integer" ; }
    const char *class_name() const { return static_class_name() ; }
  } ;

  struct bitmask : public item
  {
    bitmask_t xl ; // literal value
    set<string> xs ; // symbolic value TODO: set! not vector, and then operator==
    bool operator==(const bitmask &b) const { return xl==b.xl && xs==b.xs ; }
    // bool operator==(const bitmask &) const { return false ; }
    bool operator!=(const bitmask &b) const { return !operator==(b) ; }
    bitmask() { xl=0 ; }
    bitmask(const bitmask &y) : xl(y.xl), xs(y.xs) { }
    bitmask(bitmask_t value, const bit_codec *codec) { assign(value,codec) ; }
    bitmask_t value(const bit_codec *codec) const ;
    void add(bitmask_t bits) { xl|=bits ; }
    void add(string name) { xs.insert(name) ; }
    void assign(bitmask_t value, const bit_codec *codec) ;
    void plain_output(ostream &os, const string &prefix) const ;
    bool is_leaf() { return true ; }
    static const char *static_class_name() { return "iodata::bitmask" ; }
    const char *class_name() const { return static_class_name() ; }
  } ;

  struct bytes : public item
  {
    string x ;
    bytes(const string xx) : x(xx) { } ;
    void output(ostream &os) const ;
    void plain_output(ostream &os, const string &prefix) const ;
    bool is_leaf() { return true ; }
    static const char *static_class_name() { return "iodata::bytes" ; }
    const char *class_name() const { return static_class_name() ; }
  } ;

  struct record : public item
  {
    map<string, item*> x ;
    void add(const string &k, item *e) { if(e) x[k] = e ; }
    void add(const string &k, integer_t v) { x[k] = new integer(v) ; }
    void add(const string &k, bitmask_t v, const bit_codec *c) { x[k] = new bitmask(v,c); }
    void add(const string &k, const string &v) { x[k] = new bytes(v) ; }

    // void throw_unless_record() { } ;
    void plain_output(ostream &os, const string &prefix) const ;
    virtual ~record() ;
    bool is_leaf() { return false ; }
    static const char *static_class_name() { return "iodata::record" ; }
    const char *class_name() const { return static_class_name() ; }
  } ;

  struct array : public item
  {
    vector<item*> x ;
    void add(item *e) { x.push_back(e) ; }
    unsigned size() const { return x.size() ; }
    void plain_output(ostream &os, const string &prefix) const ;
    virtual ~array() ;
    bool is_leaf() { return false ; }
    static const char *static_class_name() { return "iodata::array" ; }
    const char *class_name() const { return static_class_name() ; }
  } ;

#if 0
  struct dictionary : public item
  {
    map<string,string> x ;
  } ;
#endif

  struct  ordered_bitmask_t
  {
    bitmask_t value ;
    ordered_bitmask_t(bitmask_t v) : value(v) {  }
    operator bitmask_t() const { return value ; }
    static int bit_count(bitmask_t x)
    {
      compile_time_assertion(sizeof(bitmask_t)<=8) ;
      compile_time_assertion(type_is_unsigned(bitmask_t)) ;
      x = (x&0x5555555555555555LL) + (((x&0xAAAAAAAAAAAAAAAALL)>>1)) ;
      x = (x&0x3333333333333333LL) + (((x&0xCCCCCCCCCCCCCCCCLL)>>2)) ;
      x = (x&0x0F0F0F0F0F0F0F0FLL) + (((x&0xF0F0F0F0F0F0F0F0LL)>>4)) ;
      x = (x&0x00FF00FF00FF00FFLL) + (((x&0xFF00FF00FF00FF00LL)>>8)) ;
      return (int)(x%0xFF) ;
    }
    bool operator<(const ordered_bitmask_t &y) const
    {
      int diff = bit_count(value)-bit_count(y.value) ;
      return diff>0 || (diff==0 && value>y.value) ;
    }
  } ;

  class bit_codec
  {
    map<string, bitmask_t> s2m ;
    map<ordered_bitmask_t, string> m2s ;
  public:
    void register_name(bitmask_t value, const string &name) ;
    bitmask_t encode(bitmask_t value, set<string> &masks) const ;
    bitmask_t decode(const string &name) const ;
    bitmask_t decode(const set<string> &names) const ;
  } ;
} ;

// Parser function generated by flex+bison:
int iodata_parse(iodata::parser *) ;

namespace iodata
{
  struct parser
  {
    void *scanner ;
    istream &input ;
    record *tree ;
    parser(istream &in) : input(in)
    {
      tree = NULL ;
      init_scanner() ;
    }
   ~parser()
    {
      destroy_scanner() ;
      if(tree)
        delete tree ;
    }
    bool parse()
    {
      iodata_parse(this) ;
      return tree!=NULL ;
    }
    record *detach()
    {
      record *tree_poniter = tree ;
      tree = NULL ;
      return tree_poniter ;
    }
    record *get_tree()
    {
      return tree ;
    }
    void init_scanner() ;
    void destroy_scanner() ;
  } ;

  struct exception : public std::exception
  {
    string msg ;
    virtual string info() const { return (string)"iodata::exception, "+msg ; }
    const string &message() const { return msg ; }
    exception(const string &m) : msg(m) { }
   ~exception() throw() { } ;
  } ;

  template <typename T>
  const T *cast_and_check_const(const item *p)
  {
    if(const T *p_cast = dynamic_cast<const T*> (p))
      return p_cast ;
    throw exception((string)T::static_class_name()+ "expected, but "+p->class_name()+" found") ;
  }

  inline  const record *item::rec() const { return cast_and_check_const<record>(this) ; }
  inline  const array *item::arr() const { return cast_and_check_const<array>(this) ; }
  inline  const string &item::str() const { return cast_and_check_const<bytes>(this)->x ; }
  inline  integer_t item::value() const { return cast_and_check_const<integer>(this)->x ; }
  inline  bitmask_t item::decode(const bit_codec *c) const { return cast_and_check_const<bitmask>(this)->value(c) ; }
  inline  unsigned item::size() const { return cast_and_check_const<array>(this)->x.size() ; }

}

// some Qt-specific stuff, has to be moved to <iodata/iodata-qt.h>,
// which should be part of iodataqt-dev package

#include <sstream>

#include <QVector>
#include <QString>

namespace iodata
{
  template <typename T>
  array *save(const QVector<T> &x)
  {
    if(x.isEmpty())
      return NULL ;
    array *a = new array ;
    for(typename QVector<T>::const_iterator it=x.begin(); it!=x.end(); ++it)
      a->add(it->save()) ;
    return a ;
  }
  template <typename T>
  void load(QVector<T> &v, const array *a)
  {
    unsigned n = a->x.size() ;
    v.resize(n) ;
    for(unsigned i=0; i<n; ++i)
      v[i].load(a->x[i]->rec()) ;
  }

  QString parse_and_print(QString input) ;

} ;

namespace iodata // and the same for std::vector
{
  template <typename T>
  array *save(const std::vector<T> &x)
  {
    if(x.size()==0)
      return NULL ;
    array *a = new array ;
    for(typename std::vector<T>::const_iterator it=x.begin(); it!=x.end(); ++it)
      a->add(it->save()) ;
    return a ;
  }
  template <typename T>
  void load(std::vector<T> &v, const array *a)
  {
    unsigned n = a->x.size() ;
    v.resize(n) ;
    for(unsigned i=0; i<n; ++i)
      v[i].load(a->x[i]->rec()) ;
  }

  inline array *save_int_array(const std::vector<unsigned> &x)
  {
    if(x.size()==0)
      return NULL ;
    array *a = new array ;
    for(std::vector<unsigned>::const_iterator it=x.begin(); it!=x.end(); ++it)
      a->add(new integer(*it)) ;
    return a ;
  }

  inline void load_int_array(std::vector<unsigned> &v, const array *a)
  {
    unsigned n = a->x.size() ;
    v.resize(n) ;
    for(unsigned i=0; i<n; ++i)
      v[i] = a->x[i]->value() ;
  }
}

#endif
