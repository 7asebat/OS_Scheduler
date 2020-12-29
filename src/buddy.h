#ifndef BUDDY_H
#define BUDDY_H

struct {
  short occupied[1024];
  short next[1024];
} buddy;

void buddy_init() {
  for (int i = 0; i < 1024; i++) {
    buddy.occupied[i] = 0;
    buddy.next[i] = i + 1024;
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

/**
 * @return index on success, -1 on failure
 */
int buddy_allocate(int bytes) {
  // Get appropriate size
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
    next = buddy.next[minimum];

    if (!buddy.occupied[minimum + size])
      buddy.next[minimum + size] = buddy.next[minimum];  // Link with old next

    buddy.occupied[minimum] = 1;
    buddy.next[minimum] = minimum + size;
    return minimum;
  }
  return -1;
}

int buddy_free(int index, int bytes) {
  if (!buddy.occupied[index]) return -1;

  // Clear
  buddy.occupied[index] = 0;

  // Get appropriate size
  int size = buddy_upperbound(bytes);

  // Check if mergable with next spot
  if (index / size % 2 == 0) {
    // Link with next
    if (!buddy.occupied[index + size])
      buddy.next[index] = buddy.next[index + size];
  }
  else if (index >= size) {
    // Link with next
    if (!buddy.occupied[index - size])
      buddy.next[index - size] = buddy.next[index];
  }
  return 0;
}
#endif
