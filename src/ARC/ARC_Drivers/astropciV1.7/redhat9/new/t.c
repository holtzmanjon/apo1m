#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
int main(void)
{ 
  int fd;
  unsigned short *mem_fd;
  
  fd = open("/dev/astropci0", O_RDWR);
  printf ("fd = %d\n", fd);
  if (fd != -1) {
    mem_fd = (unsigned short *)mmap(0, 2200*2200*2, (PROT_READ | PROT_WRITE), MAP_SHARED, (int)fd, 0);  
    printf ("mem_fd = %d; errno = %d\n", mem_fd, errno);
  }
  exit(0);
}
