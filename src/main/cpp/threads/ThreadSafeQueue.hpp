#pragma once

#include <condition_variable>
#include <memory> // std::unique_ptr
#include <mutex>
#include <optional>
#include <queue>

// std::scoped_lock unlocks when destroyed and can lock multiple mutexes atomically.
// std::condition_variable requires std::unique_lock, not std::scoped_lock.
template <typename T>
class [[nodiscard]] ThreadSafeQueue final {
private:
  struct [[nodiscard]] Node final {
    std::shared_ptr<T> data {};
    std::unique_ptr<Node> next {};
  };

  std::mutex m_pHeadMutex {};
  std::unique_ptr<Node> m_pHead;
  std::mutex m_pTailMutex {};
  Node* m_pTail;
  std::condition_variable m_condition {};

  [[nodiscard]] std::unique_ptr<Node> popHead() noexcept {
    std::unique_ptr<Node> pOldHead {std::move(m_pHead)};
    m_pHead = std::move(pOldHead->next);
    return pOldHead;
  }

  /**
   * The caller must own m_pHeadMutex. m_pTailMutex is released when this function returns.
   */
  [[nodiscard]] bool hasValueToPop() {
    const std::lock_guard<std::mutex> tailLock(m_pTailMutex);
    return m_pHead.get() != m_pTail;
  }

public:
  ThreadSafeQueue()
    : m_pHead {std::make_unique<Node>()},
      m_pTail {m_pHead.get()} {}
  ThreadSafeQueue(const ThreadSafeQueue&) = delete;
  ThreadSafeQueue(ThreadSafeQueue&&) = delete;
  ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;
  ThreadSafeQueue& operator=(ThreadSafeQueue&&) = delete;
  ~ThreadSafeQueue() = default;

  /**
   * Pop a value, unless the queue is empty.
   * @returns the popped value, or std::nullopt if the queue is empty
   */
  [[nodiscard]] std::optional<T> tryPop() {
    const std::scoped_lock lock(m_pHeadMutex, m_pTailMutex);
    if (m_pHead.get() == m_pTail) {
      // the queue is empty
      return std::nullopt;
    }
    std::optional<T> ret {std::move(*m_pHead->data)};
    popHead().reset();
    return ret;
  }

  /**
   * Wait forever until there is a value to pop, then pop it.
   */
  [[nodiscard]] T waitPop() {
    std::unique_lock<std::mutex> headLock(m_pHeadMutex);

    // Use simple while loop to avoid complex lock ordering in predicate
    while (!hasValueToPop()) {
      m_condition.wait(headLock);
    }

    T ret {std::move(*m_pHead->data)};
    popHead().reset();
    return ret;
  }

  /**
   * Push a value into the queue. newValue will be moved into the queue, and can't be used
   * afterward.
   */
  void push(T newValue) {
    auto newData = std::make_shared<T>(std::move(newValue));
    {
      auto pNode = std::make_unique<Node>();
      const std::lock_guard<std::mutex> m_pTailLock(m_pTailMutex);
      m_pTail->data = newData;
      Node* const newTail = pNode.get();
      m_pTail->next = std::move(pNode);
      m_pTail = newTail;
    }
    {
      // waitPop() owns m_pHeadMutex from its check until it sleeps: waiting for this mutex
      // guarantees that it is either sleeping and notified, or that it will see the new value
      const std::lock_guard<std::mutex> m_pHeadLock(m_pHeadMutex);
    }
    m_condition.notify_one();
  }

  [[nodiscard]] bool isEmpty() {
    const std::scoped_lock lock(m_pTailMutex, m_pHeadMutex);
    return m_pHead.get() == m_pTail;
  }
}; // class ThreadSafeQueue
