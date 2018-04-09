
#ifndef SG_PROP_H
#define SG_PROP_H

#include <string>
#include <vector>
#include "board/GoBlackWhite.h"
#include "board/GoPoint.h"
#include "lib/SgVector.h"

class SgProp;
typedef int SgPropID;
typedef int SgPropFlags;
enum SgPropPointFmt {
  SG_PROPPOINTFMT_GO,
  SG_PROPPOINTFMT_HEX
};

namespace SgPropUtil {
std::string EscapeSpecialCharacters(const std::string& s,
                                    bool escapeColon);
SgPropPointFmt GetPointFmt(int gameNumber);
std::string PointToSgfString(GoMove p, int boardSize,
                             SgPropPointFmt fmt, int fileFormat = 4);
GoPoint SgfStringToPoint(const std::string& s, int boardSize,
                         SgPropPointFmt fmt);
}

const int SG_MAX_PROPCLASS = 150;
const int SG_PROPCLASS_BLACK = 1 << 0;
const int SG_PROPCLASS_WHITE = 1 << 1;
const int SG_PROPCLASS_INFO = 1 << 2;
const int SG_PROPCLASS_ANNO = 1 << 3;
const int SG_PROPCLASS_STAT = 1 << 4;
const int SG_PROPCLASS_ROOT = 1 << 5;
const int SG_PROPCLASS_ANNO_MOVE = 1 << 6;
const int SG_PROPCLASS_ANNO_POS = 1 << 7;
const int SG_PROPCLASS_MOVE = 1 << 8;
const int SG_PROPCLASS_MARK = 1 << 9;
const int SG_PROPCLASS_TIME = 1 << 10;
const int SG_PROPCLASS_ABSTRACT = 1 << 11;
const int SG_PROPCLASS_NOT_FF3 = 1 << 12;
const int SG_PROPCLASS_NOT_FF4 = 1 << 13;
const int SG_PROPCLASS_CUSTOM = 1 << 14;
const int SG_PROPCLASS_NOTCLEAN = 1 << 15;
const int SG_PROPCLASS_NEWLINE = 1 << 16;
class SgPropList {
 public:
  SgPropList();
  ~SgPropList();
  bool IsEmpty() const;
  void Clear();
  SgProp* Get(SgPropID id) const;
  SgProp* GetPropContainingText(const std::string& findText) const;
  void Add(const SgProp* prop);
  void MoveToFront(SgPropID id);
  bool Remove(const SgProp* prop);
  void Remove(SgPropID id, const SgProp* protectProp);
  void RemoveProp(SgPropID id);
  bool AppendMoveAnnotation(std::string* s) const;

 private:
  friend class SgPropListIterator;
  SgVectorOf<SgProp> m_list;
  SgPropList(const SgPropList&);
  SgPropList& operator=(const SgPropList&);
};

inline bool SgPropList::IsEmpty() const {
  return m_list.IsEmpty();
}

inline void SgPropList::RemoveProp(SgPropID id) {
  Remove(id, 0);
}

class SgPropListIterator {
 public:

  SgPropListIterator(const SgPropList& propList);
  void operator++();
  SgProp* operator*() const;
  operator bool() const;

 private:
  SgVectorIteratorOf<SgProp> m_listIterator;
  operator int() const;
  SgPropListIterator(const SgPropListIterator&);
  SgPropListIterator& operator=(const SgPropListIterator&);
};

inline SgPropListIterator::SgPropListIterator(const SgPropList& propList)
    : m_listIterator(propList.m_list) {
}

inline void SgPropListIterator::operator++() {
  m_listIterator.operator++();
}

inline SgProp* SgPropListIterator::operator*() const {
  return m_listIterator.operator*();
}

inline SgPropListIterator::operator bool() const {
  return m_listIterator.operator bool();
}

class SgProp {
 public:
  explicit SgProp(SgPropID id);
  virtual ~SgProp();
  virtual SgProp* Duplicate() const = 0;
  SgPropID ID() const;
  virtual SgPropFlags Flags() const;
  virtual std::string Label() const;
  bool Flag(SgPropFlags flags) const;
  virtual bool ToString(std::vector<std::string>& values, int boardSize,
                        SgPropPointFmt fmt, int fileFormat) const = 0;
  virtual bool FromString(const std::vector<std::string>& values,
                          int boardSize, SgPropPointFmt fmt) = 0;
  static SgPropID Register(SgProp* prop, const char* label,
                           SgPropFlags flags = 0);
  static SgProp* CreateProperty(SgPropID id);
  static SgPropID GetIDOfLabel(const std::string& label);
  static SgPropID ConvertFindTextToPropID(const std::string& findText);
  static void Init();
  static void Fini();
  SgBlackWhite Player() const;
  bool IsPlayer(SgBlackWhite player) const;
  static SgPropID OpponentProp(SgPropID id);
  static SgPropID PlayerProp(SgPropID id, SgBlackWhite player);
  virtual void ChangeToOpponent();
  bool MatchesID(SgPropID id) const;
  virtual bool ContainsText(const std::string& findText);

