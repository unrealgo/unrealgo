
#ifndef SG_RESTORER_H
#define SG_RESTORER_H

// saves the current state of a variable of type T ,
// and restores it to that saved value upon leaving the scope
template<class T>
class SgRestorer {
 public:
  explicit SgRestorer(T *oldState)
      : m_variable(oldState),
        m_oldState(*oldState) {}

  ~SgRestorer() {
    *m_variable = m_oldState;
  }

 private:
  T *m_variable;
  T m_oldState;

  SgRestorer(const SgRestorer &) = delete;
  SgRestorer &operator=(const SgRestorer &) = delete;
};

template<class T>
class SgAssertRestored {
 public:
  explicit SgAssertRestored(T *oldState)
      : m_variable(oldState),
        m_oldState(*oldState) {}

  ~SgAssertRestored() {
    DBG_ASSERT(*m_variable == m_oldState);
  }

 private:
  T *m_variable;
  T m_oldState;

  SgAssertRestored(const SgAssertRestored &) = delete;
  SgAssertRestored &operator=(const SgAssertRestored &) = delete;
};

#endif // SG_RESTORER_H
