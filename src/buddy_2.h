#include "headers.h"

unsigned int** slots;
unsigned int* n_slots;

/* Math utilities */

unsigned int __nextPowerOf2(unsigned int n) {
  unsigned count = 0;

  if (n && !(n & (n - 1)))
    return n;

  while (n != 0) {
    n >>= 1;
    count += 1;
  }

  return 1 << count;
}

unsigned int __logOfPower2(unsigned int n) {
  unsigned count = 0;
  while (n != 0) {
    n >>= 1;
    count += 1;
  }

  return count - 1;
}

/* Slots */

int __init_slots(int size, unsigned int*** _slots, unsigned int** _n_slots) {
  unsigned int slotsNum = __logOfPower2(size) + 1;

  // allocate slots
  unsigned int** slots = (unsigned int**)malloc((slotsNum) * sizeof(unsigned int*));

  if (slots == NULL)
    return -1;

  for (int i = 0; i < slotsNum; i++) {
    slots[i] = (unsigned int*)malloc(size * sizeof(unsigned int));

    if (slots[i] == NULL) {
      for (int j = i - 1; j >= 0; j--)
        free(slots[j]);

      free(slots);

      return -1;
    }

    size /= 2;
  }

  *_slots = slots;

  // allocate n_slots
  unsigned int* n_slots = (unsigned int*)malloc((slotsNum) * sizeof(unsigned int));

  if (n_slots == NULL) {
    for (int j = slotsNum; j >= 0; j--)
      free(slots[j]);

    free(slots);

    return -1;
  }

  memset(n_slots, 0, (slotsNum) * sizeof(unsigned int));

  *_n_slots = n_slots;

  return 0;
}

/**
 * @return index inserted at on sucess, -1 on failure
 */
int __slot_insert(int slotIdx, int val) {
  if (slotIdx < 0)
    return -1;

  int i = 0;

  while (i < n_slots[slotIdx] && slots[slotIdx][i] < val) {
    i++;
  }

  for (int j = n_slots[slotIdx] - 1; j >= i; j--) {
    slots[slotIdx][j + 1] = slots[slotIdx][j];
  }

  slots[slotIdx][i] = val;

  n_slots[slotIdx]++;

  return i;
}

int __slot_remove(int slotIdx, int elemIdx) {
  if (elemIdx >= n_slots[slotIdx])  // Invalid idx
    return -1;

  for (int i = elemIdx; i < n_slots[slotIdx] - 1; i++) {
    slots[slotIdx][i] = slots[slotIdx][i + 1];
  }

  n_slots[slotIdx]--;

  return 0;
}

/* Buddy Helpers */

int __buddy_split(int slotIdx) {
  if (slotIdx == 0)
    return -1;

  if (n_slots[slotIdx] == 0)
    __buddy_split(slotIdx + 1);

  unsigned int pos = slots[slotIdx][0];

  __slot_remove(slotIdx, 0);

  __slot_insert(slotIdx - 1, pos);
  __slot_insert(slotIdx - 1, pos + (1 << (slotIdx - 1)));

  return 0;
}

/**
 * Checks if a block can be merged with its neighbours
 * @param slotIdx unsigned integer representing the slot index
 * @param elemIdx unsigned integer representing the element index inside the slot
 * @return void
 */
