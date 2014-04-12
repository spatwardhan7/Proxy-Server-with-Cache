/*
* cache.cpp: Implements 3 Caching mechanisms for holding URL content
* Size of Cache can be changed via #define CACHE_SIZE_KB
* Cache replacement policy can be changed by changing cachePolicy variable
*/
#include <map>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unistd.h>
#include <sys/select.h>
#include <curl/curl.h>
#include <iostream>
#include <list>
#include <rpc/rpc.h>
#include "gethttp.h"
#include <vector>
using namespace std;

#define CACHE_SIZE_KB 700

enum cachereplacement {LRU , FIFO , RAND };

#define CACHE_BYTES  1024 * CACHE_SIZE_KB

struct wd_in {
	size_t size;
	size_t len;
	char *data;
};

int cacheEntryCount = 0;
int cachePolicy = LRU;

long cacheUsage = 0;

extern "C" void lruUpdate(char *);
typedef std::list <char *> list_type;
typedef pair<std::string, char *> cache_insert_type;
typedef pair<std::string, list<char *>::iterator> lrucache_insert_type;
typedef std::map <std::string, list<char *>::iterator> lru_map_type;
typedef std::map <std::string, char *> cache_type;

list_type lru_age_list;
list_type fifo_List;
lru_map_type lru_cache;
cache_type main_cache;
list_type rand_list;	

/*
* cacheCheck : Search for content using url in main cache
*
*/
extern "C" char * cacheCheck(char *url)
{

	cache_type::iterator lookup;
	char *K;
	char *retval;

	lookup = main_cache.find(url);
	if(lookup == main_cache.end()) // Cache Miss
		retval = NULL;
	else
	{
		K = (char *)malloc((strlen(url) + 1) * sizeof(char));
		strcpy(K, url);

		retval = lookup->second;

		if(cachePolicy == LRU)
		{
			/*
			* In case of LRU cache, call LRU update to update LRU cache datastructures
			*/
			lruUpdate(K);
		}
		else
		{
			//Do nothing
		}
	}
	return retval;
}

extern "C" int get_random()
{
	srand((unsigned int) time (NULL));
	int target = rand();
	target = target % main_cache.size() ;
	return target;
}

extern "C" void lruUpdate(char *K)
{
	lru_map_type::iterator lru_it;
	lru_it = lru_cache.find(K);

	if(lru_it == lru_cache.end())
	{
		printf("Error: %s !\n",K);
		exit(0);
	}

	lru_age_list.erase(lru_it->second);
	lru_age_list.push_back(K);
	list<char *>::iterator it = --lru_age_list.end();
	lru_cache.erase(lru_it);
	lru_cache.insert(lrucache_insert_type(K, it));
}

extern "C" char * cacheUpdate(char *url, struct wd_in *rpc_struct, void (*CacheFree)(void *))
{
	char *K, *V;
	long newEntrySize, expectedUsage;
	int removed;
	K = (char *)malloc((strlen(url) + 1) * sizeof(char));
	strcpy(K,url);
	V = (char *)malloc((strlen(rpc_struct->data) + 1) * sizeof(char));

	V = strndup(rpc_struct->data, rpc_struct->len);
	newEntrySize = ((rpc_struct->len) * sizeof(char)) + (strlen(K) * sizeof(char)) ;
	if(cachePolicy == LRU)
	{
		removed = 0;
		expectedUsage = cacheUsage + newEntrySize;
		while(expectedUsage >= CACHE_BYTES)
		{
			cacheEntryCount--;

			list_type::iterator lru_iterator= lru_age_list.begin();      

			cache_type::iterator cache_iterator;
			lru_map_type::iterator itm;

			cache_iterator = main_cache.find(*lru_iterator);
			if(cache_iterator == main_cache.end())
			{
				printf("Error: %s\n",*lru_iterator);
				exit(0);
			}

			itm = lru_cache.find(*lru_iterator);
			if(itm == lru_cache.end())
			{
				printf("Error: %s\n",*lru_iterator);
				exit(0);
			}

			removed = (strlen(cache_iterator->first.c_str()) + strlen((char *)cache_iterator->second)) * sizeof(char);
			main_cache.erase(cache_iterator);
			lru_age_list.erase(lru_iterator);
			lru_cache.erase(itm); 
			CacheFree((void *)cache_iterator->first.c_str());
			CacheFree(cache_iterator->second);

			cacheUsage -= removed;
			expectedUsage = cacheUsage + newEntrySize;
		}

		cacheUsage += newEntrySize ;

		cacheEntryCount++;

		lru_age_list.push_back(K);
		list_type::iterator it = --lru_age_list.end();
		lru_cache.insert(lrucache_insert_type(K, it));

		main_cache.insert(cache_insert_type(K,(char *)V));

	}

	else if(cachePolicy == FIFO)
	{
		expectedUsage = cacheUsage + newEntrySize;
		while(expectedUsage >= CACHE_BYTES)
		{
			list_type::iterator fifo_iterator;
			fifo_iterator = fifo_List.begin();

			cache_type::iterator cache_iterator;
			cache_iterator = main_cache.find(*fifo_iterator);

			if(cache_iterator == main_cache.end())
			{
				printf("Error: %s\n",*fifo_iterator);
				exit(0);
			}
			removed = (strlen(cache_iterator->first.c_str()) + strlen((char *)cache_iterator->second)) * sizeof(char);
			main_cache.erase(cache_iterator);
			fifo_List.pop_front();

			CacheFree((void *)cache_iterator->first.c_str());
			CacheFree(cache_iterator->second);

			cacheUsage -= removed;
			expectedUsage = cacheUsage + newEntrySize;
		}

		cacheUsage += newEntrySize; 
		main_cache.insert(cache_insert_type(K,(char *)V));
		fifo_List.push_back(K);
	}

	else if(cachePolicy == RAND) 
	{
		rand_list.push_back(K);

		expectedUsage = cacheUsage + newEntrySize;
		while(expectedUsage >= CACHE_BYTES)
		{
			cache_type::iterator cache_iterator = main_cache.begin();			
			int target = get_random();
			list_type::iterator randitr = rand_list.begin() ;
			std::cout<<"URL = "<< *randitr<<endl;

			for(int i = 0 ; i < target ; i++)
				randitr++;
			cache_iterator = main_cache.find(*randitr);
			if (cache_iterator == main_cache.end()) 
				std::cout <<"Error " << std::endl; 
			removed = (strlen(cache_iterator->first.c_str()) + strlen((char *)cache_iterator->second) ) * sizeof(char);

			main_cache.erase(cache_iterator);
			rand_list.erase (randitr);
			CacheFree((void *)cache_iterator->first.c_str());
			CacheFree(cache_iterator->second);
			cacheUsage -= removed;
			expectedUsage = cacheUsage + newEntrySize;	
		}

		cacheUsage += newEntrySize;
		main_cache.insert(cache_insert_type(K,(char *)V));
	}
} 
