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

#ifndef MAEMO_IODATA_STORAGE_H
#define MAEMO_IODATA_STORAGE_H

#include <vector>
#include <string>

#include "iodata.h"

namespace iodata { class storage ; }

struct iodata::storage
{
private:
  std::string data_cached ;
  int data_source ;

  std::vector<std::string> path ;

  iodata::validator *type_validator ;
  bool validator_owned ;
  std::string type_name ;

public:
  storage() ;
 ~storage() ;

  void set_primary_path(const std::string &) ;
  void set_secondary_path(const std::string &) ;
  void set_validator(const std::string &path, const std::string &name) ;
  void set_validator(validator *v, const std::string &name) ;

  iodata::record *load() ;
  int save(iodata::record *rec) ;
  int source() { return data_source ; }
  bool fix_files(bool force) ;

  static int read_file_to_string(const char *file, string &input) ;
  static int write_string_to_file(const char *file, const string &data) ;
private:
  int move_files(int index_from, int index_to) ;
  int write_string(int index, const string &data) ;

  record *parse_string_to_tree(std::string &message) ;
} ;

#endif
