#include <unity.h>
#include <memory>
#include <vector>
#include <cstring>
#include <pb_encode.h>

#include "applicationInternal/hardware/arduinoLayer.h"
#include "applicationInternal/hub/hubManager.h"
#include "applicationInternal/hub/syncRequest.h"
#include "applicationInternal/hub/sceneSyncTargets.h"

class FakeHubTransport : public HubTransportBase {
public:
  bool ready = true;
  unsigned long wakeTtlMs = 5000;
  uint8_t failedSendsRemaining = 0;
  uint8_t initCalls = 0;
  uint8_t sendAttempts = 0;
  size_t inboundBudget = 0;
  std::vector<omote_OmoteCommand> sentCommands;
  std::vector<omote_RemoteEvent> sentEvents;

  bool init() override {
    initCalls++;
    return true;
  }

  void process() override {
  }

  bool sendRemoteEvent(const omote_RemoteEvent& event) override {
    sendAttempts++;

    if (failedSendsRemaining > 0) {
      failedSendsRemaining--;
      return false;
    }

    sentCommands.push_back(event.command);
    sentEvents.push_back(event);
    return true;
  }

  bool isReady() override {
    return ready;
  }

  unsigned long wakeQueueTtlMs() const override {
    return wakeTtlMs;
  }

  size_t maxInboundCommandResultBytes() const override {
    return inboundBudget;
  }

  void shutdown() override {
  }
};

static omote_RemoteEvent makeEvent(omote_OmoteCommand command) {
  omote_RemoteEvent event = omote_RemoteEvent_init_zero;
  event.command = command;
  event.type = omote_OmoteCommandType_SHORT;
  return event;
}

static FakeHubTransport* initWithFakeTransport() {
  HubManager& manager = HubManager::getInstance();
  std::unique_ptr<FakeHubTransport> transport(new FakeHubTransport());
  FakeHubTransport* fake = transport.get();

  TEST_ASSERT_TRUE(manager.init(std::unique_ptr<HubTransportBase>(transport.release())));
  TEST_ASSERT_EQUAL_UINT8(1, fake->initCalls);
  return fake;
}

static void assertSent(FakeHubTransport* transport, const omote_OmoteCommand* expected, size_t count) {
  TEST_ASSERT_EQUAL_UINT(count, transport->sentCommands.size());

  for (size_t i = 0; i < count; i++) {
    TEST_ASSERT_EQUAL(expected[i], transport->sentCommands[i]);
  }
}

static void resetManagerState() {
  HubManager& manager = HubManager::getInstance();

  manager.setSyncTargetDevices({});
  manager.shutdown();

  if (!manager.isStateSyncRequested()) {
    return;
  }

  initWithFakeTransport();
  delay(110);
  manager.process();
  TEST_ASSERT_FALSE(manager.isStateSyncRequested());
  manager.shutdown();
}

void setUp() {
  resetManagerState();
}

void tearDown() {
  resetManagerState();
}

void test_failed_direct_send_is_requeued_and_flushed_next_tick() {
  HubManager& manager = HubManager::getInstance();
  FakeHubTransport* transport = initWithFakeTransport();
  transport->failedSendsRemaining = 1;

  TEST_ASSERT_TRUE(manager.sendRemoteEvent(makeEvent(omote_OmoteCommand_DOWN)));
  TEST_ASSERT_EQUAL_UINT8(1, transport->sendAttempts);
  TEST_ASSERT_TRUE(transport->sentCommands.empty());

  manager.process();

  const omote_OmoteCommand expected[] = {omote_OmoteCommand_DOWN};
  TEST_ASSERT_EQUAL_UINT8(2, transport->sendAttempts);
  assertSent(transport, expected, 1);
}

