/*
	Author: He Chen
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SYMBOLS 256
#define MAX_LEN     50

struct tnode
{
    struct  tnode* left; /*used when in tree*/
    struct  tnode*right; /*used when in tree*/  
    int     isleaf;
    unsigned char symbol;
};

/*global variables*/
struct tnode* root=NULL; /*tree of symbols*/
int precomputed_reverse[16] = { /*lookup table for reverse bits*/
	0, 8, 4, 12, 2, 10, 6, 14,
	1, 9, 5, 13, 3, 11, 7, 15
};

/*
    @function   talloc
    @desc       allocates new node 
*/
struct tnode* talloc()
{
    struct tnode* p=(struct tnode*)malloc(sizeof(struct tnode));
    if(p!=NULL)
    {
        p->left=p->right=NULL;
        p->symbol=0;
		p->isleaf=0;
    }
    return p;
}

/*
    @function build_tree
    @desc     builds the symbol tree given the list of symbols and code.h
	NOTE: alters the global variable root that has already been allocated in main
*/
void build_tree(FILE* fp)
{
	unsigned char symbol;
	char	strcode[MAX_LEN], c;
	int		i, len, items_read;
	struct	tnode* curr=NULL;

	while(!feof(fp))
	{
		/*need to take of '\n' in the code.txt*/
		/*read in the symbol and code*/
		symbol = getc(fp);
		c = getc(fp);
		if (c == EOF) /*this is necessary, since symbol may read in 'EOF' as 255*/
			break;
		fscanf(fp,"%s",strcode);
		getc(fp);
		
		curr=root;
		len=strlen(strcode);
		for(i=0;i<len;i++)
		{
			/*create the tree as you go*/
			/* if it's '1', build a right child, or left child if it's '0' */
			if (strcode[i] == '1') {
				if (curr->right == NULL)
					curr->right = talloc();
				curr = curr->right;
			}
			else {
				if (curr->left == NULL)
					curr->left = talloc();
				curr = curr->left;
			}
		}
		/*assign code*/
		curr->isleaf=1;
		curr->symbol=symbol;
		printf("inserted symbol:%d\n",symbol);
	}
}

/*
	function reverse bits using a lookup table
*/
unsigned char reverse_bit(unsigned char c)
{
	const int kWordSize = 4;
	const int kBitMask = 0xF;
	return precomputed_reverse[c & kBitMask] << kWordSize |
		   precomputed_reverse[(c >> kWordSize) & kBitMask]; /* &kBitMask is not necessary here */
}

/*
	function decode
*/
void decode(long count, FILE* fin,FILE* fout)
{
	int i, b;
	unsigned char c;
	struct tnode* curr=root;
	while(!feof(fin)) { /*can not use (c=getc(fin))!=EOF, since it's possible to have c==-1 (255255)*/
		c = getc(fin);
		/* if (feof(fin)) break; get rid of the last 'EOF' --> not necessary, since 'count' will do the job */
		/* traverse the tree and print the symbols only if you encounter a leaf node */
		/* reverse the bits, (not necessary), could use "c>>i & 0x1" instead */
		c = reverse_bit(c);
		for (i=0; i<8 && count>0; i++) {
			b = c&0x1;
			c >>= 1;
			curr = (b == 1) ? curr->right : curr->left;
			if (curr->isleaf) {
				putc(curr->symbol, fout);
				curr = root;
				count--;
			}
		}
	}
}
/*
	@function freetree
	@desc	  cleans up resources for tree
*/

void freetree(struct tnode* root)
{
	if(root==NULL)
		return;
	freetree(root->right);
	freetree(root->left);
	free(root);
}
int main()
{
	const char* IN_FILE="encoded.txt";
	const char* CODE_FILE="code.txt";
	const char* OUT_FILE="decoded.txt";
	FILE* fout;
	FILE* fin;
	long count;
	/*allocate root*/
	root=talloc();
	fin=fopen(CODE_FILE,"r");
	/*read the total count of characters*/
	fscanf(fin, "%ld", &count);
	/*build tree*/
	build_tree(fin);
	fclose(fin);

	/*decode*/
	fin=fopen(IN_FILE,"r");
	fout=fopen(OUT_FILE,"w");
	decode(count,fin,fout);
	fclose(fin);
	fclose(fout);
	/*cleanup*/
	freetree(root);
	getchar();
	return 0;
}
