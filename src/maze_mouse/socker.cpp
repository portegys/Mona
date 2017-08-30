/*
 * Waits for TCP connections on the given port, and then
 * spawns a new process executing <command> for each connection.
 * <command> uses stdio to connection.
 * Usage: socker <port_num> <command>
 * Based on tserver by Michael Johnson and Erik Troan
 */

#ifdef _WIN32
#ifndef WIN32
#define WIN32
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#ifdef WIN32
#include <winsock2.h>
typedef int   socklen_t;
#else
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
typedef int   SOCKET;
#endif

#define debug    printf
#undef debug
void debug(char *format, ...)
{
}


// Server socket.
SOCKET sock = -1;

// Spawn process.
#ifdef WIN32
#define CMDLEN    512
char cmd[CMDLEN + 1];
void spawn(SOCKET, char *cmd);
#else
void spawn(SOCKET, char *argv[]);
#endif

#ifdef WIN32
WSADATA wsda;
#endif

void die(char *msg)
{
#ifdef WIN32
   fprintf(stderr, (char *)"%s: error=%d\n", msg, WSAGetLastError());
#else
   perror(msg);
#endif
   if (sock != -1)
   {
      shutdown(sock, 2);
#ifdef WIN32
      closesocket(sock);
      WSACleanup();
#else
      close(sock);
#endif
   }
   exit(1);
}


void handle_sig(int signum)
{
#ifdef WIN32
   fprintf(stderr, (char *)"\nSocker exiting normally.\n");
   shutdown(sock, 2);
   closesocket(sock);
   WSACleanup();
   exit(0);
#else
   if (signum == SIGCHLD)
   {
      /* Collect exit statuses.  Prevent zombies. */
      int status;
      while (0 < waitpid(-1, &status, WNOHANG))
      {
      }
   }
   else
   {
      fprintf(stderr, (char *)"\nSocker exiting normally.\n");
      shutdown(sock, 2);
      close(sock);
      exit(0);
   }
#endif
}


#define asizeof(x)    (sizeof(x) / sizeof(x[0]))

void setup_sig_handler()
{
   int sa_num;

#ifdef WIN32
   static int sigs[] =
   {
      SIGINT, SIGTERM
   };
#else
   static int sigs[] =
   {
      SIGHUP, SIGINT, SIGQUIT, SIGXCPU, SIGXFSZ, SIGTERM,
      SIGCHLD
   };
   static struct sigaction sig_actions[asizeof(sigs)];
#endif

   debug((char *)"Starting setup_sig_handler\n");
#ifdef WIN32
   for (sa_num = 1; sa_num < asizeof(sigs); sa_num++)
   {
      signal(sigs[sa_num], handle_sig);
   }
#else
   memset(sig_actions, 0, sizeof(sig_actions));
   sig_actions[0].sa_handler = handle_sig;
   sigemptyset(&(sig_actions[0].sa_mask));
   for (sa_num = 1; sa_num < asizeof(sigs); sa_num++)
   {
      memcpy(sig_actions + sa_num, sig_actions, sizeof(sig_actions[0]));
   }
   for (sa_num = 0; sa_num < asizeof(sigs); sa_num++)
   {
      if (sigaction(sigs[sa_num], sig_actions + sa_num, NULL))
      {
         die((char *)"sigaction");
      }
   }
   siginterrupt(SIGCHLD, 0);                      /* Don't let SIGCHLD interrupt socket calls */
#endif
   debug((char *)"Ending setup_sig_handler\n");
}


