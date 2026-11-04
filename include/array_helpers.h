#ifndef VISTRA_ARRAY_HELPERS_H
#define VISTRA_ARRAY_HELPERS_H

#include "config.h"

// Sorts `count` floats in ascending order (bubble sort, small N).
static inline void sortFloatArray(float arr[], byte count) {
  for (byte i = 0; i < count - 1; i++) {
    for (byte j = i + 1; j < count; j++) {
      if (arr[j] < arr[i]) {
        float temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
      }
    }
  }
}

// Sorts `count` ints in ascending order (bubble sort, small N).
static inline void sortIntArray(int arr[], byte count) {
  for (byte i = 0; i < count - 1; i++) {
    for (byte j = i + 1; j < count; j++) {
      if (arr[j] < arr[i]) {
        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
      }
    }
  }
}

// Returns the median of `count` floats. `copy` capped at 20 elements.
static inline float medianFloat(float arr[], byte count) {
  float copy[20];
  if (count > 20) {
    count = 20;
  }
  for (byte i = 0; i < count; i++) {
    copy[i] = arr[i];
  }
  sortFloatArray(copy, count);
  if (count % 2 == 1) {
    return copy[count / 2];
  }
  return (copy[(count / 2) - 1] + copy[count / 2]) / 2.0f;
}

// Returns the median of `count` ints. `copy` capped at 20 elements.
static inline int medianInt(int arr[], byte count) {
  int copy[20];
  if (count > 20) {
    count = 20;
  }
  for (byte i = 0; i < count; i++) {
    copy[i] = arr[i];
  }
  sortIntArray(copy, count);
  if (count % 2 == 1) {
    return copy[count / 2];
  }
  return (copy[(count / 2) - 1] + copy[count / 2]) / 2;
}

#endif // VISTRA_ARRAY_HELPERS_H
