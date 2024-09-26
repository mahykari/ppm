#include <sys/socket.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

// The following code is, for the most part,
// copied from the following URL:
// https://www.ibm.com/docs/en/i/7.4
// (topic=designs-using-poll-instead-select).

#define PORTSYS  1234
#define PORTMON  5678

#define TRUE            1
#define FALSE           0

enum Role {
  SYSTEM = 0,
  MONITOR = 1,
};

#define MSGSZ    8
#define MSGSZMAX 800

#define STR_(X) #X
#define STR(X) STR_(X)

class Message
{
public:
  Message()
  {
    memset(msg, 0, sizeof(msg));
  }

  operator char*()
  {
    return msg;
  }

  void finish()
  {
    char msg1[sizeof(msg)];
    strcpy(msg1, msg);
    sprintf(msg, "%0" STR(MSGSZ) "d", (int) strlen(msg1));
    strcat(msg + MSGSZ, msg1);
  }
private:
  char msg[MSGSZMAX + MSGSZ];
};

class Monitor
{
public:
  static Message nxt()
  {
    Message msg;
    int len = rand() % (16);
    char charset[] =
      "0123456789"
      "abcdefghijklmnopqrstuvwxyz"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    for (int i = 0; i < len; i++)
    {
      char c = rand() % (sizeof(charset) - 1);
      msg[i] = charset[c];
    }
    msg[len] = 0;
    return msg;
  }
};

