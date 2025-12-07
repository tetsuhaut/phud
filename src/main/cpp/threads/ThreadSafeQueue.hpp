#pragma once

#include <condition_variable>
#include <memory> // std::unique_ptr
#include <mutex>
#include <queue>

// std::lock_guard<std::mutex> unlocks only when destroyed.
// std::condition_variable does not take std::lock_guard<std::mutex>.
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
   * Pop a value and return true, unless the queue is empty, then return false.
   * @returns true If the queue contains a value
   */
  [[nodiscard]] bool tryPop(T& value) {
    const std::lock_guard<std::mutex> headLock(m_pHeadMutex);
    {
      const std::lock_guard<std::mutex> tailLock(m_pTailMutex);
      if (m_pHead.get() == m_pTail) {
        // the queue is empty
        return false;
      }
    }
    value = std::move(*m_pHead->data);
    popHead().reset();
    return true;
  }

  /**
   * Wait forever until there is a value to pop, then pop it.
   */
  void waitPop(T& value) {
    std::unique_lock<std::mutex> headLock(m_pHeadMutex);

    // Use simple while loop to avoid complex lock ordering in predicate
    while (true) {
      {
        std::lock_guard<std::mutex> tailLock(m_pTailMutex);
        if (m_pHead.get() != m_pTail) {
          break; // Data available
        }
      }
      m_condition.wait(headLock);
    }

    value = std::move(*m_pHead->data);
    popHead().reset();
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
    m_condition.notify_one();
  }

  [[nodiscard]] bool isEmpty() {
    // Acquire mutexes in consistent order: tail first, then head (prevents deadlock)
    const std::lock_guard<std::mutex> tailLock(m_pTailMutex);
    const std::lock_guard<std::mutex> headLock(m_pHeadMutex);
    return m_pHead.get() == m_pTail;
  }
}; // class ThreadSafeQueue
