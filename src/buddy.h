#ifndef BUDDY_H
#define BUDDY_H

#define __BUDDY_CAPACITY 1024

struct {
  bool occupied[__BUDDY_CAPACITY];
  short next[__BUDDY_CAPACITY];
} __buddy_block;

void buddy_init() {
  for (int i = 0; i < __BUDDY_CAPACITY; i++) {
    __buddy_block.occupied[i] = 0;
    __buddy_block.next[i] = i + __BUDDY_CAPACITY;
  }
}

int buddy_upperbound(int b) {
  // Get appropriate size
  int ep = 0;

  // Is power of two
  if (b && !(b & (b - 1))) {
    return b;
  }
  while (b) {
    ep++;
    b >>= 1;
  }

  return 1 << ep;
}

int buddy_tryAllocate(int bytes) {
  // Get appropriate size
  int size = buddy_upperbound(bytes);
  int start = 0, next;
  int dmin = __INT_MAX__, minimum = __BUDDY_CAPACITY;

  while (start < __BUDDY_CAPACITY) {
    // Find next available block
    if (__buddy_block.occupied[start]) {
      if (__buddy_block.next[start] % size) {
        start += size;
      }
      else {
        start = __buddy_block.next[start];
      }
      continue;
    }

    // Found available block
    // Find minimum block
    next = __buddy_block.next[start];
    if (next - start >= size && next - start < dmin) {
      dmin = next - start;
      minimum = start;
    }

    start += size;
  }

  if (minimum < __BUDDY_CAPACITY) return minimum;
  return -1;
}

/**
 * @return index on success, -1 on failure
 */
int buddy_allocate(int bytes) {
  int minimum = buddy_tryAllocate(bytes);
  if (minimum < 0) return -1;

  // Get appropriate size
  int size = buddy_upperbound(bytes);

  // Make the next available block point to the following block
  if (minimum + size < __BUDDY_CAPACITY &&
      !__buddy_block.occupied[minimum + size])
    __buddy_block.next[minimum + size] = min(__buddy_block.next[minimum], __BUDDY_CAPACITY);

  // Point to the next available block
  __buddy_block.next[minimum] = minimum + size;
  __buddy_block.occupied[minimum] = 1;

  // Link previous smaller block to allocated block
  int index = 0, next;
  while (index < minimum) {
    next = __buddy_block.next[index];
    if (next >= minimum) {
      __buddy_block.next[index] = minimum;
      break;
    }
    index = next;
  }
  return minimum;
}

int buddy_free(int index) {
  if (!__buddy_block.occupied[index]) return -1;

  // Clear
  __buddy_block.occupied[index] = 0;

  // Merge blocks iteratively, working up denominations
  int size = __buddy_block.next[index] - index;
  while (size < __BUDDY_CAPACITY) {
    // Check for backward merge
    // Move to occupied block
    if (index / size % 2 &&
        !__buddy_block.occupied[index - size]) index -= size;

    // Check if a merge is possible
    if (__buddy_block.occupied[index + size] ||
        __buddy_block.next[index] - index < size) break;

    // Link with next
    __buddy_block.next[index] = __buddy_block.next[index + size];

    // Erase next
    __buddy_block.next[index + size] = index + size + __BUDDY_CAPACITY;

    size <<= 1;
  }

  __buddy_block.next[index] = min(__buddy_block.next[index], __BUDDY_CAPACITY);
  return 0;
}
#endif
