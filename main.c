#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <sys/stat.h>
// ceil, floor : #include <math.h>


#include "./include/traces.h" 
#include "./include/check.h" 


#include "elt.h"

#define HEAP_ALLOCATION_OFFSET 5

typedef struct {
	unsigned int nbElt;
	unsigned int nbMaxElt;
	T_elt * tree;	
} T_heap;

#define iPARENT(i) 			(i-1)/2
#define iLCHILD(i) 			(2*i)+1
#define iRCHILD(i) 			(2*i)+2
#define iLASTINTERNAL(n)	n/2 -1
#define isINTERNAL(i,n) 	(2*i<(n-1))
#define isLEAF(i,n) 			(2*i>=(n-1))
#define isINTREE(i,n)		(i<n)
#define isROOT(i)	(i==0)
#define nbINTERNALS(n) 		n/2
#define nbLEAVES(n) 			ceil((double)n/2)
#define VALP(pHeap, i)		pHeap->data[pHeap->tree[i]]		
#define VAL(heap, i)			heap.tree[i]		



 

//-----------------------------------------------------------

#define MAXCARS 128

typedef struct {
    unsigned int nbElt;
    unsigned char tree[MAXCARS];
    int data[2*MAXCARS-1];
} T_indirectHeap;


typedef struct {
	unsigned int nbElt;
    int tree[2*MAXCARS-1];
} T_huffmanTree;




char *huffman(char string[], char nomFichier[]);
char *decompresserHuffman(char nomFichier[]);
T_indirectHeap *analyserDocument(char string[]);
T_huffmanTree *initHuffmanTree();
char **encoder(T_indirectHeap *p, T_huffmanTree *Ht);




//getion des heap
T_indirectHeap *newindirectHeap(unsigned int nbMaxElt);
void showHeap(T_indirectHeap *p);
void showHeap_rec(T_indirectHeap *p, int i, int indent);
T_elt extraireMin(T_indirectHeap *p);
int getMax(const T_huffmanTree *Ht);
int getVal(T_huffmanTree *Ht, int val);
void buildHeapV2(T_indirectHeap * p); 
void swap(T_indirectHeap *p, int i, int j);
void siftDown(T_indirectHeap *p, int k);
void siftUp(T_indirectHeap *p, int k);
void addElt(T_indirectHeap *p, T_elt e);



//getion des neuds
void AjouterNoeud(T_huffmanTree* Ht, T_indirectHeap *p, T_elt C1, T_elt C2);
void insererMI(T_indirectHeap* Mi, int Ni, int val);




//afichage des arbre avec graphviz
static void genDotPOT_rec(T_indirectHeap *p, int n, int root, FILE *fp); 
void createDotPOT(T_indirectHeap *p, const char *basename);
void showTreePlot(int *huffmanTree, int i);









int main(int argc, char *argv[]) {

	if(argc == 3){
		printf("compresser un fichier \n");
		huffman(argv[1], argv[2]);
	}else if(argc == 2){
		printf("decompresser un fichier \n");
		decompresserHuffman(argv[1]);

	}else{
		printf("erreur nombre d'argument \n");
		return 0;
	}



	return 0;
}




char *decompresserHuffman(char nomFichier[]){
	
	T_huffmanTree* Ht = malloc(sizeof(T_huffmanTree));
    FILE* fic = fopen(nomFichier,"r");
    if(fic==NULL)
    {
        printf("erreur d'ouverture du fichier");
        exit(1);
    }


    fread(Ht,sizeof(T_huffmanTree),1,fic);

    
    int valMax=getMax(Ht);


	int bit=fgetc(fic);
    int caractere=valMax;

    while(bit!=EOF)
    {   
	    
		if( bit == '1')
        {
            caractere=getVal(Ht,-caractere);
        }

        if(bit == '0')
        {
			caractere=getVal(Ht,caractere);
        }
		
		if(caractere<MAXCARS)
        {
            printf("%c",caractere);

            caractere=valMax;
        }
        bit=fgetc(fic);
    }
	printf("\n");
	fclose(fic);
	
	return "";
}



int getVal(T_huffmanTree *Ht, int val){
	for(int i = 0; i < 2*MAXCARS-1; i++){
		if(Ht->tree[i] == val){
			return i;
		}
	}
	return 0;
}


