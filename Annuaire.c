//PONCOT Cédric
//COURVOISIER Nicolas
//L2 Informatique Groupe 1A


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h> 
#include <string.h>
#include <sys/time.h>
#include <time.h>


#define NAME_LENGTH_MIN 3
#define NAME_LENGTH_MAX 10
#define TELEPHONE_LENGTH 8

//L'utilisateur n'a besoin de rentrer pour ses recherches que des chaines qui sont plus petite ou égale
//au maximum de la longueur des noms et de celle du telephone. Le +1 est necessaire pour placer le char '\0'.
#define BUFSIZE maximum(NAME_LENGTH_MAX,TELEPHONE_LENGTH)+1
#define NOMBRE_ENTREE_REP 1000000

//Afin d'optimiser les performances, nous evitons les "rehash" en donnant
//une taille a notre table de hashage qui est deux fois plus grande que
//son nombre d'éléments.
#define TAILLE_TABLE_HASH NOMBRE_ENTREE_REP*2

//Structures utilisées dans le projet
struct directory_data {
	char last_name[NAME_LENGTH_MAX + 1];
	char first_name[NAME_LENGTH_MAX + 1];
	char telephone[TELEPHONE_LENGTH + 1];
};

struct directory {
	struct directory_data **data;
	size_t size;
	size_t capacity;
};

struct index_bucket {
	const struct directory_data *data;
	struct index_bucket *next;
};

typedef size_t (*index_hash_func_t)(const struct directory_data *data);

struct index {
	struct index_bucket **buckets;
	size_t count;
	size_t size;
	index_hash_func_t func;
};


//prototype des fonctions
void creer_nom_aleatoire(struct directory_data *data,bool a);
void directory_data_random(struct directory_data *data);
void directory_data_print(const struct directory_data *data);
void directory_create(struct directory *self);
void directory_destroy(struct directory *self);
void directory_random(struct directory *self, size_t n);
void directory_search(const struct directory *self, const char *last_name);
void directory_sort(struct directory *self);
void directory_quick_sort_partial(struct directory *self,long i, long j);
long directory_partition(struct directory *self, long i, long j);
int str_compare(const char *str1, const char *str2);
size_t str_length(const char *str);
void directory_data_swap(struct directory *self,size_t i,size_t j);
void directory_search_opt(const struct directory *self,const char *last_name);
void clean_newline(char *buf, size_t size);
int maximum(size_t a,size_t b);
struct index_bucket *index_bucket_add(struct index_bucket *self,const struct directory_data *data);
void index_bucket_destroy(struct index_bucket *self);
size_t fnv_hash(const char *key);
size_t index_first_name_hash(const struct directory_data *data);
size_t index_telephone_hash(const struct directory_data *data);
void index_create(struct index *self, index_hash_func_t func);
void index_destroy(struct index *self);
void index_rehash(struct index *self);
bool besoin_rehash(struct index *self);
void index_add(struct index *self,const struct directory_data *data);
void index_fill_with_directory(struct index *self,const struct directory *dir);
void index_search_by_first_name(const struct index *self,const char *first_name);
void index_search_by_telephone(const struct index *self,const char *telephone);
void index_add_tel(struct index *self,const struct directory_data *data);
void index_fill_with_directory_tel(struct index *self,const struct directory *dir);
void index_rehash_tel(struct index *self);
void directory_destroy(struct directory *self);

