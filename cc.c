#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "malloc.h"

#include "misc.c"
#include "cc.h"
#include "products/bitStore.c"

#define Quicksort 1
#define HaffmanCode 2
#define ImgFill 3
#define FindPath 4
#define UsingBfs 0x01

#define TEST ImgFill
#define CCC_DEBUG

// Algorithm field.

/* slowsort */
void quicksort(int* arr, int l, int r);
inline void quicksort(int* arr, int l, int r){
	if(l<r){
		int R = arr[r];
		int i = l;
		int j = l;
		for(; j<r; ++j){
			if(arr[j]<R){
				swap(&arr[i], &arr[j]);
				i++;
			}
		}
		// move R to center
		for(int k=r; k>i; --k){
			arr[k] = arr[k-1];
		}
		arr[i] = R;
		#ifdef CCC_DEBUG
			printf("ori: l=%d, r=%d\nl=%d, r=%d\n", l, r, i, j);
		#endif
		// determine mid which is i
		int mid = i;
		quicksort(arr, l, mid-1);
		quicksort(arr, mid+1, r);
	}
}


int getHaffCount(int v){
	return v>>8;
}
// TODO optimise

// struct HaffInfo {
	// char ch;
	// char* code;	// bitStore
	// long int count;
	// int depth;
// };
// typedef struct HaffInfo HaffInfo;

// int buildhaffinfo(HaffInfo* target, Node root, int code, int i, int dep);
// use bitStore instead
int buildhaffinfo(HaffInfo* target, Node root, int i, int dep);
inline int buildhaffinfo(HaffInfo* target, Node root, int i, int dep){
	#ifdef CCC_DEBUG
		printf("v: %d, p: %p, l: %p, r: %p, depth: %d\n", *(int*)root.v,  &root, root.l, root.r, dep);
	#endif
	if(root.l && root.r){
		char z=0;
		bitStore_store(&z, 1);
		i = buildhaffinfo(target, *root.l, i, dep+1);
		bitStore_pop();
		z=0x80;
		bitStore_store(&z, 1);
		i = buildhaffinfo(target, *root.r, i, dep+1);
		bitStore_pop();
	}else{
		target[i].ch = (char)(*(int*)root.v)&0xff;
		target[i].code = bitStore_dup();
		printf("CODE=%s\n", char2bin(*target[i].code));
		target[i].depth = dep;
		return i+1;
	}
	return i;
}
HaffInfo* buildHaffInfo(Node root, int nodeN){
	HaffInfo* haffs = (HaffInfo*) malloc(nodeN*sizeof(HaffInfo));
	buildhaffinfo(haffs, root, 0, 0);
	return haffs;
}
int haffGetV(Node* n){
	return n->extinf&1?getHaffCount(*(int*)n->v):*(int*)n->v;
}
const int MAX = 2147483647;
void haffSelect(Node* nodes, int len, Node**const targ1, Node**const targ2){
	int v1, v2;
	v1 = v2 = MAX;
	Node *nodei1=0, *nodei2=0;
	for(int i=0; i<len; ++i){
		Node *n = &nodes[i];
		if(n && n->extinf&2){
			int v = haffGetV(n);
			if(v<v1 && v2!=MAX){
				nodei1 = n;
				v1 = v;
			}else if(v<v2){
				nodei2 = n;
				v2 = v;
			}
		}
	}
	*targ1 = nodei1;
	*targ2 = nodei2;
}
int haffmanCode(HaffInfo**const target, char* input, int strLength){
	// count all chars
	int chars[0xff] = {};
	int len = 0;
	for(int i=0; i<strLength; ++i){
		int j = (int)input[i];
		if(chars[j]==0) ++len;
		++chars[j];
	}

	// merge counts
	int haffchs[len];
	for(int i=0, j=0; i<0xff; ++i){
		if(chars[i]!=0){
			// pack as: count*24+ch*8=32
			haffchs[j++] = (chars[i]<<8)|i;
		}
	}

	// sort
	quicksort(haffchs, 0, len-1);

	// build tree

	int sN = 2*len-1;
	Node* nodes = (Node*) malloc(sN*sizeof(Node));
	// extinf: 000000 TOP PACKED
	nodes[0].v = &haffchs[0];
	nodes[0].extinf = 3;
	int np=0;
	for(int i=1, N=len; i<N; ++i){
		Node *r = &nodes[++np];
		Node *p = &nodes[++np];
		r->v = &haffchs[i];
		r->extinf = 3;

		Node *n1, *n2;
		n1 = n2 = 0;
		haffSelect(nodes, sN, &n1, &n2);
		if(n1==0 || n2==0){
			#ifdef CCC_DEBUG
				printf("haffSelect got a NULL.\n");
			#endif
		}

		n1->p = n2->p = p;
		p->extinf = 2;	// at top, not a leaf
		n1->extinf &= 1;
		n2->extinf &= 1;

		int n1v, n2v;
		int pv = (n1v=haffGetV(n1)) + (n2v=haffGetV(n2));
		p->v = &pv;
		#ifdef CCC_DEBUG
			printf("&p.v=%p, p.v=%d\n", p->v, *(int*)p->v);
		#endif

		// determine left child
		// haffSelect returns an ordered array
		// if(n1v < n2v){
			// p->l = r;
			// p->r = l;
		// }else{
			// p->l = l;
			// p->r = r;
		// }
		p->l = n1;
		p->r = n2;
	}

	#ifdef CCC_DEBUG
		int zN = 0;
		for(int i=0; i<sN; ++i){
			if(nodes[i].extinf&2) ++zN;
			if(!nodes[i].v){
				printf("Top check, NULL at %d\n", i);
			}
		}
		printf("Tops=%d\n", zN);

		printf("np=%d\n", np);
	#endif

	// Build HaffInfo
	// prepare bitStore
	bitStore_setMaxBits(len);
	HaffInfo* res = buildHaffInfo(nodes[np], len);
	*target = res;
	return len;
}


