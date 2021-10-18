#include "stdio.h"

// #include "bitStore.c"

#include "../cc.h"

#define DEBUG

int main(int argc, char** argv){
	if(argc != 3){
		printf("Error: invalid arguments.\n");
		printf("Usage: %s <inputFile> <outputFile>\n", argv[0]);
		return 1;
	}

	FILE* infp = fopen(argv[1], "rb");
	if(!infp){
		printf("Error: Invalid input.\n");
		return 1;
	}

	// load
	int N = fsizef(argv[1]);
	char* content = (char*) malloc(N*sizeof(char));
	for(int i=0; i<N; ++i){
		content[i] = fgetc(infp);
	}
	fclose(infp);

	// haffCode
	HaffInfo* haffs = 0;
	int haffC = haffmanCode(&haffs, content, N);
	HaffInfo* haffMap = (HaffInfo*) malloc(0xff*sizeof(HaffInfo));
	for(int i=0; i<haffC; ++i){
		HaffInfo haff = haffs[i];
		haffMap[(int)haff.ch] = haff;
		#ifdef DEBUG
			printf("Node%d=%s, blen=%d, char=%c\n", i, char2bin(*haff.code), haff.depth, haff.ch);
		#endif
	}

	// transcode
	int tLength = 0;
	for(int i=0; i<N; ++i){
		tLength += haffMap[(int)content[i]].depth;
	}
	bitStore_setMaxBits(tLength);
	for(int i=0; i<N; ++i){
		HaffInfo n = haffMap[(int)content[i]];
		// int code = n.code << (sizeof(int)*8 - n.depth /*- (8-n.depth)*/);
		// TODO fix code
		bitStore_store(n.code, n.depth);
	}
	char* result = bitStore_popAll();

	#ifdef DEBUG
		printf("Result:\n");
		for(int i=0, n=tLength/8+1; i<n; ++i){
			printf("%s\n", char2bin(result[i]));
		}
		printf("\n");
	#endif


	FILE* outfp = fopen(argv[2], "a+");
	if(!outfp){
		printf("Error: Invalid output.\n");
		return 1;
	}

	

	fclose(outfp);

	return 0;
}
