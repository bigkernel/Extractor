// Author: yuyue/X3130 (yuyue2200@hotmail.com)

#ifndef EXTRACTOR_INTERNAL_RULE_H_
#define EXTRACTOR_INTERNAL_RULE_H_

#include <string>
#include <vector>
#include <map>
#include <unordered_map>

#include "extractor/rule_define.h"

namespace ext {
// Filter that format and checkout extraction value has valid,
// it's declared in filter.h
typedef bool (*Filter)(std::string*);

struct Step {
  unsigned int type;

  int step;               // PREFIX/SUFFIX
  struct {
    std::string pattern;  // PREFIX/SUFFIX
    int offset;           // START_POS/END_POS
    Skip::Type skip;      // SKIP/RSKIP
    int length_len;       // LEN_LENGTH
    char split;           // SPLIT
  } s;

#define s_pattern s.pattern
#define s_offset s.offset
#define s_skip s.skip
#define s_length_len s.length_len
#define s_split s.split
};

struct Rule {
  struct Key {
    std::string key;    // extraction results key
    std::string mapped; // JSON/XML attribute name
    int type;           // temporary value in internal declared,
                        // it has be used to get filter
    Filter filter;      // format and checkout extraction result
  };

  RuleLayer::Type type;
  int gid;
  DataSource::Type data_src;
  Coordinate::Type coordinate;
  unsigned int confidence;
  unsigned int priority;
  std::vector<Step> steps;
  std::unordered_map<std::string, std::string> attribute;
  std::vector<Key> keys;
  std::vector<Rule> sub_rules;
  std::string rule_key;

  // below variables from step layer, it's treated as an attribute.
  std::string charset;
  bool big_endian;      // TYPE_LENGTH/LEN_LENGTH
  int index;            // TYPE_LENGTH/LEN_LENGTH
  std::string tlv_type; // TYPE_LENGTH/LEN_LENGTH
  int type_len;         // TYPE_LENGTH
  std::vector<Codec::Type> value_encode;

  // all special rule operators
  std::string head;         // JSON/XML/F0/F1 rule
  std::string tail;         // JSON/XML/F0/F1 rule
  std::string group_split;  // F1 rule
  std::string word_split;   // F1 rule

  Rule(): type(RuleLayer::UNKNOWN),
          gid(-1),
          data_src(DataSource::Type::UNKNOWN),
          coordinate(Coordinate::Type::UNKNOWN),
          confidence(55),
          priority(1),
          big_endian(false),
          index(0),
          type_len(0) {}
};

struct Category {
  std::unordered_map<std::string, std::string> attribute;
  std::vector<Rule> rules;
  std::unordered_map<int /* GID */, int /* rule */> gids;
  std::vector<Codec::Type> req_codec;
  std::vector<Codec::Type> res_codec;
};

struct Application {
  Protocol::Type protocol;
  std::unordered_map<std::string, std::string> attribute;
  std::unordered_map<std::string, int> index;
  std::vector<int> wild_index;
  std::vector<Category> cates;
};

struct RuleTree {
  std::unordered_map<std::string, int> index;
  std::vector<int> wild_index;
  std::vector<Application> apps;
};

// Make an rule tree with buffer that who is shows
// in the expression by XML format file.
RuleTree MakeRuleTree(const char* buf, size_t len);

} // namespace ext

#endif // EXTRACTOR_INTERNAL_RULE_H_
