/*
	Author: He Chen
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_SYMBOLS 256
#define MAX_LEN     50

struct tnode
{
    struct  tnode* left; /*used when in tree*/
    struct  tnode*right; /*used when in tree*/  
    struct  tnode*parent;/*used when in tree*/
    struct  tnode* next; /*used when in list*/ 
    float   freq;
    int     isleaf;
    unsigned char symbol;
};


/*global variables*/
int current_bit = 0; /*used in writing bits*/
unsigned char bit_buffer = '\0';
char code[MAX_SYMBOLS][MAX_LEN];
struct tnode* root=NULL; /*tree of symbols*/
struct tnode* qhead=NULL; /*list of current symbols*/

/*
    @function   talloc
    @desc       allocates new node 
*/
struct tnode* talloc(int symbol,float freq)
{
    struct tnode* p=(struct tnode*)malloc(sizeof(struct tnode));
    if(p!=NULL)
    {
        p->left=p->right=p->parent=p->next=NULL;
        p->symbol=symbol;
        p->freq=freq;
		p->isleaf=1;
    }
    return p;
}

/*
    @function display_tnode_list
    @desc     displays the list of tnodes during code construction
*/
void pq_display(struct tnode* head)
{
    struct tnode* p=NULL;
    printf("list:");
    for(p=head;p!=NULL;p=p->next)
    {
        printf("(%c,%f) ",p->symbol,p->freq);
    }
    printf("\n");
}

/*
    @function pq_insert
    @desc     inserts an element into the priority queue
    NOTE:     makes use of global variable qhead
*/
void pq_insert(struct tnode* p)
{
	printf("inserting:%c,%f\n",p->symbol,p->freq);
	struct tnode *dummy_node = (struct tnode*)malloc(sizeof(struct tnode)), *curr = dummy_node;
	curr->next = qhead;

	while (curr->next) {
		if (curr->next->freq > p->freq) {
			p->next = curr->next;
			curr->next = p;
			break;
		}
		curr = curr->next;
	}
	if (curr->next == NULL)	/* the last position or the beginning position */
		curr->next = p;

	qhead = dummy_node->next;
	free(dummy_node);
}

/*
    @function pq_pop
    @desc     removes the first element
    NOTE:     makes use of global variable qhead
*/

struct tnode* pq_pop()
{
    struct tnode* p=NULL;
    /*remove front of the queue*/
	p = qhead;
	if (qhead) {
		qhead = qhead->next;
		printf("popped:%c,%f\n",p->symbol,p->freq);
	}
    return p;
}

/*
	@func build_tree
	@desc build the Huffman codes tree
*/
struct tnode* build_tree(int NCHAR, unsigned int *freq)
{
	int i;
	struct tnode *p, *lc, *rc;

	for (i=0; i<NCHAR-1; i++) {
		lc = pq_pop();
		rc=pq_pop();
        /*create parent*/
        p=talloc(0,lc->freq+rc->freq);
        /*set parent link*/
        lc->parent=rc->parent=p;
        /*set child link*/
        p->right=rc; p->left=lc;
        /*make it non-leaf*/
        p->isleaf=0;
        /*add the new node to the queue*/
        pq_insert(p);
    }
    /*get root*/
    return pq_pop();
}

/*
	@function build_code
	@desc     generates the string codes given the tree
	NOTE: makes use of the global variable root
*/
void generate_code(struct tnode* root,int depth)
{
	int symbol;
	int len; /*length of code*/
	if(root->isleaf)
	{
		symbol=root->symbol;
		len   =depth;
		/*start backwards*/
		code[symbol][len]=0;	/* the null character */
		/*
			follow parent pointer to the top
			to generate the code string
		*/
		while (root->parent) {
			struct tnode *parent = root->parent;
			code[symbol][--len] = (parent->left == root) ? '0' : '1';
			root = root->parent;
		}
		printf("built code:%c,%s\n",symbol,code[symbol]);
	}
	else
	{
		generate_code(root->left,depth+1);
		generate_code(root->right,depth+1);
	}
	
}

/*
	@func	dump_code
	@desc	output code file
*/
void dump_code(FILE* fp)
{
	int i=0;
	for(i=0;i<MAX_SYMBOLS;i++)
	{
		if(code[i][0]) /*non empty*/
			fprintf(fp,"%c %s\n",(unsigned char)i,code[i]);
	}
}

/*
	@func	encode
	@desc	outputs compressed stream
*/
void encode(char* str,FILE* fout)
{
	while(*str)
	{
		fprintf(fout,"%s",code[*str]);
		str++;
	}
}

/*
	@func	write_bit
	@desc	use enough bits to fill a byte
*/
void write_bit(char bit, FILE *fout)
{
	bit_buffer <<= 1;
	if (bit == '1')
		bit_buffer |= 0x1;

	current_bit++;
	if (current_bit == 8) {
		putc(bit_buffer, fout);
		current_bit = 0;
		bit_buffer = 0;
	}
}

/*
	@func	encode_file
	@desc	encode fin to fout
*/
void encode_file(FILE *fin, FILE *fout)
{
	unsigned char c;
	int len, i;
	while (!feof(fin)) {
		c = getc(fin);
		len = strlen(code[c]);
		for (i=0; i<len; i++)
			write_bit(code[c][i], fout);
	}

	/*flush the bit-buffer*/
	while (current_bit)
		write_bit(0, fout);
}

/*
	@func free the tree
*/
void freetree(struct tnode* root)
{
	if(root==NULL)
		return;
	freetree(root->right);
	freetree(root->left);
	free(root);
}

/*
    @function main
*/
int main(int argc, char *argv[])
{
	/*need two input arguments*/
	if (argc < 2) {
		printf("error: you need to input file name!\n");
		return 1;
	}
	/*read from a file and output to a file*/
	FILE *fin=NULL, *fout=NULL;
	int i;
	unsigned char c;
	unsigned int freq[MAX_SYMBOLS];
	long count = 0; /*total number of characters*/
	const char *CODE_FILE="code.txt";
	const char *OUT_FILE="encoded.txt";

	for (i=0; i<MAX_SYMBOLS; i++)
		freq[i] = 0;

	fin = fopen(argv[1], "r");
	while (!feof(fin)) {
		c = getc(fin);
		freq[c]++;
		count++;
	}
	freq[c]--;/*get rid of the 'EOF' at the end of file*/
	count--;
		
    /*initialize with freq*/
	int NCHAR; /*number of characters*/
	for (i=0; i<MAX_SYMBOLS; i++) {
		if (freq[i] > 0) {
			pq_insert(talloc(i, freq[i]));
			NCHAR++;
		}
	}

	/*zero out code*/
	memset(code,0,sizeof(code));
    /*build tree and get root*/
	root = build_tree(NCHAR, freq);
	/*build code*/
	generate_code(root,0);
	/*output code*/
	fout=fopen(CODE_FILE,"w");
	fprintf(fout, "%ld\n", count);
	dump_code(fout);
	fclose(fout);

	/*encode a sample string*/
	fout=fopen(OUT_FILE, "w");
	rewind(fin);
	encode_file(fin, fout);
	fclose(fin);
	fclose(fout);
	getchar();
	/*clear resources*/
	freetree(root);
    return 0;
}