int** imgFillBook = NULL;
int** imgFillData = NULL;
int dataW=0, dataH=0;
int iFdirection[][2]={{1,0}, {-1,0}, {0, 1}, {0, -1}};
int grey(int color){
	int sum=0;
	while(color){
		sum	+= color&0xff;
		color >>= 8;
	}
	return sum/3;
}
int iFinRange(int color, int targetColor, int range){
	color = grey(color);
	targetColor = grey(targetColor);
	return (targetColor > color-range) && (targetColor < color+range);
}
int iFvalidPoint(int x, int y){
	return x>=0 && x<dataW && y>=0 && y<dataH;
}
void iFdfs(int x, int y, int tcolor, int range, int paintColor){
	if(iFvalidPoint(x, y) && !imgFillBook[x][y] && iFinRange(imgFillData[x][y], tcolor, range)){
		imgFillBook[x][y] = 1;
		imgFillData[x][y] = paintColor;
		for(int i=0; i<4; ++i){
			iFdfs(x+iFdirection[i][0], y+iFdirection[i][1], tcolor, range, paintColor);
		}
	}
}
void imgFillDfs(int** data, Point size, Point p, int range, int paintColor){
	imgFillData = data;
	int Len = size.x*size.y;
	dataW = size.x;
	dataH = size.y;
	int arrsiz;
	imgFillBook = (int**) malloc(arrsiz = Len*sizeof(int) + dataH*sizeof(int*));
	for(int i=0; i<arrsiz; ++i){
		((char*)imgFillBook)[i] = 0;
	}
	for(int i=0; i<dataH; ++i){
		imgFillBook[i] = &((int*)imgFillBook)[dataH+dataW*i];
	}

	int targetColor = data[p.x][p.y];
	iFdfs(p.x, p.y, targetColor, range, paintColor);
}
void imgFillBfs(int** data, Point size, Point p, int range, int paintColor){
	#ifdef CCC_DEBUG
		printf("imgFill: Using BFS.\n");
	#endif
	imgFillData = data;
	int Len = size.x*size.y;
	dataW = size.x;
	dataH = size.y;
	int arrsiz;
	imgFillBook = (int**) malloc(arrsiz = Len*sizeof(int) + dataH*sizeof(int*));
	for(int i=0; i<arrsiz; ++i){
		((char*)imgFillBook)[i] = 0;
	}
	for(int i=0; i<dataH; ++i){
		imgFillBook[i] = &((int*)imgFillBook)[dataH+dataW*i];
	}

	int targetColor = data[p.x][p.y];
	// iFbfs(p.x, p.y, targetColor, range, paintColor);

	Point *que = (Point*) malloc(Len*sizeof(Point));
	for(int i=0; i<Len; ++i){
		que[i].x = que[i].y = que[i].extinf = 0;
	}
	que[0] = p;
	int head=0, tail=1;


	// int lastX = p.x, lastY = p.y;
	// int notOK = 1;
	for(int step = 0; step==0 || que[tail].extinf==step-1; ++step){
		// TODO every point
		// enum every point
		int backupTail = tail;
		for(int i=head; i<tail; ++i){
			int oriX = que[i].x;
			int oriY = que[i].y;
			int oriStep = que[i].extinf;
			for(int dir=0; dir<4; ++dir){
				int nowX = oriX + iFdirection[dir][0];
				int nowY = oriY + iFdirection[dir][1];

				if(iFvalidPoint(nowX, nowY) && imgFillBook[nowX][nowY]==0 && iFinRange(imgFillData[nowX][nowY], targetColor, range)){
					imgFillBook[nowX][nowY] = 1;
					imgFillData[nowX][nowY] = paintColor;
					// notOK = 1;
					Point p;
					p.x = nowX;
					p.y = nowY;
					p.extinf = oriStep+1;
					que[tail++] = p;
				}
			}
		}
		// we do not need to record every step, so just skip it
		head = backupTail;
	}
}



