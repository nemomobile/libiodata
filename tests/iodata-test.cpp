#include <cassert>

#include <iostream>
#include <string>
#include <sstream>
using namespace std ;

#include <QtGlobal>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <qmlog-qt5>
#include <iodata-qt5/iodata.h>
#else
#include <qmlog>
#include <iodata/iodata.h>
#endif

int trivial(int ac, char **av) ;
int storage(int ac, char **av) ;

int main(int ac, char **av)
{
  log_assert(ac>1) ;
  string which = av[1] ;

  ac--, av++ ;

  int status = 1 ;
  try
  {
    if(which=="trivial")
      status = trivial(ac, av) ;
    else if(which=="storage")
      status = storage(ac, av) ;
  }
  catch(...)
  {
    cerr << "oops, uncatched exception, failure\n" ;
    status = 1 ;
  }
  return status ;
}

int storage(int, char **)
{
  return 0 ;
}

int trivial(int , char **)
{
  string data_s = "a=5, b=007, c=0x2348924, d={a=1, b=02, c=0x3, d=[1,\"xxx\"]} ." ;
  istringstream data_is(data_s) ;
  iodata::parser x(data_is) ;
#define R(p,k) (dynamic_cast<iodata::record*>(p)->x[k])
#define A(p,i) (dynamic_cast<iodata::array*>(p)->x[i])
#define I(p) (dynamic_cast<iodata::integer*>(p)->x)
#define S(p) (dynamic_cast<iodata::bytes*>(p)->x)
  if(x.parse())
  {
    assert(I(R(x.tree,"a"))==5) ;
    assert(I(R(x.tree,"b"))==007) ;
    assert(I(R(x.tree,"c"))==0x2348924) ;
    assert(I(R(R(x.tree,"d"),"a"))==1) ;
    assert(I(R(R(x.tree,"d"),"b"))==2) ;
    assert(I(R(R(x.tree,"d"),"c"))==0x3) ;
    assert(I(A(R(R(x.tree,"d"),"d"),0))==1) ;
    assert(S(A(R(R(x.tree,"d"),"d"),1))=="xxx") ;

    iodata::record *x_tree = x.detach() ;

    assert(I(R(x_tree,"a"))==5) ;
    assert(I(R(x_tree,"b"))==007) ;
    assert(I(R(x_tree,"c"))==0x2348924) ;
    assert(I(R(R(x_tree,"d"),"a"))==1) ;
    assert(I(R(R(x_tree,"d"),"b"))==2) ;
    assert(I(R(R(x_tree,"d"),"c"))==0x3) ;
    assert(I(A(R(R(x_tree,"d"),"d"),0))==1) ;
    assert(S(A(R(R(x_tree,"d"),"d"),1))=="xxx") ;

    assert(x_tree->get("a")->value()==5) ;
    assert(x_tree->get("b")->value()==007) ;
    assert(x_tree->get("c")->value()==0x2348924) ;
    assert(x_tree->get("d")->rec()->get("a")->value()==1) ;
    assert(x_tree->get("d")->rec()->get("b")->value()==2) ;
    assert(x_tree->get("d")->rec()->get("c")->value()==0x3) ;
    assert(x_tree->get("d")->rec()->get("d")->arr()->get(0)->value()==1) ;
    assert(x_tree->get("d")->rec()->get("d")->arr()->get(1)->str()=="xxx") ;

    delete x_tree ;
  }
  else
    return 1 ;
  return 0 ;
}

