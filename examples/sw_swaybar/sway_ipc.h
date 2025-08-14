#if !defined(SWAY_IPC_H)
#define SWAY_IPC_H

#if !defined(_DEFAULT_SOURCE)
#define _DEFAULT_SOURCE
#endif /* !defined(_DEFAULT_SOURCE) */

#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>

#include <stddef.h>
#include <stdint.h>
#include <limits.h>

#if !defined(SU_IMPLEMENTATION)
#define SU_IMPLEMENTATION
#endif /* !defined(SU_IMPLEMENTATION) */
#if !defined(SU_STRIP_PREFIXES)
#define SU_STRIP_PREFIXES
#endif /* !defined(SU_STRIP_PREFIXES) */
#include "sutil.h"

/* TODO: nonblock read/write */

typedef enum sway_ipc_message_type {
	/* i3 command types - see i3's I3_REPLY_TYPE constants */
	SWAY_IPC_MESSAGE_TYPE_COMMAND = 0,
	SWAY_IPC_MESSAGE_TYPE_GET_WORKSPACES = 1,
	SWAY_IPC_MESSAGE_TYPE_SUBSCRIBE = 2,
	SWAY_IPC_MESSAGE_TYPE_GET_OUTPUTS = 3,
	SWAY_IPC_MESSAGE_TYPE_GET_TREE = 4,
	SWAY_IPC_MESSAGE_TYPE_GET_MARKS = 5,
	SWAY_IPC_MESSAGE_TYPE_GET_BAR_CONFIG = 6,
	SWAY_IPC_MESSAGE_TYPE_GET_VERSION = 7,
	SWAY_IPC_MESSAGE_TYPE_GET_BINDING_MODES = 8,
	SWAY_IPC_MESSAGE_TYPE_GET_CONFIG = 9,
	SWAY_IPC_MESSAGE_TYPE_SEND_TICK = 10,
	SWAY_IPC_MESSAGE_TYPE_SYNC = 11,
	SWAY_IPC_MESSAGE_TYPE_GET_BINDING_STATE = 12,

	/* sway-specific message types */
	SWAY_IPC_MESSAGE_TYPE_GET_INPUTS = 100,
	SWAY_IPC_MESSAGE_TYPE_GET_SEATS = 101,

	/* Events sent from sway to clients. Events have the highest bits set. */
	SWAY_IPC_MESSAGE_TYPE_EVENT_WORKSPACE = -2147483648, /* ((1<<31) | 0) */
	SWAY_IPC_MESSAGE_TYPE_EVENT_OUTPUT = -2147483647, /* ((1<<31) | 1) */
	SWAY_IPC_MESSAGE_TYPE_EVENT_MODE = -2147483646, /* ((1<<31) | 2) */
	SWAY_IPC_MESSAGE_TYPE_EVENT_WINDOW = -2147483645, /* ((1<<31) | 3) */
	SWAY_IPC_MESSAGE_TYPE_EVENT_BARCONFIG_UPDATE = -2147483644, /* ((1<<31) | 4) */
	SWAY_IPC_MESSAGE_TYPE_EVENT_BINDING = -2147483643, /* ((1<<31) | 5) */
	SWAY_IPC_MESSAGE_TYPE_EVENT_SHUTDOWN = -2147483642, /* ((1<<31) | 6) */
	SWAY_IPC_MESSAGE_TYPE_EVENT_TICK = -2147483641, /* ((1<<31) | 7) */

	/* sway-specific event types */
	SWAY_IPC_MESSAGE_TYPE_EVENT_BAR_STATE_UPDATE = -2147483628, /* ((1<<31) | 20) */
	SWAY_IPC_MESSAGE_TYPE_EVENT_INPUT = -2147483627 /* ((1<<31) | 21) */
} sway_ipc_message_type_t;

typedef struct sway_ipc_response {
	sway_ipc_message_type_t type;
	PAD32;
	string_t payload;
} sway_ipc_response_t;

static bool32_t sway_ipc_get_socket_path(char out[PATH_MAX]);
static int sway_ipc_connect(char socket_path[PATH_MAX]);

static int sway_ipc_send(int fd, sway_ipc_message_type_t, string_t *payload);
static sway_ipc_response_t *sway_ipc_receive(int fd, allocator_t *alloc);

static void sway_ipc_response_free(sway_ipc_response_t *, allocator_t *alloc);