void test_ready_transport_queues_behind_existing_backlog() {
  HubManager& manager = HubManager::getInstance();
  FakeHubTransport* transport = initWithFakeTransport();

  transport->ready = false;
  TEST_ASSERT_TRUE(manager.sendRemoteEvent(makeEvent(omote_OmoteCommand_DOWN)));

  transport->ready = true;
  TEST_ASSERT_TRUE(manager.sendRemoteEvent(makeEvent(omote_OmoteCommand_UP)));
  TEST_ASSERT_EQUAL_UINT8(0, transport->sendAttempts);

  manager.process();

  const omote_OmoteCommand expected[] = {
    omote_OmoteCommand_DOWN,
    omote_OmoteCommand_UP
  };
  assertSent(transport, expected, 2);
}

void test_flush_stops_on_failed_send_and_retries_next_tick() {
  HubManager& manager = HubManager::getInstance();
  FakeHubTransport* transport = initWithFakeTransport();

  transport->ready = false;
  TEST_ASSERT_TRUE(manager.sendRemoteEvent(makeEvent(omote_OmoteCommand_DOWN)));
  TEST_ASSERT_TRUE(manager.sendRemoteEvent(makeEvent(omote_OmoteCommand_UP)));

  transport->ready = true;
  transport->failedSendsRemaining = 1;
  manager.process();

  TEST_ASSERT_EQUAL_UINT8(1, transport->sendAttempts);
  TEST_ASSERT_TRUE(transport->sentCommands.empty());

  manager.process();

  const omote_OmoteCommand expected[] = {
    omote_OmoteCommand_DOWN,
    omote_OmoteCommand_UP
  };
  TEST_ASSERT_EQUAL_UINT8(3, transport->sendAttempts);
  assertSent(transport, expected, 2);
}

void test_wake_queue_ttl_uses_transport_value() {
  HubManager& manager = HubManager::getInstance();
  FakeHubTransport* transport = initWithFakeTransport();

  transport->ready = false;
  transport->wakeTtlMs = 0;
  TEST_ASSERT_TRUE(manager.sendRemoteEvent(makeEvent(omote_OmoteCommand_DOWN)));

  transport->ready = true;
  manager.process();

  TEST_ASSERT_EQUAL_UINT8(0, transport->sendAttempts);
  TEST_ASSERT_TRUE(transport->sentCommands.empty());
}

void test_runtime_retry_uses_runtime_ttl_after_wake_window() {
  HubManager& manager = HubManager::getInstance();
  FakeHubTransport* transport = initWithFakeTransport();

  transport->wakeTtlMs = 0;
  manager.process();

  transport->failedSendsRemaining = 1;
  TEST_ASSERT_TRUE(manager.sendRemoteEvent(makeEvent(omote_OmoteCommand_DOWN)));
  manager.process();

  const omote_OmoteCommand expected[] = {omote_OmoteCommand_DOWN};
  TEST_ASSERT_EQUAL_UINT8(2, transport->sendAttempts);
  assertSent(transport, expected, 1);
}

void test_process_defers_state_sync_while_backlog_remains_pending() {
  HubManager& manager = HubManager::getInstance();
  FakeHubTransport* transport = initWithFakeTransport();

  transport->ready = false;
  TEST_ASSERT_TRUE(manager.sendRemoteEvent(makeEvent(omote_OmoteCommand_DOWN)));

  transport->ready = true;
  transport->failedSendsRemaining = 1;
  manager.requestStateSync();
  delay(110);

  manager.process();

  TEST_ASSERT_EQUAL_UINT8(1, transport->sendAttempts);
  TEST_ASSERT_TRUE(transport->sentCommands.empty());
  TEST_ASSERT_TRUE(manager.isStateSyncRequested());

  manager.process();

  const omote_OmoteCommand expected[] = {
    omote_OmoteCommand_DOWN,
    omote_OmoteCommand_SYNC_STATE
  };
  assertSent(transport, expected, 2);
  TEST_ASSERT_FALSE(manager.isStateSyncRequested());
}

