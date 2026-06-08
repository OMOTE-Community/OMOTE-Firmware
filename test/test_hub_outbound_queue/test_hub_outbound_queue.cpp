#include <unity.h>
#include <hubOutboundQueue.h>

static omote_RemoteEvent makeEvent(omote_OmoteCommand command) {
  omote_RemoteEvent event = omote_RemoteEvent_init_zero;
  event.command = command;
  event.type = omote_OmoteCommandType_SHORT;
  return event;
}

static const omote_OmoteCommand ORDERED_COMMANDS[] = {
  omote_OmoteCommand_POWER_ON,
  omote_OmoteCommand_POWER_OFF,
  omote_OmoteCommand_DOWN,
  omote_OmoteCommand_UP,
  omote_OmoteCommand_RIGHT,
  omote_OmoteCommand_LEFT,
  omote_OmoteCommand_SELECT,
  omote_OmoteCommand_HOME,
  omote_OmoteCommand_MENU,
  omote_OmoteCommand_PLAY_PAUSE,
  omote_OmoteCommand_VOL_PLUS,
  omote_OmoteCommand_VOL_MINUS,
  omote_OmoteCommand_VOL_MUTE,
  omote_OmoteCommand_SKIP_BACKWARD,
  omote_OmoteCommand_SKIP_FORWARD,
  omote_OmoteCommand_RED,
  omote_OmoteCommand_GREEN
};

static omote_OmoteCommand commandAt(uint8_t index) {
  return ORDERED_COMMANDS[index];
}

static void fillWakeWindow(HubOutboundQueue& queue) {
  for (uint8_t i = 0; i < HubOutboundQueue::CAPACITY; i++) {
    TEST_ASSERT_EQUAL(
      HubOutboundQueue::EnqueueResult::QUEUED,
      queue.enqueue(makeEvent(commandAt(i)), i, 5000)
    );
  }
}

void setUp() {
}

void tearDown() {
}

void test_enqueue_preserves_fifo_order_and_ttl() {
  HubOutboundQueue queue;

  TEST_ASSERT_EQUAL(
    HubOutboundQueue::EnqueueResult::QUEUED,
    queue.enqueue(makeEvent(omote_OmoteCommand_DOWN), 10, 5000)
  );
  TEST_ASSERT_EQUAL(
    HubOutboundQueue::EnqueueResult::QUEUED,
    queue.enqueue(makeEvent(omote_OmoteCommand_UP), 20, 1500)
  );

  const HubOutboundQueue::QueuedEvent* first = queue.peek();
  TEST_ASSERT_NOT_NULL(first);
  TEST_ASSERT_EQUAL(omote_OmoteCommand_DOWN, first->event.command);
  TEST_ASSERT_EQUAL_UINT32(10, first->enqueuedTime);
  TEST_ASSERT_EQUAL_UINT32(5000, first->ttlMs);

  queue.pop();

  const HubOutboundQueue::QueuedEvent* second = queue.peek();
  TEST_ASSERT_NOT_NULL(second);
  TEST_ASSERT_EQUAL(omote_OmoteCommand_UP, second->event.command);
  TEST_ASSERT_EQUAL_UINT32(20, second->enqueuedTime);
  TEST_ASSERT_EQUAL_UINT32(1500, second->ttlMs);
}

void test_wake_window_drops_newest_when_full() {
  HubOutboundQueue queue;
  fillWakeWindow(queue);

  TEST_ASSERT_EQUAL(
    HubOutboundQueue::EnqueueResult::DROPPED_NEWEST,
    queue.enqueue(makeEvent(omote_OmoteCommand_RED), 100, 5000)
  );

  TEST_ASSERT_EQUAL_UINT(HubOutboundQueue::CAPACITY, queue.count());
  TEST_ASSERT_EQUAL(omote_OmoteCommand_POWER_ON, queue.peek()->event.command);
}

void test_steady_state_drops_oldest_when_full() {
  HubOutboundQueue queue;
  fillWakeWindow(queue);
  queue.finishWakeWindow();

  TEST_ASSERT_EQUAL(
    HubOutboundQueue::EnqueueResult::DROPPED_OLDEST_THEN_QUEUED,
    queue.enqueue(makeEvent(omote_OmoteCommand_RED), 100, 1500)
  );

  TEST_ASSERT_EQUAL_UINT(HubOutboundQueue::CAPACITY, queue.count());
  TEST_ASSERT_EQUAL(commandAt(1), queue.peek()->event.command);

  for (uint8_t i = 1; i < HubOutboundQueue::CAPACITY; i++) {
    queue.pop();
  }

  TEST_ASSERT_EQUAL(omote_OmoteCommand_RED, queue.peek()->event.command);
}

void test_expiration_uses_stored_ttl() {
  HubOutboundQueue queue;

  queue.enqueue(makeEvent(omote_OmoteCommand_SELECT), 100, 50);

  const HubOutboundQueue::QueuedEvent* event = queue.peek();
  TEST_ASSERT_NOT_NULL(event);
  TEST_ASSERT_FALSE(queue.isExpired(*event, 149));
  TEST_ASSERT_TRUE(queue.isExpired(*event, 150));
}

void test_phase_can_restart_after_clearing_events() {
  HubOutboundQueue queue;

  queue.finishWakeWindow();
  queue.clearEvents();
  queue.startWakeWindow();

  TEST_ASSERT_TRUE(queue.isWakeWindow());
  TEST_ASSERT_FALSE(queue.hasPendingEvents());
  TEST_ASSERT_NULL(queue.peek());
}

void test_pop_and_reenqueue_wraps_without_losing_order() {
  HubOutboundQueue queue;
  fillWakeWindow(queue);

  for (uint8_t i = 0; i < 5; i++) {
    queue.pop();
  }

  queue.finishWakeWindow();

  for (uint8_t i = 0; i < 5; i++) {
    TEST_ASSERT_EQUAL(
      HubOutboundQueue::EnqueueResult::QUEUED,
      queue.enqueue(makeEvent(commandAt(HubOutboundQueue::CAPACITY + i)), 100 + i, 1500)
    );
  }

  TEST_ASSERT_EQUAL_UINT(HubOutboundQueue::CAPACITY, queue.count());

  for (uint8_t i = 5; i < HubOutboundQueue::CAPACITY + 5; i++) {
    TEST_ASSERT_EQUAL(commandAt(i), queue.peek()->event.command);
    queue.pop();
  }

  TEST_ASSERT_FALSE(queue.hasPendingEvents());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_enqueue_preserves_fifo_order_and_ttl);
  RUN_TEST(test_wake_window_drops_newest_when_full);
  RUN_TEST(test_steady_state_drops_oldest_when_full);
  RUN_TEST(test_expiration_uses_stored_ttl);
  RUN_TEST(test_phase_can_restart_after_clearing_events);
  RUN_TEST(test_pop_and_reenqueue_wraps_without_losing_order);
  return UNITY_END();
}
