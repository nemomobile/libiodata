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

/*----------------------------------------------------------+
| Notice: the API decribed in this header file has to be    |
|       considered as private during the Harmattan program; |
|       contact the author, if you wish to use this library |
+----------------------------------------------------------*/

#ifndef MAEMO_IODATA_VALIDATOR_H
#define MAEMO_IODATA_VALIDATOR_H

#include <string>
#include <exception>
#include <typeinfo>
using namespace std ;


#include <iodata/iodata>

namespace iodata
{
  enum validator_flag
  {
    RECORD = 1<<0,
    ARRAY = 1<<1,
    BITMASK = 1<<2,
    INTEGER = 1<<3,
    BYTES = 1<<4,
    MANDATORY = 1<<5,
  } ;

  struct record_type ;
#if 0
  struct node ;

  struct node
  {
    string name ;
    /*u*/int32_t integer_value ;
    string bytes_value ;
    bitmask bitmask_value ;
    validator_flag flag ;
    record_type *type ;
    string type_name ;
  } ;
#else
  struct node ;
  struct node_integer ;
  struct node_bytes ;
  struct node_bitmask ;
  struct node_record ;

  struct node
  {
    string name ;
    bool is_array, is_mandatory ;
    node(const string &n, bool a, bool m) : name(n), is_array(a), is_mandatory(m) { }
    virtual ~node() { }
    virtual bool is_integer() const { return false ; }
    virtual bool is_bytes() const { return false ; }
    virtual bool is_bitmask() const { return false ; }
    virtual bool is_record() const { return false ; }
    virtual const char *node_name() const = 0 ;
  } ;

  struct node_integer : public node
  {
    int32_t value ;
    node_integer(const string &n, bool a, bool m, int32_t v) : node(n,a,m), value(v) { }
   ~node_integer() { }
    bool is_integer() const { return true ; }
    const char *node_name() const { return "node_integer" ; }
  } ;

  struct node_bytes : public node
  {
    string value ;
    node_bytes(const string &n, bool a, bool m, const string &v) : node(n,a,m), value(v) { }
   ~node_bytes() { }
    bool is_bytes() const { return true ; }
    const char *node_name() const { return "node_bytes" ; }
  } ;

  struct node_bitmask : public node
  {
    bitmask value ;
    node_bitmask(const string &n, bool a, bool m, const bitmask &v) : node(n,a,m), value(v) { }
   ~node_bitmask() { }
    bool is_bitmask() const { return true ; }
    const char *node_name() const { return "node_bitmask" ; }
  } ;

  struct node_record : public node
  {
    string type_name ;
    record_type *type ;
    node_record(const string &n, bool a, bool m, const string &tn, record_type *t=NULL) : node(n,a,m), type_name(tn) { type = t ; }
   ~node_record() { }
    bool is_record() const { return true ; }
    const char *node_name() const { return "node_record" ; }
  } ;
#endif

  struct record_type
  {
    string name ;
    vector<node*> nodes ;
  } ;


  template <typename T>
  T *cast_and_check(item *p)
  {
    if(T *p_cast = dynamic_cast<T*> (p))
      return p_cast ;
    throw exception((string)T::static_class_name()+ " expected, but "+p->class_name()+" found") ;
  }

  struct validator
  {
    vector<string> v_namespace ;
    string v_function ;

    struct exception : public iodata::exception
    {
      string node_path ;
      exception(const string &m) : iodata::exception(m) { } ;
      exception &prepend_path(const string &name) ;
      exception &prepend_index(int i) ;
      string info() const { return (string)"iodata::validator::exception, "+message()+" at "+node_path ; }
     ~exception() throw() { }
    } ;

#if 0
    static bit_codec type_codec ;
    static void init_type_codec() ;
#endif
    static validator* from_file(const char *path) ;
    iodata::record *record_from_file(const char *path, const char *record_type, string &message) ;
    bool record_to_file(const char *path, const char *record_type, iodata::record *data, string &serialized) ;
    validator() ;
   ~validator() ;
#if 0
    {
      static bool first = true ;
      if(first)
        init_type_codec(), first=false ;
#if 0
      indent_step = 2 ;
      screen_width = 80 ;
#endif
    }
#endif
    map<string, record_type*> types ;
    void load(const record *lang) ;
    void link() ;

#if 0
    bool read_run(record *data) ;
    bool write_run(record *data) ;
    void check_node(item *p, const node &n, bool maybe_array) ;
    void check_record(record *p, const record_type *r) ;
    void check_record(record *p, const string &type_name) ;
#endif

#if 0
    void prepare_output(item *p, const node &n, bool maybe_array, bool is_mandatory) ;
    void prepare_output(record *p, const record_type *r, bool braces) ;
    void prepare_output(record *p, const string &type_name) ;

    int indent_step, screen_width ;
    void output_indent(ostream &os, int indent) ;
    int indent_to_width(int indent) ;
    void output_record(ostream &os, record *p, const record_type *r, bool braces, int width, int indent) ;
#endif


    void check_record_after_read(record *p, const string &type_name) { return check_record(p,type_by_name(type_name), false) ; }
    void check_record_before_write(record *p, const string &type_name) { return check_record(p,type_by_name(type_name), true) ; }
  private:
    const record_type* type_by_name(const string &name) ;
    void check_record(record *p, const record_type *r, bool write) ;
    void check_unknown_fields(record *p, const record_type *r) ;
    void check_fields(record *p, const record_type *r, bool add_defaults, item *items[], unsigned N) ;
    void check_children(record *p, const record_type *r, bool write, item *items[], unsigned N) ;
    void check_defaults(record *p, const record_type *r, item *items[], unsigned N) ;
  } ;
}

namespace iodata
{
  class output
  {
    vector<unsigned> width, length ;
    char *buffer ;
    unsigned allocated, position, alloc_step ;
    void realloc_to(unsigned size) ;
    bool first ;
    unsigned len_position, width_position ;
    unsigned width_of_added_string(const string &s) ;
    unsigned length_of_added_string(const string &s) ;
    unsigned prepare(item *it) ;
    unsigned get_length() { return length[len_position++] ; }
    void print_chunk() { unsigned len=get_length() ; os.write(buffer+position, len) ; position+=len ; }
    void hard_reset() ;
    void soft_reset() ;
    void do_printing(item *it, bool oneliner) ;

    ostream &os ;
    unsigned indent, indent_step, maximal_width ;
    string spaces ;
    void indent_increment() { for(indent+=indent_step; spaces.length()<indent; spaces += "           ") ; }
    void indent_decrement() { if(indent>=indent_step) indent -= indent_step ; }
    void indent_output() { os.write(spaces.c_str(), indent) ; }
    void newline_output() { os << '\n' ; indent_output() ; }
    void comma_output(bool &flag, bool one) { if(flag) { if(one) os << ", " ; else os << ",", newline_output() ; } flag = true ; }
    void open(char ch, bool one) { if(one) os << " " << ch << " " ; else newline_output(), indent_increment(), os << ch, newline_output() ; }
    void close(char ch, bool one) { if(one) os << " " << ch ; else indent_decrement(), newline_output(), os << ch ; }

    unsigned reserve_width(unsigned step) { unsigned pos=width.size() ; width.resize(pos+step) ; return pos ; }
    void push_length(unsigned value) { length.push_back(value) ; }
  public:
    output(ostream &stream, int indent=2, int width=80) ;
   ~output() ;
    void output_record(record *r) ;
  } ;
}

#endif
