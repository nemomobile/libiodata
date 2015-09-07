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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <vector>
#include <string>
#include <sstream>
using namespace std ;

#include "log.h"
#include "iodata.h"
#include "validator.h"
#include "storage.h"

using namespace iodata ;

storage::storage()
{
  data_source = -1 ;
  type_validator = NULL ;
  validator_owned = false ;
}

storage::~storage()
{
  if (validator_owned and type_validator)
    delete type_validator ;
}

void storage::set_primary_path(const string &file)
{
  log_assert(path.size()==0, "primary path '%s' already defined", path[0].c_str()) ;
  path.push_back(file) ;
}

void storage::set_secondary_path(const string &file)
{
  log_assert(path.size()<2, "secondary path '%s' already defined", path[1].c_str()) ;
  log_assert(path.size()>0, "can't set secondary path, because primary path not set") ;
  log_assert(path.size()==1) ;
  path.push_back(file) ;
}

void storage::set_validator(const string &path, const string &name)
{
  log_assert(!name.empty()) ;
  log_assert(type_validator==NULL) ;
  type_name = name ;
  type_validator = validator::from_file(path.c_str()) ;
  validator_owned = true ;
}

void storage::set_validator(validator *v, const string &name)
{
  log_assert(!name.empty()) ;
  log_assert(type_validator==NULL) ;
  type_name = name ;
  type_validator = v ;
}

record *storage::load()
{
  log_assert(path.size()>0, "no path defined, where do you want to read from?") ;

  const char *path_0 = path[0].c_str() ;

  for(unsigned i=0; i<path.size(); ++i)
  {
    // reading data file
    const char *path_i = path[i].c_str() ;
    int res = read_file_to_string(path_i, data_cached) ;
    if(res<0)
    {
      // let caller decide if non-existing files require diagnostic logging
      if(errno != ENOENT)
        log_warning("can't read '%s': %m", path_i) ;
      continue ;
    }
    // read success

    // parsing iodata structure
    string message ;
    record *tree = parse_string_to_tree(message) ;
    if(tree==NULL)
    {
      log_warning("can't parse data in '%s': %s", path_i, message.c_str()) ;
      continue ;
    }
    // parse success

    // validate the data
    if(type_validator)
    {
      try
      {
        log_debug() ;
        type_validator->check_record_after_read(tree, type_name) ;
        log_debug() ;
      }
      catch(const iodata::exception &e)
      {
        log_debug() ;
        log_warning("data in '%s' isn't valid: %s", path_i, e.info().c_str()) ;
        delete tree ;
        continue ;
      }
    }
    // data is validated, empty fields are set to default values

    data_source = i ;
    log_info("read data from '%s'", path_i) ;
    return tree ;
  }

  // no correct data on the disk
  data_source = -1 ;
  data_cached = "" ;

  if(type_validator==NULL)
  {
    log_error("no type information for data in '%s' known, give up", path_0) ;
    return NULL ;
  }

  // let's try to use default values
  record *tree = new record ;
  try
  {
    type_validator->check_record_after_read(tree, type_name) ;
  }
  catch(iodata::exception &e)
  {
    log_error("no default values for data in '%s' known: %s; give up", path_0, e.info().c_str()) ;
    delete tree ;
    return NULL ;
  }

  // success: using default values
  log_info("using default values for data in '%s'", path_0) ;

  data_cached = ".\n" ; // empty record
  return tree ;
}

// this function can throw validator::exception
// if (and only if) the data tree contains rubbish

int storage::save(record *rec)
{
  log_assert(path.size()>0, "no path defined, where do you want to write to?") ;
  const char *path_0 = path[0].c_str() ;

  // check record and reduce default values
  if(type_validator)
    type_validator->check_record_before_write(rec, type_name) ;

  // serialize the record
  ostringstream os ;
  output out(os) ;
  out.output_record(rec) ;
  string new_data = os.str() ;

  if(data_cached==new_data)
  {
    log_debug("do not write the same data to '%s' again", path[data_source].c_str()) ;
    return data_source ;
  }

  // *) no valid data on the disk or
  // *) no secondary file or
  // *) data read from the secondary
  //   => just write to the primary file

  bool no_data_on_disk = data_source<0 ;
  bool no_secondary = path.size()==1 ;
  bool data_in_secondary = data_source == 1 ;

  if(no_data_on_disk || no_secondary || data_in_secondary)
  {
    int res = write_string(0, new_data) ;
    if(res<0)
    {
      log_critical("can't write data to '%s': %m", path_0) ;
      if(data_source==0) // data destroyed by write attempt
      {
        data_source = -1 ;
        data_cached = "" ;
      }
    }
    else
    {
      log_info("data written to '%s'", path_0) ;
      data_source = 0 ;
      data_cached = new_data ;
    }
    return data_source ;
  }

  log_assert(0<=data_source) ; // old data is on disk
  log_assert(path.size()>1) ; // secondary path is given
  log_assert(data_source==0) ; // old data is in the primary file

  const char *path_1 = path[1].c_str() ;

  int index = 0 ; // we want to save the data to primary file

  if(move_files(0,1) < 0)
  {
    log_critical("can't rename files: '%s'->'%s': %m", path_0, path_1) ;
    index = 1 ; // let's write to the secondary then
  }

  const char *path_i = index==0 ? path_0 : path_1 ;

  int res = write_string(index, new_data) ;

  if(res<0)
  {
    log_critical("can't write data to '%s': %m", path_i) ;
    data_source = -1 ;
    data_cached = "" ;
  }
  else
  {
    log_info("data written to '%s'", path_i) ;
    // we have to get rid of primary file, if written to secondary
    if(index > 0 && unlink(path_0)<0)
    {
      // that's the mega paranoia!
      log_critical("written data will be lost, because can't remove '%s': %m", path_0) ;
      data_source = -1 ;
      data_cached = "" ;
    }
    else
    {
      if(index > 0) // it means we can't move primary->secondary
      {
        // but maybe it's possible to move it back ?
        if(move_files(1, 0) < 0)
          log_warning("can't move secondary to primary '%s' (%m), but never mind: data is saved", path_0) ;
        else
          index = 0 ; // yahoo, it's a miracle !!
      }
      // we're happy
      data_source = index ;
      data_cached = new_data ;
    }
  }

  return data_source ;
}

