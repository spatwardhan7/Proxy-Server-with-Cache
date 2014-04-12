/*
* gethttp_proc.c: Server code which receives the request from client via RPC
* Server code uses libcurl to get content from required website. 
* Server code also makes cache.cpp to handle caching mechanism
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <curl/curl.h>
#include <rpc/rpc.h>
#include "gethttp.h"


struct wd_in {
	size_t size;
	size_t len;
	char *data;
};

/* This function is registered as a callback with CURL.  As the data
from the requested webpage is returned in chunks, write_data is
called with each chunk.  */
static size_t write_data(void *buffer, size_t size, 
	size_t nmemb, void *userp) {
		struct wd_in *wdi = userp;

		while(wdi->len + (size * nmemb) >= wdi->size) {
			/* check for realloc failing in real code. */
			wdi->data = realloc(wdi->data, wdi->size*2);
			wdi->size*=2;
		}

		memcpy(wdi->data + wdi->len, buffer, size * nmemb);
		wdi->len+=size*nmemb;

		return size * nmemb;
}

void CacheFree(void *ptr)
{
	xdr_free(xdr_urlData_res, ptr);
}

urlData_res * gethttprequest_1_svc(url *urlname, struct svc_req *rqst)
{
	CURL *curl;
	CURLcode res;
	struct wd_in wdi;
	//char *url = "http://www.google.com";
	static urlData_res returnData; 	
	memset(&wdi, 0, sizeof(wdi));
	char *LookupCache;

	LookupCache = (char*)cacheCheck(*urlname);
	printf("LookcupCache = %s\n",LookupCache);

	if( LookupCache == NULL )
	{ 
		/* Get a curl handle.  Each thread will need a unique handle. */
		curl = curl_easy_init();

		if(NULL != curl) 
		{
			wdi.size = 1024;
			/* Check for malloc failure in real code. */
			wdi.data = malloc(wdi.size);

			/* Set the URL for the operation. */
			curl_easy_setopt(curl, CURLOPT_URL, *urlname);

			/* "write_data" function to call with returned data. */
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

			/* userp parameter passed to write_data. */
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &wdi);

			/* Actually perform the query. */
			res = curl_easy_perform(curl);

			/* Check the return value and do whatever. */

			/* Clean up after ourselves. */
			curl_easy_cleanup(curl);

			returnData.urlData_res_u.urldata = (urlInfo*) malloc (sizeof(urlInfo*));
			returnData.urlData_res_u.urldata->data = strndup(wdi.data,wdi.len);
			returnData.urlData_res_u.urldata->fromCache = 0;

			cacheUpdate(*urlname,&wdi,&CacheFree);

		}
		else 
		{
			fprintf(stderr, "Error: could not get CURL handle.\n");
			exit(EXIT_FAILURE);
		}

	}
	else 
	{
		returnData.urlData_res_u.urldata = (urlInfo*) malloc (sizeof(urlInfo*));
		returnData.urlData_res_u.urldata->data = strdup(LookupCache);
		returnData.urlData_res_u.urldata->fromCache = 1;
	}

	/* Now wdi.data has the data from the GET and wdi.len is the length
	of the data available, so do whatever. */

	/* cleanup wdi.data buffer. */
	free(wdi.data);

	return &returnData;
}