char *huffman(char string[], char nomFichier[]){
	
	
	int len = strlen(string);

	
	T_indirectHeap *Mi = analyserDocument(string);
	T_huffmanTree *Ht = initHuffmanTree();

	buildHeapV2(Mi);
	showHeap(Mi);
	createDotPOT(Mi,"tas"); 

	//printf("\nnombre element %d \n\n", Mi->nbElt);
    
	int Ni = Mi->nbElt-1;
    for(int i = 0; i < Ni; i++){
        T_elt C1 = extraireMin(Mi);
        T_elt C2 = extraireMin(Mi);
		createDotPOT(Mi,"tas"); 

        AjouterNoeud(Ht, Mi , C1 , C2);
		showTreePlot(Ht->tree, i);
		
    }
	


	//encodage des caractère avec le code de huffman
	char **code = encoder(Mi, Ht);

	//afficher le codage
	for(int i = 0; i < 128; i++){
		if(strcmp(code[i], "") != 0){
			printf("code %c : %s \n", i, code[i]);
		}
	}
	
	//encoder le texte avec le code
	char *texte = malloc(sizeof(char) * 100000000);
	strcpy(texte, "");
	for(int i = 0; i < len; i++){
		strcat(texte, code[(int)string[i]]);
	}

	printf("texte brut : %s \n", string);
	printf("texte hauffman: %s \n", texte);


	
	printf("Longueur du code binaire : %d bits \n", len*8);
	printf("Longueur du code de huffman : %d bits \n", (int)strlen(texte));
	printf("Ratio de compression : %.2f%% \n", (float)strlen(texte)/(len*8)*100);

	//ecrire dans un fichier
	FILE *fp = fopen(nomFichier, "w");
	if(fp == NULL){
		printf("erreur ouverture fichier \n");
		return NULL;
	}

	//mettre le tableau Ht->tree dans un fichier en binaire
	fwrite(Ht, sizeof(T_huffmanTree), 1, fp);
	for(int i = 0; i < strlen(texte); i++){
		//fprintf(fp, "%c", texte[i]);	
		fwrite(&texte[i], sizeof(char), 1, fp);
	}

	return texte;
}


char **encoder(T_indirectHeap *p, T_huffmanTree *Ht){
	char **code;
	code = malloc(sizeof(char*) * 128);
	for(int i = 0; i < 128; i++){
		code[i] = malloc(sizeof(char) * 128);
		strcpy(code[i], "");
	}

	

	for (int i = 0; i < 128; i++) {
        if (p->data[i] != 0) {
            int j = i;
            while (Ht->tree[j] != 0) {
                if (Ht->tree[j] > 0) {
					strcat(code[i], "0");
					
                } else {
					strcat(code[i], "1");
                }
                j = abs(Ht->tree[j]);
            }
			//retourner la chaine de caractère
			int len = strlen(code[i]);
			char *temp = malloc(sizeof(char) * len);
			strcpy(temp, code[i]);
			for(int j = 0; j < len; j++){
				code[i][j] = temp[len-j-1];
			}
        }
    }
	

	return code;
}


void AjouterNoeud(T_huffmanTree* Ht, T_indirectHeap *p, T_elt C1, T_elt C2) {
	int i = 0;
	for(i = MAXCARS; i<2*MAXCARS-1; i++){
		if(p->data[i] == 0){
			break;
		}
	}

	p->data[i] = p->data[C1] + p->data[C2];
	addElt(p, i);

	
	Ht->tree[C1] = i;
	Ht->tree[C2] = -i;
}


//compter le nombre d'occurence de chaque caractère 
T_indirectHeap *analyserDocument(char string[]){

	T_indirectHeap *Mi = newindirectHeap(MAXCARS);
	//compté nombre occurence de chaque caractère et le mettre dans data enn fonction du code ascii
	for(int i = 0; i < strlen(string); i++){
		if(Mi->data[(int)string[i]] == 0){
			Mi->data[(int)string[i]] = 1;
		}else{
			Mi->data[(int)string[i]]++;
		}
	}

	Mi->nbElt = 0;
	//mettre les caractères dans le tableau tree
	for(int i = 0; i < MAXCARS; i++){
		if(Mi->data[i] != 0){
			Mi->tree[Mi->nbElt] = (char)i;
			Mi->nbElt++;
		}
	}


	return Mi;
}

T_indirectHeap *newindirectHeap(unsigned int nbMaxElt){
	T_indirectHeap *p = (T_indirectHeap *)malloc(sizeof(T_indirectHeap));
	
	p->nbElt = nbMaxElt;
	return p;
}

T_huffmanTree *initHuffmanTree(){
	T_huffmanTree *p = (T_huffmanTree *)malloc(sizeof(T_huffmanTree));
	for(int i = 0; i < 2*MAXCARS-1; i++){
		p->tree[i] = 0;
	}
	
	return p;
}






//getion de la heap ---------------------------------------------------------------

void showHeap(T_indirectHeap *p) {
	assert(p!=NULL);
	printf("Affichage du tas (nbElt : %d)\n",p->nbElt);
	showHeap_rec(p,0,0);
}
void showHeap_rec(T_indirectHeap *p, int root, int indent){
	int i;
	if (root < p->nbElt) {
		showHeap_rec(p, iRCHILD(root), indent+1);
		for(i=0;i<indent;i++) printf("\t");
			printf("%c %d\n", p->tree[root], p->data[p->tree[root]]);
		showHeap_rec(p, iLCHILD(root), indent+1);
	}
}

