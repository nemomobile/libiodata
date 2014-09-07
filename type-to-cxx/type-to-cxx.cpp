#include <iostream>
using namespace std ;

#include <argp.h>

#include <QtGlobal>

#include "../src/iodata.h"
#include "../src/validator.h"
#include "../src/storage.h"
#include "../src/log.h"
#include <crypt.h>

void dump_h(ostringstream &h, iodata::validator *v)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
  h << "#include <iodata-qt5/validator>" << endl ;
#else
  h << "#include <iodata/validator>" << endl ;
#endif
  for (vector<string>::const_iterator it=v->v_namespace.begin(); it!=v->v_namespace.end(); ++it)
    h << "namespace " << *it << " {" << endl ;
  string foo = v->v_function ;
  if (foo.empty())
    foo = "foo" ;
  h << "iodata::validator * " << foo << "() ;" << endl ;
  for (unsigned i=0; i<v->v_namespace.size(); ++i)
    h << "}" ;
  if (not v->v_namespace.empty())
    h << endl ;
}

void dump_cpp(ostringstream &cpp, iodata::validator *v)
{
  using iodata::/*validator::*/record_type ;
  using iodata::/*validator::*/node ;
  using iodata::/*validator::*/node_integer ;
  using iodata::/*validator::*/node_bytes ;
  using iodata::/*validator::*/node_bitmask ;
  using iodata::/*validator::*/node_record ;

  string foo = v->v_function.empty() ? (string)"foo" : v->v_function ;
  for (vector<string>::const_reverse_iterator it=v->v_namespace.rbegin(); it!=v->v_namespace.rend(); ++it)
    foo = *it + "::" + foo ;

  cpp << "iodata::validator * " << foo << "()" << "{" << endl ;
  cpp << "static bool init_done = false ;" << endl ;
  cpp << "static iodata::validator A ;" << endl ;
  map<record_type*, int> record_type_to_num ;
  int rec_no = 0 ;
  for (map<string,record_type*>::iterator it = v->types.begin(); it != v->types.end() and ++rec_no; ++it)
  {
    record_type *rt = it->second ;
    record_type_to_num[rt] = rec_no ;
    cpp << "static iodata::/*validator::*/record_type record_type" << rec_no << " = { " ;
    cpp << '"' << it->first << '"' << ", " ;
    cpp << "vector<iodata::/*validator::*/node*>(" << rt->nodes.size() << ") } ;" << endl ;
  }
  map<node*, int> node_to_num ;
  int node_no = 0 ;
  int bitmask_no = 0 ;
  for (map<string,record_type*>::iterator it = v->types.begin(); it != v->types.end() and ++rec_no; ++it)
  {
    record_type *rt = it->second ;
    for (vector<node*>::iterator nit = rt->nodes.begin(); nit !=rt->nodes.end(); ++nit)
    {
      node *no = *nit ;
      node_to_num[no] = ++node_no ;
      if (node_bitmask *nn = dynamic_cast<node_bitmask*> (no))
      {
        cpp << "static const char *bitmask_list" << ++bitmask_no << "[] = { " ;
        for(set<string>::const_iterator it=nn->value.xs.begin(); it!=nn->value.xs.end(); ++it)
          cpp << '"' << *it << '"' << ", " ;
        cpp << "NULL } ;" << endl ;
      }
      cpp << "static iodata::/*validator::*/" << no->node_name() << " node" << node_no ;
      cpp << "(" << '"' << no->name << '"' << ", " ;
      cpp << no->is_array << ", " << no->is_mandatory << ", " ;
      if (node_integer *nn = dynamic_cast<node_integer*> (no))
        cpp << nn->value ;
      else if (node_bytes *nn = dynamic_cast<node_bytes*> (no))
        cpp << '"' << nn->value << '"' ;
      else if (node_record *nn = dynamic_cast<node_record*> (no))
      {
        cpp << '"' << nn->type_name << '"' ;
        int x = record_type_to_num[nn->type] ;
        log_assert(x>0, "unknown record index for name '%s'", nn->type_name.c_str()) ;
        cpp << ", " << "&record_type" << x ;
      }
      else if (node_bitmask *nn = dynamic_cast<node_bitmask*> (no))
        cpp << "iodata::bitmask(" << nn->value.xl << ", bitmask_list" << bitmask_no << ")" ;
      cpp << ") ;" << endl ;
    }
  }
  cpp << "if (not init_done) { init_done = true ;" << endl ;
  cpp << "A.set_static() ;" << endl ;
  for (map<string,record_type*>::iterator it = v->types.begin(); it != v->types.end(); ++it)
  {
    record_type *r = it->second ;
    int x = record_type_to_num[r] ;
    log_assert(x>0, "unknown record index for name '%s'", r->name.c_str()) ;
    for (unsigned i=0; i<r->nodes.size(); ++i)
    {
      int j = node_to_num[r->nodes[i]] ;
      log_assert(j>0, "node index not found for type '%s' node #%d", it->first.c_str(), i) ;
      cpp << "record_type" << x << ".nodes[" << i << "] = &node" << j << " ;" << endl ;
    }
    cpp << "A.types[" << '"' << it->first << '"' << "] = &record_type" << x << " ;" << endl ;
  }
  cpp << "}" << endl << "return &A ;" << endl << "}" << endl << endl ;
}

