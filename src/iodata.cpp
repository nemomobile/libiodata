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
#include <sstream>

#include "iodata.h"

namespace iodata
{
  void bitmask::assign(bitmask_t value, const bit_codec *codec)
  {
    xl = codec ? codec->encode(value, xs) : value ;
  }

  bitmask_t bitmask::value(const bit_codec *codec) const
  {
    return xl | codec->decode(xs) ;
  }

  void bit_codec::register_name(bitmask_t value, const string &name)
  {
    s2m.insert(make_pair(name, value)) ;
    m2s.insert(make_pair((ordered_bitmask_t)value, name)) ;
  }

  bitmask_t bit_codec::encode(bitmask_t value, set<string> &masks) const
  {
    typedef map<ordered_bitmask_t,string>::const_iterator iterator ;
    for(iterator it=m2s.begin(); value!=0 && it!=m2s.end(); ++it)
    {
      bitmask_t m = it->first ;
      if((value&m)==m)
      {
        masks.insert(it->second) ;
        value &= ~m ;
      }
    }
    return value ;
  }

  bitmask_t bit_codec::decode(const string &name) const
  {
    map<string,bitmask_t>::const_iterator it = s2m.find(name) ;
    if(it==s2m.end())
      throw iodata::exception(name+": invalid bit mask name") ;
    return it->second ;
  }

  bitmask_t bit_codec::decode(const set<string> &names) const
  {
    bitmask_t res = 0 ;
    for(set<string>::const_iterator it=names.begin(); it!=names.end(); ++it)
      res |= decode(*it) ;
    return res ;
  }

  ostream &operator<<(ostream &os, const record &x)
  {
    x.plain_output(os, "") ;
    return os ;
  }

  void integer::plain_output(ostream &os, const string &prefix) const
  {
    os << prefix << "=" << x << endl ;
  }

  void bytes::output(ostream &os) const
  {
    for(string::const_iterator it=x.begin(); it!=x.end(); ++it)
    {
      unsigned char ch = *it ;
#define __hex(x) char((x)<10?'0'+(x):'a'+(x)-10)
      if(ch<0x20 || ch>=0x7F || ch=='\\')
        os << '\\' << __hex(ch>>4) << __hex(ch&0x0F) ;
#undef __hex
      else
        os << ch ;
    }
  }

  void bytes::plain_output(ostream &os, const string &prefix) const
  {
    os << prefix ;
    os << '"' ;
    output(os) ;
    os << endl ;
  }

  void bitmask::plain_output(ostream &os, const string &prefix) const
  {
    if(xl)
      os << prefix << '+' << xl << endl ;
    for(set<string>::const_iterator it=xs.begin(); it!=xs.end(); ++it)
      os << prefix << '|' << *it << endl ;
  }

  void array::plain_output(ostream &os, const string &prefix) const
  {
    for(unsigned i=0; i<x.size(); ++i)
    {
      ostringstream num ;
      num << prefix << "/" << i ;
      x[i]->plain_output(os, num.str()) ;
    }
  }

  const item *item::get(unsigned i) const
  {
    const array *a = arr() ;
    if(a->x.size()<=i)
    {
      ostringstream os ;
      os << "index " << i << " is out or range" ;
      throw exception(os.str()) ;
    }
    return a->x[i] ;
  }

  const item *item::get(const string &key) const
  {
    const record *r = rec() ;
    map<string,item*>::const_iterator it = r->x.find(key) ;
    if(it==r->x.end())
      throw exception(key+": key not found") ;
    return it->second ;
  }

  void record::plain_output(ostream &os, const string &prefix) const
  {
    for(map<string,item*>::const_iterator it=x.begin(); it!=x.end(); ++it)
    {
      ostringstream num ;
      num << prefix << "." << it->first ;
      it->second->plain_output(os, num.str()) ;
    }
  }

  record::~record()
  {
    for(map<string,item*>::const_iterator it=x.begin(); it!=x.end(); ++it)
      delete it->second ;
  }

  array::~array()
  {
    for(unsigned i=0; i<x.size(); ++i)
      delete x[i] ;
  }

  QString parse_and_print(QString input)
  {
    string in = input.toStdString() ;
    istringstream in_s(in) ;
    iodata::parser x(in_s) ;
    if(x.parse())
    {
      ostringstream out ;
      out << * x.tree ;
      return QString::fromStdString(out.str()) ;
    }
    else
      return "NULL\n" ;
  }
}