bool storage::fix_files(bool force)
{
  if(data_cached.empty())
    return false ;

  log_assert(path.size()>0, "primary storage file not defined") ;
  const char *path_0 = path[0].c_str() ;

  if(!force && data_source==0) // may be it's enough to read
  {
    string in_file ;
    if(read_file_to_string(path_0, in_file)==0 && data_cached==in_file)
      return true ;
    log_info("primary file '%s' doesn't match cached data", path_0) ;
  }

  // now we have to write cached data to primary file

  if(force && data_source==0 && path.size()>1) // primary file should be backed up
  {
    if(move_files(0,1) < 0)
      return false ;
  }

  // now just write the cached data to primary

  if(write_string(0, data_cached)<0)
    return false ;

  data_source = 0 ;

#if 0
  // now we do not need the secondary file
  // so we are trying to remove it
  // but if we can't, it doesn't matter:
  // a warning is enough

  if(path.size() > 1)
  {
    const char *path_1 = path[1].c_str() ;
    if(unlink(path_1) < 0)
      log_warning("can't unlink the secondary file '%s': %m", path_1) ;
  }

  commented out this piece, because as a matter of fact we do need the secondary:
  it is a kind of place holder for the future 'disk full' situation
#endif

  return true ;
}

int storage::move_files(int from, int to)
{
  const char *path_from = path[from].c_str() ;
  const char *path_to = path[to].c_str() ;

  return rename(path_from, path_to) ;
}

int storage::write_string(int index, const string &data)
{
  const char *file = path[index].c_str() ;
  return write_string_to_file(file, data) ;
}

int storage::write_string_to_file(const char *file, const string &data)
{
  int fd = open(file, O_WRONLY|O_CREAT|O_TRUNC, 0666) ;

  if(fd < 0)
    return -1 ;

  int size = data.length(), done = 0 ;
  const char *start = data.c_str() ;

  while(done < size)
  {
    ssize_t bytes = write (fd, start + done, size - done) ;
    if(bytes>0)
      done += bytes ;
    else if(bytes==0 || errno!=EINTR) // try again only if interrupted
      break ;
    // write(2) man page says: "zero indicates nothing  was  written"
    // So we're not trying to write again (probably in an infinite loop)
    // But it's still not clear, what kind of condition indicates bytes=0 :-(
  }

  if(done < size || fsync(fd) < 0 || close(fd) < 0) // that's it
  {
    int errno_copy = errno ;
    close(fd) ; // don't care about result, even if it's already closed
    errno = errno_copy ;
    return -1 ;
  }

  return 0 ;
}

int storage::read_file_to_string(const char *file, string &input)
{
  int fd = open(file, O_RDONLY) ;

  if(fd < 0)
    return -1 ;

  struct stat st ;
  if(fstat(fd, &st) < 0)
  {
    int errno_copy = errno ;
    close(fd) ;
    errno = errno_copy ;
    return -1 ;
  }

  int size = st.st_size ;

  if (size==0)
  {
    input.resize(0) ;
    return 0 ;
  }

  if(size<0)
  {
    close(fd) ;
    errno = EIO ; // TODO find a better one?
    return -1 ;
  }

  int done = 0 ;
  char *buffer = new char[size+1] ;
  log_assert(buffer) ;

  while(done < size)
  {
    ssize_t bytes = read (fd, buffer + done, size - done) ;
    if(bytes>0)
      done += bytes ;
    else if(bytes==0 || errno!=EINTR) // EOF or error (not interrupt)
      break ;
    else if(lseek(fd, done, SEEK_SET)!=done) // fix the position, if interrupted
      break ;
    // read(2) man page is self contratictory:
    // on the one side bytes=0 means EOF: "zero indicates end of file"
    // on the other side it states "It is not an error if this number is
    // smaller than the number of bytes requested; this may happen for example
    // [...] because read() was interrupted by a signal". As zero is smaller
    // than the number of bytes requested, it may indicate an interrupt then.
    // As we can't distinguish between "zero because of EOF" and "zero because
    // of interrupt" let's assume the first to be paranoid (if someone has
    // truncated our file during we're reading it and we assume interrupt, we
    // would remain in an infinite loop).
    // In the case of an interrupt (bytes<0 and errno=EINTR) "it is left
    // unspecified whether the  file position [...] changes". That's the
    // reason for the weird lseek() call.
  }

  int errno_copy = errno ;
  close(fd) ; // failed? who cares, we got data already

  if(done < size)
  {
    delete[] buffer ;
    errno = errno_copy ;
    return -1 ;
  }

  buffer[size] = '\0' ;

  if(strlen(buffer)!=(unsigned)size) // some '\0' inside ?
  {
    delete[] buffer ;
    errno = EILSEQ ;
    return -1 ;
  }

  input = buffer ;

  delete[] buffer ;
  return 0 ;
}

record * storage::parse_string_to_tree(std::string &message)
{
  record *rec = NULL ;
  try
  {
    istringstream in(data_cached) ;
    parser p(in) ;
    p.parse() ;
    rec = p.detach() ;
  }
  catch(iodata::exception &e)
  {
    message = e.info() ;
    return NULL ;
  }
  return rec ;
}