// Test field

#ifndef TESTER_NOMAIN
	int main(void){
		srand((unsigned)time(NULL));

		#if TEST==Quicksort
			int len = 10;
			// int arr[]={47, 29, 71, 99, 78, 19, 24, 47, 40, 60};
			int arr[len];
			for(int i=0; i<len; ++i){
				arr[i] = rand()%0xff;
			}
			quicksort(arr, 0, len-1);
			for(int i=0; i<len; ++i){
				printf("%d, ", arr[i]);
			}
			printf("\n");
		#elif TEST==HaffmanCode
			char* str="aaaaaabbbbccddd";
			HaffInfo* haffs = 0;
			int len = haffmanCode(&haffs, str, strlen(str));
			for(int i=0; i<len; ++i){
				HaffInfo haff = haffs[i];
				printf("Node%d: %c=%s\n", i, haff.ch, char2bin(*haff.code));
			}
		#elif TEST==ImgFill
			// input an array as an image
			// TODO input an image
			FILE* fp = fopen("./imgFill.bmp", "rb");
			if(!fp){
				printf("Error: Cannot open file ./imgFill.bmp\n");
				return 2;
			}

			// read header
			int dataOffset = 0;
			int bmpWidth=0, bmpHeight=0;
			fseek(fp, 0xaL, 0);
			fread((char*)&dataOffset, 4, 1, fp);
			fseek(fp, 0x12L, 0);
			fread((char*)&bmpWidth, 4, 1, fp);
			// fseek(fp, 0x16L, 0);
			fread((char*)&bmpHeight, 4, 1, fp);
			#ifdef CCC_DEBUG
				printf("BMP: %d, %d, %d\n", dataOffset, bmpWidth, bmpHeight);
			#endif

			int imgSiz = bmpWidth * bmpHeight;
			int** imgData = (int**)malloc(imgSiz * sizeof(int) + bmpHeight*sizeof(int*));
			for(int i=0; i<bmpHeight; ++i){
				imgData[i] = &((int*)imgData)[bmpHeight+bmpWidth*i];
			}

			fseek(fp, dataOffset, 0);

			// read data
			for(int i=0; i<bmpHeight; ++i){
				for(int j=0; j<bmpWidth; ++j){
					imgData[i][j] =
						( fgetc(fp) << 16 ) +
						( fgetc(fp) << 8 ) +
						fgetc(fp);
					
				}
			}

			fclose(fp);

			#ifdef CCC_DEBUG
				printf("Image data:\n");
				for(int i=0; i<bmpHeight; ++i){
					for(int j=0; j<bmpWidth; ++j){
						printf("%c ",imgData[i][j]?'#':'.');
					}
					printf("\n");
				}
			#endif

			
			Point siz;
			siz.x = bmpWidth;
			siz.y = bmpHeight;
			Point p;
			p.x = 10;
			p.y = 10;

			#if UsingBfs
				imgFillBfs(imgData, siz, p, 1, 0);
			#else
				imgFillDfs(imgData, siz, p, 1, 0);
			#endif

			#ifdef CCC_DEBUG
				printf("Image data:\n");
				for(int i=0; i<bmpHeight; ++i){
					for(int j=0; j<bmpWidth; ++j){
						printf("%c ",imgData[i][j]?'#':'.');
					}
					printf("\n");
				}
			#endif

			
		#else
			printf("Warning: Invalid Tester.\n");
		#endif

		return 0;
	}
#endif
