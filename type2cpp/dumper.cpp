#include <iostream>
using namespace std ;
#include <iodata/iodata>
#include <iodata/validator>

void dump_cpp(ostringstream &cpp, const string &full_name, iodata::validator *v)
{
  using iodata::/*validator::*/record_type ;
  using iodata::/*validator::*/node ;
  using iodata::/*validator::*/node_integer ;
  using iodata::/*validator::*/node_bytes ;
  using iodata::/*validator::*/node_bitmask ;
  using iodata::/*validator::*/node_record ;

  cpp << "iodata::validator * " << full_name << "()" << "{" << endl ;
  cpp << "static bool init_done = false ;" << endl ;
  cpp << "static iodata::validator A ;" << endl ;
  map<record_type*, int> record_type_to_num ;
  int rec_no = 0 ;
  for (map<string,record_type*>::iterator it = v->types.begin(); it != v->types.end() and ++rec_no; ++it)
  {
    record_type *rt = it->second ;
    record_type_to_num[rt] = rec_no ;
    cpp << "static iodata::validator::record_type record_type" << rec_no << " = { " ;
    cpp << '"' << it->first << '"' << ", " << rt->nodes.size() << " } ;" << endl ;
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
      cpp << "static iodata::validator::" << no->node_name() << " node" << node_no ;
      cpp << "(" << '"' << no->name << '"' << ", " ;
      cpp << no->is_array << ", " << no->is_mandatory << ", " ;
      if (node_integer *nn = dynamic_cast<node_integer*> (no))
        cpp << nn->value ;
      else if (node_bytes *nn = dynamic_cast<node_bytes*> (no))
        cpp << '"' << nn->value << '"' ;
      else if (node_record *nn = dynamic_cast<node_record*> (no))
        cpp << '"' << nn->type_name << '"' ;
      else if (node_bitmask *nn = dynamic_cast<node_bitmask*> (no))
        cpp << "iodata::bitmask(" << nn->value.xl << ", bitmask_list" << bitmask_no << ")" ;
      cpp << ") ;" << endl ;
    }
  }
}

int main(int ac, char **av)
{
  for (int i=1; i<ac; ++i)
  {
    cout << "// " << av[i] << endl ;
    iodata::validator *v = iodata::validator::from_file(av[i]) ;
    ostringstream cpp ;
    dump_cpp (cpp, (string)"blah", v) ;
    cout << cpp.str() << endl << endl ;
  }
  return 0 ;
}