void __buddy_check_merge(unsigned int slotIdx, unsigned int elemIdx) {
  unsigned int elemPos = slots[slotIdx][elemIdx];
  unsigned int isOddIdx = ((elemPos / (1 << slotIdx)) % 2);

  if (isOddIdx)  // oddIdx -> check if left of me is free
  {
    if (elemIdx > 0 && elemPos - slots[slotIdx][elemIdx - 1] == (1 << slotIdx)) {
      // I have a left and it is the one right before me
      unsigned int slotStart = slots[slotIdx][elemIdx - 1];

      __slot_remove(slotIdx, elemIdx);
      __slot_remove(slotIdx, elemIdx - 1);

      unsigned int insertedAt = __slot_insert(slotIdx + 1, slotStart);

      __buddy_check_merge(slotIdx + 1, insertedAt);
    }
  }
  else {  // evenIdx -> check if right of me is free
    if (elemIdx < ((int)n_slots[slotIdx] - 1) && slots[slotIdx][elemIdx + 1] - elemPos == (1 << slotIdx)) {
      // I have a right and it is the one right after me
      unsigned int slotStart = slots[slotIdx][elemIdx];

      __slot_remove(slotIdx, elemIdx + 1);
      __slot_remove(slotIdx, elemIdx);

      unsigned int insertedAt = __slot_insert(slotIdx + 1, slotStart);

      __buddy_check_merge(slotIdx + 1, insertedAt);
    }
  }
}

/* Buddy Interface */

/**
 * Allocates a block in the memory with a given size
 * @param size unsigned integer representing the block size wanted
 * @return position of block on success, -1 on failure
 */
int buddy_allocate(unsigned int size) {
  unsigned int allocSize = __nextPowerOf2(size);
  unsigned int allocIdx = __logOfPower2(allocSize);

  if (n_slots[allocIdx] == 0) {
    if (__buddy_split(allocIdx + 1) == -1)
      return -1;
  }

  unsigned int alloc_pos = slots[allocIdx][0];  // position to be allocated

  __slot_remove(allocIdx, 0);  // can remove last idx to optimize removal

  return alloc_pos;
}

/**
 * Frees a block from the memory
 * @param loc integer represents the location to be freed
 * @param size integer represents the size of that block
 * @return 0 on success, -1 on failure
 */
int buddy_free(int loc, int size) {
  unsigned int allocSize = __nextPowerOf2(size);
  unsigned int allocIdx = __logOfPower2(allocSize);

  int insertedAt = __slot_insert(allocIdx, loc);

  if (insertedAt == -1)
    return -1;

  __buddy_check_merge(allocIdx, insertedAt);

  return 0;
}

/**
 * Initializes buddy memory with given size
 * @param size unsigned integer representing the size of the memory
 *             must be a power of two
 * @return void
 */
void buddy_init(unsigned int size) {
  __init_slots(size, &slots, &n_slots);

  unsigned int logSize = __logOfPower2(size);
  slots[logSize][0] = 0;
  n_slots[logSize] = 1;
}

/**
 * Prints the buddy memory representation, useful for debugging
 * @param size unsigned integer representing the size of the memory
 * @return void
 */
void buddy_print(unsigned int size) {
  unsigned int numSlots = __logOfPower2(size) + 1;

  for (int i = 0; i < numSlots; i++) {
    printf("Slot %d:{ ", (1 << i));
    for (int j = 0; j < n_slots[i]; j++) {
      printf("%d ", slots[i][j]);
    }
    printf("}\n");
  }
  printf("-----------------\n");

  return;
}

/*
int main() {
  buddy_init(128);

  int pos[10];

  pos[0] = buddy_allocate(20);
  buddy_print(128);

  pos[1] = buddy_allocate(15);
  buddy_print(128);

  pos[2] = buddy_allocate(10);
  buddy_print(128);

  pos[3] = buddy_allocate(25);
  buddy_print(128);

  buddy_free(pos[0], 20);
  buddy_print(128);

  buddy_free(pos[2], 10);
  buddy_print(128);

  pos[4] = buddy_allocate(8);
  buddy_print(128);

  pos[5] = buddy_allocate(30);
  buddy_print(128);

  buddy_free(pos[1], 15);
  buddy_print(128);

  pos[6] = buddy_allocate(15);
  buddy_print(128);

  buddy_free(pos[4], 8);
  buddy_print(128);

  buddy_free(pos[5], 30);
  buddy_print(128);

  buddy_free(pos[1], 15);
  buddy_print(128);
}
*/