void siftDown(T_indirectHeap *p, int k) {

	int fini = isLEAF(k, p->nbElt);
	while (!fini) {
		int i = iLCHILD(k);
		if (iRCHILD(k) < p->nbElt && VALP(p, iRCHILD(k)) > VALP(p, iLCHILD(k))) {
			i = iRCHILD(k);
		}
		if (VALP(p, k) <= VALP(p, i)) {
			fini = 1;
		}
		else {
			swap(p, k, i);
			k = i;
			fini = isLEAF(k, p->nbElt);
		}
	}
}
void siftUp(T_indirectHeap *p, int k) {
	while (iPARENT(k) >= 0 && VALP(p, k) > VALP(p, iPARENT(k))) {
		swap(p, k, iPARENT(k));
		k = iPARENT(k);
	}
}

void buildHeapV2(T_indirectHeap * p){
	for (int k = p->nbElt / 2 - 1; k >= 0; k--) {
		siftDown(p, k);
	}
}

void swap(T_indirectHeap *p, int i, int j) {
	T_elt aux; 
	aux = p->tree[i];
	p->tree[i] = p->tree[j];
	p->tree[j] = aux;
 
}

T_elt extraireMin(T_indirectHeap *p){
	T_elt aux; 
	aux = p->tree[0];
	p->tree[0] = p->tree[p->nbElt - 1];
	p->tree[p->nbElt - 1] = aux;
	p->nbElt--;
	siftDown(p, 0);

	return aux;
}

void addElt(T_indirectHeap *p, T_elt e) {
	p->tree[p->nbElt] = e;
	p->nbElt++;
	siftUp(p, p->nbElt - 1);
}

int getMax(const T_huffmanTree *Ht){
	int max = 0;

	for(int i = 0; i < 2*MAXCARS-1; i++){
		if(abs(Ht->tree[i]) > max){
			max = abs(Ht->tree[i]);
		}
	}

	return max;
}


//generation des image ---------------------------------------------------------------

static void genDotPOT_rec(T_indirectHeap *p, int n, int root, FILE *fp){
	// t : tas
	// n : taille du tas
	// root : indice de la racine du sous-arbre à produire
	// fp : flux correspondant à un fichier ouvert en écriture où écrire le sous-arbre

	

	if(!isINTREE(root,n)) return;	

	fprintf(fp, "\t%d",root); 
    fprintf(fp, " [label = \"%c (%d)\"];\n",p->tree[root], p->data[p->tree[root]]);
    if ( isINTREE(iRCHILD(root),n) && isINTREE(iLCHILD(root),n) ) {
        fprintf(fp, "\t%d", root);
		fprintf(fp, " [label = \"%c (%d)\"];\n", p->tree[root], p->data[p->tree[root]]);
	}
    else 
		if ( isINTREE(iRCHILD(root),n) ) {
    		fprintf(fp, "\t%d", root);
			fprintf(fp, " [label = \"%c (%d)\"];\n",p->tree[root], p->data[p->tree[root]]);
		}
		else 
			if ( isINTREE(iLCHILD(root),n) ) {
        		fprintf(fp, "\t%d",root);
				fprintf(fp, " [label = \"%c (%d)\"];\n",p->tree[root], p->data[p->tree[root]]);
			}
	
    if ( isINTREE(iLCHILD(root),n) ) {
        fprintf(fp, "\t%d",root);
		fprintf(fp, ":sw -> %d;\n", iLCHILD(root));
		genDotPOT_rec(p, n, iLCHILD(root), fp);
    }

    if ( isINTREE(iRCHILD(root),n) ) {
        fprintf(fp, "\t%d",root);
		fprintf(fp,":se -> %d;\n", iRCHILD(root));
        genDotPOT_rec(p, n, iRCHILD(root), fp);
    }
}

