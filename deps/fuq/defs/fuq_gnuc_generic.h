#ifndef FUQ_GNUC_GENERIC_
#define FUQ_GNUC_GENERIC_

static inline void fuq__read_barrier(void) {
  __sync_synchronize();
}

static inline void fuq__write_barrier(void) {
  __sync_synchronize();
}

#endif  /* FUQ_GNUC_GENERIC_ */
