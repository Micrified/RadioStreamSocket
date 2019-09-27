#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#include "inet.h"




/*
 *******************************************************************************
 *                            Function Definitions                             *
 *******************************************************************************
*/


// [NON-REENTRANT] Converts port (< 99999) to string. Returns pointer.
const char *portToString (unsigned int port) {
	static char b[6]; // Five digits for 65536 + one for null character.
	snprintf(b, 6, "%u", port);
	return b;
}

// Returns string describing address of addrinfo struct. Returns NULL on error.
const char *addrinfoToString (struct addrinfo *ap, char *b) {
	void *addr;

	// If IPv4, use sin_addr. Otherwise use sin6_addr for IPv6.
	if (ap->ai_family == AF_INET) {
		struct sockaddr_in *ipv4 = (struct sockaddr_in *)ap->ai_addr;
		addr = &(ipv4->sin_addr);
	} else {
		struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)ap->ai_addr;
		addr = &(ipv6->sin6_addr);
	}

	// Convert format to string: Buffer must be INET6_ADDRSTRLEN large.
	return inet_ntop(ap->ai_family, addr, b, INET6_ADDRSTRLEN);
}

// Returns a socket connected to given address and port. Exits fatally on error.
int getConnectedSocket (const char *addr, const char *port) {
	struct addrinfo hints, *res, *p;
	int s, stat;

	// Setup hints. 
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	// Lookup connection options.
	if ((stat = getaddrinfo(addr, port, &hints, &res)) != 0) {
		fprintf(stderr, "getaddrinfo failed: %s", gai_strerror(stat));
	}

	// Bind to first suitable result.
	for (p = res; p != NULL; p = p->ai_next) {

		// Try initializing a socket.
		if ((s = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			fprintf(stderr, "Socket init failed on getaddrinfo result!");
			continue;
		}

		// Try connecting to the socket.
		if (connect(s, p->ai_addr, p->ai_addrlen) == -1) {
			continue;
		}

		break;
	}

	// Free linked-list of results.
	freeaddrinfo(res);

	// Return result.
	return ((p == NULL) ? -1 : s);
}

// Gets socket address (use buffer length INET6_ADDRSTRLEN) and port.
void getSocketInfo (int s, char *addr_buf, size_t buf_size, 
	unsigned int *port) {
	struct sockaddr_storage addrinfo;
	socklen_t size = sizeof(addrinfo);
	void *addr;
	int family;

	// Verify input.
	if (addr_buf != NULL && buf_size < INET6_ADDRSTRLEN) {
		fprintf(stderr, "dsm_getSocketInfo failed", "buf_size too small!"); 
	}
	
	// Extract socket information.
	if (getsockname(s, (struct sockaddr *)&addrinfo, &size) != 0) {
		fprintf(stderr, "getsockname failed!");
	}

	// Verify family, cast to appropriate structure. Assign port.
	if (addrinfo.ss_family == AF_INET) {
		struct sockaddr_in *ipv4 = (struct sockaddr_in *)&addrinfo;
		family = ipv4->sin_family;
		addr = &(ipv4->sin_addr);
		if (port != NULL) {
			*port = ntohs(ipv4->sin_port);
		}
	} else {
		struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)&addrinfo;
		family = ipv6->sin6_family;
		addr = &(ipv6->sin6_addr);
		if (port != NULL) {
			*port = ntohs(ipv6->sin6_port);
		}
	}

	// Convert address to string, write to buffer.
	if (inet_ntop(family, addr, addr_buf, buf_size) == NULL) {
		fprintf(stderr, "inet_ntop failed!");
	}
}

// [DEBUG] Outputs socket's address and port.
void showSocketInfo (int s) {
	unsigned int port;
	char b[INET6_ADDRSTRLEN];

	getSocketInfo(s, b, INET6_ADDRSTRLEN, &port);

	printf("FD: %d, ADDR: \"%s\", PORT: %u\n", s, b, port);
}

// Ensures 'size' data is sent to fd. Exits fatally on error.
void sendall (int fd, unsigned char *b, size_t size) {
	size_t sent = 0;
	int n;

	do {
		if ((n = send(fd, (b + sent), size - sent, 0)) == -1) {
			fprintf(stderr, "Syscall error on send!");
		}
		sent += n;
	} while (sent < size);
}

// Ensures 'size' data is received from fd. Exits fatally on error.
// Returns zero if all is normal. Returns nonzero if connection is closed.
int recvall (int fd, unsigned char *b, size_t size) {
	size_t received = 0;
	int n;

	do {
		if ((n = recv(fd, (b + received), size - received, 0)) == -1) {
			fprintf(stderr, "Syscall error on recv!");
		}
		
		// Check if socket is closed.
		if (n == 0) {
			return -1;
		}

		received += n;
	} while (received < size);

	return 0;
}