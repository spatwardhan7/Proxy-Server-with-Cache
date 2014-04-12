Proxy-Server-with-Cache
=======================
In this project, I have implemented a RPC based proxy server and a client. 
The client makes a RPC to the server to request a URL. 
The server opens a socket to the requested url using libcurl to download content and presents it to the client. 
I have also implemented a simple caching mechanism using 3 replacement policies Least Recently Used (LRU), Random and First in First out (FIFO) for caching URL content. 

Structure:

rhttp.c : This is the client code. A call to the Remote Procedure is made from this code

gethttp_proc.c: This is the server code. This file implements the function which is being called by rhttp.c. 
                This File is makes use of libcurl to request the required url. It is also linked with the cache file which implements all caching techniques.

gethttp.x: This is the declaration of the remote procedure call. It is used by rpcgen to generate client and service stubs. 

cache.cpp: Contains implementation of all caches which have been implemented. 
                    It contains a #define for defining size of the cache to be used. 
					Please vary this size to set required cache size. 
					It also has a variable which can be set to pick current cache implementation policy. 
					Please assign a value from any of the enums 




Compiling: 

make

Executing: 
To execute server:  ./server

To execute client: ./client <machineName> <url>
 example:              ./client localhost www.google.com

Running the script: 

> bash ./script.sh <MethodNum> <MachineName>

eg: bash ./script.sh 1  localhost

Script file depends on urlList.Txt file. Please make sure both are located same directory to ensure correct working of script.  
