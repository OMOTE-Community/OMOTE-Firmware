// This list is copied from 'IRremoteESP8266/src/IRremoteESP8266.h' (except GC, which was added here)
// We need to copy the list because in the simulator there is no access to the IRremoteESP8266 library.
// Copying is not dangerous, because entries will never be removed or changed.
#include <stdint.h>

/// Enumerator for defining and numbering of supported IR protocol.
/// @note Always add to the end of the list and should never remove entries
///  or change order. Projects may save the type number for later usage
///  so numbering should always stay the same.
enum IRprotocols_new {
  IR_PROTOCOL_UNKNOWN = -1,
  IR_PROTOCOL_UNUSED = 0,
  IR_PROTOCOL_RC5,
  IR_PROTOCOL_RC6,
  IR_PROTOCOL_NEC,
  IR_PROTOCOL_SONY,
  IR_PROTOCOL_PANASONIC,  // (5)
  IR_PROTOCOL_JVC,
  IR_PROTOCOL_SAMSUNG,
  IR_PROTOCOL_WHYNTER,
  IR_PROTOCOL_AIWA_RC_T501,
  IR_PROTOCOL_LG,  // (10)
  IR_PROTOCOL_SANYO,
  IR_PROTOCOL_MITSUBISHI,
  IR_PROTOCOL_DISH,
  IR_PROTOCOL_SHARP,
  IR_PROTOCOL_COOLIX,  // (15)
  IR_PROTOCOL_DAIKIN,
  IR_PROTOCOL_DENON,
  IR_PROTOCOL_KELVINATOR,
  IR_PROTOCOL_SHERWOOD,
  IR_PROTOCOL_MITSUBISHI_AC,  // (20)
  IR_PROTOCOL_RCMM,
  IR_PROTOCOL_SANYO_LC7461,
  IR_PROTOCOL_RC5X,
  IR_PROTOCOL_GREE,
  IR_PROTOCOL_PRONTO,  // Technically not a protocol, but an encoding. (25)
  IR_PROTOCOL_NEC_LIKE,
  IR_PROTOCOL_ARGO,
  IR_PROTOCOL_TROTEC,
  IR_PROTOCOL_NIKAI,
  IR_PROTOCOL_RAW,  // Technically not a protocol, but an encoding. (30)
  IR_PROTOCOL_GLOBALCACHE,  // Technically not a protocol, but an encoding.
  IR_PROTOCOL_TOSHIBA_AC,
  IR_PROTOCOL_FUJITSU_AC,
  IR_PROTOCOL_MIDEA,
  IR_PROTOCOL_MAGIQUEST,  // (35)
  IR_PROTOCOL_LASERTAG,
  IR_PROTOCOL_CARRIER_AC,
  IR_PROTOCOL_HAIER_AC,
  IR_PROTOCOL_MITSUBISHI2,
  IR_PROTOCOL_HITACHI_AC,  // (40)
  IR_PROTOCOL_HITACHI_AC1,
  IR_PROTOCOL_HITACHI_AC2,
  IR_PROTOCOL_GICABLE,
  IR_PROTOCOL_HAIER_AC_YRW02,
  IR_PROTOCOL_WHIRLPOOL_AC,  // (45)
  IR_PROTOCOL_SAMSUNG_AC,
  IR_PROTOCOL_LUTRON,
  IR_PROTOCOL_ELECTRA_AC,
  IR_PROTOCOL_PANASONIC_AC,
  IR_PROTOCOL_PIONEER,  // (50)
  IR_PROTOCOL_LG2,
  IR_PROTOCOL_MWM,
  IR_PROTOCOL_DAIKIN2,
  IR_PROTOCOL_VESTEL_AC,
  IR_PROTOCOL_TECO,  // (55)
  IR_PROTOCOL_SAMSUNG36,
  IR_PROTOCOL_TCL112AC,
  IR_PROTOCOL_LEGOPF,
  IR_PROTOCOL_MITSUBISHI_HEAVY_88,
  IR_PROTOCOL_MITSUBISHI_HEAVY_152,  // 60
  IR_PROTOCOL_DAIKIN216,
  IR_PROTOCOL_SHARP_AC,
  IR_PROTOCOL_GOODWEATHER,
  IR_PROTOCOL_INAX,
  IR_PROTOCOL_DAIKIN160,  // 65
  IR_PROTOCOL_NEOCLIMA,
  IR_PROTOCOL_DAIKIN176,
  IR_PROTOCOL_DAIKIN128,
  IR_PROTOCOL_AMCOR,
  IR_PROTOCOL_DAIKIN152,  // 70
  IR_PROTOCOL_MITSUBISHI136,
  IR_PROTOCOL_MITSUBISHI112,
  IR_PROTOCOL_HITACHI_AC424,
  IR_PROTOCOL_SONY_38K,
  IR_PROTOCOL_EPSON,  // 75
  IR_PROTOCOL_SYMPHONY,
  IR_PROTOCOL_HITACHI_AC3,
  IR_PROTOCOL_DAIKIN64,
  IR_PROTOCOL_AIRWELL,
  IR_PROTOCOL_DELONGHI_AC,  // 80
  IR_PROTOCOL_DOSHISHA,
  IR_PROTOCOL_MULTIBRACKETS,
  IR_PROTOCOL_CARRIER_AC40,
  IR_PROTOCOL_CARRIER_AC64,
  IR_PROTOCOL_HITACHI_AC344,  // 85
  IR_PROTOCOL_CORONA_AC,
  IR_PROTOCOL_MIDEA24,
  IR_PROTOCOL_ZEPEAL,
  IR_PROTOCOL_SANYO_AC,
  IR_PROTOCOL_VOLTAS,  // 90
  IR_PROTOCOL_METZ,
  IR_PROTOCOL_TRANSCOLD,
  IR_PROTOCOL_TECHNIBEL_AC,
  IR_PROTOCOL_MIRAGE,
  IR_PROTOCOL_ELITESCREENS,  // 95
  IR_PROTOCOL_PANASONIC_AC32,
  IR_PROTOCOL_MILESTAG2,
  IR_PROTOCOL_ECOCLIM,
  IR_PROTOCOL_XMP,
  IR_PROTOCOL_TRUMA,  // 100
  IR_PROTOCOL_HAIER_AC176,
  IR_PROTOCOL_TEKNOPOINT,
  IR_PROTOCOL_KELON,
  IR_PROTOCOL_TROTEC_3550,
  IR_PROTOCOL_SANYO_AC88,  // 105
  IR_PROTOCOL_BOSE,
  IR_PROTOCOL_ARRIS,
  IR_PROTOCOL_RHOSS,
  IR_PROTOCOL_AIRTON,
  IR_PROTOCOL_COOLIX48,  // 110
  IR_PROTOCOL_HITACHI_AC264,
  IR_PROTOCOL_KELON168,
  IR_PROTOCOL_HITACHI_AC296,
  IR_PROTOCOL_DAIKIN200,
  IR_PROTOCOL_HAIER_AC160,  // 115
  IR_PROTOCOL_CARRIER_AC128,
  IR_PROTOCOL_TOTO,
  IR_PROTOCOL_CLIMABUTLER,
  IR_PROTOCOL_TCL96AC,
  IR_PROTOCOL_BOSCH144,  // 120
  IR_PROTOCOL_SANYO_AC152,
  IR_PROTOCOL_DAIKIN312,
  IR_PROTOCOL_GORENJE,
  IR_PROTOCOL_WOWWEE,
  IR_PROTOCOL_CARRIER_AC84,  // 125
  IR_PROTOCOL_YORK,
  // Add new entries before this one, and update it to point to the last entry.
  IR_PROTOCOL_kLastDecodeType = IR_PROTOCOL_YORK,
};

