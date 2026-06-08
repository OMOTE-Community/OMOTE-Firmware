#pragma once

#include "remote_messages.pb.h"
#include <cstddef>
#include <cstdint>

class HubOutboundQueue {
public:
  enum class LinkPhase { WAKE_WINDOW, STEADY };
  enum class EnqueueResult { QUEUED, DROPPED_NEWEST, DROPPED_OLDEST_THEN_QUEUED };

  struct QueuedEvent {
    omote_RemoteEvent event;
    unsigned long enqueuedTime;
    unsigned long ttlMs;
  };

  static const uint8_t CAPACITY = 12;

  void clearEvents();
  void startWakeWindow();
  void finishWakeWindow();

  EnqueueResult enqueue(const omote_RemoteEvent& event, unsigned long nowMs, unsigned long ttlMs);
  bool hasPendingEvents() const;
  bool isWakeWindow() const;
  size_t count() const;
  const QueuedEvent* peek() const;
  void pop();
  bool isExpired(const QueuedEvent& event, unsigned long nowMs) const;

private:
  QueuedEvent events[CAPACITY];
  uint8_t head = 0;
  uint8_t tail = 0;
  uint8_t queued = 0;
  LinkPhase linkPhase = LinkPhase::WAKE_WINDOW;

  bool isFull() const;
  void push(const omote_RemoteEvent& event, unsigned long nowMs, unsigned long ttlMs);
};
