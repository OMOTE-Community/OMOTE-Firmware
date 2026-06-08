#include <unity.h>
#include <memory>
#include <vector>

#include "applicationInternal/hardware/arduinoLayer.h"
#include "applicationInternal/hub/hubManager.h"

class FakeHubTransport : public HubTransportBase {
public:
  bool ready = true;
  unsigned long wakeTtlMs = 5000;
  uint8_t failedSendsRemaining = 0;
  uint8_t initCalls = 0;
  uint8_t sendAttempts = 0;
  std::vector<omote_OmoteCommand> sentCommands;

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
    return true;
  }

  bool isReady() override {
    return ready;
  }

  unsigned long wakeQueueTtlMs() const override {
    return wakeTtlMs;
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

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_failed_direct_send_is_requeued_and_flushed_next_tick);
  RUN_TEST(test_ready_transport_queues_behind_existing_backlog);
  RUN_TEST(test_flush_stops_on_failed_send_and_retries_next_tick);
  RUN_TEST(test_wake_queue_ttl_uses_transport_value);
  RUN_TEST(test_runtime_retry_uses_runtime_ttl_after_wake_window);
  RUN_TEST(test_process_defers_state_sync_while_backlog_remains_pending);
  return UNITY_END();
}