int main(int argc, char *argv[])
{
   struct sockaddr_in address;
   SOCKET             conn;
   int                i, portnum;
   socklen_t          addrLength = sizeof(struct sockaddr_in);

   if ((argc < 3) || !(portnum = atoi(argv[1])))
   {
      fprintf(stderr, (char *)"Usage: socker <port_num> <command>\n");
      fprintf(stderr, (char *)"Waits for TCP connections on the given port, and then\n");
      fprintf(stderr, (char *)"spawns a new process executing <command> for each connection.\n");
      fprintf(stderr, (char *)"Exit status: 0=caught signal and exited; 1=error\n");
      exit(1);
   }

#ifdef WIN32
   // Set up command line.
   cmd[0] = '\0';
   for (i = 2; i < argc; i++)
   {
      if ((strlen(cmd) + strlen(argv[i]) + 1) < CMDLEN)
      {
         strcat(cmd, argv[i]);
         if (i < argc - 1)
         {
            strcat(cmd, " ");
         }
      }
      else
      {
         fprintf(stderr, (char *)"Command line too long\n");
         exit(1);
      }
   }

   // Load version 2.2 of Winsock
   if (WSAStartup(MAKEWORD(2, 2), &wsda))
   {
      fprintf(stderr, (char *)"WSAStartup failed error=%d\n", WSAGetLastError());
      exit(1);
   }
   // Find a protocol where socket descriptors returned by the provider
   // are operating system Installable File System (IFS) handles.
   // First, have WSAEnumProtocols tell you how big a buffer you need.
   bool               bProtocolFound = false;
   LPWSAPROTOCOL_INFO lpProtocolBuf  = NULL;
   DWORD              dwBufLen, dwErr;
   int                nRet = WSAEnumProtocolsA(NULL, lpProtocolBuf, &dwBufLen);
   if (SOCKET_ERROR != nRet)
   {
      die((char *)"WSAEnumProtocols: should not have succeeded");
   }
   else if (WSAENOBUFS != (dwErr = WSAGetLastError()))
   {
      // WSAEnumProtocols failed for some reason not relating to buffer size.
      die((char *)"WSAEnumProtocols");
   }
   else
   {
      // WSAEnumProtocols failed for the "expected" reason. Therefore,
      // you need to allocate a buffer of the appropriate size.
      lpProtocolBuf = (WSAPROTOCOL_INFO *)malloc(dwBufLen);
      if (lpProtocolBuf)
      {
         // Now call WSAEnumProtocols again with the expectation
         // that it will succeed because you have allocated a big enough
         // buffer.
         nRet = WSAEnumProtocols(NULL, lpProtocolBuf, &dwBufLen);
         if (SOCKET_ERROR == nRet)
         {
            die((char *)"WSAEnumProtocols");
         }
         else
         {
            // Loop through protocols, looking for the first service
            // provider that meets the matching criteria.
            for (i = 0; i < nRet; i++)
            {
               if ((IPPROTO_TCP == lpProtocolBuf[i].iProtocol) &&
                   (XP1_IFS_HANDLES == (XP1_IFS_HANDLES &
                                        lpProtocolBuf[i].dwServiceFlags1)))
               {
                  bProtocolFound = true;

                  // Create socket.
                  sock = WSASocket(
                     FROM_PROTOCOL_INFO,
                     FROM_PROTOCOL_INFO,
                     FROM_PROTOCOL_INFO,
                     &lpProtocolBuf[i],
                     0,
                     0);
                  if (INVALID_SOCKET == sock)
                  {
                     die((char *)"WSASocket");
                  }
                  break;
               }
            }
         }
      }
   }
   if (bProtocolFound)
   {
      free(lpProtocolBuf);
   }
   else
   {
      die((char *)"socket");
   }
#else
   if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
   {
      die((char *)"socket");
   }
#endif

   debug((char *)"Calling setup_sig_handler\n");
   setup_sig_handler();

   /* Let the kernel reuse the socket address. This lets us run
    * twice in a row, without waiting for the (ip, port) tuple
    * to time out. */
   i = 1;
   setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&i, i);

   address.sin_family = AF_INET;
   address.sin_port   = htons(portnum);
   memset(&address.sin_addr, 0, sizeof(address.sin_addr));

   if (bind(sock, (struct sockaddr *)&address, sizeof(address)))
   {
      die((char *)"bind");
   }

   if (listen(sock, 5))
   {
      die((char *)"listen");
   }

   int errs = 0;
   while (1)
   {
      conn = accept(sock, (struct sockaddr *)&address, &addrLength);
#ifdef WIN32
      if (conn == INVALID_SOCKET)
#else
      if (conn < 0)
#endif
      {
         errs++;
         if (errs < 2)
         {
            continue;
         }
         break;
      }
      else
      {
         errs = 0;
      }
#ifdef WIN32
      spawn(conn, cmd);
#else
      spawn(conn, argv + 2);
#endif
   }

   if (conn < 0)
   {
      die((char *)"accept");
   }

   /* Should not get here */
   shutdown(sock, 2);
#ifdef WIN32
   closesocket(sock);
   WSACleanup();
#else
   close(sock);
#endif
   return(2);
}


#ifdef WIN32

void spawn(SOCKET conn, char *cmd)
{
   PROCESS_INFORMATION pi;

   ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

   //Information about where the IO goes
   STARTUPINFO si;
   ZeroMemory(&si, sizeof(STARTUPINFO));

   //cb is the size of the STARTUPINFO structure
   si.cb = sizeof(si);
   //Tell it we do not want to see the application we are redirecting to/from
   si.wShowWindow = SW_HIDE;
   //Tell it we want to be able to use .wShowWinwow, .hStdInput, .hstdOutput, .hstdError members.
   si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;

   //Tell it we want the standard input to come from conn
   si.hStdInput = (HANDLE)conn;
   //Tell it we want the standard output to go to conn
   si.hStdOutput = (HANDLE)conn;
   //Tell it we want the stardard error to appear on conn
   si.hStdError = (HANDLE)conn;

   //Spawn the process we want to redirect using the information we just declared.
   if (CreateProcess(NULL, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi) == FALSE)
   {
      fprintf(stderr, (char *)"CreateProcess failed (error=%d).  Please make sure that '%s' refers to a valid program.\n", GetLastError(), cmd);
   }
   else
   {
      // Close handles to the child process and its primary thread.
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
   }
}


#else  // !WIN32
void spawn(SOCKET conn, char *argv[])
{
   if (fork())
   {
      close(conn);
   }
   else
   {
      close(sock);

      /* Redirect stdin & stdout to socket */
      if ((0 != dup2(conn, 0)) || (1 != dup2(conn, 1)) || (2 != dup2(conn, 2)))
      {
         die((char *)"dup2 redirection");
      }

      /* Turn off buffering */
      /* Note: this seems to have no effect beyond execvp */
      setbuf(stdin, 0);
      setbuf(stdout, 0);

      /* Execute command */
      execvp(argv[0], argv);

      /* Error if we get here */
      fprintf(stderr, (char *)"execvp failed.  Please make sure that '%s' refers to a valid program.\n", argv[0]);

      // man fork says _exit should be called to prevent parent from being corrupted.
      _exit(1);
   }
}


#endif