void test_request_shape_full_when_budget_covers_full_command_result() {
  TEST_ASSERT_EQUAL_STRING("time,devices",
    Hub::pickSyncRequestData(omote_CommandResult_size, "ANDROID_TV").c_str());
  // Full sync does not need a specific active device.
  TEST_ASSERT_EQUAL_STRING("time,devices",
    Hub::pickSyncRequestData(omote_CommandResult_size, "").c_str());
}

void test_request_shape_single_device_for_mid_budget_with_active_device() {
  TEST_ASSERT_EQUAL_STRING("time,devices:ANDROID_TV",
    Hub::pickSyncRequestData(Hub::STATE_SYNC_SINGLE_DEVICE_BUDGET, "ANDROID_TV").c_str());
}

void test_request_shape_time_only_for_tiny_budget() {
  // ESP-NOW v1 (250 B) cannot carry even one rich device.
  TEST_ASSERT_EQUAL_STRING("time",
    Hub::pickSyncRequestData(250, "ANDROID_TV").c_str());
}

void test_request_shape_time_only_when_no_active_device() {
  TEST_ASSERT_EQUAL_STRING("time",
    Hub::pickSyncRequestData(Hub::STATE_SYNC_SINGLE_DEVICE_BUDGET, "").c_str());
}

static omote_CommandResult makeWorstCaseSingleDeviceStateSync() {
  omote_CommandResult result = omote_CommandResult_init_zero;
  result.kind = omote_ResponseKind_STATE_SYNC;
  result.which_data = omote_CommandResult_state_sync_tag;

  omote_StateSync& sync = result.data.state_sync;
  sync.has_time = true;
  sync.time.timestamp = 0xFFFFFFFF;
  sync.time.timezone_offset = -720;
  sync.devices_count = 1;

  omote_DeviceState& dev = sync.devices[0];
  memset(dev.device_id, 'D', sizeof(dev.device_id) - 1);
  dev.is_on = true;
  dev.has_volume = true;
  dev.volume.level = -30.5f;
  dev.volume.is_muted = true;
  dev.has_media_player = true;
  dev.media_player.has_playback = true;

  omote_Metadata& md = dev.media_player.playback;
  memset(md.title, 'T', sizeof(md.title) - 1);
  memset(md.artist, 'A', sizeof(md.artist) - 1);
  memset(md.album, 'L', sizeof(md.album) - 1);
  memset(md.state, 'S', sizeof(md.state) - 1);
  md.duration = 0xFFFFFFFF;
  md.position = 0xFFFFFFFF;
  return result;
}

void test_single_device_budget_covers_worst_case_encoded() {
  omote_CommandResult worst = makeWorstCaseSingleDeviceStateSync();

  uint8_t buffer[omote_CommandResult_size];
  pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
  TEST_ASSERT_TRUE(pb_encode(&stream, omote_CommandResult_fields, &worst));

  // Budget must cover the worst-case rich single-device response; fails loudly
  // if a proto/.options change grows it past the constant.
  TEST_ASSERT_GREATER_OR_EQUAL_UINT(stream.bytes_written, Hub::STATE_SYNC_SINGLE_DEVICE_BUDGET);
}

static const omote_RemoteEvent& driveStateSync(HubManager& manager, FakeHubTransport* transport) {
  manager.requestStateSync();
  delay(110);
  manager.process();
  TEST_ASSERT_FALSE(manager.isStateSyncRequested());
  TEST_ASSERT_FALSE(transport->sentEvents.empty());
  return transport->sentEvents.back();
}

static void assertSyncRequestData(const omote_RemoteEvent& event, const char* expected) {
  TEST_ASSERT_EQUAL(omote_OmoteCommand_SYNC_STATE, event.command);
  const size_t expectedLen = strlen(expected);
  // strlen, no trailing NUL counted in the payload.
  TEST_ASSERT_EQUAL_UINT(expectedLen, event.data.size);
  TEST_ASSERT_EQUAL_INT(0, memcmp(event.data.bytes, expected, expectedLen));
}