int main(int argc, char *argv[]) {

	//déclaration des objets
	struct directory unNom;
	struct index index_tel;
	struct index index_first_name;
	struct timeval temps_depart;
	struct timeval temps_stop;

	//création des 2 index et du répertoire
	index_create(&index_tel,index_telephone_hash);
	index_create(&index_first_name,index_first_name_hash);
	directory_create(&unNom);

	//Remplissage du repertoire avec compteur de temps
	printf("\n--Fill directory--");
	gettimeofday (&temps_depart, NULL);
	srand(time(NULL));
	directory_random(&unNom,NOMBRE_ENTREE_REP);
	gettimeofday (&temps_stop, NULL);
	printf("Time: %ld us\n", ((temps_stop.tv_sec - temps_depart.tv_sec) * 1000000 + temps_stop.tv_usec) - temps_depart.tv_usec);

	//Remplissage de l'index en fonction des prénoms avec compteur de temps
	printf("\n--Fill index 1--");
	gettimeofday (&temps_depart, NULL);
	index_fill_with_directory(&index_first_name,&unNom);
	gettimeofday (&temps_stop, NULL);
	printf("Time: %ld us\n", ((temps_stop.tv_sec - temps_depart.tv_sec) * 1000000 + temps_stop.tv_usec) - temps_depart.tv_usec);

	//Remplissage de l'index en fonction des numéros de telephone avec compteur de temps
	printf("\n--Fill index 2--");
	gettimeofday (&temps_depart, NULL);
	index_fill_with_directory(&index_tel,&unNom);
	gettimeofday (&temps_stop, NULL);
	printf("Time: %ld us\n", ((temps_stop.tv_sec - temps_depart.tv_sec) * 1000000 + temps_stop.tv_usec) - temps_depart.tv_usec);

	//Tri du répertoire, implémentation d'un quicksort (tri rapide), O(n)=n*log(n), avec compteur de temps
	printf("\n--Sort directory--");
	gettimeofday (&temps_depart, NULL);
	directory_sort(&unNom);
	gettimeofday (&temps_stop, NULL);
	printf("Time: %ld us\n", ((temps_stop.tv_sec - temps_depart.tv_sec) * 1000000 + temps_stop.tv_usec) - temps_depart.tv_usec);

	//déclaration d'une chaine de caractère qui va récuperer les entrées utilisateur
	char buf[BUFSIZE];
	for (;;) {
		printf("\nWhat do you want to do?\n");
		printf("	1: Search by last name (not optimised)\n");
		printf("	2: Search by last name (optimised)\n");
		printf("	3: Search by first name\n");
		printf("	4: Search by telephone\n");
		printf("	q: Quit\n");
		printf("Your choice:\n");
		printf("> ");
		fgets(buf, BUFSIZE, stdin);
		clean_newline(buf, BUFSIZE);
		if(buf[1]=='\0'){
			if(buf[0]=='q'){
				break;
			}
			
			switch (buf[0])
				{
					case '1':
						printf("\n-Enter a last name : \n");
						fgets(buf, BUFSIZE, stdin);
						clean_newline(buf, BUFSIZE);
						gettimeofday (&temps_depart, NULL);
						//recherche "naive", 0(n)=n
						directory_search(&unNom,buf);
						gettimeofday (&temps_stop, NULL);
						printf("\n-Time: %ld us\n", ((temps_stop.tv_sec - temps_depart.tv_sec) * 1000000 + temps_stop.tv_usec) - temps_depart.tv_usec);
					break;

					case '2':
						printf("\n-Enter a last name : \n");
						fgets(buf, BUFSIZE, stdin);
						clean_newline(buf, BUFSIZE);
						gettimeofday (&temps_depart, NULL);
						//recherche dichotomique (car le répertoire est trié), O(n)=log(n)
						directory_search_opt(&unNom,buf);
						gettimeofday (&temps_stop, NULL);
						printf("\n-éxecuté en: %ld us\n", ((temps_stop.tv_sec - temps_depart.tv_sec) * 1000000 + temps_stop.tv_usec) - temps_depart.tv_usec);
					break;

					case '3':
						printf("\n-Enter a first name : \n");
						fgets(buf, BUFSIZE, stdin);
						clean_newline(buf, BUFSIZE);
						gettimeofday (&temps_depart, NULL);
						//recherche dans la table de hashage, complexité en 0(1)
						index_search_by_first_name(&index_first_name,buf);
						gettimeofday (&temps_stop, NULL);
						printf("\n-éxecuté en: %ld us\n", ((temps_stop.tv_sec - temps_depart.tv_sec) * 1000000 + temps_stop.tv_usec) - temps_depart.tv_usec);
					break;

					case '4':
						printf("\n-Enter a phone number (%d number) : \n",TELEPHONE_LENGTH);
						fgets(buf, BUFSIZE, stdin);
						clean_newline(buf, BUFSIZE);
						gettimeofday (&temps_depart, NULL);
						//recherche dans la table de hashage, complexité en 0(1)
						index_search_by_telephone(&index_tel,buf);
						gettimeofday (&temps_stop, NULL);
						printf("\n-éxecuté en: %ld us\n", ((temps_stop.tv_sec - temps_depart.tv_sec) * 1000000 + temps_stop.tv_usec) - temps_depart.tv_usec);
					break;

					default:
						printf("\nEntrée invalide \n");
					break;
				}
		}
		else{
			printf("\nEntrée invalide \n");
		}
		
	}
	//on libère la mémoire
	directory_destroy(&unNom);
	index_destroy(&index_tel);
	index_destroy(&index_first_name);
}

