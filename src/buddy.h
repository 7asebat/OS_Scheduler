#ifndef BUDDY_H
#define BUDDY_H

typedef struct buddy_cell {
  short occupied;
  short next;
} buddy_cell;

buddy_cell block[1024];

void buddy_init() {
  for (int i = 0; i < 1024; i++) {
    block[i] = (buddy_cell){0, i+1024};
  }
}

/**
 * @return index on success, -1 on failure
 */
int buddy_allocate(int bytes) {
  // Get appropriate size
  int size = bytes, ep = 0;

  // Is power of two
  if (bytes && !(bytes & (bytes - 1))) {
    size = bytes;
  }
  else {
    while (size) {
      ep++;
      size >>= 1;
    }
    size = 1 << ep;
  }

  int start = 0, next;
  int dmin = __INT_MAX__, minimum = 0;

  while (start < 1024) {
    next = block[start].next;
    if (block[start].occupied) {
      if (block[start].next % size) {
        start += size;
      } 
      else {
        start = block[start].next;
      }
      continue;
    }

    // Found empty segment
    next = block[start].next;

    // Occupied, next is size steps afterwards
    if (next - start >= size && next - start < dmin) {
      dmin = next - start;
      minimum = start;
    }
    start += size;
  }

  if (minimum < 1024) {
    next = block[minimum].next;

    if (next - minimum >= size) {
      if (!block[minimum + size].occupied) 
        block[minimum + size].next = block[minimum].next;  // Link with old next

      block[minimum] = (buddy_cell){1, minimum + size};
      return minimum;
    }
  }
  return -1;
}

int buddy_free(int index, int bytes) {
  if (!block[index].occupied) return -1;

  // Clear
  block[index].occupied = 0;

  // Get appropriate size
  int size = bytes, ep = 0;

  // Is power of two
  if (bytes && !(bytes & (bytes - 1))) {
    size = bytes;
  }
  else {
    while (size) {
      ep++;
      size >>= 1;
    }
    size = 1 << ep;
  }

  // Check if mergable with next spot
  if (index / size % 2 == 0) {
    if (!block[index + size].occupied) {
      // Link with next
      block[index] = block[index + size];
    }
  }
  else if (index >= size) {
    if (!block[index - size].occupied) {
      // Link with next
      block[index - size] = block[index];
    }
  }
  return 0;
}
#endif