 protected:
  SgPropID m_id;
  static bool Initialized();

 private:

  static bool s_initialized;
  static int s_numPropClasses;
  static SgPropFlags s_flags[SG_MAX_PROPCLASS];
  static std::string s_label[SG_MAX_PROPCLASS];
  static SgProp* s_prop[SG_MAX_PROPCLASS];
  SgProp(const SgProp&);
  SgProp& operator=(const SgProp&);
};

inline SgProp::SgProp(SgPropID id)
    : m_id(id) {
}

inline SgPropID SgProp::ID() const {
  return m_id;
}

inline bool SgProp::Flag(SgPropFlags flags) const {
  return (Flags() & flags) != 0;
}

class SgPropUnknown : public SgProp {
 public:
  explicit SgPropUnknown(SgPropID id);
  SgPropUnknown(SgPropID id, std::string label,
                const std::vector<std::string>& values);
  SgProp* Duplicate() const;
  bool ToString(std::vector<std::string>& values, int boardSize,
                SgPropPointFmt fmt, int fileFormat) const;
  bool FromString(const std::vector<std::string>& values,
                  int boardSize, SgPropPointFmt fmt);
  std::string Label() const;

 private:
  std::string m_label;
  std::vector<std::string> m_values;
};

inline SgPropUnknown::SgPropUnknown(SgPropID id)
    : SgProp(id) {
}

inline SgPropUnknown::SgPropUnknown(SgPropID id, std::string label,
                                    const std::vector<std::string>& values)
    : SgProp(id),
      m_label(label),
      m_values(values) {
}

inline std::string SgPropUnknown::Label() const {
  return m_label;
}

class SgPropInt : public SgProp {
 public:
  explicit SgPropInt(SgPropID id);
  SgPropInt(SgPropID id, int value);
  SgProp* Duplicate() const;
  bool ToString(std::vector<std::string>& values, int boardSize,
                SgPropPointFmt fmt, int fileFormat) const;
  bool FromString(const std::vector<std::string>& values,
                  int boardSize, SgPropPointFmt fmt);
  int Value() const;
  bool IsValue(int value) const;
  void SetValue(int value);

 protected:
  int m_value;
};

inline SgPropInt::SgPropInt(SgPropID id)
    : SgProp(id),
      m_value(0) {
}

inline SgPropInt::SgPropInt(SgPropID id, int value)
    : SgProp(id),
      m_value(value) {
}

inline int SgPropInt::Value() const {
  DBG_ASSERT(Initialized());
  return m_value;
}

inline bool SgPropInt::IsValue(int value) const {
  return m_value == value;
}

inline void SgPropInt::SetValue(int value) {
  m_value = value;
}

class SgPropReal : public SgProp {
 public:
  explicit SgPropReal(SgPropID id);
  SgPropReal(SgPropID id, double value, int precision = 0);
  SgProp* Duplicate() const;
  bool ToString(std::vector<std::string>& values, int boardSize,
                SgPropPointFmt fmt, int fileFormat) const;
  bool FromString(const std::vector<std::string>& values,
                  int boardSize, SgPropPointFmt fmt);
  double Value() const;
  void SetValue(double value, int precision = 0);

 protected:
  int m_precision;
  double m_value;
};

inline SgPropReal::SgPropReal(SgPropID id)
    : SgProp(id),
      m_precision(0),
      m_value(0) {
}

inline SgPropReal::SgPropReal(SgPropID id, double value, int precision)
    : SgProp(id),
      m_precision(precision),
      m_value(value) {
}

inline double SgPropReal::Value() const {
  return m_value;
}

inline void SgPropReal::SetValue(double value, int precision) {
  m_value = value;
  m_precision = precision;
}

class SgPropSimple : public SgProp {
 public:
  explicit SgPropSimple(SgPropID id);
  SgProp* Duplicate() const;
  bool ToString(std::vector<std::string>& values, int boardSize,
                SgPropPointFmt fmt, int fileFormat) const;
  bool FromString(const std::vector<std::string>& values,
                  int boardSize, SgPropPointFmt fmt);
};