// Message lengths & required repeat values
const uint16_t kNoRepeat = 0;
const uint16_t kSingleRepeat = 1;

const uint16_t kAirtonBits = 56;
const uint16_t kAirtonDefaultRepeat = kNoRepeat;
const uint16_t kAirwellBits = 34;
const uint16_t kAirwellMinRepeats = 2;
const uint16_t kAiwaRcT501Bits = 15;
const uint16_t kAiwaRcT501MinRepeats = kSingleRepeat;
const uint16_t kAlokaBits = 32;
const uint16_t kAmcorStateLength = 8;
const uint16_t kAmcorBits = kAmcorStateLength * 8;
const uint16_t kAmcorDefaultRepeat = kSingleRepeat;
const uint16_t kArgoStateLength = 12;
const uint16_t kArgoShortStateLength = 4;
const uint16_t kArgoBits = kArgoStateLength * 8;
const uint16_t kArgoShortBits = kArgoShortStateLength * 8;
const uint16_t kArgo3AcControlStateLength = 6;  // Bytes
const uint16_t kArgo3iFeelReportStateLength = 2;  // Bytes
const uint16_t kArgo3TimerStateLength = 9;  // Bytes
const uint16_t kArgo3ConfigStateLength = 4;  // Bytes
const uint16_t kArgoDefaultRepeat = kNoRepeat;
const uint16_t kArrisBits = 32;
const uint16_t kBosch144StateLength = 18;
const uint16_t kBosch144Bits = kBosch144StateLength * 8;
const uint16_t kCoolixBits = 24;
const uint16_t kCoolix48Bits = kCoolixBits * 2;
const uint16_t kCoolixDefaultRepeat = kSingleRepeat;
const uint16_t kCarrierAcBits = 32;
const uint16_t kCarrierAcMinRepeat = kNoRepeat;
const uint16_t kCarrierAc40Bits = 40;
const uint16_t kCarrierAc40MinRepeat = 2;
const uint16_t kCarrierAc64Bits = 64;
const uint16_t kCarrierAc64MinRepeat = kNoRepeat;
const uint16_t kCarrierAc84StateLength = 11;
const uint16_t kCarrierAc84Bits = kCarrierAc84StateLength * 8 - 4;
const uint16_t kCarrierAc84MinRepeat = kNoRepeat;
const uint16_t kCarrierAc128StateLength = 16;
const uint16_t kCarrierAc128Bits = kCarrierAc128StateLength * 8;
const uint16_t kCarrierAc128MinRepeat = kNoRepeat;
const uint16_t kCoronaAcStateLengthShort = 7;
const uint16_t kCoronaAcStateLength = kCoronaAcStateLengthShort * 3;
const uint16_t kCoronaAcBitsShort = kCoronaAcStateLengthShort * 8;
const uint16_t kCoronaAcBits = kCoronaAcStateLength * 8;
const uint16_t kDaikinStateLength = 35;
const uint16_t kDaikinBits = kDaikinStateLength * 8;
const uint16_t kDaikinStateLengthShort = kDaikinStateLength - 8;
const uint16_t kDaikinBitsShort = kDaikinStateLengthShort * 8;
const uint16_t kDaikinDefaultRepeat = kNoRepeat;
const uint16_t kDaikin2StateLength = 39;
const uint16_t kDaikin2Bits = kDaikin2StateLength * 8;
const uint16_t kDaikin2DefaultRepeat = kNoRepeat;
const uint16_t kDaikin64Bits = 64;
const uint16_t kDaikin64DefaultRepeat = kNoRepeat;
const uint16_t kDaikin160StateLength = 20;
const uint16_t kDaikin160Bits = kDaikin160StateLength * 8;
const uint16_t kDaikin160DefaultRepeat = kNoRepeat;
const uint16_t kDaikin128StateLength = 16;
const uint16_t kDaikin128Bits = kDaikin128StateLength * 8;
const uint16_t kDaikin128DefaultRepeat = kNoRepeat;
const uint16_t kDaikin152StateLength = 19;
const uint16_t kDaikin152Bits = kDaikin152StateLength * 8;
const uint16_t kDaikin152DefaultRepeat = kNoRepeat;
const uint16_t kDaikin176StateLength = 22;
const uint16_t kDaikin176Bits = kDaikin176StateLength * 8;
const uint16_t kDaikin176DefaultRepeat = kNoRepeat;
const uint16_t kDaikin200StateLength = 25;
const uint16_t kDaikin200Bits = kDaikin200StateLength * 8;
const uint16_t kDaikin200DefaultRepeat = kNoRepeat;
const uint16_t kDaikin216StateLength = 27;
const uint16_t kDaikin216Bits = kDaikin216StateLength * 8;
const uint16_t kDaikin216DefaultRepeat = kNoRepeat;
const uint16_t kDaikin312StateLength = 39;
const uint16_t kDaikin312Bits = kDaikin312StateLength * 8;
const uint16_t kDaikin312DefaultRepeat = kNoRepeat;
const uint16_t kDelonghiAcBits = 64;
const uint16_t kDelonghiAcDefaultRepeat = kNoRepeat;
const uint16_t kTechnibelAcBits = 56;
const uint16_t kTechnibelAcDefaultRepeat = kNoRepeat;
const uint16_t kDenonBits = 15;
const uint16_t kDenon48Bits = 48;
const uint16_t kDenonLegacyBits = 14;
const uint16_t kDishBits = 16;
const uint16_t kDishMinRepeat = 3;
const uint16_t kDoshishaBits = 40;
const uint16_t kEcoclimBits = 56;
const uint16_t kEcoclimShortBits = 15;
const uint16_t kEpsonBits = 32;
const uint16_t kEpsonMinRepeat = 2;
const uint16_t kElectraAcStateLength = 13;
const uint16_t kElectraAcBits = kElectraAcStateLength * 8;
const uint16_t kElectraAcMinRepeat = kNoRepeat;
const uint16_t kEliteScreensBits = 32;
const uint16_t kEliteScreensDefaultRepeat = kSingleRepeat;
const uint16_t kFujitsuAcMinRepeat = kNoRepeat;
const uint16_t kFujitsuAcStateLength = 16;
const uint16_t kFujitsuAcStateLengthShort = 7;
const uint16_t kFujitsuAcBits = kFujitsuAcStateLength * 8;
const uint16_t kFujitsuAcMinBits = (kFujitsuAcStateLengthShort - 1) * 8;
const uint16_t kGicableBits = 16;
const uint16_t kGicableMinRepeat = kSingleRepeat;
const uint16_t kGoodweatherBits = 48;
const uint16_t kGoodweatherMinRepeat = kNoRepeat;
const uint16_t kGorenjeBits = 8;
const uint16_t kGreeStateLength = 8;
const uint16_t kGreeBits = kGreeStateLength * 8;
const uint16_t kGreeDefaultRepeat = kNoRepeat;
const uint16_t kHaierACStateLength = 9;
const uint16_t kHaierACBits = kHaierACStateLength * 8;
const uint16_t kHaierAcDefaultRepeat = kNoRepeat;
const uint16_t kHaierACYRW02StateLength = 14;
const uint16_t kHaierACYRW02Bits = kHaierACYRW02StateLength * 8;
const uint16_t kHaierAcYrw02DefaultRepeat = kNoRepeat;
const uint16_t kHaierAC160StateLength = 20;
const uint16_t kHaierAC160Bits = kHaierAC160StateLength * 8;
const uint16_t kHaierAc160DefaultRepeat = kNoRepeat;
const uint16_t kHaierAC176StateLength = 22;
const uint16_t kHaierAC176Bits = kHaierAC176StateLength * 8;
const uint16_t kHaierAc176DefaultRepeat = kNoRepeat;
const uint16_t kHitachiAcStateLength = 28;
const uint16_t kHitachiAcBits = kHitachiAcStateLength * 8;
const uint16_t kHitachiAcDefaultRepeat = kNoRepeat;
const uint16_t kHitachiAc1StateLength = 13;
const uint16_t kHitachiAc1Bits = kHitachiAc1StateLength * 8;
const uint16_t kHitachiAc2StateLength = 53;
const uint16_t kHitachiAc2Bits = kHitachiAc2StateLength * 8;
const uint16_t kHitachiAc3StateLength = 27;
const uint16_t kHitachiAc3Bits = kHitachiAc3StateLength * 8;
const uint16_t kHitachiAc3MinStateLength = 15;
const uint16_t kHitachiAc3MinBits = kHitachiAc3MinStateLength * 8;
const uint16_t kHitachiAc264StateLength = 33;
const uint16_t kHitachiAc264Bits = kHitachiAc264StateLength * 8;
const uint16_t kHitachiAc296StateLength = 37;
const uint16_t kHitachiAc296Bits = kHitachiAc296StateLength * 8;
const uint16_t kHitachiAc344StateLength = 43;
const uint16_t kHitachiAc344Bits = kHitachiAc344StateLength * 8;
const uint16_t kHitachiAc424StateLength = 53;
const uint16_t kHitachiAc424Bits = kHitachiAc424StateLength * 8;
const uint16_t kInaxBits = 24;
const uint16_t kInaxMinRepeat = kSingleRepeat;
const uint16_t kJvcBits = 16;
const uint16_t kKelonBits = 48;
const uint16_t kKelon168StateLength = 21;
const uint16_t kKelon168Bits = kKelon168StateLength * 8;
const uint16_t kKelvinatorStateLength = 16;
const uint16_t kKelvinatorBits = kKelvinatorStateLength * 8;
const uint16_t kKelvinatorDefaultRepeat = kNoRepeat;
const uint16_t kLasertagBits = 13;
const uint16_t kLasertagMinRepeat = kNoRepeat;
const uint16_t kLegoPfBits = 16;
const uint16_t kLegoPfMinRepeat = kNoRepeat;
const uint16_t kLgBits = 28;
const uint16_t kLg32Bits = 32;
const uint16_t kLgDefaultRepeat = kNoRepeat;
const uint16_t kLutronBits = 35;
const uint16_t kMagiquestBits = 56;
const uint16_t kMetzBits = 19;
const uint16_t kMetzMinRepeat = kNoRepeat;
const uint16_t kMideaBits = 48;
const uint16_t kMideaMinRepeat = kNoRepeat;
const uint16_t kMidea24Bits = 24;
const uint16_t kMidea24MinRepeat = kSingleRepeat;
const uint16_t kMirageStateLength = 15;
const uint16_t kMirageBits = kMirageStateLength * 8;
const uint16_t kMirageMinRepeat = kNoRepeat;
const uint16_t kMitsubishiBits = 16;
// TODO(anyone): Verify that the Mitsubishi repeat is really needed.
//               Based on marcosamarinho's code.
const uint16_t kMitsubishiMinRepeat = kSingleRepeat;
const uint16_t kMitsubishiACStateLength = 18;
const uint16_t kMitsubishiACBits = kMitsubishiACStateLength * 8;
const uint16_t kMitsubishiACMinRepeat = kSingleRepeat;
const uint16_t kMitsubishi136StateLength = 17;
const uint16_t kMitsubishi136Bits = kMitsubishi136StateLength * 8;
const uint16_t kMitsubishi136MinRepeat = kNoRepeat;
const uint16_t kMitsubishi112StateLength = 14;
const uint16_t kMitsubishi112Bits = kMitsubishi112StateLength * 8;
const uint16_t kMitsubishi112MinRepeat = kNoRepeat;
const uint16_t kMitsubishiHeavy88StateLength = 11;
const uint16_t kMitsubishiHeavy88Bits = kMitsubishiHeavy88StateLength * 8;
const uint16_t kMitsubishiHeavy88MinRepeat = kNoRepeat;
const uint16_t kMitsubishiHeavy152StateLength = 19;
const uint16_t kMitsubishiHeavy152Bits = kMitsubishiHeavy152StateLength * 8;
const uint16_t kMitsubishiHeavy152MinRepeat = kNoRepeat;
const uint16_t kMultibracketsBits = 8;
const uint16_t kMultibracketsDefaultRepeat = kSingleRepeat;
const uint16_t kNikaiBits = 24;
const uint16_t kNECBits = 32;
const uint16_t kNeoclimaStateLength = 12;
const uint16_t kNeoclimaBits = kNeoclimaStateLength * 8;
const uint16_t kNeoclimaMinRepeat = kNoRepeat;
const uint16_t kPanasonicBits = 48;
const uint32_t kPanasonicManufacturer = 0x4004;
const uint32_t kPanasonic40Manufacturer = 0x34;
const uint16_t kPanasonic40Bits = 40;
const uint16_t kPanasonicAcStateLength = 27;
const uint16_t kPanasonicAcStateShortLength = 16;
const uint16_t kPanasonicAcBits = kPanasonicAcStateLength * 8;
const uint16_t kPanasonicAcShortBits = kPanasonicAcStateShortLength * 8;
const uint16_t kPanasonicAcDefaultRepeat = kNoRepeat;
const uint16_t kPanasonicAc32Bits = 32;
const uint16_t kPioneerBits = 64;
const uint16_t kProntoMinLength = 6;
const uint16_t kRC5RawBits = 14;
const uint16_t kRC5Bits = kRC5RawBits - 2;
const uint16_t kRC5XBits = kRC5RawBits - 1;
const uint16_t kRC6Mode0Bits = 20;  // Excludes the 'start' bit.
const uint16_t kRC6_36Bits = 36;  // Excludes the 'start' bit.
const uint16_t kRCMMBits = 24;
const uint16_t kSamsungBits = 32;
const uint16_t kSamsung36Bits = 36;
const uint16_t kSamsungAcStateLength = 14;
const uint16_t kSamsungAcBits = kSamsungAcStateLength * 8;
const uint16_t kSamsungAcExtendedStateLength = 21;
const uint16_t kSamsungAcExtendedBits = kSamsungAcExtendedStateLength * 8;
const uint16_t kSamsungAcDefaultRepeat = kNoRepeat;
const uint16_t kSanyoAcStateLength = 9;
const uint16_t kSanyoAcBits = kSanyoAcStateLength * 8;
const uint16_t kSanyoAc88StateLength = 11;
const uint16_t kSanyoAc88Bits = kSanyoAc88StateLength * 8;
const uint16_t kSanyoAc88MinRepeat = 2;
const uint16_t kSanyoAc152StateLength = 19;
const uint16_t kSanyoAc152Bits = kSanyoAc152StateLength * 8;
const uint16_t kSanyoAc152MinRepeat = kNoRepeat;
const uint16_t kSanyoSA8650BBits = 12;
const uint16_t kSanyoLC7461AddressBits = 13;
const uint16_t kSanyoLC7461CommandBits = 8;
const uint16_t kSanyoLC7461Bits = (kSanyoLC7461AddressBits +
                                   kSanyoLC7461CommandBits) * 2;
