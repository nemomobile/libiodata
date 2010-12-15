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
#include <typeinfo>
#include <cassert>
#include <fstream>
#include <iostream>
using namespace std ;

#include <qmlog>

#include "iodata.h"
#include "validator.h"

iodata::validator::validator()
{
  static bool first = true ;
  if(first)
    init_type_codec(), first=false ;
}
iodata::validator::~validator()
{
  log_debug("deleting validator::types") ;
  for(map<string,record_type*>::iterator it=types.begin(); it!=types.end(); ++it)
    delete it->second ;
}

void iodata::validator::check_record(record *p, const record_type *r, bool write)
{
  item *items[r->nodes.size()+1] ; // '+1' is used just for the case N=0
  unsigned N=0, not_present=0 ;
  for(vector<node>::const_iterator n=r->nodes.begin(); n!=r->nodes.end(); ++n)
  {
    map<string,item*>::iterator it = p->x.find(n->name) ;
    items[N++] = (it==p->x.end()) ? ++not_present, (item*)NULL : it->second ;
  }
  if(r->nodes.size() != p->x.size() + not_present) // found unknown fields
    check_unknown_fields(p, r) ;
  check_fields(p, r, !write, items, N) ;
  check_children(p, r, write, items, N) ;
  if(write)
    check_defaults(p, r, items, N) ;
}

void iodata::validator::check_unknown_fields(record *p, const record_type *r)
{
  // this finction is only called if there are unknown fields
  // thus it could be slow:
  // nobody cares because that's an exceptional situation
  string unknown_fields = "" ;
  unsigned counter = 0 ;
  for(map<string,item*>::iterator p_it=p->x.begin(); p_it!=p->x.end(); ++p_it)
  {
    bool found = false ;
    for(vector<node>::const_iterator r_it=r->nodes.begin(); !found && r_it!=r->nodes.end(); ++r_it)
      found = r_it->name == p_it->first ;
    if(found)
      continue ;
    if(counter++)
      unknown_fields += ", " ;
    unknown_fields += p_it->first ;
  }
  assert(counter>0) ;
  throw validator::exception((string)"unknown field" + (counter>0 ? "s: ":": ") + unknown_fields) ;
}

void iodata::validator::check_fields(record *p, const record_type *r, bool add_defaults, item *items[], unsigned N)
{
  string *missed_fields=NULL ;

  for(unsigned i=0; i<N; ++i)
  {
    const node *n = &r->nodes[i] ;
    if(items[i]==NULL) // field is not present
    {
      if(n->flag & MANDATORY) // but it's mandatory!
      {
        if(missed_fields)
          *missed_fields += ", ", *missed_fields += n->name ;
        else
          missed_fields = new string(n->name) ;
        add_defaults = false ;
      }
      else if(add_defaults) // use the default value
      {
#define _append(bit, type) if((n->flag&(bit|ARRAY))==bit) items[i] = p->x[n->name] = new type(n->type##_value) ;
        _append(BITMASK, bitmask) ;
        _append(INTEGER, integer) ;
        _append(BYTES, bytes) ;
#undef _append
#define _append_empty(bit, type) if((n->flag&(bit|ARRAY))==bit) items[i] = p->x[n->name] = new type  ;
        _append_empty(ARRAY, array) ;
        _append_empty(RECORD, record) ;
#undef _append_empty

      }
    }
    else // field is present, check the type
    {
      try
      {
#define _check(bit, type) if((n->flag&(bit|ARRAY))==bit) cast_and_check<type>(items[i])
        _check(BITMASK, bitmask) ;
        _check(INTEGER, integer) ;
        _check(BYTES, bytes);
#undef _check
      }
      catch(exception &e)
      {
        throw e.prepend_path(n->name) ;
      }
    }
  }
  if(missed_fields)
  {
    string tmp = "mandatory filed(s) missed: " + *missed_fields ;
    delete missed_fields ;
    throw exception(tmp) ;
  }
}

