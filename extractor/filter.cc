// Author: yuyue/X3130 (yuyue2200@hotmail.com)

#include "extractor/filter.h"

#include <cmath>
#include <cctype>
#include <cassert>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <algorithm>
#include <stdexcept>

namespace ext {
namespace {
#define KEY_TYPE_MAP(XX)                              \
  XX(PHONENUM,                  KeyType::PHONE)       \
  XX(IDFAorIDFV,                KeyType::IDFX)        \
  XX(TAOBAO_ACCOUNT,            KeyType::UNKNOWN)     \
  XX(TAOBAO_ACCOUNT,            KeyType::UNKNOWN)     \
  XX(ALIPAY_ACCOUNT,            KeyType::UNKNOWN)     \
  XX(QQ_ACCOUNT,                KeyType::UNKNOWN)     \
  XX(WEIXIN_ACCOUNT,            KeyType::UNKNOWN)     \
  XX(WEIBO_ACCOUNT,             KeyType::UNKNOWN)     \
  XX(APP_IMEI,                  KeyType::IMEI)        \
  XX(APP_IMSI,                  KeyType::IMSI)        \
  XX(APP_MAC,                   KeyType::MAC)         \
  XX(APP_IDFA,                  KeyType::IDFX)        \
  XX(APP_IDFV,                  KeyType::IDFX)        \
  XX(APP_LONGITUDE,             KeyType::LONGITUDE)   \
  XX(APP_LATITUDE,              KeyType::LATITUDE)    \
  XX(GOODSID,                   KeyType::UNKNOWN)     \
  XX(BALANCE,                   KeyType::UNKNOWN)     \
  XX(VEHICLE_FRAME_MODEL,       KeyType::UNKNOWN)     \
  XX(VEHICLE_CODE,              KeyType::UNKNOWN)     \
  XX(VEHICLE_MOTOR_MODEL,       KeyType::UNKNOWN)     \
  XX(SERVICE_ADDRESS,           KeyType::UNKNOWN)     \
  XX(SERVICE_PEOPLE,            KeyType::UNKNOWN)     \
  XX(SERVICE_TIME,              KeyType::UNKNOWN)     \
  XX(ABOVE_SEALEVEL,            KeyType::UNKNOWN)     \
  XX(PROVINCE,                  KeyType::UNKNOWN)     \
  XX(CITY,                      KeyType::UNKNOWN)     \
  XX(CURRENT_CITY,              KeyType::UNKNOWN)     \
  XX(BIRTHPLACE,                KeyType::UNKNOWN)     \
  XX(COMPANYADDRESS,            KeyType::UNKNOWN)     \
  XX(OFFICEPHONE,               KeyType::UNKNOWN)     \
  XX(WORKYEAR,                  KeyType::UNKNOWN)     \
  XX(NATIONALITY,               KeyType::UNKNOWN)     \
  XX(CONTACTADDRESS,            KeyType::UNKNOWN)     \
  XX(HOMEPHONE,                 KeyType::UNKNOWN)     \
  XX(EDUCATIONDESC,             KeyType::UNKNOWN)     \
  XX(RELATIONSHIP_ADDRESS,      KeyType::UNKNOWN)     \
  XX(DESCRIPTION,               KeyType::UNKNOWN)     \
  XX(NATION,                    KeyType::UNKNOWN)     \
  XX(BIRTHDAY,                  KeyType::UNKNOWN)     \
  XX(APARTMENT,                 KeyType::UNKNOWN)     \
  XX(CONSTELLATION,             KeyType::UNKNOWN)     \
  XX(SEX,                       KeyType::UNKNOWN)     \
  XX(REALNAME,                  KeyType::UNKNOWN)     \
  XX(BLOODTYPE,                 KeyType::UNKNOWN)     \
  XX(POSITION,                  KeyType::UNKNOWN)     \
  XX(CLASSINFO,                 KeyType::UNKNOWN)     \
  XX(DEPART_ADDRESS,            KeyType::UNKNOWN)     \
  XX(FROM_LONGITUDE,            KeyType::LONGITUDE)   \
  XX(FROM_LATITUDE,             KeyType::LATITUDE)    \
  XX(ORGINSITE,                 KeyType::UNKNOWN)     \
  XX(DEST_ADDRESS,              KeyType::UNKNOWN)     \
  XX(TO_LONGITUDE,              KeyType::LONGITUDE)   \
  XX(TO_LATITUDE,               KeyType::LATITUDE)    \
  XX(DESTSITE,                  KeyType::UNKNOWN)     \
  XX(DISTANCE,                  KeyType::UNKNOWN)     \
  XX(HOTELADDRESS,              KeyType::UNKNOWN)     \
  XX(HOTELNAME,                 KeyType::UNKNOWN)     \
  XX(PRICE,                     KeyType::UNKNOWN)     \
  XX(COUNT,                     KeyType::UNKNOWN)     \
  XX(SPEED,                     KeyType::UNKNOWN)     \
  XX(WEIGHT,                    KeyType::UNKNOWN)     \
  XX(TITLE,                     KeyType::UNKNOWN)     \
  XX(TOPIC_ID,                  KeyType::UNKNOWN)     \
  XX(COPY_FROM,                 KeyType::UNKNOWN)     \
  XX(AUTHOR,                    KeyType::UNKNOWN)     \
  XX(AUTHOR_ID,                 KeyType::UNKNOWN)     \
  XX(FAX,                       KeyType::UNKNOWN)     \
  XX(COMPANY,                   KeyType::UNKNOWN)     \
  XX(KEYWORD,                   KeyType::UNKNOWN)     \
  XX(ENGLISHNAME,               KeyType::UNKNOWN)     \
  XX(APP_VERSION,               KeyType::UNKNOWN)     \
  XX(APP_PACKAGE_NAME,          KeyType::UNKNOWN)     \
  XX(PASSWORD,                  KeyType::UNKNOWN)     \
  XX(NICKNAME,                  KeyType::UNKNOWN)     \
  XX(SHOPID,                    KeyType::UNKNOWN)     \
  XX(SHOPNAME,                  KeyType::UNKNOWN)     \
  XX(PAYTIME,                   KeyType::UNKNOWN)     \
  XX(POSTCODE,                  KeyType::UNKNOWN)     \
  XX(FILESIZE,                  KeyType::UNKNOWN)     \
  XX(FILENAME,                  KeyType::UNKNOWN)     \
  XX(EMAIL,                     KeyType::EMAIL)       \
  XX(ANCHOR_ID,                 KeyType::UNKNOWN)     \
  XX(ANCHOR,                    KeyType::UNKNOWN)     \
  XX(APP_MSISDN,                KeyType::PHONE)       \
  XX(APP_IDCARD,                KeyType::IDCARD)      \
  XX(PAYMONEY,                  KeyType::UNKNOWN)     \
  XX(BANKCARDNO,                KeyType::UNKNOWN)     \
  XX(INTEREST,                  KeyType::UNKNOWN)     \
  XX(RELATIONSHIP_MOBILEPHONE,  KeyType::PHONE)       \
  XX(RELATIONSHIP_NAME,         KeyType::UNKNOWN)     \
  XX(RELATIONSHIP_EMAIL,        KeyType::EMAIL)       \
  XX(RELATIONSHIP_REMARK,       KeyType::UNKNOWN)     \
  XX(RELATIONSHIP_ACCOUNT,      KeyType::UNKNOWN)     \
  XX(USERAGE,                   KeyType::UNKNOWN)     \
  XX(NET_ORG_PLATE,             KeyType::UNKNOWN)     \
  XX(SUBSCRIBE_COUNT,           KeyType::UNKNOWN)     \
  XX(RESUME_DATE,               KeyType::UNKNOWN)     \
  XX(COMMENT_CONTENT,           KeyType::UNKNOWN)     \
  XX(CONTENT,                   KeyType::UNKNOWN)     \
  XX(MEMBERTOTAL,               KeyType::UNKNOWN)     \
  XX(FROM_ID,                   KeyType::UNKNOWN)     \
  XX(FROM_NICKNAME,             KeyType::UNKNOWN)     \
  XX(FROM_ACCOUNT,              KeyType::UNKNOWN)     \
  XX(MANAGER_NICKNAME,          KeyType::UNKNOWN)     \
  XX(MANAGER_ACCOUNT,           KeyType::UNKNOWN)     \
  XX(MEMBERCARD,                KeyType::UNKNOWN)     \
  XX(TO_ID,                     KeyType::UNKNOWN)     \
  XX(TO_NICKNAME,               KeyType::UNKNOWN)     \
  XX(TO_ACCOUNT,                KeyType::UNKNOWN)     \
  XX(GROUP_NAME,                KeyType::UNKNOWN)     \
  XX(GROUPOWNER_NUM,            KeyType::UNKNOWN)     \
  XX(GROUPOWNER_NICKNAME,       KeyType::UNKNOWN)     \
  XX(PUSERID,                   KeyType::UNKNOWN)     \
  XX(USERNAME,                  KeyType::UNKNOWN)     \
  XX(ORDER_ID,                  KeyType::UNKNOWN)     \
  XX(ORDER_CREATE_TIME,         KeyType::UNKNOWN)     \
  XX(FULLADDRESS,               KeyType::UNKNOWN)     \
  XX(GOODSNAME,                 KeyType::UNKNOWN)     \
  XX(EXPRESS_COMPANY,           KeyType::UNKNOWN)     \
  XX(EXPRESS_SITE,              KeyType::UNKNOWN)     \
  XX(DVR_CITY,                  KeyType::UNKNOWN)     \
  XX(DVR_ADDRESSDETAIL,         KeyType::UNKNOWN)     \
  XX(DVR_MOBILE,                KeyType::UNKNOWN)     \
  XX(RECEIVER_NAME,             KeyType::UNKNOWN)     \
  XX(DVR_DEVISIONCODE,          KeyType::UNKNOWN)     \
  XX(DVR_AREA,                  KeyType::UNKNOWN)     \
  XX(DVR_PROVINCE,              KeyType::UNKNOWN)     \
  XX(ADDREESSOR_CITY,           KeyType::UNKNOWN)     \
  XX(ADDREESSOR_ADDRESS,        KeyType::UNKNOWN)     \
  XX(ADDREESSOR_MOBILE,         KeyType::UNKNOWN)     \
  XX(ADDREESSOR_NAME,           KeyType::UNKNOWN)     \
  XX(ADDREESSOR_DEVISIONCOCE,   KeyType::UNKNOWN)     \
  XX(ADDREESSOR_AREA,           KeyType::UNKNOWN)     \
  XX(ADDREESSOR_PROVINCE,       KeyType::UNKNOWN)     \
  XX(FILE_MD5,                  KeyType::UNKNOWN)     \
  XX(SEND_TIME,                 KeyType::UNKNOWN)     \
  XX(WIFI_NAME,                 KeyType::UNKNOWN)     \
  XX(PROGRAM_NAME,              KeyType::UNKNOWN)     \
  XX(ALBUM_NAME,                KeyType::UNKNOWN)     \
  XX(FROM_PART,                 KeyType::UNKNOWN)     \
  XX(TO_PART,                   KeyType::UNKNOWN)     \
  XX(MAIL_SUBJECT,              KeyType::UNKNOWN)     \
  XX(APP_NAME,                  KeyType::UNKNOWN)     \
  XX(DEPT,                      KeyType::UNKNOWN)     \
  XX(ORDER_NUM,                 KeyType::UNKNOWN)     \
  XX(APP_ICCID,                 KeyType::UNKNOWN)     \
  XX(APP_IP,                    KeyType::UNKNOWN)     \
  XX(APP_IP_ADDRESS,            KeyType::UNKNOWN)     \
  XX(APP_IP_CITY,               KeyType::UNKNOWN)     \
  XX(APP_IP_PROVINCE,           KeyType::UNKNOWN)     \
  XX(APP_MCC,                   KeyType::UNKNOWN)     \
  XX(NETTYPE,                   KeyType::UNKNOWN)     \
  XX(PAY_PWD,                   KeyType::UNKNOWN)     \
  XX(APP_DEVSCREEN,             KeyType::UNKNOWN)     \
  XX(APP_SSID,                  KeyType::UNKNOWN)     \
  XX(WIFI_MAC,                  KeyType::MAC)         \
  XX(WIFI_STRENGTH,             KeyType::UNKNOWN)     \
  XX(FLIGHTID,                  KeyType::UNKNOWN)     \
  XX(ORIGIN_AIRPORT,            KeyType::UNKNOWN)     \
  XX(BEGINDATE,                 KeyType::UNKNOWN)     \
  XX(BEGINTIME,                 KeyType::UNKNOWN)     \
  XX(DEST_AIRPORT,              KeyType::UNKNOWN)     \
  XX(PASSWORD2,                 KeyType::UNKNOWN)     \
  XX(SERVICE_NAME,              KeyType::UNKNOWN)     \
  XX(DEADLINE,                  KeyType::UNKNOWN)     \
  XX(DEGREE,                    KeyType::UNKNOWN)     \
  XX(MAJOR,                     KeyType::UNKNOWN)     \
  XX(SEATID,                    KeyType::UNKNOWN)     \
  XX(ROOMID,                    KeyType::UNKNOWN)     \
  XX(CHECKOUTTIME,              KeyType::UNKNOWN)     \
  XX(CHECKINTIME,               KeyType::UNKNOWN)     \
  XX(LEVEL,                     KeyType::UNKNOWN)     \
  XX(HIGHT,                     KeyType::UNKNOWN)     \
  XX(COMMENTTIME,               KeyType::UNKNOWN)     \
  XX(LANGUAGE,                  KeyType::UNKNOWN)     \
  XX(FILECONTENT,               KeyType::UNKNOWN)     \
  XX(GOODSTYPE,                 KeyType::UNKNOWN)     \
  XX(AUDIENCE_NUM,              KeyType::UNKNOWN)     \
  XX(ACCESS_POINT,              KeyType::UNKNOWN)     \
  XX(BANK_CITY,                 KeyType::UNKNOWN)     \
  XX(BANK_PROVINCE,             KeyType::UNKNOWN)     \
  XX(COMPANY_ID,                KeyType::UNKNOWN)     \
  XX(CPU_MODEL,                 KeyType::UNKNOWN)     \
  XX(CUSTOMER_SERVICE,          KeyType::UNKNOWN)     \
  XX(DEVICE_BRAND,              KeyType::UNKNOWN)     \
  XX(DEVICE_PLATFORM,           KeyType::UNKNOWN)     \
  XX(EMPLOYEE_NAME,             KeyType::UNKNOWN)     \
  XX(FAMILY_NAME,               KeyType::UNKNOWN)     \
  XX(LAST_NAME,                 KeyType::UNKNOWN)     \
  XX(GAME_VERSION,              KeyType::UNKNOWN)     \
  XX(JOB_ID,                    KeyType::UNKNOWN)     \
  XX(LOCAL_IP,                  KeyType::UNKNOWN)     \
  XX(LOCATION_CITY,             KeyType::UNKNOWN)     \
  XX(LOCATION_COUNTY,           KeyType::UNKNOWN)     \
  XX(LOCATION_DETAIL,           KeyType::UNKNOWN)     \
  XX(LOCATION_NAME,             KeyType::UNKNOWN)     \
  XX(LOCATION_PROVINCE,         KeyType::UNKNOWN)     \
  XX(LOCATION_STEET,            KeyType::UNKNOWN)     \
  XX(LOCATION_STREET_NO,        KeyType::UNKNOWN)     \
  XX(LOCATION_TYPE,             KeyType::UNKNOWN)     \
  XX(MODIFY_DATE,               KeyType::UNKNOWN)     \
  XX(OS_VERSION,                KeyType::UNKNOWN)     \
  XX(RESIDENCE_ADDRESS,         KeyType::UNKNOWN)     \
  XX(ROLE_ID,                   KeyType::UNKNOWN)     \
  XX(ROLE_LEVEL,                KeyType::UNKNOWN)     \
  XX(SCREEN_DPI,                KeyType::UNKNOWN)     \
  XX(SCREEN_HEIGHT,             KeyType::UNKNOWN)     \
  XX(SCREEN_WIDTH,              KeyType::UNKNOWN)     \
  XX(SERVER_ZONE,               KeyType::UNKNOWN)     \
  XX(TIMESTAMP,                 KeyType::UNKNOWN)     \
  XX(EMPLOYEE_ID,               KeyType::UNKNOWN)     \
  XX(RAM_SIZE,                  KeyType::UNKNOWN)     \
  XX(CPU_CORE_NUM,              KeyType::UNKNOWN)     \
  XX(CAR_SERIES,                KeyType::UNKNOWN)     \
  XX(CAR_CODE_AREA,             KeyType::UNKNOWN)     \
  XX(CAR_CODE,                  KeyType::UNKNOWN)     \
  XX(PRODUCT_ID,                KeyType::UNKNOWN)     \
  XX(CAR_TYPE,                  KeyType::UNKNOWN)     \
  XX(BUY_TIME,                  KeyType::UNKNOWN)     \
  XX(STUDENT_NAME,              KeyType::UNKNOWN)     \
  XX(SEAT_TYPE,                 KeyType::UNKNOWN)     \
  XX(CLASS_NO,                  KeyType::UNKNOWN)     \
  XX(MEAL_NAME,                 KeyType::UNKNOWN)     \
  XX(SPONSOR_NAME,              KeyType::UNKNOWN)     \
  XX(COACH_NO,                  KeyType::UNKNOWN)     \
  XX(PASSENGER_ID,              KeyType::UNKNOWN)     \
  XX(PUBLISH_DATA,              KeyType::UNKNOWN)     \
  XX(CARRIER_NAME,              KeyType::UNKNOWN)     \
  XX(EDUCATION,                 KeyType::UNKNOWN)     \
  XX(BUILDING_ORIENTATION,      KeyType::UNKNOWN)     \
  XX(FLOOR_AREA,                KeyType::UNKNOWN)     \
  XX(HEIGHT,                    KeyType::UNKNOWN)     \
  XX(SCHOOL_NAME,               KeyType::UNKNOWN)     \
  XX(REGISTER_TIME,             KeyType::UNKNOWN)     \
  XX(LOGIN_DATA,                KeyType::UNKNOWN)     \
  XX(JOB_RECRUITMENT,           KeyType::UNKNOWN)     \
  XX(PAY_TYPE,                  KeyType::UNKNOWN)     \
  XX(DRIVER_YEAR,               KeyType::UNKNOWN)     \
  XX(STREET_NO,                 KeyType::UNKNOWN)     \
  XX(TICKET_TYPE,               KeyType::UNKNOWN)     \
  XX(PAY_CONTENT,               KeyType::UNKNOWN)     \
  XX(BUILDING_STRUCTURE,        KeyType::UNKNOWN)     \
  XX(PASSENGER_NAME,            KeyType::UNKNOWN)     \
  XX(COMPANY_TYPE,              KeyType::UNKNOWN)     \
  XX(CLASS_NAME,                KeyType::UNKNOWN)     \
  XX(UNIT,                      KeyType::UNKNOWN)     \
  XX(BUILDING_NUMBER,           KeyType::UNKNOWN)     \
  XX(TEACHER_NAME,              KeyType::UNKNOWN)     \
  XX(SERVICE_RANGE,             KeyType::UNKNOWN)     \
  XX(BUILDBING_TYPE,            KeyType::UNKNOWN)     \
  XX(PAPER_QUALITY,             KeyType::UNKNOWN)     \
  XX(HOUSING_BEARER,            KeyType::UNKNOWN)     \
  XX(TRAIN_NO,                  KeyType::UNKNOWN)     \
  XX(SOCIAL_SECURITY_NUMBER,    KeyType::UNKNOWN)     \
  XX(TABLE_NO,                  KeyType::UNKNOWN)     \
  XX(SCHOOL_DEPARTMENT,         KeyType::UNKNOWN)     \
  XX(DESTSITE_AIRPORT,          KeyType::UNKNOWN)     \
  XX(JOB_NUMBER,                KeyType::UNKNOWN)     \
  XX(TRAIN_START_DATE,          KeyType::UNKNOWN)     \
  XX(WORK_EXPERIENCE,           KeyType::UNKNOWN)     \
  XX(CURRENT_ADDRESS,           KeyType::UNKNOWN)     \
  XX(TICKET_PRICE,              KeyType::UNKNOWN)     \
  XX(DRIVER_PHONE,              KeyType::PHONE)       \
  XX(DRIVER_NAME,               KeyType::UNKNOWN)     \
  XX(CELL_NUMBER,               KeyType::UNKNOWN)     \
  XX(PUBLISHER,                 KeyType::UNKNOWN)     \
  XX(CUSTOMER_NAME,             KeyType::UNKNOWN)     \
  XX(DRIVER_LICENSE_DATE,       KeyType::UNKNOWN)     \
  XX(TICKET_ENTRANCE,           KeyType::UNKNOWN)     \
  XX(TICKET_PURCHASE_TIME,      KeyType::UNKNOWN)     \
  XX(ISBN,                      KeyType::UNKNOWN)     \
  XX(RECEIVER_ID,               KeyType::UNKNOWN)     \
  XX(QUERY_INFO,                KeyType::UNKNOWN)     \
  XX(UUID,                      KeyType::UNKNOWN)     \
  XX(VERIFICATION_CODE,         KeyType::UNKNOWN)     \
  XX(TOKEN,                     KeyType::UNKNOWN)     \
  XX(COORDINATE_SYSTEM,         KeyType::UNKNOWN)     \
  XX(EXPIRE_YEAR,               KeyType::UNKNOWN)     \
  XX(BUY_YEAR,                  KeyType::UNKNOWN)     \
  XX(OWNER_NAME,                KeyType::UNKNOWN)     \
  XX(INCOME,                    KeyType::UNKNOWN)     \
  XX(NOVEL_NAME,                KeyType::UNKNOWN)     \
  XX(NOVEL_ID,                  KeyType::UNKNOWN)     \
  XX(LABEL,                     KeyType::UNKNOWN)     \
  XX(SONGNAME,                  KeyType::UNKNOWN)     \
  XX(SONGER_NAME,               KeyType::UNKNOWN)     \
  XX(DEPART_TIME,               KeyType::UNKNOWN)     \
  XX(OPERATION,                 KeyType::UNKNOWN)     \
  XX(REGISTER_PHONE,            KeyType::PHONE)       \
  XX(BRANTCH_BANK_NAME,         KeyType::UNKNOWN)     \
  XX(BANK_USER_NAME,            KeyType::UNKNOWN)     \
  XX(TAX,                       KeyType::UNKNOWN)     \
  XX(TAX_RATE,                  KeyType::UNKNOWN)     \
  XX(DISPLACEMENT,              KeyType::UNKNOWN)     \
  XX(CAR_COLOR,                 KeyType::UNKNOWN)     \
  XX(CAR_BRAND,                 KeyType::UNKNOWN)     \
  XX(CAR4S_NAME,                KeyType::UNKNOWN)     \
  XX(INSURANCE_COMPANY,         KeyType::UNKNOWN)     \
  XX(CREDIT_OFFICER,            KeyType::UNKNOWN)     \
  XX(LOAN_TYPE,                 KeyType::UNKNOWN)     \
  XX(INSURE_MAN,                KeyType::UNKNOWN)     \
  XX(SELL_TIME,                 KeyType::UNKNOWN)     \
  XX(MONTH_PAY,                 KeyType::UNKNOWN)     \
  XX(INSURE_RATE,               KeyType::UNKNOWN)     \
  XX(LOAN_AMOUNT,               KeyType::UNKNOWN)     \
  XX(REPAY_METHOD,              KeyType::UNKNOWN)     \
  XX(BANK_INTEREST,             KeyType::UNKNOWN)     \
  XX(SERVICE_CHARGE,            KeyType::UNKNOWN)     \
  XX(LOAN_END_DATE,             KeyType::UNKNOWN)     \
  XX(PRODUCT_TYPE,              KeyType::UNKNOWN)     \
  XX(LOAN_BANK_ACOUNT,          KeyType::UNKNOWN)     \
  XX(DEPARTMENT_MANAGER,        KeyType::UNKNOWN)     \
  XX(INSPECTION_DATE,           KeyType::UNKNOWN)     \
  XX(INITIAL_MILEAGE,           KeyType::UNKNOWN)     \
  XX(INSURANCE_DATE,            KeyType::UNKNOWN)     \
  XX(MAINTAIN_DATE,             KeyType::UNKNOWN)     \
  XX(CAR_OWNER,                 KeyType::UNKNOWN)     \
  XX(YEAR_LIMIT,                KeyType::UNKNOWN)     \
  XX(PLATFORM,                  KeyType::UNKNOWN)     \
  XX(FACTORY_PRICE,             KeyType::UNKNOWN)     \
  XX(BUYER_NICKNAME,            KeyType::UNKNOWN)     \
  XX(SELLER_ID,                 KeyType::UNKNOWN)     \
  XX(TOTAL_COUNT,               KeyType::UNKNOWN)     \
  XX(TOTAL_PRICE,               KeyType::UNKNOWN)     \
  XX(STOCK_CODE,                KeyType::UNKNOWN)     \
  XX(STOCK_PRICE,               KeyType::UNKNOWN)     \
  XX(STOCK_NAME,                KeyType::UNKNOWN)     \
  XX(YEAR_INCOME,               KeyType::UNKNOWN)     \
  XX(MONTH_INCOME,              KeyType::UNKNOWN)     \
  XX(INCOME_SOURCE,             KeyType::UNKNOWN)     \
  XX(RELATIONSHIP,              KeyType::UNKNOWN)     \
  XX(OCCUPATION,                KeyType::UNKNOWN)     \
  XX(COMPANY_PHONE,             KeyType::PHONE)       \
  XX(WORK_NATURE,               KeyType::UNKNOWN)     \
  XX(SESSION_ID,                KeyType::UNKNOWN)     \
  XX(TRADE_FEE,                 KeyType::UNKNOWN)     \
  XX(ORDER_SOURCE,              KeyType::UNKNOWN)     \
  XX(TAKE_WAY,                  KeyType::UNKNOWN)     \
  XX(DRIVING_LICENSE_TYPE,      KeyType::UNKNOWN)     \
  XX(MAILING_ADDRESS,           KeyType::UNKNOWN)     \
  XX(COURT,                     KeyType::UNKNOWN)     \
  XX(CASE_NO,                   KeyType::UNKNOWN)     \
  XX(CASE_TYPE,                 KeyType::UNKNOWN)     \
  XX(DOC_ID,                    KeyType::UNKNOWN)     \
  XX(REGISTER_ADDRESS,          KeyType::UNKNOWN)     \
  XX(BUSINESS_SCOPE,            KeyType::UNKNOWN)     \
  XX(CORPORATION_NAME,          KeyType::UNKNOWN)     \
  XX(CORPORATION_IDCARD,        KeyType::UNKNOWN)     \
  XX(COMPANY_PRINCIPAL,         KeyType::UNKNOWN)     \
  XX(PRINCIPAL_IDCARD,          KeyType::UNKNOWN)     \
  XX(QUALITY_PRINCIPAL,         KeyType::UNKNOWN)     \
  XX(QUALITY_IDCARD,            KeyType::UNKNOWN)     \
  XX(TOPIC,                     KeyType::UNKNOWN)     \
  XX(REMARK,                    KeyType::UNKNOWN)     \
  XX(SDK_VERSION,               KeyType::UNKNOWN)     \
  XX(EMAIL_SENDER_NICKNAME,     KeyType::UNKNOWN)     \
  XX(ORDER_STATE,               KeyType::UNKNOWN)     \
  XX(CAR_PRODUCE_YEAR,          KeyType::UNKNOWN)     \
  XX(MILE,                      KeyType::UNKNOWN)     \
  XX(TRANSMISSION_TYPE,         KeyType::UNKNOWN)     \
  XX(WITHDRAWAL_AMOUNT,         KeyType::UNKNOWN)     \
  XX(WITHDRAWAL_CURRENCY,       KeyType::UNKNOWN)     \
  XX(WITHDRAWAL_PWD,            KeyType::UNKNOWN)     \
  XX(BANKNAME,                  KeyType::UNKNOWN)     \
  XX(BANKNODE,                  KeyType::UNKNOWN)

const char* kAsciiDigit = "1234567890";
const char* kAsciiHexDigit = "1234567890abcdefABCDEF";
//const char* kMacCharSet = "1234567890abcdefABCDEF:-";

bool ascii_digit(char c) {
  int x = static_cast<unsigned char>(c);
  return x >= '0' && x <= '9';
}

inline int hex_digit_to_int(char c) {
  int x = static_cast<unsigned char>(c);
  if (x > '9')
    x += 9;
  return x & 0xf;
}

void ToLower(std::string* s) {
  for (size_t i = 0; i < s->size(); ++i) {
    if ((*s)[i] >= 'A' && (*s)[i] <= 'Z')
      (*s)[i] += 32;
  }
}

bool phone_filter(std::string* s) {
  assert(!s->empty());
  const size_t size = s->size();
  std::unique_ptr<char[]> buf(new char[size]);
  if (!buf)
    throw std::runtime_error("No enough memory");
  char* p = buf.get();
  for (size_t i = 0; i < size; ++i) {
    if (ascii_digit((*s)[i]))
      *p++ = (*s)[i];
  }
  s->assign(buf.get(), p);

  if ((*s)[0] == '8' && (*s)[1] == '6') {
    *s = s->substr(2);
  } else if ((*s)[0] == '0' && (*s)[1] == '8' && (*s)[2] == '6') {
    *s = s->substr(3);
  }

  if (s->size() >= 3 && s->size() <= 6)
    return (*s)[0] == '6' && (*s)[1] >= '1' && (*s)[1] <= '9';

  if (s->size() != 11 || (*s)[0] != '1')
    return false;

  switch ((*s)[1]) {
  case '3':
  case '8': return (*s)[2] >= '0' && (*s)[2] <= '9';
  case '4': return (*s)[2] >= '5' && (*s)[2] <= '9';
  case '5': return (*s)[2] >= '0' && (*s)[2] <= '9' && (*s)[2] != '4';
  case '6': return (*s)[2] == '5' || (*s)[2] == '6';
  case '7': return (*s)[2] >= '0' && (*s)[2] <= '8';
  case '9': return (*s)[2] == '1' || (*s)[2] == '5' || (*s)[2] == '8' || (*s)[2] == '9';
  default: return false;
  }

  return true;
}

#if 0
bool city_code_filter(std::string* s) {
  assert(!s->empty());
  if (s->size() != 6)
    return false;
  static const std::unordered_set<std::string> city_code{
#include "extractor/city_code.inline.h"
  };

  return city_code.find(*s) != city_code.end();
}
#endif

bool imsi_filter(std::string* s) {
  assert(!s->empty());
  if (s->size() != 15)
    return false;
  auto pos = s->find_first_not_of(kAsciiDigit);
  if (pos != std::string::npos)
    return false;

  // 460 08 7495038274
  // MCC = 460
  // MNC = 08

  static const std::unordered_set<std::string> imsi_mcc{
    "202", "204", "206", "208", "212", "213", "214", "216",
    "218", "219", "220", "222", "225", "226", "228", "230",
    "231", "232", "234", "235", "238", "240", "242", "244",
    "246", "247", "248", "250", "255", "257", "259", "260",
    "262", "266", "268", "270", "272", "274", "276", "278",
    "280", "282", "283", "284", "286", "288", "290", "292",
    "293", "294", "295", "297", "302", "308", "310", "311",
    "312", "313", "314", "315", "316", "330", "332", "334",
    "338", "340", "342", "344", "346", "348", "350", "352",
    "354", "356", "358", "360", "362", "363", "364", "365",
    "366", "368", "370", "372", "374", "376", "400", "401",
    "402", "404", "405", "406", "410", "412", "413", "414",
    "415", "416", "417", "418", "419", "420", "421", "422",
    "424", "425", "426", "427", "428", "429", "430", "431",
    "432", "434", "436", "437", "438", "440", "441", "450",
    "452", "454", "455", "456", "457", "460", "461", "466",
    "467", "470", "472", "502", "505", "510", "514", "515",
    "520", "525", "528", "530", "534", "535", "536", "537",
    "539", "540", "541", "542", "543", "544", "545", "546",
    "547", "548", "549", "550", "551", "552", "555", "602",
    "603", "604", "605", "606", "607", "608", "609", "610",
    "611", "612", "613", "614", "615", "616", "617", "618",
    "619", "620", "621", "622", "623", "624", "625", "626",
    "627", "628", "629", "630", "631", "632", "633", "634",
    "635", "636", "637", "638", "639", "640", "641", "642",
    "643", "645", "646", "647", "648", "649", "650", "651",
    "652", "653", "654", "655", "657", "702", "704", "706",
    "708", "710", "712", "714", "716", "722", "724", "730",
    "732", "734", "736", "738", "740", "742", "744", "746",
    "748", "750", "901",
  };

  static const std::unordered_set<std::string> imsi_china_mnc{
    "00", "01", "02", "03", "04", "05", "06", "07", "08",
    "09", "11", "20"
  };

  const std::string mcc = s->substr(0, 3);
  if (imsi_mcc.find(mcc) == imsi_mcc.end())
    return false;
  return mcc == "460" ?
      (imsi_china_mnc.find(s->substr(3, 2)) != imsi_china_mnc.end()) : true;
}

bool imei_filter(std::string* s) {
  assert(!s->empty());
  switch (s->size()) {
  case 14: {
    // 99, 98, 97
    if ((*s)[0] == '9' &&
        ((*s)[1] == '9' || (*s)[1] == '8' || (*s)[1] == '7'))
      return true;
    auto pos = s->find_first_not_of(kAsciiDigit);
    if (pos == std::string::npos)
      return true;
    if ((*s)[0] == 'A' &&
        (((*s)[1] >= '0' && (*s)[1] <= '9') || ((*s)[1] >= 'A' && (*s)[1] <= 'F'))) {
      auto pos = s->find_first_not_of(kAsciiHexDigit);
      if (pos == std::string::npos)
        return true;
    }
    return false;
  }
  case 15: {
    auto pos = s->find_first_not_of(kAsciiDigit);
    return pos == std::string::npos;
  }
  default:
    return false;
  }
}

bool CheckMacContinuedZero(const char* s, size_t n, int limit) {
  int counts = 0;
  for (size_t i = 0; i < n; i += (n == 12 ? 2 : 3)) {
    if (s[i] == '0' && s[i + 1] == '0')
      ++counts;
    else
      counts = 0;
    if (counts >= limit)
      return false;
  }
  return true;
}

bool mac_filter(std::string* s) {
  assert(!s->empty());
  switch (s->size()) {
  case 17: {
    static const int offset[] = {2, 4, 6, 8, 10};
    for (unsigned int i = 0; i < sizeof(offset) / sizeof(offset[0]); ++i) {
      *reinterpret_cast<uint16_t*>(&(*s)[offset[i]])
        = *reinterpret_cast<uint16_t*>(&(*s)[offset[i] + i + 1]);
    }
    s->resize(12);
  }
  case 12: {
    if (s->find_first_not_of(kAsciiHexDigit) != std::string::npos)
      return false;
    ToLower(s);
    return CheckMacContinuedZero(s->data(), s->size(), 3);
  }
  default:
    return false;
  }
}

bool IsOutofChina(double lon, double lat) {
  return !((lon >= 73.33 && lon <= 135.05) && (lat >= 3.51 && lat <= 53.33));
}

struct LatLonInfo {
    double COORD_A;
    double b;
    double c;
    double d;
    double e;
    double f;
    double g;
    double h;
    double i;
    double j;
};

const LatLonInfo COORD_MER2LL[6] = {
    {
        1.410526172116255E-008,
        8.98305509648872E-006,
        -1.9939833816331,
        200.98243831067961,
        -187.2403703815547,
        91.608751666984304,
        -23.38765649603339,
        2.57121317296198,
        -0.03801003308653,
        17337981.199999999
    },{
        -7.435856389565537E-009,
        8.983055097726239E-006,
        -0.78625201886289,
        96.326875997598464,
        -1.85204757529826,
        -59.369359054858769,
        47.400335492967372,
        -16.50741931063887,
        2.28786674699375,
        10260144.859999999
    },{
        -3.030883460898826E-008,
        8.98305509983578E-006,
        0.30071316287616,
        59.742936184422767,
        7.357984074871,
        -25.383710026647449,
        13.45380521110908,
        -3.29883767235584,
        0.32710905363475,
        6856817.3700000001
    },{
        -1.981981304930552E-008,
        8.983055099779535E-006,
        0.03278182852591,
        40.316785277057441,
        0.65659298677277,
        -4.44255534477492,
        0.85341911805263,
        0.12923347998204,
        -0.04625736007561,
        4482777.0599999996
    },{
        3.09191371068437E-009,
        8.983055096812155E-006,
        6.995724061999999E-005,
        23.109343041449009,
        -0.00023663490511,
        -0.6321817810242,
        -0.00663494467273,
        0.03430082397953,
        -0.00466043876332,
        2555164.3999999999
    },{
        2.890871144776878E-009,
        8.983055095805407E-006,
        -3.068298E-008,
        7.47137025468032,
        -3.53937994E-006,
        -0.02145144861037,
        -1.234426596E-005,
        0.00010322952773,
        -3.23890364E-006,
        826088.5
    }
};

double COORD_MERBAND[7] = {
    12890594.859999999,
    8362377.8700000001,
    5591021.0,
    3481989.8300000001,
    1678043.1200000001,
    0.0,
    -1.0
};

const double COORD_A = 6378245.0;

const double COORD_ECCENTRICITY = 0.00669342162296594323;

const double COORD_PI = 3.1415926535897932384626;

double TransformLat(double x, double y) {
  double ret = -100.0 + 2.0 * x + 3.0 * y + 0.2 * y * y + 0.1 * x * y
      + 0.2 * sqrt(fabs(x));
  ret += (20.0 * sin(6.0 * x * COORD_PI) + 20.0 * sin(2.0 * x * COORD_PI)) * 2.0
      / 3.0;
  ret += (20.0 * sin(y * COORD_PI) + 40.0 * sin(y / 3.0 * COORD_PI)) * 2.0
      / 3.0;
  ret += (160.0 * sin(y / 12.0 * COORD_PI) + 320 * sin(y * COORD_PI / 30.0))
      * 2.0 / 3.0;
  return ret;
}

double TransformLng(double x, double y) {
  double ret = 300.0 + x + 2.0 * y + 0.1 * x * x + 0.1 * x * y
      + 0.1 * sqrt(fabs(x));
  ret += (20.0 * sin(6.0 * x * COORD_PI) + 20.0 * sin(2.0 * x * COORD_PI))
      * 2.0 / 3.0;
  ret += (20.0 * sin(x * COORD_PI) + 40.0 * sin(x / 3.0 * COORD_PI)) * 2.0
      / 3.0;
  ret += (150.0 * sin(x / 12.0 * COORD_PI) + 300.0 * sin(x / 30.0 * COORD_PI))
      * 2.0 / 3.0;
  return ret;
}

bool Transform(double f_lon, double f_lat, double* t_lon, double* t_lat) {
  if (IsOutofChina(f_lon, f_lat))
    return false;

  double l_dLat = TransformLat(f_lon - 105.0, f_lat - 35.0);
  double l_dLng = TransformLng(f_lon - 105.0, f_lat - 35.0);
  double l_radLat = f_lat / 180.0 * COORD_PI;
  double l_magic = sin(l_radLat);
  l_magic = 1 - COORD_ECCENTRICITY * l_magic * l_magic;
  double l_sqrtMagic = sqrt(l_magic);
  l_dLat = (l_dLat * 180.0)
      / ((COORD_A * (1 - COORD_ECCENTRICITY)) / (l_magic * l_sqrtMagic)
          * COORD_PI);
  l_dLng = (l_dLng * 180.0)
      / (COORD_A / l_sqrtMagic * cos(l_radLat) * COORD_PI);

  *t_lon = f_lon + l_dLng;
  *t_lat = f_lat + l_dLat;
  return true;
}

bool WGS84ToGCJ02(double f_lon, double f_lat, double* t_lon, double* t_lat) {
  if (IsOutofChina(f_lon, f_lat))
    return false;

  double l_dLat = TransformLat(f_lon - 105.0, f_lat - 35.0);
  double l_dLng = TransformLng(f_lon - 105.0, f_lat - 35.0);
  double l_radLat = f_lat / 180.0 * COORD_PI;
  double l_magic = sin(l_radLat);
  l_magic = 1 - COORD_ECCENTRICITY * l_magic * l_magic;
  double l_sqrtMagic = sqrt(l_magic);
  l_dLat = (l_dLat * 180.0)
      / ((COORD_A * (1 - COORD_ECCENTRICITY)) / (l_magic * l_sqrtMagic)
          * COORD_PI);
  l_dLng = (l_dLng * 180.0)
      / (COORD_A / l_sqrtMagic * cos(l_radLat) * COORD_PI);

  *t_lat = f_lat + l_dLat;
  *t_lon = f_lon + l_dLng;
  return true;
}

bool WGS84ToMercator(double f_lon, double f_lat, double* t_lon, double* t_lat) {
  *t_lon = f_lon * 20037508.34 / 180;
  *t_lat = log(tan((90 + f_lat) * COORD_PI / 360)) / (COORD_PI / 180);
  *t_lat = *t_lat * 20037508.34 / 180;
  return true;
}

bool GCJ02ToBD09(double f_lon, double f_lat, double* t_lon, double* t_lat) {
  double z = sqrt(f_lon * f_lon + f_lat * f_lat) + 0.00002 * sin(f_lat * COORD_PI);
  double theta = atan2(f_lat, f_lon) + 0.000003 * cos(f_lon * COORD_PI);
  *t_lon = z * cos(theta) + 0.0065;
  *t_lat = z * sin(theta) + 0.006;
  return true;
}

bool WGS84ToBD09(double f_lon, double f_lat, double* t_lon, double* t_lat) {
  return !WGS84ToGCJ02(f_lon, f_lat, t_lon, t_lat)
      ? GCJ02ToBD09(f_lon, f_lat, t_lon, t_lat) : false;
}

bool GCJ02ToWGS84(double f_lon, double f_lat, double* t_lon, double* t_lat) {
  Transform(f_lon, f_lat, &f_lon, &f_lat);
  *t_lon = f_lon * 2 - f_lon;
  *t_lat = f_lat * 2 - f_lat;
  return true;
}

bool GCJ02ToBd09(double f_lon, double f_lat, double *t_lon, double *t_lat) {
  double z = sqrt(f_lon * f_lon + f_lat * f_lat) + 0.00002 * sin(f_lat * COORD_PI);
  double theta = atan2(f_lat, f_lon) + 0.000003 * cos(f_lon * COORD_PI);
  *t_lon = z * cos(theta) + 0.0065;
  *t_lat = z * sin(theta) + 0.006;
  return true;
}

bool BD09ToGCJ02(double f_lon, double f_lat, double* t_lon, double* t_lat) {
  double x = f_lon - 0.0065, y = f_lat - 0.006;
  double z = sqrt(x * x + y * y) - 0.00002 * sin(y * COORD_PI);
  double theta = atan2(y, x) - 0.000003 * cos(x * COORD_PI);
  *t_lon = z * cos(theta);
  *t_lat = z * sin(theta);
  return true;
}

bool BD09ToWGS84(double f_lon, double f_lat, double* t_lon, double* t_lat) {
  double x, y;
  BD09ToGCJ02(f_lon, f_lat, &x, &y);
  GCJ02ToWGS84(x, y, t_lon, t_lat);
  return true;
}

// flag = true calculating longitude otherwise.
double TransformSogou(double lon, double lat, int flag) {
  bool is_west_lon = false;
  bool is_south_lat = false;

  if (lon < 0.0) {
    lon *= -1.0;
    is_west_lon = true;
  }

  if (lat < 0.0) {
    lat *= -1.0;
    is_south_lat = true;
  }

  int param = 0;
  LatLonInfo localf = COORD_MER2LL[0];

  while (COORD_MERBAND[param] != -1.0) {
    if (lat > COORD_MERBAND[param]) {
      localf = COORD_MER2LL[param];
      break;
    }
    param++;
  }

  double ret = 0.0;
  if (flag) {
    ret = (localf.COORD_A + localf.b * lon);
    if (is_west_lon) {
      ret = (-1.0 * ret);
    }
  } else {
    double d = lat / localf.j;
    ret = (localf.c + localf.d * d + localf.e * d * d + localf.f * d * d * d
        + localf.g * d * d * d * d + localf.h * d * d * d * d * d
        + localf.i * d * d * d * d * d * d);
    if (is_south_lat) {
      ret = (-1.0 * ret);
    }
  }

  return ret;
}

bool SogouToWGS84(double f_lon, double f_lat, double* t_lon, double* t_lat) {
  *t_lon = TransformSogou(f_lon, f_lat, true);
  *t_lat = TransformSogou(f_lon, f_lat, false);
  return true;
}

bool MapBarToWGS84(double f_lon, double f_lat, double* t_lon, double* t_lat) {
  double l_lng = int(f_lon * 100000) % 36000000;
  double l_lat = int(f_lat * 100000) % 36000000;

  double l_lng1 = int(
      -(((cos(l_lat / 100000)) * (l_lng / 18000))
          + ((sin(l_lng / 100000)) * (l_lat / 9000))) + l_lng);
  double l_lat1 = int(
      -(((sin(l_lat / 100000)) * (l_lng / 18000))
          + ((cos(l_lng / 100000)) * (l_lat / 9000))) + l_lat);
  double l_lng2 = int(
      -(((cos(l_lat1 / 100000)) * (l_lng1 / 18000))
          + ((sin(l_lng1 / 100000)) * (l_lat1 / 9000))) + l_lng
          + (l_lng > 0 ? 1 : -1));
  double l_lat2 = int(
      -(((sin(l_lat1 / 100000)) * (l_lng1 / 18000))
          + ((cos(l_lng1 / 100000)) * (l_lat1 / 9000))) + l_lat
          + (l_lat > 0 ? 1 : -1));

  *t_lon = l_lng2 / 100000.0;
  *t_lat = l_lat2 / 100000.0;
  return true;
}

bool MapBarToBD09(double f_lon, double f_lat, double* t_lon, double* t_lat) {
  double x, y;
  if (MapBarToWGS84(f_lon, f_lat, &x, &y) &&
      WGS84ToGCJ02(x, y, &f_lon, &f_lat)) {
    return GCJ02ToBd09(f_lon, f_lat, t_lon, t_lat);
  }
  return false;
}

bool MercatorToWGS84(double f_lon, double f_lat, double* t_lon, double* t_lat) {
  *t_lon = f_lon / 20037508.34 * 180;
  *t_lat = f_lat / 20037508.34 * 180;
  *t_lat = 180 / COORD_PI * (2 * atan(exp(*t_lat * COORD_PI / 180)) - COORD_PI / 2);
  return true;
}

bool CoordinateTranslateImpl(
    Coordinate::Type from, double f_lon, double f_lat,
    Coordinate::Type to, double* to_lon, double* to_lat) {
  typedef bool (*Translater)(double, double, double*, double*);
  static const Translater translaters[Coordinate::Type::UNKNOWN][Coordinate::Type::UNKNOWN] = {
  /* from\to */ /* WGS84            GCJ02         BD09          MERCATOR        MAPBAR  SOUGOU */
  /* WGS84    */ { NULL,            WGS84ToGCJ02, WGS84ToBD09,  WGS84ToMercator,NULL,   NULL },
  /* GCJ02    */ { GCJ02ToWGS84,    NULL,         GCJ02ToBd09,  NULL,           NULL,   NULL },
  /* BD09     */ { BD09ToWGS84,     BD09ToGCJ02,  NULL,         NULL,           NULL,   NULL },
  /* MERCATOR */ { MercatorToWGS84, NULL,         NULL,         NULL,           NULL,   NULL },
  /* MAPBAR   */ { MapBarToWGS84,   NULL,         MapBarToBD09, NULL,           NULL,   NULL },
  /* SOUGOU   */ { SogouToWGS84,    NULL,         NULL,         NULL,           NULL,   NULL },
  };

  return (translaters[from][to])
      ? (translaters[from][to])(f_lon, f_lat, to_lon, to_lat) : false;
}

bool lonlat_filter(std::string* s) {
  assert(!s->empty());
  auto pos = s->find_first_not_of("1234567890.");
  if (pos != std::string::npos)
    return false;
  int v = 0;
  const char* b = s->data();
  const char* e = s->data() + s->size();
  while (b < e && *b != '.') {
    v *= 10;
    v += *b++ - '0';
  }
  return v < 180 && v > 0;
}

bool idfx_filter(std::string* s) {
  assert(!s->empty());
  auto pos = s->find_first_not_of("1234567890abcdefABCDEF-");
  if (pos != std::string::npos)
    return false;
  if (s->size() == 32)
    return true;
  return (*s)[8] == (*s)[13] &&
         (*s)[18] == (*s)[23] &&
         (*s)[23] == (*s)[8];
}

bool email_filter(std::string* s) {
  assert(!s->empty());
  return s->find_first_not_of(
      "1234567890"
      "abcdefghijklmnopqrstuvwxyz"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "@_-.") == std::string::npos;
}

} // anonymous namespace

bool CoordinateTranslate(
    Coordinate::Type from, const std::string& f_lon, const std::string& f_lat,
    Coordinate::Type to, std::string* to_lon, std::string* to_lat) {
  assert(from != to &&
      from != Coordinate::Type::UNKNOWN &&
      to != Coordinate::Type::UNKNOWN);

  double longitude, latitude;
  bool ret = CoordinateTranslateImpl(from, std::stod(f_lon), std::stod(f_lat),
                                     to, &longitude, &latitude);
  if (ret) {
    *to_lon = std::to_string(static_cast<long double>(longitude));
    *to_lat = std::to_string(static_cast<long double>(latitude));
  }
  return ret;
}

int MakeType(const std::string& key) {
  static const std::unordered_map<std::string, int> map{
#define XX(key, type) { #key, type },
    KEY_TYPE_MAP(XX)
#undef XX
  };

  auto iter = map.find(key);
  if (iter != map.end())
    return iter->second;
  return KeyType::UNKNOWN;
}

Filter FilterFactory(int type) {
  static const std::unordered_map<int, Filter> filters{
    { KeyType::PHONE,     phone_filter  },
    { KeyType::IMEI,      imei_filter   },
    { KeyType::IMSI,      imsi_filter   },
    { KeyType::MAC,       mac_filter    },
    { KeyType::LONGITUDE, lonlat_filter },
    { KeyType::LATITUDE,  lonlat_filter },
    { KeyType::IDFX,      idfx_filter   },
    { KeyType::EMAIL,     email_filter  },
  };

  auto iter = filters.find(type);
  if (iter != filters.end())
    return iter->second;
  return NULL;
}

} // namespace ext
