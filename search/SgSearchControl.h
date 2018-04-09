
#ifndef SG_SEARCHCONTROL_H
#define SG_SEARCHCONTROL_H

class SgSearchControl {
 public:
  SgSearchControl();
  virtual ~SgSearchControl();
  
  virtual bool Abort(double elapsedTime, int numNodes) = 0;
  
  virtual bool StartNextIteration(int depth, double elapsedTime,
                                  int numNodes);

 private:
  
  SgSearchControl(const SgSearchControl &);
  
  SgSearchControl &operator=(const SgSearchControl &);
};

inline SgSearchControl::SgSearchControl() {}


class SgTimeSearchControl
    : public SgSearchControl {
 public:
  SgTimeSearchControl(double maxTime);
  virtual ~SgTimeSearchControl();
  virtual bool Abort(double elapsedTime, int ignoreNumNodes);
  double GetMaxTime() const;
  void SetMaxTime(double maxTime);

 private:
  double m_maxTime;
  
  SgTimeSearchControl(const SgTimeSearchControl &);
  
  SgTimeSearchControl &operator=(const SgTimeSearchControl &);
};

inline double SgTimeSearchControl::GetMaxTime() const {
  return m_maxTime;
}

inline void SgTimeSearchControl::SetMaxTime(double maxTime) {
  m_maxTime = maxTime;
}


class SgNodeSearchControl
    : public SgSearchControl {
 public:
  SgNodeSearchControl(int maxNumNodes);
  virtual ~SgNodeSearchControl();
  virtual bool Abort(double ignoreElapsedTime, int numNodes);
  void SetMaxNumNodes(int maxNumNodes);

 private:
  int m_maxNumNodes;
  
  SgNodeSearchControl(const SgNodeSearchControl &);
  
  SgNodeSearchControl &operator=(const SgNodeSearchControl &);
};

inline void SgNodeSearchControl::SetMaxNumNodes(int maxNumNodes) {
  m_maxNumNodes = maxNumNodes;
}


class SgCombinedSearchControl
    : public SgSearchControl {
 public:
  SgCombinedSearchControl(double maxTime, int maxNumNodes);
  virtual ~SgCombinedSearchControl();
  virtual bool Abort(double elapsedTime, int numNodes);

 private:
  double m_maxTime;
  int m_maxNumNodes;
  
  SgCombinedSearchControl(const SgCombinedSearchControl &);
  
  SgCombinedSearchControl &operator=(const SgCombinedSearchControl &);
};

inline SgCombinedSearchControl::SgCombinedSearchControl(double maxTime,
                                                        int maxNumNodes)
    : m_maxTime(maxTime),
      m_maxNumNodes(maxNumNodes) {}


class SgRelaxedSearchControl
    : public SgSearchControl {
 public:
  static const int MIN_NODES_PER_SECOND = 1000;
  SgRelaxedSearchControl(double maxTime);
  virtual ~SgRelaxedSearchControl();
  virtual bool Abort(double elapsedTime, int numNodes);

 private:
  double m_maxTime;
  
  SgRelaxedSearchControl(const SgRelaxedSearchControl &);
  
  SgRelaxedSearchControl &operator=(const SgRelaxedSearchControl &);
};

inline SgRelaxedSearchControl::SgRelaxedSearchControl(double maxTime)
    : m_maxTime(maxTime) {}

#endif