void iodata::validator::check_children(record *p, const record_type *r, bool write, item *items[], unsigned N)
{
  for(unsigned i=0; i<N; ++i)
  {
    if(items[i]==NULL)
      continue ;
    int f = r->nodes[i].flag ;
    if((f&(ARRAY|RECORD))==0)
      continue ;
    int element_type = f & (RECORD|INTEGER|BITMASK|BYTES) ;
    if(f&ARRAY)
    {
      array *pa = cast_and_check<array> (items[i]) ;
      for(unsigned j=0; j<pa->x.size(); ++j)
      {
        try
        {
          switch(element_type)
          {
            default: throw exception((string)"internal error in"+__PRETTY_FUNCTION__) ;
            case BITMASK: cast_and_check<bitmask> (pa->x[j]) ; break ;
            case INTEGER: cast_and_check<integer> (pa->x[j]) ; break ;
            case BYTES:   cast_and_check<bytes> (pa->x[j]) ; break ;
            case RECORD:  check_record(cast_and_check<record>(pa->x[j]), r->nodes[i].type, write) ;
          }
        }
        catch(exception &e)
        {
          throw e.prepend_index(j).prepend_path(r->nodes[i].name) ;
        }
      }
    }
    else // it's a record, because it's not an array
      check_record(cast_and_check<record>(items[i]), r->nodes[i].type, write) ;
  }
}

void iodata::validator::check_defaults(record *p, const record_type *r, item *items[], unsigned N)
{
  for(unsigned i=0; i<N; ++i)
  {
    if(items[i]==NULL)
      continue ;
    int f = r->nodes[i].flag ;
    if(f & MANDATORY)
      continue ;
    if(f & ARRAY)
      f = ARRAY ;
    bool flag = false ;
    switch(f)
    {
      default: throw exception((string)"internal error in"+__PRETTY_FUNCTION__) ;
      case ARRAY: flag = cast_and_check<array>(items[i])->x.size()==0 ; break ;
      case RECORD: flag = cast_and_check<record>(items[i])->x.size()==0 ; break ;
      case INTEGER: flag = cast_and_check<integer>(items[i])->x==r->nodes[i].integer_value ; break ;
      case BYTES: flag = cast_and_check<bytes>(items[i])->x==r->nodes[i].bytes_value ; break ;
      case BITMASK: flag = *cast_and_check<bitmask>(items[i])==r->nodes[i].bitmask_value ; break ;
    }
    if(flag)
    {
      delete items[i] ;
      items[i] = NULL ;
      p->x.erase(r->nodes[i].name) ;
    }
  }
}

void iodata::validator::load(const record *lang)
{
  typedef map<string, item*>::const_iterator iterator ;
  for(iterator it=lang->x.begin(); it!=lang->x.end(); ++it)
  {
    const array *ap = dynamic_cast<const array*> (it->second) ;
    unsigned N = ap->x.size() ;
    // assert(N>0) ; // WHY >0 ? Empty type is okey?
    assert(types.find(it->first)==types.end()) ;
    record_type *t = types[it->first] = new record_type ;
    t->name = it->first ;
    t->nodes.resize(N) ;
    for(unsigned i=0; i<N; ++i)
    {
      const record *r = dynamic_cast<const record *> (ap->x[i]) ;
      node &n = t->nodes[i] ;
      const bytes *item_name = dynamic_cast<const bytes*> (r->x.find("name")->second) ;
      const bitmask *item_type = dynamic_cast<const bitmask*> (r->x.find("type")->second) ;
      n.flag = (validator_flag) (unsigned) item_type->value(&type_codec) ;
      n.name = item_name->x ;
      n.type = NULL ;
      if(n.flag & RECORD)
        n.type_name = dynamic_cast<const bytes*> (r->x.find("record")->second)->x ;
      map<string,item*>::const_iterator value_it = r->x.find("value") ;
      if(value_it!=r->x.end())
      {
        const item *value_item = value_it->second ;
        if(n.flag & INTEGER)
          n.integer_value = dynamic_cast<const integer *> (value_item) -> x ;
        if(n.flag & BYTES)
          n.bytes_value = dynamic_cast<const bytes *> (value_item) -> x ;
        if(n.flag & BITMASK)
          n.bitmask_value = * dynamic_cast<const bitmask *> (value_item) ;
      }
    }
  }
}

iodata::bit_codec iodata::validator::type_codec ;

void iodata::validator::init_type_codec()
{
  type_codec.register_name(RECORD, "record") ;
  type_codec.register_name(ARRAY, "array") ;
  type_codec.register_name(BITMASK, "bitmask") ;
  type_codec.register_name(INTEGER, "integer") ;
  type_codec.register_name(BYTES, "bytes") ;
  type_codec.register_name(MANDATORY, "mandatory") ;
}

