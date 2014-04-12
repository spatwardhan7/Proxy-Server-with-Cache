/*
* rhttp.c: Origin of client code
* This code makes a RPC to the server to request a URL
* and calculates the cache latency and finally logs everything in a log file
*/

#include <stdio.h>  
#include <rpc/rpc.h>     /* always needed */  
#include "gethttp.h" /* gethttp.h will be generated by rpcgen */  

#include <unistd.h>
#include <sys/time.h>

long TimeDiff(struct timeval *t1, struct timeval *t2)
{
	unsigned long t_sec,t_usec,totalTime;

	t_sec=(t2->tv_sec-t1->tv_sec);
	t_usec=(t2->tv_usec-t1->tv_usec);

	if(t1->tv_usec>t2->tv_usec) 
	{
		t_usec=1000000-t1->tv_usec+t2->tv_usec;
		t_sec-=1;
	}

	totalTime = (1000000 * t_sec) + (t_usec);
	//printf("%ld:%ld\n",t_sec,t_usec);
	//fflush(stdout);

	return totalTime;
}


int main(int argc, char** argv) 
{    
	CLIENT *cl;
	char *server;  

	urlData_res* result; 

	url urlname; 

	long timeDiff ;
	if (argc != 3)  
	{ 
		fprintf(stderr,  
			"usage: %s host url\n", argv[0]); 

		exit (-1);
	}		

	/*  Remember what our command line arguments refer to  */  

	server = argv[1]; 
	urlname= argv[2]; 


	/* Create client "handle" used for calling GETHTTPPROG on the 
	* server designated on the command line. We tell the RPC  
	* package to use the "tcp" protocol when contacting 
	* the server. 
	*/  

	cl = clnt_create(server, GETHTTPPROG, GETHTTPVERS, "tcp"); 
	if (cl == NULL)  
	{ 

		/*
		* Couldn't establish connection with server.  
		* Print error message and die. 
		*/  

		clnt_pcreateerror (server); 

		exit(-1);
	}

	/* Call the remote procedure "gethttprequest" on the server */ 
	struct timeval tStartTime,tStopTime;

	gettimeofday(&tStartTime,NULL); // Start Timer

	result = gethttprequest_1(&urlname, cl);  // Make RPC to server

	gettimeofday(&tStopTime,NULL);  // Stop timer

	if (result == NULL)  
	{ 
		/* An error  occurred while calling the server.  
		* Print error message and die.
		*/  

		clnt_perror (cl, server); 

		exit (-1);
	} 

	/* Okay, we successfully called the remote procedure. */  
	if (result->err !=0) 
	{	
		/*
		* A remote system error occurred.
		* Print error message and die.
		*/
		exit (result->err);
	} 

	/* Successfully got a directory listing. Print it out. */  
	//    printf("Return from proxy server: %s!\n", server); 

	timeDiff= TimeDiff(&tStartTime,&tStopTime); // Calculate Timer Diff

	printf("%ld:%d:%ld:\n",strlen(result->urlData_res_u.urldata->data),result->urlData_res_u.urldata->fromCache,timeDiff);

	//printf("URL Data: %s",result->urlData_res_u.urldata->data);

	FILE *fp;

	printf("\n %p",fp);
	fp = fopen("Log.txt","aw");
	fprintf(fp,"%s:%ld:%d:%ld:\n",urlname,strlen(result->urlData_res_u.urldata->data),result->urlData_res_u.urldata->fromCache,timeDiff);
	fclose(fp);

	exit ((int)timeDiff); 
} 

