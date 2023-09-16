#pragma once

#include "system/memory.hpp" // sptr, uptr
#include "threads/ConditionVariable.hpp" // Mutex, LockGuard, UniqueLock
#include <queue>

// informations: LockGuard unlocks only when destroyed. ConditionVariable does not take LockGuard.

// inspired from https://www.justsoftwaresolutions.co.uk/threading/implementing-a-thread-safe-queue-using-condition-variables.html
template <typename T>
class [[nodiscard]] LockFullThreadSafeQueue final {
private:
  mutable Mutex m_mutex {}; // mutable so that isEmpty() can be const
  std::queue<T> m_queue {};
  ConditionVariable m_condition {};

  void pop(T& t) noexcept {
    t = std::move(m_queue.front());
    m_queue.pop();
  }

public:
  LockFullThreadSafeQueue() noexcept = default;
  LockFullThreadSafeQueue(const LockFullThreadSafeQueue&) = delete;
  LockFullThreadSafeQueue(LockFullThreadSafeQueue&&) = delete;
  LockFullThreadSafeQueue& operator=(const LockFullThreadSafeQueue&) = delete;
  LockFullThreadSafeQueue& operator=(LockFullThreadSafeQueue&&) = delete;
  ~LockFullThreadSafeQueue() = default;

  void push(const T& t) {
    const LockGuard lock(m_mutex);
    m_queue.push(t);
    m_condition.notify_one();
  }

  void push(T&& t) {
    const LockGuard lock(m_mutex);
    m_queue.push(std::move(t));
    m_condition.notify_one();
  }

  [[nodiscard]] bool tryPop(T& t) {
    const LockGuard lock(m_mutex);

    if (m_queue.empty()) { return false; }

    pop(t);
    return true;
  }

  void waitPop(T& t) {
    UniqueLock lock(m_mutex);
    m_condition.wait(lock, [this] { return !m_queue.empty(); });
    pop(t);
  }

  [[nodiscard]] bool isEmpty() const {
    const LockGuard lock(m_mutex);
    return m_queue.empty();
  }

  [[nodiscard]] int size() const {
    const LockGuard lock(m_mutex);
    return m_queue.ssize();
  }
}; // class LockFullThreadSafeQueue

// comes from http://mainstream.inf.elte.hu/csordasmarton/CodeCompass_OS/commit/128ec25afb4c269be03ae671856ab6875aab9727#a67761fa4ec1dd20340ba3a1a1377b1c8584b981_0_44
template <typename T>
class [[nodiscard]] LockFreeThreadSafeQueue final {
private:
  struct [[nodiscard]] Node final {
    sptr<T> data {};
    uptr<Node> next {};
  };

  Mutex m_headMutex {};
  uptr<Node> m_head;
  Mutex m_tailMutex {};
  Node* m_tail;
  ConditionVariable m_condition {};

  [[nodiscard]] Node* getTail() {
    const LockGuard m_tailLock(m_tailMutex);
    return m_tail;
  }

  [[nodiscard]] uptr<Node> popHead() noexcept {
    uptr<Node> oldHead { std::move(m_head) };
    m_head = std::move(oldHead->next);
    return oldHead;
  }

  [[nodiscard]] UniqueLock waitForData() {
    UniqueLock headLock(m_headMutex);

    while (m_head.get() == getTail()) { m_condition.wait(headLock); }

    return headLock;
  }

  [[nodiscard]] uptr<Node> waitPopHead() {
    UniqueLock headLock(waitForData());
    return popHead();
  }

  void waitPopHead(T& value) {
    UniqueLock head_lock(waitForData());
    value = std::move(*m_head->data);
    popHead().reset();
  }

  [[nodiscard]] uptr<Node> tryPopHead() {
    const LockGuard headLock(m_headMutex);
    return (m_head.get() == getTail()) ? uptr<Node>() : popHead();
  }

  [[nodiscard]] uptr<Node> tryPopHead(T& value) {
    const LockGuard headLock(m_headMutex);

    if (m_head.get() == getTail()) { return uptr<Node>(); }

    value = std::move(*m_head->data);
    return popHead();
  }

public:
  LockFreeThreadSafeQueue() : m_head { mkUptr<Node>() }, m_tail { m_head.get() } {}
  LockFreeThreadSafeQueue(const LockFreeThreadSafeQueue&) = delete;
  LockFreeThreadSafeQueue(LockFreeThreadSafeQueue&&) = delete;
  LockFreeThreadSafeQueue& operator=(const LockFreeThreadSafeQueue&) = delete;
  LockFreeThreadSafeQueue& operator=(LockFreeThreadSafeQueue&&) = delete;
  ~LockFreeThreadSafeQueue() = default;

  [[nodiscard]] sptr<T> tryPop() {
    uptr<Node> oldHead { tryPopHead() };
    return oldHead ? oldHead->data : sptr<T>();
  }

  [[nodiscard]] bool tryPop(T& value) { return nullptr != tryPopHead(value); }

  [[nodiscard]] sptr<T> waitPop() { return waitPopHead()->data; }

  void waitPop(T& value) { waitPopHead(value); }

  void push(T newValue) {
    auto newData { mkSptr<T>(std::move(newValue)) };
    auto pNode { mkUptr<Node>() };
    {
      const LockGuard m_tailLock(m_tailMutex);
      m_tail->data = newData;
      Node* const newTail { pNode.get() };
      m_tail->next = std::move(pNode);
      m_tail = newTail;
    }
    m_condition.notify_one();
  }

  [[nodiscard]] bool isEmpty() {
    const LockGuard headLock(m_headMutex);
    return (m_head.get() == getTail());
  }
}; // class LockFreeThreadSafeQueue

template <typename T>
using ThreadSafeQueue = LockFreeThreadSafeQueue<T>;