void test_sync_state_requests_full_when_budget_covers_full() {
  HubManager& manager = HubManager::getInstance();
  FakeHubTransport* transport = initWithFakeTransport();
  transport->inboundBudget = omote_CommandResult_size;

  assertSyncRequestData(driveStateSync(manager, transport), "time,devices");
}

void test_sync_state_requests_primary_device_for_mid_budget() {
  HubManager& manager = HubManager::getInstance();
  FakeHubTransport* transport = initWithFakeTransport();
  transport->inboundBudget = Hub::STATE_SYNC_SINGLE_DEVICE_BUDGET;
  manager.setSyncTargetDevices({"ANDROID_TV", "DENON_AVR"});  // primary = front

  assertSyncRequestData(driveStateSync(manager, transport), "time,devices:ANDROID_TV");
}

void test_sync_state_requests_time_only_for_tiny_budget() {
  HubManager& manager = HubManager::getInstance();
  FakeHubTransport* transport = initWithFakeTransport();
  transport->inboundBudget = 250;
  manager.setSyncTargetDevices({"ANDROID_TV"});

  assertSyncRequestData(driveStateSync(manager, transport), "time");
}

void test_sync_state_requests_time_only_without_target_devices() {
  HubManager& manager = HubManager::getInstance();
  FakeHubTransport* transport = initWithFakeTransport();
  transport->inboundBudget = Hub::STATE_SYNC_SINGLE_DEVICE_BUDGET;

  assertSyncRequestData(driveStateSync(manager, transport), "time");
}

void test_scene_sync_targets_map_to_priority_ordered_devices() {
  std::vector<std::string> shield = Hub::hubSyncTargetsForScene("Shield");
  TEST_ASSERT_EQUAL_UINT(3, shield.size());
  TEST_ASSERT_EQUAL_STRING("ANDROID_TV", shield[0].c_str());  // primary = media device
  TEST_ASSERT_EQUAL_STRING("DENON_AVR", shield[1].c_str());
  TEST_ASSERT_EQUAL_STRING("LG_TV", shield[2].c_str());

  std::vector<std::string> tv = Hub::hubSyncTargetsForScene("TV");
  TEST_ASSERT_EQUAL_UINT(1, tv.size());
  TEST_ASSERT_EQUAL_STRING("LG_TV", tv[0].c_str());
}

void test_scene_sync_targets_empty_for_non_hub_scenes() {
  TEST_ASSERT_TRUE(Hub::hubSyncTargetsForScene("Fire TV").empty());
  TEST_ASSERT_TRUE(Hub::hubSyncTargetsForScene("Off").empty());
  TEST_ASSERT_TRUE(Hub::hubSyncTargetsForScene("").empty());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_scene_sync_targets_map_to_priority_ordered_devices);
  RUN_TEST(test_scene_sync_targets_empty_for_non_hub_scenes);
  RUN_TEST(test_sync_state_requests_full_when_budget_covers_full);
  RUN_TEST(test_sync_state_requests_primary_device_for_mid_budget);
  RUN_TEST(test_sync_state_requests_time_only_for_tiny_budget);
  RUN_TEST(test_sync_state_requests_time_only_without_target_devices);
  RUN_TEST(test_request_shape_full_when_budget_covers_full_command_result);
  RUN_TEST(test_request_shape_single_device_for_mid_budget_with_active_device);
  RUN_TEST(test_request_shape_time_only_for_tiny_budget);
  RUN_TEST(test_request_shape_time_only_when_no_active_device);
  RUN_TEST(test_single_device_budget_covers_worst_case_encoded);
  RUN_TEST(test_failed_direct_send_is_requeued_and_flushed_next_tick);
  RUN_TEST(test_ready_transport_queues_behind_existing_backlog);
  RUN_TEST(test_flush_stops_on_failed_send_and_retries_next_tick);
  RUN_TEST(test_wake_queue_ttl_uses_transport_value);
  RUN_TEST(test_runtime_retry_uses_runtime_ttl_after_wake_window);
  RUN_TEST(test_process_defers_state_sync_while_backlog_remains_pending);
  return UNITY_END();
}
