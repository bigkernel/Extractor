// Author: yuyue/X3130 (X3130@njsecnet.com)

#ifndef EXTRACTOR_RULE_OPS_H_
#define EXTRACTOR_RULE_OPS_H_

#include <string>
#include <vector>
#include <unordered_map>

namespace ext {
struct RuleTree;

typedef std::unordered_map<std::string, std::string> Attributes;
typedef std::pair<std::string, std::string> Pair;

// Make an new application layer in rule tree that constructed
// from attributes.
// Returns SUCCESS on success, otherwise an error code occurred,
// in which case, categories and rule of this must be abandoned.
int AppendApplication(RuleTree* rt, Attributes& attrs);

// Make an new category layer in rule tree that constructed
// from attributes.
// Returns SUCCESS on success, otherwise an error code occurred,
// in which case, rule and step of this must be abandoned.
int AppendCategory(RuleTree* rt, Attributes& attrs);

// Make an new rule layer in rule tree that constructed
// from attributes of rule and step.
// Returns SUCCESS on success, otherwise an error code occurred,
// in which case, step of this must be abandoned.
// Note: The error code NEGATIVE_RULE can be occurred but
// just indicated that has become invalid of the rule.
int AppendRule(RuleTree* rt, Attributes& attrs,
               const std::vector<Pair>& step_attrs);

} // namespace ext

#endif // EXTRACTOR_RULE_OPS_H_