void iodata::validator::link()
{
  for(map<string,record_type*>::iterator t=types.begin(); t!=types.end(); ++t)
  {
    assert(t->first==t->second->name) ;
    for(vector<node>::iterator n=t->second->nodes.begin(); n!=t->second->nodes.end(); ++n)
      if(n->flag & RECORD)
      {
        if(n->type)
          continue ;
        map<string,record_type*>::iterator res = types.find(n->type_name) ;
        assert(res!=types.end()) ;
        n->type = res->second ;
      }
  }
}

iodata::validator::exception &iodata::validator::exception::prepend_index(int i)
{
  ostringstream new_path ;
  new_path << "[" << i << "]" << node_path ;
  node_path = new_path.str() ;
  return *this ;
}

iodata::validator::exception &iodata::validator::exception::prepend_path(const string &name)
{
  node_path = string(".") + name + node_path ;
  return *this ;
}

const iodata::record_type* iodata::validator::type_by_name(const string &name)
{
  map<string, record_type*>::const_iterator it = types.find(name) ;
  if(it==types.end())
    throw validator::exception((string)"unknown type: "+name) ;
  return it->second ;
}

iodata::validator *iodata::validator::from_file(const char *path)
{
  ifstream type_info(path) ;
  iodata::parser p(type_info) ;
  if(!p.parse())
    throw validator::exception("parse error") ;

  validator *x = new validator ;
  x->load(p.tree) ;
  x->link() ;

  return x ;
}

// never use this function with a tyoe having mandatory field
// or at least catch the exceptions
iodata::record *iodata::validator::record_from_file(const char *path, const char *record_type, string &message)
{
  iodata::record *r = NULL ;
  ifstream s(path) ;
  bool file_is_ok = s.good() ;
  if(file_is_ok)
  {
    try
    {
      iodata::parser p = s ;
      p.parse() ;
      r = p.detach() ;
      check_record_after_read(r, record_type) ;
    }
    catch(const iodata::exception &e)
    {
      message = (string)"in file '"+path+"': " + e.info() ;
      delete r ;
      r = NULL ;
    }
  }
  if(r==NULL)
  {
    if(!file_is_ok)
      message = (string)"can't read file '"+path+"'" ;
    message += ", using default values" ;
    r = new iodata::record ;
    try
    {
      check_record_after_read(r, record_type) ;
    }
    catch(const iodata::exception &e)
    {
      delete r ;
      r = NULL ;
      message += "; can't use default values" ;
    }
  }
  return r ;
}

bool iodata::validator::record_to_file(const char *path, const char *record_type, iodata::record *data, string &serialized)
{
  check_record_before_write(data, record_type) ;
  ostringstream os ;
  iodata::output out(os) ;
  out.output_record(data) ;

  bool result = false ;
  try
  {
    ofstream of ;
    of.exceptions(ofstream::failbit | ofstream::badbit) ;
    of.open(path, ofstream::out) ;
    of << os.str() ;
    of.close() ;
    result = true ;
  }
  catch(const ofstream::failure &e)
  {
    serialized = os.str() ;
  }
  return result ;
}


unsigned iodata::output::prepare(item *it)
{
  if(integer *p = dynamic_cast<integer*> (it))
  {
    ostringstream os ;
    os << p->x ;
    return length_of_added_string(os.str()) ;
  }
  else if(bitmask *p = dynamic_cast<bitmask*> (it))
  {
    ostringstream os ;
    bool flag = false ;
    for(set<string>::const_iterator x=p->xs.begin(); x!=p->xs.end(); ++x, flag=true)
    {
      if(flag)
        os << "|" ;
      os << "$" << *x ;
    }
    if(!flag)
      os << "$" << p->xl ;
    else if(p->xl)
      os << "|$" << p->xl ;
    return length_of_added_string(os.str()) ;
  }
  else if(bytes *p = dynamic_cast<bytes*> (it))
  {
    ostringstream os ;
    os << "\"" ;
    for(string::const_iterator s=p->x.begin(); s!=p->x.end(); ++s)
    {
      unsigned char ch = *s ;
#define __hex(x) char((x)<10?'0'+(x):'a'+(x)-10)
      if(ch<0x20 || ch=='"' || ch=='\\')
        os << "\\x" << __hex(ch>>4) << __hex(ch&0x0F) ;
#undef __hex
      else
        os << ch ;
    }
    os << "\"" ;
    return width_of_added_string(os.str()) ;
  }
  else if(array *p = dynamic_cast<array*> (it))
  {
    unsigned pos = reserve_width(p->x.size()), sum = 0 ;
    for(unsigned i=0; i<p->x.size(); ++i)
    {
      unsigned w = prepare(p->x[i]) ;
      sum += w ;
      width[pos++] = w ;
    }
    return sum==0 ? 3 : sum+4+2*(p->x.size()-1) ;
  }
  else if(record *p = dynamic_cast<record*> (it))
  {
    unsigned pos = reserve_width(p->x.size()), sum = first ? 0 : 2 ;
    first = false ;
    for(map<string,item*>::const_iterator xx=p->x.begin(); xx!=p->x.end(); ++xx)
    {
      unsigned w = prepare(xx->second) ;
      sum += xx->first.length() + 3 + w + 2 ; // "name = value, "
      width[pos++] = w ;
    }
    return sum<=2 ? sum + 1 : sum ;
  }
  else
    assert(! "oopsista") ; // should never happen
}

