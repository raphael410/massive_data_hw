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

int getDimensions(edgelist * el){
    int i;
    int maximumNode = 0;
    for (i=0;i<el->e;i++){
        maximumNode = max3(el->edges[i].s, el->edges[i].t, maximumNode);
    }
    return maximumNode;
}

void free_edgelist(edgelist *g){
	free(g->edges);
	free(g);
}

void applyTransitionMatrix(edgelist * el, int dim, double * matrix, int*mult, float alpha){
    int i;
    double mult_s;
    double nEdges = el->e;

    for (i=0;i<el->e;i++) {
        edge e = el->edges[i];
        //if (i%1000000==0) printf("node %d : %d -> %d\n", i, e.s, e.t);
        mult_s = mult[e.s];
        matrix[e.t] *= (1-alpha)/mult_s;
        matrix[e.t] += alpha/nEdges;
        }
}

void storeMultiplicity(edgelist*el, int*mult){
    edge*allEdges = el->edges;
    int i;
    for (i=0 ; i<el->e ; i++){
        mult[allEdges[i].s] += 1;
    }
}

int access(int * matrix, int dim, int i, int j){return matrix[i*dim+j];}


void saveProbabilities(double*matrix, int dim){
    char*filename = "proba.txt";

    FILE *f = fopen(filename, "w");
    if (f == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }

    int i;
    for (i=0;i<dim;i++){
        fprintf(f,"%.50f\n",matrix[i]);
    }
    fclose(f);
}





int main(int argc,char** argv){
	edgelist* g;
	time_t t1,t2;

	// IMPORTANT : put here the name of the txt representing the list of the edges of the graph
	char* filename = "linkswiki.txt";
	double alpha = 0.15;
	//int indexGraph = 2;

	t1=time(NULL);

	printf("Reading edgelist from file %s\n",filename);
	g=readedgelist(filename);


	printf("Number of nodes: %lu\n",g->n);
	printf("Number of edges: %lu\n",g->e);




    int dim = g->n;
    int i;

	int * mult = calloc(dim,sizeof(int));

	for (i=0;i<dim;i++) mult[i]=0;
	storeMultiplicity(g, mult);


    printf("matrix initialization ...");
    double * matrix = calloc(dim,sizeof(double));

    double regularizer = dim;
    for (i=0;i<dim;i++){matrix[i]=1/regularizer;}
    printf("initialized\n");

    int n_iterate = 100;
    for (i=0;i<n_iterate;i++){
            applyTransitionMatrix(g,dim,matrix,mult,alpha);
            if (i%10 == 0) printf("iteration %d\n",i);
    }

    saveProbabilities(matrix,dim);

	free_edgelist(g);

	t2=time(NULL);




	printf("- Overall time = %ldh%ldm%lds\n",(t2-t1)/3600,((t2-t1)%3600)/60,((t2-t1)%60));
	return 0;
}