int main_try(int ac, char **av)
{
  string c_output, h_output ;
  vector<string> input ;

  const char *usage = "iodata-type-to-c++ -o output.c++ -d output.h [input.type]..." ;
  for (int c; (c=getopt(ac, av, "ho:d:")) != -1; )
  {
    if (c=='h')
    {
      log_notice("usage: %s", usage) ;
      return 0 ;
    }
    else if(c=='o')
    {
      if (not c_output.empty())
      {
        log_error("only a single '-o' option allowed") ;
        return 1 ;
      }
      c_output = optarg ;
    }
    else if(c=='d')
    {
      if (not h_output.empty())
      {
        log_error("only a single '-d' option allowed") ;
        return 1 ;
      }
      h_output = optarg ;
    }
    else
    {
      log_error("usage: %s", usage) ;
      return 1 ;
    }
  }
  for (int i=optind; i < ac; ++i)
    input.push_back(av[i]) ;

  if (input.size()==0)
  {
    log_error("no input files. usage: %s", usage) ;
    return 1 ;
  }

  if (input.size()>1 and c_output.empty() and h_output.empty())
  {
    log_error("output file(s) must be specified if multiple input") ;
    return 1 ;
  }

  if (c_output.empty() and input.size()==1)
    c_output = input[0] + ".c++" ;

  if (h_output.empty() and input.size()==1)
    h_output = input[0] + ".h" ;

  ostringstream cpp, h ;
  for (unsigned i=0; i<input.size(); ++i)
  {
    // log_notice("processing input file: '%s'", input[i].c_str()) ;
    iodata::validator *v = iodata::validator::from_file(input[i].c_str()) ;
    dump_cpp(cpp, v) ;
    dump_h(h, v) ;
  }

  bool failure = false ;

  if (not h_output.empty())
  {
    string header = h.str() ;

    ostringstream sum ;
    sum << "iodata_type_to_cxx_" ;
    string salt = "$5$salt$" ;
    if (const char *enc = crypt(header.c_str(), salt.c_str()))
      for (const char *p=enc; *p; ++p)
        sum << (unsigned int)(unsigned char) *p ;
    else
      sum << header.length() ;

    header = "#ifndef "+sum.str()+"\n" + "#define "+sum.str()+ "\n" + header + "#endif\n" ;

    if (iodata::storage::write_string_to_file(h_output.c_str(), header.c_str()) < 0)
    {
      log_error("can't write header file to '%s': %m", h_output.c_str()) ;
      failure = true ;
    }
  }

  if (not c_output.empty())
  {
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    string include = "#include <iodata-qt5/validator>\n" ;
#else
    string include = "#include <iodata/validator>\n" ;
#endif

    if (not h_output.empty())
      include += "#include \"" + h_output + "\"\n" ;

    string program = include + cpp.str() ;

    if (iodata::storage::write_string_to_file(c_output.c_str(), program.c_str()) < 0)
    {
      log_error("can't write c++ output to '%s': %m", c_output.c_str()) ;
      failure = true ;
    }
  }

  return failure ? 1 : 0 ;
}

int main(int ac, char **av)
{
  try
  {
    return main_try(ac, av) ;
  }
  catch(const std::exception &e)
  {
    log_error("exception: %s", e.what()) ;
    return 1 ;
  }
}
