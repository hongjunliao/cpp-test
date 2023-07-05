/*
  BlueZ example code to build an rfcomm server.
  This code just creates a socket and accepts
  connections from a remote bluetooth device.

  Programmed by Bastian Ballmann
  http://www.geektown.de

  Compile with gcc -lbluetooth <executable> <source>
*/
#include <stdio.h>
#ifdef CPP_TEST_WITH_LIBBLUEZ
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>

#include "bluetooth/bluetooth.h"
#include "bluetooth/rfcomm.h"

#define CHANNEL 4
#define QUEUE 10

int rfcomm_server_main(int argc, char * argv[])
{
  int sock, client, alen;
  struct sockaddr_rc addr;

  if( (sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM)) < 0)
    {
      perror("socket");
      exit(1);
    }

  addr.rc_family = AF_BLUETOOTH;
  bacpy(&addr.rc_bdaddr, BDADDR_ANY);
  addr.rc_channel = htobs(CHANNEL);
  alen = sizeof(addr);

  if(bind(sock, (struct sockaddr *)&addr, alen) < 0)
    {
      perror("bind");
      exit(1);
    }

  listen(sock,QUEUE);
  printf("Waiting for connections...\n\n");

  while(client = accept(sock, (struct sockaddr *)&addr, &alen))
    {
      printf("Got a connection attempt!\n");
      close(client);
    }

  close(sock);
  return 0;
}
#else
int rfcomm_server_main(int argc, char * argv[])
{
	fprintf(stderr, "%s: define WITH_LIBBLUEZ to enable this test\n", __FUNCTION__);
	return 0;
}

#endif /* WITH_LIBBLUEZ */
