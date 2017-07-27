#include <stdio.h>
#include "pcx.h"
#include "io.h"

main()
{
  char inputline[132];

  while (1) {
    printf("Enter command to send to OMS card: ");
    getline(inputline,sizeof(inputline)); 

    pc38_send_commands(inputline);
  }

}