inline SgPropSimple::SgPropSimple(SgPropID id)
    : SgProp(id) {
}

class SgPropMultiple : public SgPropInt {
 public:
  explicit SgPropMultiple(SgPropID id);
  SgPropMultiple(SgPropID id, int value);
  SgProp* Duplicate() const;
};

inline SgPropMultiple::SgPropMultiple(SgPropID id)
    : SgPropInt(id, 1) {
}

inline SgPropMultiple::SgPropMultiple(SgPropID id, int value)
    : SgPropInt(id, value) {
}

class SgPropValue : public SgPropInt {
 public:
  explicit SgPropValue(SgPropID id);
  SgPropValue(SgPropID id, int value);
  SgProp* Duplicate() const;
  virtual void ChangeToOpponent();
};

inline SgPropValue::SgPropValue(SgPropID id)
    : SgPropInt(id) {
}

inline SgPropValue::SgPropValue(SgPropID id, int value)
    : SgPropInt(id, value) {
}

class SgPropTime : public SgPropReal {
 public:
  SgPropTime(SgPropID id, double value = 0, int precision = 1);
  virtual ~SgPropTime();
  SgProp* Duplicate() const;
};

inline SgPropTime::SgPropTime(SgPropID id, double value, int precision)
    : SgPropReal(id, value, precision) {
}

class SgPropMSec : public SgPropTime {
 public:
  explicit SgPropMSec(SgPropID id);
  SgPropMSec(SgPropID id, double value);
  virtual ~SgPropMSec();
  SgProp* Duplicate() const;
};

inline SgPropMSec::SgPropMSec(SgPropID id)
    : SgPropTime(id, 0, 3) {
}

inline SgPropMSec::SgPropMSec(SgPropID id, double value)
    : SgPropTime(id, value, 3) {
}

class SgPropMove : public SgProp {
 public:
  explicit SgPropMove(SgPropID id);
  SgPropMove(SgPropID id, GoMove move);
  SgProp* Duplicate() const;
  GoPoint Value() const;
  bool IsValue(GoPoint move) const;
  bool ToString(std::vector<std::string>& values, int boardSize,
                SgPropPointFmt fmt, int fileFormat) const;
  bool FromString(const std::vector<std::string>& values,
                  int boardSize, SgPropPointFmt fmt);

 protected:
  GoPoint m_move;
};

inline SgPropMove::SgPropMove(SgPropID id)
    : SgProp(id),
      m_move(GO_NULLMOVE) {
}

inline SgPropMove::SgPropMove(SgPropID id, GoMove move)
    : SgProp(id),
      m_move(move) {
}

inline GoPoint SgPropMove::Value() const {
  return m_move;
}

inline bool SgPropMove::IsValue(GoPoint move) const {
  return m_move == move;
}

class SgPropPointList : public SgProp {
 public:
  explicit SgPropPointList(SgPropID id);
  SgPropPointList(SgPropID id, const SgVector<GoPoint>& list);
  virtual ~SgPropPointList();
  SgProp* Duplicate() const;
  const SgVector<GoPoint>& Value() const;
  SgVector<GoPoint>& Value();
  void SetValue(const SgVector<GoPoint>& list);
  void PushBack(GoPoint p);
  bool ToString(std::vector<std::string>& values, int boardSize,
                SgPropPointFmt fmt, int fileFormat) const;
  bool FromString(const std::vector<std::string>& values,
                  int boardSize, SgPropPointFmt fmt);

 private:
  SgVector<GoPoint> m_list;
};

inline SgPropPointList::SgPropPointList(SgPropID id)
    : SgProp(id) {
}

inline const SgVector<GoPoint>& SgPropPointList::Value() const {
  return m_list;
}

inline SgVector<GoPoint>& SgPropPointList::Value() {
  return m_list;
}

inline void SgPropPointList::SetValue(const SgVector<GoPoint>& list) {
  m_list = list;
}

inline void SgPropPointList::PushBack(GoPoint p) {
  m_list.PushBack(p);
}

class SgPropText : public SgProp {
 public:
  explicit SgPropText(SgPropID id);
  SgPropText(SgPropID id, const std::string& text);
  SgProp* Duplicate() const;
  const std::string& Value() const;
  std::string& Value();
  void SetValue(const std::string& value);
  void AppendText(const std::string& text);
  bool ToString(std::vector<std::string>& values, int boardSize,
                SgPropPointFmt fmt, int fileFormat) const;
  bool FromString(const std::vector<std::string>& values,
                  int boardSize, SgPropPointFmt fmt);
  virtual bool ContainsText(const std::string& findText);