void createDotPOT(T_indirectHeap *p,const char *basename) {
	int n = p->nbElt;
	static char oldBasename[FILENAME_MAX + 1] = "";
	static unsigned int noVersion = 0;

	char DOSSIER_DOT[FILENAME_MAX + 1]; 
	char DOSSIER_PNG[FILENAME_MAX + 1]; 

	char fnameDot [FILENAME_MAX + 1];
	char fnamePng [FILENAME_MAX + 1];
	char	cmdLine [2 * FILENAME_MAX + 20];
	FILE *fp;
	struct stat sb;
	char *outputPath = "./img";
	

	// Au premier appel, création (si nécessaire) des répertoires
	// où seront rangés les fichiers .dot et .png générés par cette fonction	

	// il faut créer le répertoire outputPath s'il n'existe pas 
	if (stat(outputPath, &sb) == 0 && S_ISDIR(sb.st_mode)) {
    } else {
        printf("Création du répertoire %s\n", outputPath);
		mkdir(outputPath, 0777);
    }

	// il faut créer les répertoires outputPath/png et /dot 
	sprintf(DOSSIER_DOT, "%s/dot/",outputPath);
	sprintf(DOSSIER_PNG, "%s/png/",outputPath);

	if (oldBasename[0] == '\0') {
		mkdir(DOSSIER_DOT,	S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
		mkdir(DOSSIER_PNG,	S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
	}

	 // S'il y a changement de nom de base alors recommencer à zéro
	 // la numérotation des fichiers 

	if (strcmp(oldBasename, basename) != 0) {
		noVersion = 0;
		strcpy(oldBasename, basename); 
	}

	sprintf(fnameDot, "%s%s_v%02u.dot", DOSSIER_DOT, basename, noVersion);
	sprintf(fnamePng, "%s%s_v%02u.png", DOSSIER_PNG, basename, noVersion);

	CHECK_IF(fp = fopen(fnameDot, "w"), NULL, "erreur fopen dans saveDotBST"); 
	
	noVersion ++;
	fprintf(fp, "digraph %s {\n", basename);
	fprintf(fp, 
	"\tnode [\n"
		"\t\tfontname  = \"Arial bold\" \n"
		"\t\tfontsize  = \"14\"\n"
		"\t\tfontcolor = \"red\"\n"
		"\t\tstyle     = \"rounded, filled\"\n"
		"\t\tshape     = \"circle\"\n"
		"\t\tfillcolor = \"grey90\"\n"
		"\t\tcolor     = \"blue\"\n"
		"\t\twidth     = \"0.5\"\n"
		"\t]\n"
	"\n"
	"\tedge [\n"
		"\t\tcolor     = \"blue\"\n"
	"\t]\n\n"
	);


	genDotPOT_rec(p,n,0,fp);

	fprintf(fp, "}\n");	
	fclose(fp);

	sprintf(cmdLine, "dot -Tpng  %s -o %s", fnameDot, fnamePng);
	system(cmdLine);

	printf("Creation de '%s' et '%s' ... effectuee\n", fnameDot, fnamePng);
}


void showTreePlot(int *huffmanTree, int i) {

    // ouverture du fichier de sortie sous le nom de "tree-i.dot"
    char output[50];
    sprintf(output, "./img/dot/tree-%d.dot", i);

    FILE *fp = fopen(output, "w");
    if(fp == NULL) {
        perror("Erreur lors de l'ouverture du fichier\n");
        exit(EXIT_FAILURE);
    }


    fprintf(fp, "digraph G {\n");
    fprintf(fp,
            "\tnode [\n"
            "\t\tfontname  = \"Arial bold\" \n"
            "\t\tfontsize  = \"14\"\n"
            "\t\tfontcolor = \"red\"\n"
            "\t\tstyle     = \"rounded, filled\"\n"
            "\t\tshape     = \"circle\"\n"
            "\t\tfillcolor = \"grey90\"\n"
            "\t\tcolor     = \"blue\"\n"
            "\t\twidth     = \"0.5\"\n"
            "\t]\n"
            "\n"
            "\tedge [\n"
            "\t\tcolor     = \"blue\"\n"
            "\t]\n\n");
	

    for (int i = 0; i < 2*MAXCARS-1; i++) {
        if (huffmanTree[i] != 0) {
            if (huffmanTree[i] > 0) {
                if(i >= 128) fprintf(fp, "%d -> %d [label=\"0\"];\n", huffmanTree[i], i);
                else fprintf(fp, "%d -> \"'%c'\" [label=\"0\"];\n", huffmanTree[i], i);
                // On affiche les caractères en tant que caractères et les noeuds internes en tant que nombres

            } 
            else {
                if(i >= 128) fprintf(fp, "%d -> %d [label=\"1\"];\n", abs(huffmanTree[i]), i);
                else fprintf(fp, "%d -> \"'%c'\" [label=\"1\"];\n", abs(huffmanTree[i]), i);
                // On affiche les caractères en tant que caractères et les noeuds internes en tant que nombres
            }
        }
    }
    fprintf(fp, "}");
	fclose(fp);
    // on le convertit en png sous le nom de "tree-i.png"
    char command[255];
	//sprintf(cmdLine, "dot -Tpng  %s -o %s", fnameDot, fnamePng);
    sprintf(command, "dot -Tpng ./img/dot/tree-%d.dot -o ./img/png/tree-%d.png", i, i);   
    system(command);
}