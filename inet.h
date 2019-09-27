#if !defined(INET_H)
#define INET_H

/*
 * Stripped down version of dsm_inet.h from the Libdsm project
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


/*
 *******************************************************************************
 *                            Function Declarations                            *
 *******************************************************************************
*/


// [NON-REENTRANT] Converts port (< 99999) to string. Returns pointer.
const char *portToString (unsigned int port);

// Returns string describing address of addrinfo struct. Returns NULL on error.
const char *addrinfoToString (struct addrinfo *ap, char *b); 

// Returns a socket connected to given address and port. Exits fatally on error.
int getConnectedSocket (const char *addr, const char *port);

// Gets socket address (use buffer length INET6_ADDRSTRLEN) and port.
void getSocketInfo (int s, char *addr_buf, size_t buf_size, 
	unsigned int *port);

// [DEBUG] Outputs socket's address and port.
void showSocketInfo (int s);

// Ensures 'size' data is sent to fd. Exits fatally on error.
void sendall (int fd, unsigned char *b, size_t size);

// Ensures 'size' data is received from fd. Exits fatally on error.
// Returns zero if all is normal. Returns nonzero if connection is closed.
int recvall (int fd, unsigned char *b, size_t size);


#endif