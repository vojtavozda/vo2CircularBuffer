
#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <mutex>

/**
 * @brief Circular buffer with fixed size.
 * Access to private `buffer` which is accessed by `push()` and `pop()` methods
 * is protected by a mutex.
 * 
 * @tparam T
 * @tparam Size 
 */
template <typename T, size_t Size>
class CircularBuffer {
public:
    /** Constructor */
    CircularBuffer();

    /** Push new item into the buffer.
     * - If item is high priority, all low priority items will be removed.
     * - If buffer is full of high priority items and new item is high priority,
     *   the oldest item will be replaced.
     * - If buffer is full of high priority items and new item is low priority,
     *   the new item will be ignored.
    */
    bool push(const T& item);
    /** Pop the oldest item from the buffer.
     * Returns false if the buffer is empty.
     * @param item Reference to the item to be popped.
     * @param lock Should be true unless accessing from `push()`.
    */
    bool pop(T& item, bool lock = true);
    /** Check if the buffer is empty. */
    bool isEmpty() const;
    /** Check if the buffer is full. */
    bool isFull() const;
    /** Empty buffer */
    void clear();

private:
    /** Buffer */
    T buffer[Size];
    /** Head index */
    size_t head;
    /** Tail index */
    size_t tail;
    /** Full flag */
    bool full;
    /** Mutex to lock buffer */
    std::mutex bufferMutex;

    /** Replace the oldest item with low priority. */
    bool replaceOldestWithLowPriority(const T& item);
    /** Remove all low priority items. */
    void removeAllLowPriority();
};

#include "CircularBuffer.tpp"

#endif // CIRCULAR_BUFFER_H