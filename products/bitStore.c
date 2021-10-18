#include "malloc.h"
#include "sys/stat.h"

// #include "../misc.c"

#define DEBUG

// typedef unsigned char Byte

char* bitStore = NULL;
int bitStore_maxLen = 4;
int bitStore_nowBPos = 0;	// < maxLen*8

void bitStore_init(){
	bitStore = malloc(bitStore_maxLen*sizeof(char));
}

void bitStore_setMaxBits(int blen){
	bitStore_maxLen = blen/8 + 1;
	if(bitStore){
		bitStore = realloc(bitStore, bitStore_maxLen*sizeof(char));
	}else{
		bitStore_init();
	}
}

void bitStore_store(const void* data, int blen){
	if(!bitStore) bitStore_init();
	#ifdef DEBUG
		printf("bitStore_store.data=%s, blen=%d\n", char2bin(*(char*)data), blen);
	#endif
	int bRest = bitStore_nowBPos%8;
	int dataBRest = 8 - blen%8;
	int sN = bitStore_nowBPos/8;
	int N = blen / 8;
	// fix data
	((char*)data)[N] &= 0xff << dataBRest;
	#ifdef DEBUG
		printf("Fixed data at %d=%s\n", N, char2bin(((char*)data)[N]));
	#endif
	// alloc for storing new fixed data
	int nN = N;
	short overflowed = 0;
	char overflow = 0;
	if(dataBRest < bRest){
		overflowed = 1;
		++nN;

		// calc overflow
		int oflen = (blen - (8-bRest));
		overflow = (((char*)data)[N] >> dataBRest) & (0xff >> (8-oflen));
		overflow <<= 8-oflen;
		#ifdef DEBUG
			printf("Overflow detected: %s\n", char2bin(overflow));
		#endif
	}
	char* nData = (char*) malloc((nN+1)*sizeof(char));
	for(int i=0; i<=nN; ++i){
		nData[i] = 0;
	}

	// offset into nData
	if(overflowed) nData[nN] = overflow;
	int dataLastPos = N;
	for(; dataLastPos>=0; --dataLastPos){
		char I = ((char*)data)[dataLastPos];
		char lI = dataLastPos? ((char*)data)[dataLastPos-1]: 0;
		nData[dataLastPos] = (I >> bRest) | (dataLastPos? (lI & (0xff >> bRest)): 0);
		#ifdef DEBUG
			printf("after: nData[%d]=%d\n", dataLastPos, nData[dataLastPos]);
		#endif
	}

	// put them into mainline
	for(int i=sN, n=sN+nN+1; i<n; ++i){
		bitStore[i] |= nData[i-sN];
		#ifdef DEBUG
			printf("bitStore[%d]=%s\n", i, char2bin(bitStore[i]));
		#endif
	}

	// update variables
	bitStore_nowBPos += blen;
	if(bitStore_nowBPos > bitStore_maxLen*8){
		#ifdef DEBUG
			printf("Warning: bitStore got an overflow.\n");
		#endif
	}
}

char* bitStore_dup(){
	if(!bitStore) return NULL;
	int N = bitStore_nowBPos / 8 + 1;
	char* res = (char*) malloc((N+1)*sizeof(char));
	for(int i=0; i<N; ++i){
		res[i] = bitStore[i];
	}
	return res;
}

char bitStore_pop(){
	if(bitStore_nowBPos){
		int npos = bitStore_nowBPos/8;
		int pos = bitStore_nowBPos%8;
		if(!pos){
			if(!npos) goto F;
			--npos;
		}else --pos;
		--bitStore_nowBPos;
		return (bitStore[npos] &= (0xff << (8-pos))) << pos;
	}
	F:
	#ifdef DEBUG
		printf("bitStore: pop failed.\n");
	#endif
	return 0;
}

char* bitStore_popAll(){
	char* res = bitStore_dup();
	for(int i=0, n=bitStore_nowBPos/8; i<n; ++i){
		bitStore[i] = 0;
	}
	bitStore_nowBPos = 0;
	return res;
}


int fsizef(char* file){
	struct stat statbuf;
	stat(file, &statbuf);
	return statbuf.st_size;
}
