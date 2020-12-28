#include "headers.h"

FILE* openFile(const char* fileName) {
  FILE* pFile = fopen(fileName, "w");
  return pFile;
}