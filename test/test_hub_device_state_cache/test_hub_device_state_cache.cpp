#include <unity.h>
#include <cstring>
#include <hubDeviceStateCache.h>

void setUp() {}
void tearDown() {}

static omote_DeviceState makeDevice(const char* id) {
  omote_DeviceState state = omote_DeviceState_init_zero;
  strncpy(state.device_id, id, sizeof(state.device_id) - 1);
  return state;
}

static omote_DeviceState makeDeviceWithVolume(const char* id, float level, bool muted) {
  omote_DeviceState state = makeDevice(id);
  state.has_volume = true;
  state.volume.level = level;
  state.volume.is_muted = muted;
  return state;
}

void test_insert_stores_device() {
  HubDeviceStateCache cache;

  TEST_ASSERT_TRUE(cache.merge(makeDevice("APPLE_TV")));

  TEST_ASSERT_EQUAL_UINT(1, cache.size());
  const omote_DeviceState* stored = cache.find("APPLE_TV");
  TEST_ASSERT_NOT_NULL(stored);
  TEST_ASSERT_EQUAL_STRING("APPLE_TV", stored->device_id);
}

void test_update_replaces_record_and_clears_stale_optionals() {
  HubDeviceStateCache cache;

  cache.merge(makeDeviceWithVolume("APPLE_TV", 12.0f, false));
  const omote_DeviceState* first = cache.find("APPLE_TV");
  TEST_ASSERT_NOT_NULL(first);
  TEST_ASSERT_TRUE(first->has_volume);

  omote_DeviceState mediaOnly = makeDevice("APPLE_TV");
  mediaOnly.has_media_player = true;
  TEST_ASSERT_TRUE(cache.merge(mediaOnly));

  TEST_ASSERT_EQUAL_UINT(1, cache.size());
  const omote_DeviceState* updated = cache.find("APPLE_TV");
  TEST_ASSERT_NOT_NULL(updated);
  TEST_ASSERT_FALSE(updated->has_volume);        // stale volume cleared by full-snapshot replace
  TEST_ASSERT_TRUE(updated->has_media_player);
}

void test_preserves_other_devices() {
  HubDeviceStateCache cache;

  cache.merge(makeDeviceWithVolume("APPLE_TV", 12.0f, false));
  cache.merge(makeDeviceWithVolume("ANDROID_TV", 5.0f, true));

  cache.merge(makeDevice("APPLE_TV"));  // update APPLE_TV, clearing its volume

  TEST_ASSERT_EQUAL_UINT(2, cache.size());
  const omote_DeviceState* android = cache.find("ANDROID_TV");
  TEST_ASSERT_NOT_NULL(android);
  TEST_ASSERT_TRUE(android->has_volume);
  TEST_ASSERT_EQUAL_FLOAT(5.0f, android->volume.level);
}

void test_ignores_empty_device_id() {
  HubDeviceStateCache cache;

  TEST_ASSERT_FALSE(cache.merge(makeDevice("")));
  TEST_ASSERT_EQUAL_UINT(0, cache.size());
}

void test_eight_entry_bound_drops_unknown_when_full() {
  HubDeviceStateCache cache;

  char id[8];
  for (int i = 0; i < (int)HubDeviceStateCache::CAPACITY; i++) {
    snprintf(id, sizeof(id), "DEV_%d", i);
    TEST_ASSERT_TRUE(cache.merge(makeDevice(id)));
  }
  TEST_ASSERT_EQUAL_UINT(HubDeviceStateCache::CAPACITY, cache.size());

  TEST_ASSERT_FALSE(cache.merge(makeDevice("OVERFLOW")));
  TEST_ASSERT_EQUAL_UINT(HubDeviceStateCache::CAPACITY, cache.size());
  TEST_ASSERT_NULL(cache.find("OVERFLOW"));

  // updating an existing device still works when full
  TEST_ASSERT_TRUE(cache.merge(makeDeviceWithVolume("DEV_0", 3.0f, false)));
  const omote_DeviceState* dev0 = cache.find("DEV_0");
  TEST_ASSERT_NOT_NULL(dev0);
  TEST_ASSERT_TRUE(dev0->has_volume);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_insert_stores_device);
  RUN_TEST(test_update_replaces_record_and_clears_stale_optionals);
  RUN_TEST(test_preserves_other_devices);
  RUN_TEST(test_ignores_empty_device_id);
  RUN_TEST(test_eight_entry_bound_drops_unknown_when_full);
  return UNITY_END();
}
