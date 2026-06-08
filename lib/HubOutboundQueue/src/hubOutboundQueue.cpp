#include "hubOutboundQueue.h"

void HubOutboundQueue::clearEvents() {
  head = 0;
  tail = 0;
  queued = 0;
}

void HubOutboundQueue::startWakeWindow() {
  linkPhase = LinkPhase::WAKE_WINDOW;
}

void HubOutboundQueue::finishWakeWindow() {
  linkPhase = LinkPhase::STEADY;
}

HubOutboundQueue::EnqueueResult HubOutboundQueue::enqueue(
  const omote_RemoteEvent& event,
  unsigned long nowMs,
  unsigned long ttlMs
) {
  if (isFull() && isWakeWindow()) {
    return EnqueueResult::DROPPED_NEWEST;
  }

  if (isFull()) {
    pop();
    push(event, nowMs, ttlMs);
    return EnqueueResult::DROPPED_OLDEST_THEN_QUEUED;
  }

  push(event, nowMs, ttlMs);
  return EnqueueResult::QUEUED;
}

bool HubOutboundQueue::hasPendingEvents() const {
  return queued > 0;
}

bool HubOutboundQueue::isWakeWindow() const {
  return linkPhase == LinkPhase::WAKE_WINDOW;
}

size_t HubOutboundQueue::count() const {
  return queued;
}

const HubOutboundQueue::QueuedEvent* HubOutboundQueue::peek() const {
  if (!hasPendingEvents()) {
    return nullptr;
  }

  return &events[head];
}

void HubOutboundQueue::pop() {
  if (!hasPendingEvents()) {
    return;
  }

  head = (head + 1) % CAPACITY;
  queued--;
}

bool HubOutboundQueue::isExpired(const QueuedEvent& event, unsigned long nowMs) const {
  return (nowMs - event.enqueuedTime) >= event.ttlMs;
}

bool HubOutboundQueue::isFull() const {
  return queued == CAPACITY;
}

void HubOutboundQueue::push(
  const omote_RemoteEvent& event,
  unsigned long nowMs,
  unsigned long ttlMs
) {
  events[tail].event = event;
  events[tail].enqueuedTime = nowMs;
  events[tail].ttlMs = ttlMs;
  tail = (tail + 1) % CAPACITY;
  queued++;
}