//retourne le maximum entre deux size_t (utilisé dans la définition de la constante BUFSIZE)
int maximum(size_t a,size_t b){
	if(a>=b){
		return a;
	}
	return b;
}

//PARTIE 1 : Génération de l'annuaire

//Question 1.1
void directory_data_print(const struct directory_data *data) {
	assert(data);
	printf("- ");
	printf("%s", data->last_name);
	printf(" ");
	printf("%s", data->first_name);
	printf(": ");
	printf("%s", data->telephone);
	printf("\n");
}

//Question 1.2
void directory_data_random(struct directory_data *data) {
	creer_nom_aleatoire(data,true);
	creer_nom_aleatoire(data,false);

	for(size_t i=0;i<TELEPHONE_LENGTH;++i){
		size_t num=rand()%10;
		data->telephone[i]=(char)(num+'0');
	}
		data->telephone[TELEPHONE_LENGTH]='\0';
}

//Cette fonction donne un nom (ou prenom) à l'entrée data. 
//le paramètre booléen "a" vaut true lorsque que l'on rempli un last_name, false lorsque c'est un first_name
void creer_nom_aleatoire(struct directory_data *data,bool a) {
	char consonnes[15] = {'B', 'C', 'D', 'F', 'G', 'H', 'J', 'L', 'M', 'N', 'P', 'R', 'S', 'T', 'V'};
	char voyelles[5] = {'A', 'E', 'I', 'O', 'U'};
	size_t size_l_name=rand()%8+3;
	bool consonne=true;
	for(size_t i = 0 ; i < size_l_name ; ++i){
		if(consonne){
			size_t aleatoire=rand()%15;
			if(a){
				data->last_name[i]=consonnes[aleatoire];
			}
			else{
				data->first_name[i]=consonnes[aleatoire];
			}
			consonne=false;
	
		}
		else{
			size_t aleatoire=rand()%5;
			if(a){
				data->last_name[i]=voyelles[aleatoire];
			}
			else {
				data->first_name[i]=voyelles[aleatoire];
			}
			consonne=true;
		}
		
	}
	if(a)data->last_name[size_l_name]='\0';
	else data->first_name[size_l_name]='\0';
}

//Question 1.3
void directory_create(struct directory *self) {
	self->size = 0;
	//on initialise au nombre entrée du rep (qui est connu) afin de ne pas avoir à faire un directory_grow.
	self->capacity = NOMBRE_ENTREE_REP;
	self->data = calloc(self->capacity, sizeof(struct directory_data*));
}

//permet d'agrandir le répertoire si besoin.
void directory_grow(struct directory *self){
	size_t capacity=self->capacity*2;
	struct directory_data **data=calloc(capacity,sizeof(struct directory_data*));
	memcpy(data,self->data,(self->size)*sizeof(struct directory_data*));
	free(self->data);
	self->data=data;
	self->capacity=capacity;
}

//Question 1.4
void directory_destroy(struct directory *self) {
	for (size_t i = 0; i < self->size; ++i) {
		free(self->data[i]);
		self->data[i] = NULL;
	}
free(self->data);
}

//Question 1.5
void directory_add(struct directory *self, struct directory_data *data){
	if(self->size==self->capacity)directory_grow(self);
	self->data[self->size]=data;
	self->size++;
}

//Question 1.6
void directory_random(struct directory *self, size_t n){
	for(size_t i=0;i<n;++i){
		struct directory_data *entree=malloc(sizeof(struct directory_data));
		directory_data_random(entree);
		directory_add(self,entree);
	}
}

