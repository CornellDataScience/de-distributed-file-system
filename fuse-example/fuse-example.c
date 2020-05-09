/**
 * Less Simple, Yet Stupid Filesystem.
 * 
 * Mohammed Q. Hussain - http://www.maastaar.net
 *
 * This is an example of using FUSE to build a simple filesystem. 
 * It is a part of a tutorial in MQH Blog with the title "Writing Less Simple, 
 * Yet Stupid Filesystem Using FUSE in C": http://maastaar.net/fuse/linux/filesystem/c/2019/09/28/writing-less-simple-yet-stupid-filesystem-using-FUSE-in-C/
 *
 * License: GNU GPL
 */

#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include "lockServerMessage.h"

// ... //

#define PORT 8889 /* the port client will be connecting to */

#define MAXDATASIZE 100 /* max number of bytes we can get at once */

int create_connection()
{
	int sockfd, numbytes;
	char buf[MAXDATASIZE];
	struct hostent *he;
	struct sockaddr_in their_addr; /* connector's address information */

	if ((he = gethostbyname("localhost")) == NULL)
	{ /* get the host info */
		herror("gethostbyname");
		exit(1);
	}

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket");
		exit(1);
	}

	their_addr.sin_family = AF_INET;	 /* host byte order */
	their_addr.sin_port = htons(PORT); /* short, network byte order */
	their_addr.sin_addr = *((struct in_addr *)he->h_addr);
	bzero(&(their_addr.sin_zero), 8); /* zero the rest of the struct */

	if (connect(sockfd, (struct sockaddr *)&their_addr,
							sizeof(struct sockaddr)) == -1)
	{
		perror("connect");
		exit(1);
	}
	//int connection_accepted = 0;
	return sockfd;
}

int send_receive_msg(message_t msg, char *filepath, int status, int messageType, char *buffer, int sockfd)
{
	printf("SENDING MESSAGE \n");
	strcpy(msg.file_path, filepath);
	msg.isSuccess = status;
	msg.messageType = messageType;
	encodeMessage(msg, buffer);
	send(sockfd, buffer, strlen(buffer), 0);
	printf("SENT MESSAGE\n");
	recv(sockfd, buffer, MAXDATASIZE, 0);
	message_t received;
	printf("RECEIVED MESSAGE\n");
	printf("MESSAGE IS %s", buffer);
	received = decodeMessage(buffer);
	printf("RETURNING\n");
	return received.isSuccess;
}

// *** //
char dir_list[256][256];
int curr_dir_idx = -1;

char files_list[256][256];
int curr_file_idx = -1;

char files_content[256][256];
int curr_file_content_idx = -1;

void add_dir(const char *dir_name)
{
	curr_dir_idx++;
	strcpy(dir_list[curr_dir_idx], dir_name);
}

int is_dir(const char *path)
{
	path++; // Eliminating "/" in the path

	for (int curr_idx = 0; curr_idx <= curr_dir_idx; curr_idx++)
		if (strcmp(path, dir_list[curr_idx]) == 0)
			return 1;

	return 0;
}

void add_file(const char *filename)
{
	curr_file_idx++;
	strcpy(files_list[curr_file_idx], filename);

	curr_file_content_idx++;
	strcpy(files_content[curr_file_content_idx], "");
}

int is_file(const char *path)
{
	path++; // Eliminating "/" in the path

	for (int curr_idx = 0; curr_idx <= curr_file_idx; curr_idx++)
		if (strcmp(path, files_list[curr_idx]) == 0)
			return 1;

	return 0;
}

int get_file_index(const char *path)
{
	path++; // Eliminating "/" in the path

	for (int curr_idx = 0; curr_idx <= curr_file_idx; curr_idx++)
		if (strcmp(path, files_list[curr_idx]) == 0)
			return curr_idx;

	return -1;
}

