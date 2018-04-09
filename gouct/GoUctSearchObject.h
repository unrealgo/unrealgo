
#ifndef GOUCT_OBJECTWITHSEARCH_H
#define GOUCT_OBJECTWITHSEARCH_H

class GoUctSearch;
class GoUctSearchObject {
 public:
  virtual ~GoUctSearchObject();
  virtual GoUctSearch &Search() = 0;
  virtual const GoUctSearch &Search() const = 0;
};

#endif // GOUCT_OBJECTWITHSEARCH_H
