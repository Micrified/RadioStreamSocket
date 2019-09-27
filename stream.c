#include <stdio.h>
#include <stdlib.h>

#include "inet.h"

/*
 * ABOUT: This program demonstrates how to use a TCP/IP socket
 * to initiate a stream. It outputs the number of bytes
 * read and the rate at which data is being streamed
*/


// Default address (string) to use for the streaming socket
#define DEFAULT_STREAM_ADDR			"icy-5.radioparadise.com"

// Default address (host) to user for the streaming socket
#define DEFAULT_STREAM_HOST			"37.130.228.60"


// Default port (string) to use for the streaming socket
#define DEFAULT_STREAM_PORT			"80"


// Default path (string) to use to access stream at address
#define DEFAULT_STREAM_PATH			"mp3-128"


// Default number of bytes to read at a time from the stream
#define DEFAULT_STREAM_CHUNK_SIZE	32


// Format string for HTTP GET requests
const char *g_get_request_fmt = "GET /%s HTTP/1.1\r\n" \
                                "Host: %s\r\n" \
						        "Connection: keep-alive\r\n" \
						        "Accept: */*\r\n\r\n";

int main (void) {
	char buffer[512];
	int len, recv, byte_count = 0;

	// Create the socket
	int sock = getConnectedSocket(DEFAULT_STREAM_HOST, DEFAULT_STREAM_PORT);

	if (-1 == sock) {
		fprintf(stderr, "Err: Couldn't connect the socket!\n");
		return EXIT_FAILURE;
	} else {
		printf("Connected to %s on port %s\n", DEFAULT_STREAM_ADDR,
		 DEFAULT_STREAM_PORT); 
	}


	// Create the get request
	len = snprintf(buffer, 512, g_get_request_fmt, DEFAULT_STREAM_PATH, 
		DEFAULT_STREAM_ADDR);

	if (-1 == len || len >= 512) {
		fprintf(stderr, "Buffer not large enough to fit GET request!\n");
		return EXIT_FAILURE;
	}


	// Send the get request
	sendall(sock, (unsigned char *)buffer, len);

	// Read back the stream in chunks of N bytes at a time
	do {
		recv = recvall(sock, buffer, DEFAULT_STREAM_CHUNK_SIZE);
		if (recv != 0) {
			break;
		}
		byte_count += DEFAULT_STREAM_CHUNK_SIZE;
		printf("\rRead: %d", byte_count);
	} while (1);

	printf("End: Connection closed\n");

	return EXIT_SUCCESS;
}
