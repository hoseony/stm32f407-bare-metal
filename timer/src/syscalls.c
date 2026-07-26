#include "../include/uart.h"

int _write(int fd, uint8_t *ptr, int len) {
  (void) fd, (void) ptr, (void) len;
  if (fd == 1) UART_transmitBytes(UART4, ptr, (size_t) len);
  return -1;
}
