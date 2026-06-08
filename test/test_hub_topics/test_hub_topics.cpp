#include <unity.h>

#include "shared/hubTopics.h"

void setUp() {}
void tearDown() {}

void test_response_topic_is_recognized() {
  TEST_ASSERT_TRUE(Hub::isHubResponseTopic("remote_responses"));
  TEST_ASSERT_TRUE(Hub::isHubResponseTopic(Hub::RESPONSE_TOPIC));
}

void test_other_topics_are_rejected() {
  TEST_ASSERT_FALSE(Hub::isHubResponseTopic("OMOTE/test"));
  TEST_ASSERT_FALSE(Hub::isHubResponseTopic("remote_commands"));
  TEST_ASSERT_FALSE(Hub::isHubResponseTopic(""));
  TEST_ASSERT_FALSE(Hub::isHubResponseTopic("remote_responses_extra"));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_response_topic_is_recognized);
  RUN_TEST(test_other_topics_are_rejected);
  return UNITY_END();
}