 private:
  std::string m_text;
};

inline SgPropText::SgPropText(SgPropID id)
    : SgProp(id),
      m_text() {
}

inline SgPropText::SgPropText(SgPropID id, const std::string& text)
    : SgProp(id),
      m_text(text) {
}

inline const std::string& SgPropText::Value() const {
  return m_text;
}

inline std::string& SgPropText::Value() {
  return m_text;
}

inline void SgPropText::SetValue(const std::string& value) {
  m_text = value;
}

inline void SgPropText::AppendText(const std::string& text) {
  m_text += text;
}

class SgPropTextList : public SgProp {
 public:
  explicit SgPropTextList(SgPropID id);
  SgPropTextList(SgPropID id, const SgVector<GoPoint>& points,
                 SgVectorOf<std::string> strings);
  virtual ~SgPropTextList();
  SgProp* Duplicate() const;
  const SgVector<GoPoint>& GetPointsWithText() const;
  bool GetStringAtPoint(GoPoint p, std::string* s) const;
  void AddStringAtPoint(GoPoint p, const std::string& s);
  void AppendToStringAtPoint(GoPoint p, const std::string& s);
  void ClearStringAtPoint(GoPoint p);
  bool ToString(std::vector<std::string>& values, int boardSize,
                SgPropPointFmt fmt, int fileFormat) const;
  bool FromString(const std::vector<std::string>& values,
                  int boardSize, SgPropPointFmt fmt);
  virtual bool ContainsText(const std::string& findText);

 private:
  SgVector<GoPoint> m_points;
  SgVectorOf<std::string> m_strings;
};

inline SgPropTextList::SgPropTextList(SgPropID id)
    : SgProp(id),
      m_points(),
      m_strings() {
}

inline const SgVector<GoPoint>& SgPropTextList::GetPointsWithText() const {
  return m_points;
}

class SgPropPlayer : public SgProp {
 public:
  explicit SgPropPlayer(SgPropID id);
  SgPropPlayer(SgPropID id, int player);
  SgProp* Duplicate() const final;
  SgBlackWhite Value() const;
  void SetValue(SgBlackWhite player);
  virtual void ChangeToOpponent() final;
  bool ToString(std::vector<std::string>& values, int boardSize,
                SgPropPointFmt fmt, int fileFormat) const;
  bool FromString(const std::vector<std::string>& values,
                  int boardSize, SgPropPointFmt fmt);

 private:
  SgBlackWhite m_player;
};

inline SgPropPlayer::SgPropPlayer(SgPropID id)
    : SgProp(id),
      m_player(SG_BLACK) {
}

inline SgPropPlayer::SgPropPlayer(SgPropID id, int player)
    : SgProp(id),
      m_player(player) {
}

inline SgBlackWhite SgPropPlayer::Value() const {
  return m_player;
}

inline void SgPropPlayer::SetValue(SgBlackWhite player) {
  m_player = player;
}

class SgPropAddStone
    : public SgPropPointList {
 public:
  explicit SgPropAddStone(SgPropID id);
  SgPropAddStone(SgPropID id, const SgVector<GoPoint>& list);
  virtual ~SgPropAddStone();
  SgProp* Duplicate() const;
};

inline SgPropAddStone::SgPropAddStone(SgPropID id)
    : SgPropPointList(id) {
}

inline SgPropAddStone::SgPropAddStone(SgPropID id,
                                      const SgVector<GoPoint>& list)
    : SgPropPointList(id, list) {
}

