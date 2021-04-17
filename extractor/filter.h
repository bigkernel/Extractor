// Author: yuyue/X3130 (yuyue2200@hotmail.com)

#ifndef EXTRACTOR_FILTER_H_
#define EXTRACTOR_FILTER_H_

#include <string>
#include "extractor/rule_define.h"

namespace ext {
enum KeyType {
  PHONE,
  IMEI,
  IMSI,
  MAC,
  IDCARD,
  LONGITUDE,
  LATITUDE,
  IDFX,
  EMAIL,
  CITYCODE,
  UNKNOWN,
};

// Format and check value valid
typedef bool (*Filter)(std::string*);

// Returns an number in relation to the key.
int MakeType(const std::string& key);

// Returns an functions used to format and filter by the type
// that in relation to the key.
Filter FilterFactory(int type);

bool CoordinateTranslate(
    Coordinate::Type from, const std::string& f_lon, const std::string& f_lat,
    Coordinate::Type to, std::string* to_lon, std::string* to_lat);

} // namespace ext

#endif // EXTRACTOR_FILTER_H_
