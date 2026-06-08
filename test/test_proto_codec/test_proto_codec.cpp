#include <unity.h>
#include <cstring>

#include "applicationInternal/hub/protoCodec.h"

using Hub::ProtoCodec;

void setUp() {}
void tearDown() {}

static void assertMaps(const char* cmd, omote_OmoteCommand expected) {
  TEST_ASSERT_EQUAL(expected, ProtoCodec::stringToCommand(cmd));
}

void test_synonyms_map_to_existing_enums() {
  assertMaps("SOURCE", omote_OmoteCommand_SRC);
  assertMaps("MUTE_TOGGLE", omote_OmoteCommand_VOL_MUTE);
  assertMaps("RETURN", omote_OmoteCommand_BACK);
  assertMaps("EXIT", omote_OmoteCommand_BACK);
  assertMaps("KEY_A", omote_OmoteCommand_RED);
  assertMaps("KEY_B", omote_OmoteCommand_GREEN);
  assertMaps("KEY_C", omote_OmoteCommand_YELLOW);
  assertMaps("KEY_D", omote_OmoteCommand_BLUE);
}

void test_existing_mappings_still_hold() {
  assertMaps("SRC", omote_OmoteCommand_SRC);
  assertMaps("BACK", omote_OmoteCommand_BACK);
  assertMaps("VOL_MUTE", omote_OmoteCommand_VOL_MUTE);
  assertMaps("POWER_ON", omote_OmoteCommand_POWER_ON);
}

void test_unknown_command_is_unspecified() {
  assertMaps("NOPE", omote_OmoteCommand_OMOTE_COMMAND_UNSPECIFIED);
  assertMaps("", omote_OmoteCommand_OMOTE_COMMAND_UNSPECIFIED);
}

void test_valid_command_result_round_trips() {
  omote_CommandResult source = omote_CommandResult_init_zero;
  source.kind = omote_ResponseKind_VOLUME;
  source.which_data = omote_CommandResult_volume_tag;
  source.data.volume.level = -30.5f;
  source.data.volume.is_muted = true;

  uint8_t buffer[omote_CommandResult_size];
  pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
  TEST_ASSERT_TRUE(pb_encode(&stream, omote_CommandResult_fields, &source));

  omote_CommandResult decoded = ProtoCodec::decodeCommandResult(buffer, stream.bytes_written);

  TEST_ASSERT_EQUAL(omote_ResponseKind_VOLUME, decoded.kind);
  TEST_ASSERT_EQUAL(omote_CommandResult_volume_tag, decoded.which_data);
  TEST_ASSERT_EQUAL_FLOAT(-30.5f, decoded.data.volume.level);
  TEST_ASSERT_TRUE(decoded.data.volume.is_muted);
}

void test_pairing_command_result_round_trips_enum_step() {
  omote_CommandResult source = omote_CommandResult_init_zero;
  source.kind = omote_ResponseKind_PAIRING;
  source.which_data = omote_CommandResult_pairing_tag;
  strncpy(source.data.pairing.device_id, "AppleTV", sizeof(source.data.pairing.device_id) - 1);
  source.data.pairing.step = omote_PairingStep_PAIRING_STEP_AWAITING_PIN;
  strncpy(source.data.pairing.message, "Enter PIN", sizeof(source.data.pairing.message) - 1);
  source.data.pairing.requires_pin = true;
  source.data.pairing.expected_pin_length = 4;
  strncpy(source.data.pairing.context_token, "ctx", sizeof(source.data.pairing.context_token) - 1);

  uint8_t buffer[omote_CommandResult_size];
  pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
  TEST_ASSERT_TRUE(pb_encode(&stream, omote_CommandResult_fields, &source));

  omote_CommandResult decoded = ProtoCodec::decodeCommandResult(buffer, stream.bytes_written);

  TEST_ASSERT_EQUAL(omote_ResponseKind_PAIRING, decoded.kind);
  TEST_ASSERT_EQUAL(omote_CommandResult_pairing_tag, decoded.which_data);
  TEST_ASSERT_EQUAL_STRING("AppleTV", decoded.data.pairing.device_id);
  TEST_ASSERT_EQUAL(omote_PairingStep_PAIRING_STEP_AWAITING_PIN, decoded.data.pairing.step);
  TEST_ASSERT_EQUAL_STRING("Enter PIN", decoded.data.pairing.message);
  TEST_ASSERT_TRUE(decoded.data.pairing.requires_pin);
  TEST_ASSERT_EQUAL_UINT32(4, decoded.data.pairing.expected_pin_length);
  TEST_ASSERT_EQUAL_STRING("ctx", decoded.data.pairing.context_token);
}

void test_garbage_bytes_decode_to_error() {
  const uint8_t garbage[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  omote_CommandResult decoded = ProtoCodec::decodeCommandResult(garbage, sizeof(garbage));

  TEST_ASSERT_EQUAL(omote_ResponseKind_ERROR, decoded.kind);
  TEST_ASSERT_EQUAL(omote_CommandResult_error_tag, decoded.which_data);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_synonyms_map_to_existing_enums);
  RUN_TEST(test_existing_mappings_still_hold);
  RUN_TEST(test_unknown_command_is_unspecified);
  RUN_TEST(test_valid_command_result_round_trips);
  RUN_TEST(test_pairing_command_result_round_trips_enum_step);
  RUN_TEST(test_garbage_bytes_decode_to_error);
  return UNITY_END();
}