void iodata::output::realloc_to(unsigned size)
{
  if(size<=allocated)
    return ;
  allocated = size + (alloc_step - size%alloc_step) ;
  buffer = (char*) realloc(buffer, allocated) ;
  assert(buffer != NULL) ;
}

unsigned iodata::output::length_of_added_string(const string &s)
{
  unsigned len = s.length() ;
  realloc_to(position+len) ;
  memcpy(buffer+position, s.c_str(), len) ;
  position += len ;
  length.push_back(len) ;
  return len ;
}

unsigned iodata::output::width_of_added_string(const string &s)
{
  unsigned start = position ;
  unsigned length = length_of_added_string(s) ;
  for(const char *p=buffer+start; p<buffer+position; ++p)
    if((*p & 0xC0) == 0x80) // see utf8(7), 10xxxxxx is not character
      length -- ;
  return length ;
}

void iodata::output::do_printing(item *it, bool oneliner)
{
  if(array *p = dynamic_cast<array*> (it))
  {
    open('[', oneliner) ;

    unsigned pos = width_position ;
    width_position += p->x.size() ;

    bool comma_needed = false ;
    for(unsigned i=0; i<p->x.size(); ++i, ++pos)
    {
      comma_output(comma_needed, oneliner) ;
      if(p->x[i]->is_leaf())
        print_chunk() ;
      else if(oneliner)
        do_printing(p->x[i], true) ;
      else
      {
        unsigned column = indent ;
        unsigned w = width[pos] ;
        bool one_line =  column + w <= maximal_width ;
        do_printing(p->x[i], one_line) ;
      }
    }
    close(']', oneliner) ;
  }
  else if(record *p = dynamic_cast<record*> (it))
  {
    bool braces = !first ;
    first = false ;

    if(braces)
      open('{', oneliner) ;

    unsigned pos = width_position ;
    width_position += p->x.size() ;

    bool comma_needed = false ;
    for(map<string,item*>::const_iterator xx=p->x.begin(); xx!=p->x.end(); ++xx, ++pos)
    {
      comma_output(comma_needed, oneliner) ;
      if(xx->second->is_leaf())
        os << xx->first << " = ", print_chunk() ;
      else if(oneliner)
        os << xx->first << " =", do_printing(xx->second, true) ;
      else
      {
        unsigned column = indent + 2 + xx->first.length() ;
        unsigned w = width[pos] ;
        bool one_line =  column + w <= maximal_width ;
        os << xx->first << " =" ;
        do_printing(xx->second, one_line) ;
      }
    }
    close(braces ? '}' : '.', oneliner) ;
  }
  else
    assert(! "oopsista") ; // should never happen
}

iodata::output::output(ostream &stream, int indent, int width)
  : os(stream), indent_step(indent), maximal_width(width)
{
  buffer=NULL ;
  allocated=0 ;
  alloc_step=1024 ;
  hard_reset() ;
  log_debug("iodata::output::constructor %p", this) ;
}

iodata::output::~output()
{
  log_debug("iodata::output::destructor %p buffer=%p", this, buffer) ;
  free(buffer) ;
}

void iodata::output::hard_reset()
{
  width.resize(0) ;
  length.resize(0) ;
  soft_reset() ;
}

void iodata::output::soft_reset()
{
  len_position = 0 ;
  position = 0 ;
  width_position = 0 ;
  indent = 0 ;
  first = true ;
}

void iodata::output::output_record(record *r)
{
  hard_reset() ;
  reserve_width(1) ;
  unsigned width0 = prepare(r) ;
  width[0] = width0 ;

  soft_reset() ;
  width_position++ ;
  do_printing(r, width0<=maximal_width) ;
  os << '\n' ;
}
