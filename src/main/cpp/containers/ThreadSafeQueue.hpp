#pragma once

#include <condition_variable>
#include <memory> // std::unique_ptr
#include <mutex>
#include <queue>

// informations: std::lock_guard<std::mutex> unlocks only when destroyed. std::condition_variable does not take std::lock_guard<std::mutex>.

// inspired from https://www.justsoftwaresolutions.co.uk/threading/implementing-a-thread-safe-queue-using-condition-variables.html
template <typename T>
class [[nodiscard]] LockFullThreadSafeQueue final {
private:
  mutable std::mutex m_mutex {}; // mutable so that isEmpty() can be const
  std::queue<T> m_queue {};
  std::condition_variable m_condition {};

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
    const std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push(t);
    m_condition.notify_one();
  }

  void push(T&& t) {
    const std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push(std::move(t));
    m_condition.notify_one();
  }

  [[nodiscard]] bool tryPop(T& t) {
    const std::lock_guard<std::mutex> lock(m_mutex);

    if (m_queue.empty()) { return false; }

    pop(t);
    return true;
  }

  void waitPop(T& t) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_condition.wait(lock, [this] { return !m_queue.empty(); });
    pop(t);
  }

  [[nodiscard]] bool isEmpty() const {
    const std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.empty();
  }

  [[nodiscard]] int size() const {
    const std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.ssize();
  }
}; // class LockFullThreadSafeQueue

// comes from http://mainstream.inf.elte.hu/csordasmarton/CodeCompass_OS/commit/128ec25afb4c269be03ae671856ab6875aab9727#a67761fa4ec1dd20340ba3a1a1377b1c8584b981_0_44
template <typename T>
class [[nodiscard]] LockFreeThreadSafeQueue final {
private:
  struct [[nodiscard]] Node final {
    std::shared_ptr<T> data {};
    std::unique_ptr<Node> next {};
  };

  std::mutex m_headMutex {};
  std::unique_ptr<Node> m_head;
  std::mutex m_tailMutex {};
  Node* m_tail;
  std::condition_variable m_condition {};

  [[nodiscard]] Node* getTail() {
    const std::lock_guard<std::mutex> m_tailLock(m_tailMutex);
    return m_tail;
  }

  [[nodiscard]] std::unique_ptr<Node> popHead() noexcept {
    std::unique_ptr<Node> oldHead { std::move(m_head) };
    m_head = std::move(oldHead->next);
    return oldHead;
  }

  [[nodiscard]] std::unique_lock<std::mutex> waitForData() {
    std::unique_lock<std::mutex> headLock(m_headMutex);

    while (m_head.get() == getTail()) { m_condition.wait(headLock); }

    return headLock;
  }

  [[nodiscard]] std::unique_ptr<Node> waitPopHead() {
    std::unique_lock<std::mutex> headLock(waitForData());
    return popHead();
  }

  void waitPopHead(T& value) {
    std::unique_lock<std::mutex> head_lock(waitForData());
    value = std::move(*m_head->data);
    popHead().reset();
  }

  [[nodiscard]] std::unique_ptr<Node> tryPopHead() {
    const std::lock_guard<std::mutex> headLock(m_headMutex);
    return (m_head.get() == getTail()) ? std::unique_ptr<Node>() : popHead();
  }

  [[nodiscard]] std::unique_ptr<Node> tryPopHead(T& value) {
    const std::lock_guard<std::mutex> headLock(m_headMutex);

    if (m_head.get() == getTail()) { return std::unique_ptr<Node>(); }

    value = std::move(*m_head->data);
    return popHead();
  }

public:
  LockFreeThreadSafeQueue() : m_head { std::make_unique<Node>() }, m_tail { m_head.get() } {}
  LockFreeThreadSafeQueue(const LockFreeThreadSafeQueue&) = delete;
  LockFreeThreadSafeQueue(LockFreeThreadSafeQueue&&) = delete;
  LockFreeThreadSafeQueue& operator=(const LockFreeThreadSafeQueue&) = delete;
  LockFreeThreadSafeQueue& operator=(LockFreeThreadSafeQueue&&) = delete;
  ~LockFreeThreadSafeQueue() = default;

  [[nodiscard]] std::shared_ptr<T> tryPop() {
    std::unique_ptr<Node> oldHead { tryPopHead() };
    return oldHead ? oldHead->data : std::shared_ptr<T>();
  }

  [[nodiscard]] bool tryPop(T& value) { return nullptr != tryPopHead(value); }

  [[nodiscard]] std::shared_ptr<T> waitPop() { return waitPopHead()->data; }

  void waitPop(T& value) { waitPopHead(value); }

  void push(T newValue) {
    auto newData { std::make_shared<T>(std::move(newValue)) };
    auto pNode { std::make_unique<Node>() };
    {
      const std::lock_guard<std::mutex> m_tailLock(m_tailMutex);
      m_tail->data = newData;
      Node* const newTail { pNode.get() };
      m_tail->next = std::move(pNode);
      m_tail = newTail;
    }
    m_condition.notify_one();
  }

  [[nodiscard]] bool isEmpty() {
    const std::lock_guard<std::mutex> headLock(m_headMutex);
    return (m_head.get() == getTail());
  }
}; // class LockFreeThreadSafeQueue

template <typename T>
using ThreadSafeQueue = LockFreeThreadSafeQueue<T>;