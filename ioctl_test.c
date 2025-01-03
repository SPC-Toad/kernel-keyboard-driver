#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define IOCTL_TEST _IOW(0, 6, struct ioctl_test_t)

/*
  This is just testing the ioctl call.
  This does not do anything useful. :)
*/

int main () {

  /* attribute structures */
  struct ioctl_test_t {
    int field1;
    char field2;
  } ioctl_test;

  int fd = open("/proc/ioctl_test", O_RDONLY);

  ioctl_test.field1 = 10;
  ioctl_test.field2 = 'a';

  ioctl(fd, IOCTL_TEST, &ioctl_test);

  return 0;
}