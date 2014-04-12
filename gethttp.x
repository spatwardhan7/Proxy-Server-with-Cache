/* gethttp.x: Remote message printing protocol */ 
/* Written according to RPCGEN standards for making RPC */

const MAXNAMELEN = 1024;
/*
const URLNOTFOUND = 1;
const CONNECTIONERROR = 2;
const MEMORY_ERROR = 3;
*/

typedef string url<MAXNAMELEN>;
typedef struct urlInfo *urlData;

struct urlInfo {
	string data<>;
	int fromCache;
};

union urlData_res switch (int err) {
	case 0:
		urlData urldata;
	default:
		void;
};
 
program GETHTTPPROG  
{  
    version GETHTTPVERS  
    {  
        urlData_res
			GETHTTPREQUEST(url) = 1;  
    } = 1;  
} = 76;  