//PARTIE 2 : Recherche suivant le nom

//Question 2.1
void directory_search(const struct directory *self, const char *last_name){
	size_t s =self->size;
	for(size_t i=0;i<s;++i){
		if(strcmp(self->data[i]->last_name,last_name)==0){
			directory_data_print(self->data[i]);
		}
	}

}

//Echange deux entrées dans un répertoire
void directory_data_swap(struct directory *self,size_t i,size_t j){
	struct directory_data *temp= self->data[i];
	self->data[i]=self->data[j];
	self->data[j]=temp;
}

//Compare deux chaines de caractères 
int str_compare(const char *str1, const char *str2) {
	size_t i=0;
	size_t l1=str_length(str1);
	size_t l2=str_length(str2);
	while(str1[i]==str2[i]&&l1>i&&l2>i){
		++i;
	}
	if(str1[i]<str2[i]){
		return -1;
	}
	else{
		if(str1[i]>str2[i]){
			return 1;
		}
	}
  return 0;
}

//Retourne la taille d'une chaine de caractère
size_t str_length(const char *str) {
	size_t t=0;
	if (str != NULL){
		while(str[t]!='\0'){
			++t;	
		}
	}
  	return t;
}

//Implémentation Quick Sort
long directory_partition(struct directory *self, long i, long j) {
	long pivot_index = i;
	const char *pivot = self->data[pivot_index]->last_name;
	directory_data_swap(self, pivot_index, j);
	long l = i;
	for (long k = i; k < j; ++k) {
		if (str_compare(self->data[k]->last_name,pivot)==-1) {
			directory_data_swap(self, k, l);
			l++;
		}
	}
	directory_data_swap(self, l, j);
	return l;
}

//Implémentation Quick Sort
void directory_quick_sort_partial(struct directory *self,long i, long j) {
	if (i < j) {
		long p = directory_partition(self, i, j);
		directory_quick_sort_partial(self, i, p - 1);
		directory_quick_sort_partial(self, p + 1, j);
	}
}

//Question 2.2.  0(n)=n*log(n)
void directory_sort(struct directory *self) {
	directory_quick_sort_partial(self, 0, self->size - 1);
}
//Question 2.3.  0(n)=log(n) Recherche dichotomique
void directory_search_opt(const struct directory *self,const char *last_name){
  bool trouve=false;
  size_t indice_debut=0;
  size_t indice_fin=self->size;
  size_t indice_milieu=(indice_debut + indice_fin)/2;
  while(!trouve && ((indice_fin - indice_debut) > 1)){
    indice_milieu = (indice_debut + indice_fin)/2;
    if(str_compare(self->data[indice_milieu]->last_name, last_name)==0){
    	trouve=true;
    }
    else{
	    if(str_compare(self->data[indice_milieu]->last_name, last_name)>0){
	    	indice_fin = indice_milieu;
	    } 
	    else{
	     indice_debut = indice_milieu;
	  	}
	}
  }
  if(trouve){
  	directory_data_print(self->data[indice_milieu]);
  	size_t i=1;
  	size_t j=1;
  	while(str_compare(self->data[indice_milieu-i]->last_name,last_name)==0){
  		directory_data_print(self->data[indice_milieu-i]);
  		++i;
  	}
  	while(str_compare(self->data[indice_milieu+j]->last_name,last_name)==0){
  		directory_data_print(self->data[indice_milieu+j]);
  		++j;
  	}
  }

  if(str_compare(self->data[indice_debut]->last_name, last_name)==0){
  	directory_data_print(self->data[indice_debut]);
  }
}


//PARTIE 3: Création et utilisation d'index

//Question 3.1
struct index_bucket *index_bucket_add(struct index_bucket *self,const struct directory_data *data){
	struct index_bucket *noeud_ajout=malloc(sizeof(struct index_bucket));
	noeud_ajout->next=self;
	noeud_ajout->data=data;
	return noeud_ajout;
}

//Question 3.2
void index_bucket_destroy(struct index_bucket *self){
	while(self!=NULL){
		
		struct index_bucket *index_a_supprimer=self;
		if(self->next!=NULL){
			self=self->next;
		}
		else{
			self=NULL;
		}
		free(index_a_supprimer);
	}
}

