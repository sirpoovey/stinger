#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/types.h>
#include <time.h>
#include <netdb.h>

#include "stinger_core/stinger.h"
#include "stinger_core/xmalloc.h"
#include "stinger_utils/stinger_sockets.h"
#include "stinger_utils/timer.h"

#include "human_edge_generator.h"

using namespace gt::stinger;



int
main(int argc, char *argv[])
{
  /* global options */
  int port = 10102;
  struct hostent * server = NULL;

  int opt = 0;
  while(-1 != (opt = getopt(argc, argv, "p:a:"))) {
    switch(opt) {
      case 'p': {
	port = atoi(optarg);
      } break;

      case 'a': {
	server = gethostbyname(optarg);
	if(NULL == server) {
	  LOG_E_A ("ERROR: server %s could not be resolved.", optarg);
	  exit(-1);
	}
      } break;

      case '?':
      case 'h': {
	printf("Usage:    %s [-p port] [-a server_addr]\n", argv[0]);
	printf("Defaults:\n\tport: %d\n\tserver: localhost\n", port);
	exit(0);
      } break;
    }
  }

  LOG_D_A ("Running with: port: %d", port);

  /* connect to localhost if server is unspecified */
  if(NULL == server) {
    server = gethostbyname("localhost");
    if(NULL == server) {
      LOG_E_A ("ERROR: server %s could not be resolved.", "localhost");
      exit (-1);
    }
  }

  /* start the connection */
  int sock_handle = connect_to_batch_server (server, port);
  if (sock_handle == -1) exit(-1);

  /* actually generate and send the batches */
  int64_t batch_num = 0;
  int64_t time = 0;

  while (1)
  {
    StingerBatch batch;
    batch.set_make_undirected(true);
    batch.set_type(STRINGS_ONLY);
    batch.set_keep_alive(true);

    std::string src, dest;
    std::cout << "\nEnter the source vertex: ";
    std::getline (std::cin, src);

    std::cout << "Enter the destination vertex: ";
    std::getline (std::cin, dest);



    time = get_current_timestamp();

    if (src[0] == '-') {
      std::string del;
      std::cout << "Type X to delete otherwise will decrement the weight: ";
      std::getline (std::cin, del);
      std::string src2 = src.substr(1);
      EdgeDeletion * deletion = batch.add_deletions();
      deletion->set_source_str(src2);
      deletion->set_destination_str(dest);
      if (del.compare("X") == 0) {
        deletion->set_weight(0);
      } else {
        deletion->set_weight(1);
      }
      LOG_I("DELETION");
    } else {
      EdgeInsertion * insertion = batch.add_insertions();
      insertion->set_source_str(src);
      insertion->set_destination_str(dest);
      insertion->set_weight(1);
      insertion->set_time(time);
    }
    LOG_I_A ("%ld: <%s, %s> at time %ld", batch_num, src.c_str(), dest.c_str(), time);
    LOG_D ("Sending message");
    send_message(sock_handle, batch);

    batch_num++;
    
  }

  StingerBatch batch;
  batch.set_make_undirected(true);
  batch.set_type(STRINGS_ONLY);
  batch.set_keep_alive(false);
  send_message(sock_handle, batch);

  return 0;
}
