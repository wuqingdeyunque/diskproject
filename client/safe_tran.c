#include "func.h"

int recv_file(int sfd,char *filename)
{
	char buf[1000];
	int len,ret,filesize,finished=0,old=0;
	int fd=open(filename,O_RDWR|O_CREAT,0666);
	error_check(-1,fd,"open");
	recv(sfd,&filesize,4,0);
	if(filesize>100*1024*1024)
	{
		ftruncate(fd,filesize);
		char *p=(char *)mmap(NULL,filesize,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);	
		while(finished<filesize)
		{
			recv(sfd,&len,4,0);
	//	printf("len=%d\n",len);
			ret=recv_s(sfd,p+finished,len);
	//	printf("%s\n",buf);
			error_check(-1,ret,"recv_s");
			finished+=len;
			if((finished-old)>(filesize/100))
			{
				printf("have gets %d%s\r",finished*100/filesize,"%");
			}
		}
	}
	else
	{
		while(finished<filesize)
		{
			recv(sfd,&len,4,0);
	//	printf("len=%d\n",len);
			ret=recv_s(sfd,buf,len);
	//	printf("%s\n",buf);
			error_check(-1,ret,"recv_s");
			finished+=len;
			write(fd,buf,len);
			if((finished-old)>(filesize/100))
			{
				printf("have gets %d%s\r",finished*100/filesize,"%");
			}
		}
	}
	printf("gets file success!\n");
	return 0;
}

int send_file(int sfd,char *filename)
{
	msgtrain msg;
	struct stat buf;
	int len,filesize,finished=0,old=0;
	int fd=open(filename,O_RDONLY);
	error_check(-1,fd,"open");
	fstat(fd,&buf);
	filesize=buf.st_size;
	send(sfd,&filesize,4,0);
	while(finished<filesize)
	{
		msg.len=read(fd,msg.context,sizeof(msg.context));
		len=send_s(sfd,(char *)&msg,4+msg.len);
		finished+=msg.len;
		if((finished-old)>(filesize/100))
		{
			printf("have puts %d%s\r",finished*100/filesize,"%");
		}
	}
}
int send_s(int fd,char *buf,int len)//循环发送可以解决接受和发送速度不匹配导致的缓冲区访问越界问题
{
	int finished=0;//已经发送的字节数
	int ret;
	while(finished<len)//没有发送完
	{
		ret=send(fd,buf+finished,len-finished,0);//ret为成功发送的数量
		/*if(ret!=1004)
		  {
		  printf("send_s ret=%d\n",ret);
		  }*/
		if(-1==ret)
		{
			perror("send");
			return -1;
		}
		/*	if(ret==0)
			{
			close(fd);
			return -1;
			}*/
		finished+=ret;

	}
	return 0;
}
int recv_s(int fd,char *buf,int len)
{
	int finished=0;
	int ret;
	while(finished<len)
	{
		ret=recv(fd,buf+finished,len-finished,0);
		if(-1==ret)
		{
			perror("recv");
			return -1;
		}
		finished+=ret;
	}
	return 0;
}