const uint8_t  kSharpAddressBits = 5;
const uint8_t  kSharpCommandBits = 8;
const uint16_t kSharpBits = kSharpAddressBits + kSharpCommandBits + 2;  // 15
const uint16_t kSharpAcStateLength = 13;
const uint16_t kSharpAcBits = kSharpAcStateLength * 8;  // 104
const uint16_t kSharpAcDefaultRepeat = kNoRepeat;
const uint8_t  kSherwoodBits = kNECBits;
const uint16_t kSherwoodMinRepeat = kSingleRepeat;
const uint16_t kSony12Bits = 12;
const uint16_t kSony15Bits = 15;
const uint16_t kSony20Bits = 20;
const uint16_t kSonyMinBits = 12;
const uint16_t kSonyMinRepeat = 2;
const uint16_t kSymphonyBits = 12;
const uint16_t kSymphonyDefaultRepeat = 3;
const uint16_t kTcl96AcStateLength = 12;
const uint16_t kTcl96AcBits = kTcl96AcStateLength * 8;
const uint16_t kTcl96AcDefaultRepeat = kNoRepeat;
const uint16_t kTcl112AcStateLength = 14;
const uint16_t kTcl112AcBits = kTcl112AcStateLength * 8;
const uint16_t kTcl112AcDefaultRepeat = kNoRepeat;
const uint16_t kTecoBits = 35;
const uint16_t kTecoDefaultRepeat = kNoRepeat;
const uint16_t kTeknopointStateLength = 14;
const uint16_t kTeknopointBits = kTeknopointStateLength * 8;
const uint16_t kToshibaACStateLength = 9;
const uint16_t kToshibaACBits = kToshibaACStateLength * 8;
const uint16_t kToshibaACMinRepeat = kSingleRepeat;
const uint16_t kToshibaACStateLengthShort = kToshibaACStateLength - 2;
const uint16_t kToshibaACBitsShort = kToshibaACStateLengthShort * 8;
const uint16_t kToshibaACStateLengthLong = kToshibaACStateLength + 1;
const uint16_t kToshibaACBitsLong = kToshibaACStateLengthLong * 8;
const uint16_t kTotoBits = 24;
const uint16_t kTotoShortBits = kTotoBits;
const uint16_t kTotoLongBits = kTotoShortBits * 2;
const uint16_t kTotoDefaultRepeat = kSingleRepeat;
const uint16_t kTranscoldBits = 24;
const uint16_t kTranscoldDefaultRepeat = kNoRepeat;
const uint16_t kTrotecStateLength = 9;
const uint16_t kTrotecBits = kTrotecStateLength * 8;
const uint16_t kTrotecDefaultRepeat = kNoRepeat;
const uint16_t kTrumaBits = 56;
const uint16_t kWhirlpoolAcStateLength = 21;
const uint16_t kWhirlpoolAcBits = kWhirlpoolAcStateLength * 8;
const uint16_t kWhirlpoolAcDefaultRepeat = kNoRepeat;
const uint16_t kWhynterBits = 32;
const uint16_t kWowweeBits = 11;
const uint16_t kWowweeDefaultRepeat = kNoRepeat;
const uint8_t  kVestelAcBits = 56;
const uint16_t kXmpBits = 64;
const uint16_t kZepealBits = 16;
const uint16_t kZepealMinRepeat = 4;
const uint16_t kVoltasBits = 80;
const uint16_t kVoltasStateLength = 10;
const uint16_t kMilesTag2ShotBits = 14;
const uint16_t kMilesTag2MsgBits = 24;
const uint16_t kMilesMinRepeat = 0;
const uint16_t kBoseBits = 16;
const uint16_t kRhossStateLength = 12;
const uint16_t kRhossBits = kRhossStateLength * 8;
const uint16_t kRhossDefaultRepeat = 0;
const uint16_t kClimaButlerBits = 52;
const uint16_t kYorkBits = 136;
const uint16_t kYorkStateLength = 17;

std::string concatenateIRsendParams(std::string data, uint16_t nbits, uint16_t repeat);