void write_to_file(const char *path, const char *new_content)
{
	int file_idx = get_file_index(path);

	if (file_idx == -1) // No such file
		return;

	strcpy(files_content[file_idx], new_content);
}

// ... //

static int do_getattr(const char *path, struct stat *st)
{
	st->st_uid = getuid();		 // The owner of the file/directory is the user who mounted the filesystem
	st->st_gid = getgid();		 // The group of the file/directory is the same as the group of the user who mounted the filesystem
	st->st_atime = time(NULL); // The last "a"ccess of the file/directory is right now
	st->st_mtime = time(NULL); // The last "m"odification of the file/directory is right now

	if (strcmp(path, "/") == 0 || is_dir(path) == 1)
	{
		st->st_mode = S_IFDIR | 0755;
		st->st_nlink = 2; // Why "two" hardlinks instead of "one"? The answer is here: http://unix.stackexchange.com/a/101536
	}
	else if (is_file(path) == 1)
	{
		st->st_mode = S_IFREG | 0644;
		st->st_nlink = 1;
		st->st_size = 1024;
	}
	else
	{
		return -ENOENT;
	}

	return 0;
}

static int do_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
	filler(buffer, ".", NULL, 0);	 // Current Directory
	filler(buffer, "..", NULL, 0); // Parent Directory

	if (strcmp(path, "/") == 0) // If the user is trying to show the files/directories of the root directory show the following
	{
		for (int curr_idx = 0; curr_idx <= curr_dir_idx; curr_idx++)
			filler(buffer, dir_list[curr_idx], NULL, 0);

		for (int curr_idx = 0; curr_idx <= curr_file_idx; curr_idx++)
			filler(buffer, files_list[curr_idx], NULL, 0);
	}

	return 0;
}

static int do_read(const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi)
{
	message_t msg;
	char buffer[1024];
	int sockfd = create_connection();
	while (send_receive_msg(msg, path, SUCCESS, REQUEST_READ, buffer, sockfd) != 1)
		;
	int file_idx = get_file_index(path);

	if (file_idx == -1)
		return -1;

	char *content = files_content[file_idx];

	memcpy(buffer, content + offset, size);

	return strlen(content) - offset;
}

static int do_mkdir(const char *path, mode_t mode)
{
	message_t msg;
	char buffer[1024];
	int sockfd = create_connection();
	while (send_receive_msg(msg, path, SUCCESS, ACQUIRE_LOCK, buffer, sockfd) != 1)
		; // spin lock
	path++;
	add_dir(path);
	send_receive_msg(msg, path, SUCCESS, RELEASE_LOCK, buffer, sockfd);
	return 0;
}

// user wishes to create a new file
static int do_mknod(const char *path, mode_t mode, dev_t rdev)
{
	message_t msg;
	char buffer[1024];
	int sockfd = create_connection();
	while (send_receive_msg(msg, path, SUCCESS, ACQUIRE_LOCK, buffer, sockfd) != 1)
		; // spin lock
	path++;
	add_file(path);
	send_receive_msg(msg, path, SUCCESS, RELEASE_LOCK, buffer, sockfd);
	return 0;
}

static int do_write(const char *path, const char *buffer, size_t size, off_t offset, struct fuse_file_info *info)
{
	message_t msg;
	char buffer[1024];
	int sockfd = create_connection();
	while (send_receive_msg(msg, path, SUCCESS, ACQUIRE_LOCK, buffer, sockfd) != 1)
		; // spin lock
	write_to_file(path, buffer);
	send_receive_msg(msg, path, SUCCESS, RELEASE_LOCK, buffer, sockfd);
	return size;
}

static struct fuse_operations operations = {
		.getattr = do_getattr,
		.readdir = do_readdir,
		.read = do_read,
		.mkdir = do_mkdir,
		.mknod = do_mknod,
		.write = do_write,
};

int main(int argc, char *argv[])
{
	return fuse_main(argc, argv, &operations, NULL);
}