void serve(Role role)
{
  int    len, rc, on = 1;
  int    listen_sd = -1, new_sd = -1, connect_sd = -1;
  int    desc_ready, end_server = FALSE, compress_array = FALSE;
  int    close_conn;
  char   buffer[80];
  struct sockaddr_in   addr;
  int    timeout;
  struct pollfd fds[4];
  int    nfds = 1, current_size = 0, i, j;
  int    msglen = -1, nchunks = -1;
  Message              msg;
  //************************************************************/
  /* Create an AF_INET stream socket to receive incoming       */
  /* connections on                                            */
  /*************************************************************/
  listen_sd = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_sd < 0)
  {
    perror("socket() failed");
    exit(-1);
  }

  /*************************************************************/
  /* Allow socket descriptor to be reuseable                   */
  /*************************************************************/
  rc = setsockopt(listen_sd, SOL_SOCKET,  SO_REUSEADDR,
                  (char *)&on, sizeof(on));
  if (rc < 0)
  {
    perror("setsockopt() failed");
    close(listen_sd);
    exit(-1);
  }

  /*************************************************************/
  /* Set socket to be nonblocking. All of the sockets for      */
  /* the incoming connections will also be nonblocking since   */
  /* they will inherit that state from the listening socket.   */
  /*************************************************************/
  rc = ioctl(listen_sd, FIONBIO, (char *)&on);
  if (rc < 0)
  {
    perror("ioctl() failed");
    close(listen_sd);
    exit(-1);
  }

  /*************************************************************/
  /* Bind the socket                                           */
  /*************************************************************/
  memset(&addr, 0, sizeof(addr));
  addr.sin_family      = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  int portNumber = role == SYSTEM ? PORTSYS : PORTMON;
  addr.sin_port        = htons(portNumber);
  rc = bind(listen_sd,
            (struct sockaddr *)&addr, sizeof(addr));
  if (rc < 0)
  {
    perror("bind() failed");
    close(listen_sd);
    exit(-1);
  }

  /*************************************************************/
  /* Set the listen back log                                   */
  /*************************************************************/
  rc = listen(listen_sd, 32);
  if (rc < 0)
  {
    perror("listen() failed");
    close(listen_sd);
    exit(-1);
  }

  /*************************************************************/
  /* Role is MONITOR, so we should connect to SYSTEM           */
  /*************************************************************/
  if (role == MONITOR) {
    connect_sd = socket(AF_INET, SOCK_STREAM, 0);
    if (connect_sd < 0)
    {
      perror("socket() failed");
      exit(-1);
    }
    struct sockaddr_in caddr;
    memset(&caddr, 0, sizeof(caddr));
    caddr.sin_family = AF_INET;
    caddr.sin_addr.s_addr = INADDR_ANY;
    caddr.sin_port = htons(PORTSYS);
    rc = connect(connect_sd, 
                 (struct sockaddr *)&caddr, sizeof(caddr));
    if (rc < 0)
    {
      perror("connect() failed");
      close(connect_sd);
      exit(-1);
    }
    printf("Connected to SYSTEM.\n");
  }

  /*************************************************************/
  /* Initialize the pollfd structure                           */
  /*************************************************************/
  memset(fds, 0 , sizeof(fds));

  /*************************************************************/
  /* Set up the initial listening socket                       */
  /*************************************************************/
  fds[0].fd = listen_sd;
  fds[0].events = POLLIN;
  if (role == MONITOR) {
    fds[1].fd = connect_sd;
    fds[1].events = POLLIN;
    fds[2].fd = STDIN_FILENO;
    fds[2].events = POLLIN;
    nfds = 3;
  }
  /*************************************************************/
  /* Initialize the timeout to 3 minutes. If no                */
  /* activity after 3 minutes this program will end.           */
  /* timeout value is based on milliseconds.                   */
  /*************************************************************/
  timeout = (3 * 60 * 1000);

  /*************************************************************/
  /* Loop waiting for incoming connects or for incoming data   */
  /* on any of the connected sockets.                          */
  /*************************************************************/
  do
  {
    if (role == MONITOR)
    {
      msg = Monitor::nxt();
      printf("  Sending message to SYSTEM: %s\n", (char*) msg);
      msg.finish();
      rc = send(connect_sd, msg, strlen(msg), 0);
      if (rc < 0)
      {
        perror("  send() failed");
        close_conn = TRUE;
      }
    }
    /***********************************************************/
    /* Call poll() and wait 3 minutes for it to complete.      */
    /***********************************************************/
    printf("Waiting on poll()...\n");
    rc = poll(fds, nfds, timeout);

    /***********************************************************/
    /* Check to see if the poll call failed.                   */
    /***********************************************************/
    if (rc < 0)
    {
      perror("  poll() failed");
      break;
    }

    /***********************************************************/
    /* Check to see if the 3 minute time out expired.          */
    /***********************************************************/
    if (rc == 0)
    {
      printf("  poll() timed out.  End program.\n");
      break;
    }

    /***********************************************************/
    /* One or more descriptors are readable.  Need to          */
    /* determine which ones they are.                          */
    /***********************************************************/
    current_size = nfds;
    for (i = 0; i < current_size; i++)
    {
      /*********************************************************/
      /* Loop through to find the descriptors that returned    */
      /* POLLIN and determine whether it's the listening       */
      /* or the active connection.                             */
      /*********************************************************/
      if(fds[i].revents == 0)
        continue;

      /*********************************************************/
      /* If revents is not POLLIN, it's an unexpected result,  */
      /* log and end the server.                               */
      /*********************************************************/
      if(fds[i].revents != POLLIN)
      {
        printf("  Error! revents = %d\n", fds[i].revents);
        end_server = TRUE;
        break;

      }
      if (fds[i].fd == listen_sd)
      {
        /*******************************************************/
        /* Listening descriptor is readable.                   */
        /*******************************************************/
        printf("  Listening socket is readable\n");

        /*******************************************************/
        /* Accept all incoming connections that are            */
        /* queued up on the listening socket before we         */
        /* loop back and call poll again.                      */
        /*******************************************************/
        do
        {
          /*****************************************************/
          /* Accept each incoming connection. If               */
          /* accept fails with EWOULDBLOCK, then we            */
          /* have accepted all of them. Any other              */
          /* failure on accept will cause us to end the        */
          /* server.                                           */
          /*****************************************************/
          new_sd = accept(listen_sd, NULL, NULL);
          if (new_sd < 0)
          {
            if (errno != EWOULDBLOCK)
            {
              perror("  accept() failed");
              end_server = TRUE;
            }
            break;
          }

          /*****************************************************/
          /* Add the new incoming connection to the            */
          /* pollfd structure                                  */
          /*****************************************************/
          printf("  New incoming connection - %d\n", new_sd);
          fds[nfds].fd = new_sd;
          fds[nfds].events = POLLIN;
          nfds++;

          /*****************************************************/
          /* Loop back up and accept another incoming          */
          /* connection                                        */
          /*****************************************************/
        } while (new_sd != -1);
      }

      /*********************************************************/
      /* This is not the listening socket, therefore an        */
      /* existing connection must be readable                  */
      /*********************************************************/

      else
      {
        printf("  Descriptor %d is readable\n", fds[i].fd);
        close_conn = FALSE;
        /*******************************************************/
        /* Receive all incoming data on this socket            */
        /* before we loop back and call poll again.            */
        /*******************************************************/

        /*****************************************************/
        /* Receive data on this connection until the         */
        /* recv fails with EWOULDBLOCK. If any other         */
        /* failure occurs, we will close the                 */
        /* connection.                                       */
        /*****************************************************/

        /*****************************************************/
        /* First 8 bytes determine message size, namely M    */
        /* With a fixed buffer size of B,                    */
        /* we read exactly ceil(M/B) times,                  */
        /* assuming no error occurs during the recv's.       */
        /*****************************************************/

        // rc = recv(fds[i].fd, buffer, sizeof(buffer), 0);
        memset(buffer, 0, sizeof(buffer));
        rc = recv(fds[i].fd, buffer, MSGSZ, 0);

        /*****************************************************/
        /* Check to see if the connection has been           */
        /* closed by the client                              */
        /*****************************************************/
        if (rc == 0)
        {
          printf("  Connection closed\n");
          close_conn = TRUE;
        }

        else if (rc < 0)
        {
          if (errno != EWOULDBLOCK)
          {
            perror("  recv() failed");
            close_conn = TRUE;
          }
        }

        /*****************************************************/
        /* Data was received                                 */
        /*****************************************************/
        else
        {
          printf("  received %s", buffer);
          sscanf(buffer, "%d", &msglen);
          printf("  Message size: %d\n", msglen);
          msg = Message();
          nchunks = msglen / sizeof(buffer)
                    + (msglen % sizeof(buffer) != 0);
          for (j = 0; j < nchunks; j++)
          {
            memset(buffer, 0, sizeof(buffer));
            rc = recv(fds[i].fd, buffer, sizeof(buffer), 0);
            /***************************************************/
            /* No error should occur during recv of chunks.    */
            /***************************************************/
            len = rc;
            if (rc < 0)
            {
              perror("  recv() failed");
              close_conn = TRUE;
              break;
            }

            printf("  %d bytes received\n", rc);
            strcat(msg, buffer);
          }
          /*****************************************************/
          /* Echo the data back to the client                  */
          /*****************************************************/
          switch (role)
          {
          case SYSTEM:
            printf("  Received message from MONITOR: %s\n", (char*) msg);
            msg = Monitor::nxt();
            printf("  Sending message back to MONITOR: %s\n", (char*) msg);
            msg.finish();
            rc = send(fds[i].fd, msg, msglen, 0);
            if (rc < 0)
            {
              perror("  send() failed");
              close_conn = TRUE;
            }
          default: {}
          }
        }

        /*******************************************************/
        /* If the close_conn flag was turned on, we need       */
        /* to clean up this active connection. This            */
        /* clean up process includes removing the              */
        /* descriptor.                                         */
        /*******************************************************/
        if (close_conn)
        {
          close(fds[i].fd);
          fds[i].fd = -1;
          compress_array = TRUE;
        }
      }  /* End of existing connection is readable             */
    } /* End of loop through pollable descriptors              */

    /***********************************************************/
    /* If the compress_array flag was turned on, we need       */
    /* to squeeze together the array and decrement the number  */
    /* of file descriptors. We do not need to move back the    */
    /* events and revents fields because the events will always*/
    /* be POLLIN in this case, and revents is output.          */
    /***********************************************************/
    
    if (compress_array)
    {
      compress_array = FALSE;
      for (i = 0; i < nfds; i++)
      {
        if (fds[i].fd == -1)
        {
          for(j = i; j < nfds-1; j++)
          {
            fds[j].fd = fds[j+1].fd;
          }
          i--;
          nfds--;
        }
      }
    }
  } while (end_server == FALSE); /* End of serving running.    */

  /*************************************************************/
  /* Clean up all of the sockets that are open                 */
  /*************************************************************/
  for (i = 0; i < nfds; i++)
  {
    if(fds[i].fd >= 0)
      close(fds[i].fd);
  }
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    printf("Usage: %s [sys|mon]\n", argv[0]);
    return 1;
  }
  Role role;
  if (strcmp(argv[1], "sys") == 0) {
    role = Role::SYSTEM;
  } else if (strcmp(argv[1], "mon") == 0) {
    role = Role::MONITOR;
  } else {
    printf("Usage: %s [sys|mon]\n", argv[0]);
    return 1;
  }
  serve(role);
}