extern SgPropID SG_PROP_NONE;
extern SgPropID SG_PROP_UNKNOWN;
extern SgPropID SG_PROP_MOVE;
extern SgPropID SG_PROP_MOVE_BLACK;
extern SgPropID SG_PROP_MOVE_WHITE;
extern SgPropID SG_PROP_ADD_BLACK;
extern SgPropID SG_PROP_ADD_WHITE;
extern SgPropID SG_PROP_ADD_EMPTY;
extern SgPropID SG_PROP_PLAYER;
extern SgPropID SG_PROP_VALUE;
extern SgPropID SG_PROP_TERR_BLACK;
extern SgPropID SG_PROP_TERR_WHITE;
extern SgPropID SG_PROP_MARKS;
extern SgPropID SG_PROP_SELECT;
extern SgPropID SG_PROP_MARKED;
extern SgPropID SG_PROP_TRIANGLE;
extern SgPropID SG_PROP_SQUARE;
extern SgPropID SG_PROP_DIAMOND;
extern SgPropID SG_PROP_CIRCLE;
extern SgPropID SG_PROP_DIMMED;
extern SgPropID SG_PROP_LABEL;
extern SgPropID SG_PROP_TIMES;
extern SgPropID SG_PROP_TIME_BLACK;
extern SgPropID SG_PROP_TIME_WHITE;
extern SgPropID SG_PROP_OT_BLACK;
extern SgPropID SG_PROP_OT_WHITE;
extern SgPropID SG_PROP_OT_NU_MOVES;
extern SgPropID SG_PROP_OT_PERIOD;
extern SgPropID SG_PROP_OVERHEAD;
extern SgPropID SG_PROP_LOSE_TIME;
extern SgPropID SG_PROP_COUNT;
extern SgPropID SG_PROP_TIME_USED;
extern SgPropID SG_PROP_NUM_NODES;
extern SgPropID SG_PROP_NUM_LEAFS;
extern SgPropID SG_PROP_MAX_DEPTH;
extern SgPropID SG_PROP_DEPTH;
extern SgPropID SG_PROP_PART_DEPTH;
extern SgPropID SG_PROP_EVAL;
extern SgPropID SG_PROP_EXPECTED;
extern SgPropID SG_PROP_SELF_TEST;
extern SgPropID SG_PROP_FORMAT;
extern SgPropID SG_PROP_SIZE;
extern SgPropID SG_PROP_GAME;
extern SgPropID SG_PROP_SPEC_BLACK;
extern SgPropID SG_PROP_SPEC_WHITE;
extern SgPropID SG_PROP_CHINESE;
extern SgPropID SG_PROP_APPLIC;
extern SgPropID SG_PROP_ANNOTATE;
extern SgPropID SG_PROP_COMMENT;
extern SgPropID SG_PROP_NAME;
extern SgPropID SG_PROP_CHECK;
extern SgPropID SG_PROP_SIGMA;
extern SgPropID SG_PROP_HOTSPOT;
extern SgPropID SG_PROP_FIGURE;
extern SgPropID SG_PROP_POS_ANNO;
extern SgPropID SG_PROP_GOOD_BLACK;
extern SgPropID SG_PROP_GOOD_WHITE;
extern SgPropID SG_PROP_EVEN_POS;
extern SgPropID SG_PROP_UNCLEAR;
extern SgPropID SG_PROP_MOVE_ANNO;
extern SgPropID SG_PROP_GOOD_MOVE;
extern SgPropID SG_PROP_BAD_MOVE;
extern SgPropID SG_PROP_INTERESTING;
extern SgPropID SG_PROP_DOUBTFUL;
extern SgPropID SG_PROP_INFO;
extern SgPropID SG_PROP_GAME_NAME;
extern SgPropID SG_PROP_GAME_COMMENT;
extern SgPropID SG_PROP_EVENT;
extern SgPropID SG_PROP_ROUND;
extern SgPropID SG_PROP_DATE;
extern SgPropID SG_PROP_PLACE;
extern SgPropID SG_PROP_PLAYER_BLACK;
extern SgPropID SG_PROP_PLAYER_WHITE;
extern SgPropID SG_PROP_RESULT;
extern SgPropID SG_PROP_USER;
extern SgPropID SG_PROP_TIME;
extern SgPropID SG_PROP_SOURCE;
extern SgPropID SG_PROP_COPYRIGHT;
extern SgPropID SG_PROP_ANALYSIS;
extern SgPropID SG_PROP_RANK_BLACK;
extern SgPropID SG_PROP_RANK_WHITE;
extern SgPropID SG_PROP_TEAM_BLACK;
extern SgPropID SG_PROP_TEAM_WHITE;
extern SgPropID SG_PROP_OPENING;
extern SgPropID SG_PROP_RULES;
extern SgPropID SG_PROP_HANDICAP;
extern SgPropID SG_PROP_KOMI;
extern SgPropID SG_PROP_FIND_MOVE;
extern SgPropID SG_PROP_FIND_TEXT;
extern SgPropID SG_PROP_BRANCH;
extern SgPropID SG_PROP_TERMINAL;
extern SgPropID SG_PROP_MOTIVE;
extern SgPropID SG_PROP_SEQUENCE;
extern SgPropID SG_PROP_NOT_EMPTY;
extern SgPropID SG_PROP_NOT_BLACK;
extern SgPropID SG_PROP_NOT_WHITE;

#endif
