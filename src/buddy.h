#ifndef BUDDY_H
#define BUDDY_H

#define __BUDDY_CAPACITY 1024

struct {
  char occupied[__BUDDY_CAPACITY];
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

bool buddy_check(int bytes) {
  // Get appropriate size

  FILE* debugLog = fopen("logs/debug.log", "w");
  fprintf(debugLog, "here \n");
  fflush(debugLog);

  int size = buddy_upperbound(bytes);

  int start = 0, next;
  int dmin = __INT_MAX__, minimum = 1024;

  while (start < 1024) {
    // Next spot
    next = buddy.next[start];

    // Find next available delimiter
    if (buddy.occupied[start]) {
      if (buddy.next[start] % size) {
        start += size;
      }
      else {
        start = buddy.next[start];
      }
      continue;
    }

    // Found empty segment
    next = buddy.next[start];

    // Occupied, next is size steps afterwards
    if (next - start >= size && next - start < dmin) {
      dmin = next - start;
      minimum = start;
    }
    start += size;
  }

  if (minimum < 1024) {
    return true;
  }
  return false;
}

/**
 * @return index on success, -1 on failure
 */
int buddy_allocate(int bytes) {
  // Get appropriate size
  int size = buddy_upperbound(bytes);

  int start = 0, next;
  int dmin = __INT_MAX__, minimum = __BUDDY_CAPACITY;

  while (start < __BUDDY_CAPACITY) {
    // Find next available segment
    if (__buddy_block.occupied[start]) {
      if (__buddy_block.next[start] % size) {
        start += size;
      }
      else {
        start = __buddy_block.next[start];
      }
      continue;
    }

    // Found available segment
    // Find minimum segment
    next = __buddy_block.next[start];
    if (next - start >= size && next - start < dmin) {
      dmin = next - start;
      minimum = start;
    }

    start += size;
  }

  if (minimum < __BUDDY_CAPACITY) {
    next = __buddy_block.next[minimum];

    if (!__buddy_block.occupied[minimum + size])
      __buddy_block.next[minimum + size] = __buddy_block.next[minimum];  // Link with old next

    __buddy_block.occupied[minimum] = 1;
    __buddy_block.next[minimum] = minimum + size;
    return minimum;
  }

  return -1;
}

int buddy_free(int index, int bytes) {
  if (!__buddy_block.occupied[index]) return -1;

  // Clear
  __buddy_block.occupied[index] = 0;

  // Get appropriate size
  int size = buddy_upperbound(bytes);

  // Check if mergable with next segment
  if ((index / size % 2 == 0) && (!__buddy_block.occupied[index + size])) {
    // Link with next
    __buddy_block.next[index] = __buddy_block.next[index + size];

    // Erase next
    __buddy_block.next[index + size] = index + size + __BUDDY_CAPACITY;
  }
  // Check if mergable with previous segment
  else if ((index >= size) && (!__buddy_block.occupied[index - size])) {
    // Link with next
    __buddy_block.next[index - size] = __buddy_block.next[index];

    // Erase next
    __buddy_block.next[index] = index + __BUDDY_CAPACITY;
  }

  return 0;
}
#endif