static char sway_ipc__magic[] = {'i', '3', '-', 'i', 'p', 'c'};

#define SWAY_IPC__HEADER_SIZE (sizeof(sway_ipc__magic) + sizeof(uint32_t) + sizeof(uint32_t))

static bool32_t sway_ipc_get_socket_path(char out[PATH_MAX]) {
	char *sock = getenv("SWAYSOCK");
	if (sock && *sock) {
		size_t len = strlen(sock) + 1;
		if (len < PATH_MAX) {
			memcpy(out, sock, len + 1);
			return TRUE;
		}
	}
	/* TODO: sway --get-socketpath 2>/dev/null */
	sock = getenv("I3SOCK");
	if (sock && *sock) {
		size_t len = strlen(sock) + 1;
		if (len < PATH_MAX) {
			memcpy(out, sock, len + 1);
			return TRUE;
		}
	}
	/* TODO: i3 --get-socketpath 2>/dev/null */
	return FALSE;
}

static int sway_ipc_connect(char socket_path[PATH_MAX]) {
	struct sockaddr_un addr;
	size_t len;
	int fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd == -1) {
		return -1;
	}
	addr.sun_family = AF_UNIX;
	len = MIN((sizeof(addr.sun_path) - 1), strlen(socket_path));
	strncpy(addr.sun_path, socket_path, len);
	addr.sun_path[len] = '\0';
	if (connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1) {
		return -1;
	}
	fd_set_cloexec(fd);
	return fd;
}

static int sway_ipc_send(int fd, sway_ipc_message_type_t type, string_t *payload) {
	char header[SWAY_IPC__HEADER_SIZE];
	uint32_t len = payload ? (uint32_t)payload->len : 0;
	size_t total = 0;

	memcpy(header, sway_ipc__magic, sizeof(sway_ipc__magic));
	memcpy(header + sizeof(sway_ipc__magic), &len, sizeof(len));
	memcpy(header + sizeof(sway_ipc__magic) + sizeof(len), &type, sizeof(type));

	while (total < sizeof(header)) {
		ssize_t written_bytes = write(fd, &header[total], sizeof(header) - total);
		if (written_bytes == -1) {
			return -1;
		}
		total += (size_t)written_bytes;
	}

	total = 0;
	while (total < len) {
		ssize_t written_bytes = write(fd, &payload->s[total], len - total);
		if (written_bytes == -1) {
			return -1;
		}
		total += (size_t)written_bytes;
	}

	return 1;
}

static void sway_ipc_response_free(sway_ipc_response_t *response, allocator_t *alloc) {
	string_fini(&response->payload, alloc);
	alloc->free(alloc, response);
}

static sway_ipc_response_t *sway_ipc_receive(int fd, allocator_t *alloc) {
	sway_ipc_response_t *response;
	uint32_t len;
	char header[SWAY_IPC__HEADER_SIZE];
	
	size_t total = 0;
	while (total < sizeof(header)) {
		ssize_t read_bytes = read(fd, &header[total], sizeof(header) - total);
		switch (read_bytes) {
		case 0:
			errno = EPIPE;
			return NULL;
		case -1:
			if (errno == EINTR) {
				continue;
			}
			return NULL;
		default:
			total += (size_t)read_bytes;
		}
	}

	response = alloc->alloc(alloc, sizeof(*response), ALIGNOF(*response));

	memcpy(&len, header + sizeof(sway_ipc__magic), sizeof(uint32_t));
	memcpy(&response->type, header + sizeof(sway_ipc__magic) + sizeof(uint32_t), sizeof(uint32_t));
	response->payload.s = alloc->alloc(alloc, len + 1, ALIGNOF(*response->payload.s));
	response->payload.len = len;
	response->payload.free_contents = TRUE;
	response->payload.nul_terminated = TRUE;
	response->payload.s[len] = '\0';

	total = 0;
	while (total < len) {
		ssize_t read_bytes = read(fd, &response->payload.s[total], len - total);
		switch (read_bytes) {
		case 0:
			errno = EPIPE;
			ATTRIBUTE_FALLTHROUGH;
		case -1:
			if (errno == EINTR) {
				continue;
			}
			sway_ipc_response_free(response, alloc);
			return NULL;
		default:
			total += (size_t)read_bytes;
		}
	}

	return response;
}

#endif /* SWAY_IPC_H */
