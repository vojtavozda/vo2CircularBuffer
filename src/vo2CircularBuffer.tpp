
#include "vo2CircularBuffer.h"

template <typename T, size_t Size>
CircularBuffer<T, Size>::CircularBuffer() : head(0), tail(0), full(false) {}

template <typename T, size_t Size>
bool CircularBuffer<T, Size>::push(const T& item) {
    // Lock buffer here, it will be released upon return
    std::lock_guard<std::mutex> lock(bufferMutex);

    // When a high priority item is pushed, remove all low priority items
    if (item.highPriority) {
        removeAllLowPriority();
    }

    if (full) {
        // Buffer is full, need to manage priorities
        if (!replaceOldestWithLowPriority(item)) {
            // If the buffer is full of high priority items and the new item
            // is low priority, return false. This is ok.
            return false;
        }
    } else {
        buffer[head] = item;
        head = (head + 1) % Size;
        full = (head == tail);
    }

    return true;
}

template <typename T, size_t Size>
bool CircularBuffer<T, Size>::pop(T& item, bool lock) {
    // Define lockGuard and lock only if `lock=true`. This is because `pop()`
    // can be called also from `push()` where the buffer is already locked.
    std::unique_lock<std::mutex> lockGuard(bufferMutex, std::defer_lock);
    if (lock) lockGuard.lock();

    if (isEmpty()) {
        return false;
    }

    item = buffer[tail];
    tail = (tail + 1) % Size;
    full = false;
    return true;
}

template <typename T, size_t Size>
bool CircularBuffer<T, Size>::isEmpty() const {
    return (!full && (head == tail));
}

template <typename T, size_t Size>
bool CircularBuffer<T, Size>::isFull() const {
    return full;
}

template <typename T, size_t Size>
void CircularBuffer<T, Size>::clear() {
    std::lock_guard<std::mutex> lock(bufferMutex);
    head = 0;
    tail = 0;
    full = false;
}


template <typename T, size_t Size>
bool CircularBuffer<T, Size>::replaceOldestWithLowPriority(const T& item) {
    // Find the oldest item with low priority
    size_t pos = tail;
    bool foundLowPriority = false;

    for (size_t i = 0; i < Size; ++i) {
        if (!buffer[pos].highPriority) {
            foundLowPriority = true;
            break;
        }
        pos = (pos + 1) % Size;
    }

    if (foundLowPriority) {
        buffer[pos] = item;
        if (pos == head) {
            head = (head + 1) % Size;
        }
        return true;
    } else if (item.highPriority) {
        // If we can't find a low priority item, replace the oldest
        buffer[tail] = item;
        tail = (tail + 1) % Size;
        head = (head + 1) % Size;
        return true;
    }

    // Return false only if the buffer is full of high priority items and
    // the new item is low priority
    return false;
}

template <typename T, size_t Size>
void CircularBuffer<T, Size>::removeAllLowPriority() {
    T tempBuffer[Size];
    size_t tempHead = 0;
    size_t tempTail = 0;
    bool tempFull = false;

    while (!isEmpty()) {
        T item;
        pop(item,false);
        if (item.highPriority) {
            tempBuffer[tempHead] = item;
            tempHead = (tempHead + 1) % Size;
            tempFull = (tempHead == tempTail);
        }
    }

    for (size_t i = 0; i < Size; ++i) {
        buffer[i] = tempBuffer[i];
    }
    head = tempHead;
    tail = tempTail;
    full = tempFull;
}