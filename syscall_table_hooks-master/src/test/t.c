#include<stdio.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/types.h>
int main()
{
	int fd = open("abc.txt", O_RDONLY);
	char buff[2048];
	read(fd, &buff, 2048);
	printf("%s\n",buff);
	return 0;
}