//Question 3.3
size_t fnv_hash(const char *key){
	//constante pour système 64 bits
	const size_t FNV_offset_basis = 14695981039346656037;
    const size_t FNV_prime = 1099511628211;
    size_t hash = FNV_offset_basis;
    size_t longueur= str_length(key);
    for (size_t i=0; i<longueur ;++i){
            hash = hash * FNV_prime;
            hash = hash ^ (key[i]);
    }
	return hash;
}

//Question 3.4
size_t index_first_name_hash(const struct directory_data *data){
	return fnv_hash(data->first_name);
}

//Question 3.5
size_t index_telephone_hash(const struct directory_data *data){
	return fnv_hash(data->telephone);
}

//Question 3.6
void index_create(struct index *self, index_hash_func_t func){
	self->size=TAILLE_TABLE_HASH;
	self->count=0;
	self->buckets=calloc(self->size,sizeof(struct index_bucket*));
	self->func=func;
}

//Question 3.7
void index_destroy(struct index *self){
	for(size_t i=0;i<self->size;++i){
		index_bucket_destroy(self->buckets[i]);
	}
	free(self->buckets);
}

//Question 3.8
void index_rehash(struct index *self){
	struct index_bucket **tmp=calloc(self->size*2,sizeof(struct index_bucket*));
	for(size_t i=0;i<self->size;++i){
		if(self->buckets[i]!=NULL){
			size_t nouvelindex=self->func(self->buckets[i]->data)%self->size*2;
			tmp[nouvelindex]=index_bucket_add(tmp[nouvelindex],self->buckets[i]->data);
			struct index_bucket *curr=self->buckets[i]->next;
			
			while(curr!=NULL){
				nouvelindex=self->func(curr->data)%self->size*2;
				tmp[nouvelindex]=index_bucket_add(tmp[nouvelindex],curr->data);
				curr=curr->next;
			}
		}
	}
	index_destroy(self);
	self->size=self->size*2;
	self->buckets=tmp;
}

//Cette fonction détermine si le facteur de compression est superieur à 0,5 (division entiere donc 0 si le facteur est inferieur ou egal à 0.5)
bool besoin_rehash(struct index *self){
	return self->count/self->size!=0;
}

//Question 3.9
void index_add(struct index *self,const struct directory_data *data){
	if(besoin_rehash(self)){
		index_rehash(self);
		printf("-REHASH-\n");
	}
	size_t index=self->func(data)%self->size;
	self->buckets[index]=index_bucket_add(self->buckets[index],data);
	++self->count;
}

//Question 3.10
void index_fill_with_directory(struct index *self,const struct directory *dir){
	for(size_t i=0;i<dir->size;++i){
		index_add(self,dir->data[i]);
	}
}

//Question 3.11
void index_search_by_first_name(const struct index *self,const char *first_name){
	size_t index=fnv_hash(first_name)%self->size;
	if(self->buckets[index]!=NULL){
		if(str_compare(self->buckets[index]->data->first_name,first_name)==0){
			directory_data_print(self->buckets[index]->data);

		}
		struct index_bucket *curr=self->buckets[index]->next;
		while(curr!=NULL){
			if(str_compare(curr->data->first_name,first_name)==0){
				directory_data_print(curr->data);
			}
			curr=curr->next;
		}
	}
	
}

//Question 3.12
void index_search_by_telephone(const struct index *self,const char *telephone){
	size_t index=fnv_hash(telephone)%self->size;
	if(self->buckets[index]!=NULL){
		if(str_compare(self->buckets[index]->data->telephone,telephone)==0){
			directory_data_print(self->buckets[index]->data);
		}
		struct index_bucket *curr=self->buckets[index]->next;
		while(curr!=NULL){
			if(str_compare(curr->data->telephone,telephone)==0){
				directory_data_print(curr->data);
			}
		curr=curr->next;
		}
		
	}
}

//PARTIE 4: Application interactive

//Question 4.1
void clean_newline(char *buf, size_t size){
	size_t i=0;
	while(buf[i]!=(char)266 && i<size){
		++i;
	}
	buf[i]='\0';
}

//Question 4.2 à 4.5 dans le main
