// misc field
struct Node {
	void* v;
	int extinf;
	struct Node* p;
	struct Node* l;
	struct Node* r;
};
typedef struct Node Node;
// void freeTree(struct Node node){
	// if(node.v) free(node.v)
// }

void swap(int* const p1, int* const p2){
	int temp = *p1;
	*p1 = *p2;
	*p2 = temp;
}
int strlen(char* str){
	int len=0;
	while(str[++len]);
	return len;
}
char* char2bin(unsigned char c){
	const int N = sizeof(c);
	static char res[sizeof(c)*8+1]={};
	for(int i=N*8-1; i>=0; --i){
		res[i] = (c&1)+48;
		c>>=1;
	}
	res[sizeof(c)*8] = 0;
	return res;
}
char* int2bin(unsigned int n){
	const int N = sizeof(n);
	static char res[sizeof(n)*8+1]={};
	for(int i=N*8-1; i>=0; --i){
		res[i] = (n&1)+48;
		n>>=1;
	}
	res[sizeof(n)*8] = 0;
	return res;
}
char* int2binf(unsigned int n){
	const int N = sizeof(n);
	static char res[sizeof(n)*9+1]={};
	for(int i=N*8-1, j=N*8+N-1; i>=0; --i, --j){
		res[j] = (n&1)+48;
		if(! (i%8)){
			res[--j] = ' ';
		}
		n>>=1;
	}
	res[sizeof(n)*9] = 0;
	return res+1;
}
char* int2binl(unsigned int n, int l){
	return int2bin(n)+(sizeof(n)*8-l);
}
char* int2chars(unsigned int n){
	static char chs[sizeof(int)] = {};
	for(int i=sizeof(int)-1; i>=0; --i){
		chs[i] = (char) n & 0xff;
		n >>= 8;
	}

	// static int N = 0;
	// N = n;
	// char* chs = (char*)&N;
	// 
	// char tmp = chs[0];
	// chs[0] = chs[3];
	// chs[3] = tmp;
	// 
	// tmp = chs[1];
	// chs[1] = chs[2];
	// chs[2] = tmp;

	return chs;
}
