/*
Maximilien Danisch
September 2017
http://bit.ly/danisch
maximilien.danisch@gmail.com

Info:
Feel free to use these lines as you wish. This program loads a graph in main memory as a list of edges.

To compile:
"gcc edgelist.c -O9 -o edgelist".

To execute:
"./edgelist edgelist.txt".
"edgelist.txt" should contain the graph: one edge on each line (two unsigned long (nodes' ID)) separated by a space.
The prograph loads the graph in main memory and then it terminates.

Note:
If the graph is directed (and weighted) with selfloops and you want to make it undirected unweighted without selfloops, use the following linux command line.
awk '{if ($1<$2) print $1" "$2;else if ($2<$1) print $2" "$1}' net.txt | sort -n -k1,2 -u > net2.txt

Performance:
Up to 500 million edges on my laptop with 8G of RAM:
Takes more or less 1.6G of RAM and 25 seconds (I have an SSD hardrive) for 100M edges.
*/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>//to estimate the runing time

#define NLINKS 100000000 //maximum number of edges for memory allocation, will increase if needed

typedef struct {
	unsigned long s;
	unsigned long t;
} edge;

//edge list structure:
typedef struct {
	unsigned long n;//number of nodes
	unsigned long e;//number of edges
	edge *edges;//list of edges
} edgelist;

typedef struct m{
    int val;
    struct m * suiv;
} maillon;

typedef struct {
    int length;
    maillon * start;
}chainlist;

void addValInList(int val, chainlist* cl){
    maillon *newMaillon = malloc(sizeof(maillon));
    newMaillon->val = val;
    newMaillon->suiv = cl->start;
    cl->start = newMaillon;
    cl->length++;
}

int chainlistToTab(chainlist* cl){
    int tab[cl->length];
    int i=0;
    maillon * now =cl->start;
    while(now!=NULL){
        tab[i]=now->val;i++;
        now = now->suiv;
    }
    return tab;
}

//compute the maximum of three unsigned long
inline unsigned long max3(unsigned long a,unsigned long b,unsigned long c){
	a=(a>b) ? a : b;
	return (a>c) ? a : c;
}

//reading the edgelist from file
edgelist* readedgelist(char* input){
	unsigned long e1=NLINKS;
	FILE *file=fopen(input,"r");

	edgelist *g=malloc(sizeof(edgelist));
	g->n=0;
	g->e=0;
	g->edges=malloc(e1*sizeof(edge));//allocate some RAM to store edges

	while (fscanf(file,"%lu %lu", &(g->edges[g->e].s), &(g->edges[g->e].t))==2) {
		g->n=max3(g->n,g->edges[g->e].s,g->edges[g->e].t);
		if (g->e++==e1) {//increase allocated RAM if needed
			e1+=NLINKS;
			g->edges=realloc(g->edges,e1*sizeof(edge));
		}
	}
	fclose(file);

	g->n++;

	g->edges=realloc(g->edges,g->e*sizeof(edge));

	return g;
}

void free_edgelist(edgelist *g){
	free(g->edges);
	free(g);
}

/**
returns if two edges are the same
*/
int same_edge(edge * e1, edge * e2){
    int same = (e1->s == e2->s) && (e1->t == e2->t) ;
    int sameReversed = (e1->t == e2->s) && (e1->s == e2->t) ;
    return same || sameReversed;
}

int self_loop(edge * e){return e->s == e->t;}

int maxNodeValue(edgelist el){
    edge* allEdges = el.edges;
    int i;
    int maxVal = 0;
    for (i=0 ; i<el.e ; i++){
        maxVal = max3(allEdges[i].s, allEdges[i].t, maxVal);
    }
    return maxVal;
}

void multiplicity(edgelist el, int*mult){
    edge*allEdges = el.edges;
    int i;
    for (i=0 ; i<el.e ; i++){
        mult[allEdges[i].s] += 1;
    }
}

unsigned long greedySum(edgelist el, int* mult){
    int i;
    unsigned long sum = 0;
    edge e;
    for(i=0;i<el.e;i++){
        e = el.edges[i];
        sum+=mult[e.s]*mult[e.t];
    }
    return sum;
}

int smarterSum(int* mult, int length){
    return 0;
}

void saveDegreeDistribution(int*mult, int length){
    char*filename = "degrees.txt";

    FILE *f = fopen(filename, "w");
    if (f == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }

    int i, maxDegree = 0;
    for (i=0;i<length;i++){
        maxDegree = mult[i]>maxDegree ? mult[i] : maxDegree;
    }

    int n,d;
    for (d=0;d<maxDegree+1;d++){
        n = 0;
        for(i=0;i<length;i++){
            if(mult[i]==d) n+=1;
        }
        fprintf(f,"%d %d\n",d,n);
    }
    fclose(f);
}


/**
Returns the neighbors of the node
**/
int* neighbors(int node, edgelist el){
    chainlist * neigh = malloc(sizeof(chainlist));
    int i;
    for (i=0;i<el.e;i++){
        if (el.edges[i].s==node || el.edges[i].t==node) addValInList(node,neigh);
    }
    printf("end fun neighbors");
    return chainlistToTab(neigh);
}

void bfs(edgelist el, int startNode) {

    int queue[el.n];
    int startQueue = 0, endQueue = 0;

    int visited[el.n];
    visited[startNode] = 1;
    queue[endQueue] = startNode;
    endQueue+=1;

    while(startQueue!=endQueue){
        int currentNode = (queue[startQueue]);
        startQueue = (startQueue + 1)%(el.n);

        printf("Visited %d\n", currentNode);

        printf("finding neighbors");
        int* temp = neighbors(currentNode,el);
        printf("found");
        int i;
        for (i=0;i<sizeof(temp)/sizeof(int);i++){
            if(visited[temp[i]] == 0){
                visited[temp[i]] = 1;
                queue[endQueue] = temp[i];
                endQueue=(endQueue+1)%el.n;
            }
        }

    }
}




int main(int argc,char** argv){
	edgelist* g;
	time_t t1,t2;

	char* filename = "email-Eu-core.txt";
	//int indexGraph = 2;

	t1=time(NULL);

	printf("Reading edgelist from file %s\n",filename);
	g=readedgelist(filename);


	printf("Number of nodes: %lu\n",g->n);
	printf("Number of edges: %lu\n",g->e);

    int maxNValue = maxNodeValue(*g);

	printf("%d\n",maxNValue);

	int i;
	int * mult[maxNValue]; for (i=0;i<maxNValue+1;i++) mult[i]=0;
    //memset(mult, 0, sizeof mult);

	multiplicity(*g, mult);
	for (i=0;i<5;i++) printf("%d\n",mult[i]);

	time_t tgreedy=time(NULL);
	long s = greedySum(*g,mult);time_t tendgreedy=time(NULL);
	printf("\ngreedy sum = %d\n (calculated in %ds)", s, (tendgreedy-tgreedy)%60);

	/*s = smarterSum(mult, maxNValue);time_t tendsmarter=time(N
                                                           dULL);
	printf("\nsmarter sum = %d\n (calculated in %ds)", s, (tendsmarter-tendgreedy)%60);
    */

    saveDegreeDistribution(mult,maxNValue);
    bfs(*g, 0);

	free_edgelist(g);

	t2=time(NULL);

	printf("- Overall time = %ldh%ldm%lds\n",(t2-t1)/3600,((t2-t1)%3600)/60,((t2-t1)%60));

	return 0;
}


