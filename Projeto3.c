#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#define MAXKEYS 3
#define MINKEYS (MAXKEYS-1)/2
#define NIL (-1)
#define NOKEY -1
#define NO 0
#define YES 1
#define PAGESIZE sizeof(BTPAGE)

struct estrutura
  	{
    	char cliente[3],codfilme[3],nome[50], filme[50], genero[50];
  	}film; 
 //////////
 typedef struct{
        int keycount; // number of keys in page
        int key[MAXKEYS]; // the actual keys
        int child[MAXKEYS+1]; // ptrs to rrns of descendants
        int pos[MAXKEYS+1];
}BTPAGE;

int root; // rrn of root page
FILE *btfd; // global file descriptor for "btree.dat"

/* prototypes */
int insert (int rrn, int key, int *promo_r_child, int *promo_key, int poss,int *encontrou);
int btopen();
void btclose();
int getroot();
void putroot(int root);
int create_tree();
int getpage();
void btread (int rrn, BTPAGE *page_ptr);
void btwrite(int rrn, BTPAGE *page_ptr);
int create_root(int key, int left, int right, int pos);
void pageinit(BTPAGE *p_page);
int search_node(int key, BTPAGE *p_page, int *pos);
void ins_in_page(int key,int r_child, BTPAGE *p_page, int pos);
void split(int key, int r_child, BTPAGE *p_oldpage, int *promo_key, int *promo_r_child, BTPAGE *p_newpage,int poss, int *promo_pos); 	
  	
 ///////////////////////////// 	
char *identifica_campo(char *ts,int num)
{
	if(num==1)
		return strtok(ts,"#");
}	
int pega_registro(FILE *p_out, char *p_reg) //utilizado para saber se o registro está vazio ou não
{
     unsigned char bytes;
     unsigned int a=0 ;
     if (!fread(&bytes, sizeof(char), 1, p_out))
       return 0;
     else {
            fread(p_reg, bytes, 1, p_out); 
 			a= (int)bytes; //retorna em int
            return a;
          }  
}
void insercao(FILE  *out,FILE *insere)
{
    int temp,temp_2=0,tam_reg,tam_arq,temp3,tamanho;
	char reg[160];
	
	//////////
	int promoted; // boolean: tells if a promotion from below
    int root, // rrn of root page
    promo_rrn; // rrn promoted from below
    int promo_key, // key promoted from below
    key; // next key to insert in tree

    if (btopen()){
       root = getroot();
    }else{
       root = create_tree();
    }
    //////////////
	
	fseek(insere,0,SEEK_END); //coloca o arquivo no fim
	tam_arq=ftell(insere);   //pega tamnho do arquivo
	fseek(insere,0,0);	     //coloca o arquivo no início
	
	fseek(out,0,SEEK_END);
	int pos=ftell(out);
	fseek(out,0,0);
	printf("\nPos:%d\n",pos);
	
	tam_reg=pega_registro(out,reg);
	if (tam_reg==0) //nada no arquivo
	{
		
   		fread(&film,sizeof(film),1,insere); 
   		temp=(ftell(insere))/156; 
   		sprintf(reg,"##%s#%s#%s#%s#%s#%c",film.cliente,film.codfilme,film.nome,film.filme,film.genero,temp);
   		temp_2=strlen(reg); //temp2 armazenará o tamanho do registro reg que será usado para se locomover no arquivo saida
   		//conf temp_2
		sprintf(reg,"%c#%s#%s#%s#%s#%s#%c",temp_2,film.cliente,film.codfilme,film.nome,film.filme,film.genero,temp);
   		fwrite(reg, sizeof(char), strlen(reg), out); //registro é escrito no arquivo
   		
   		int con1,con2;
   		con1=atoi(film.cliente);
   		con2=atoi(film.codfilme);
   		
   		//printf("%d %d",con1,con2);
   		
          root = create_root((con1*100+con2), NIL, NIL,pos);
   		  printf("\nChave %d inserida com sucesso\n",(con1*100+con2));
   		}
	else //já tem conteúdo
	{	
		fseek(out,0,SEEK_END); 
		temp3=ftell(out);
		fseek(out,ftell(out)-1,0); //arquivo na penultima posição
		fread(&temp_2,sizeof(int),1,out); //será lido a temp_2 para saber em que parte do arquivo está
		fseek(out,0,SEEK_END); //coloca o arquivo saida na ultima posição para inserção
		fseek(insere,(temp_2)*156,0); //locomove o arquivo insere com a temp_2 salva

		if((((int)temp_2)*156)>=tam_arq)  //se a temp_2 for maior que o tamanho arquivo insere significa que chegou ao fim
		{ 
			printf("\nArquivo insere.bin chegou ao fim\n"); return;
		}
		
   		fread(&film,sizeof(film),1,insere); 
   		temp=(ftell(insere)+2)/156; //temp que será utilizada para se mover no arquivo insere
   		//coloca dados no registro reg
   		sprintf(reg,"##%s#%s#%s#%s#%s#%c",film.cliente,film.codfilme,film.nome,film.filme,film.genero,temp);
   		temp_2=strlen(reg); //temp2 armazenará o tamanho do registro reg que será usado para se locomover no arquivo saida
   		sprintf(reg,"%c#%s#%s#%s#%s#%s#%c",temp_2,film.cliente,film.codfilme,film.nome,film.filme,film.genero,temp);
   		
   		int con1,con2,con3=0;
   		con1=atoi(film.cliente);
   		con2=atoi(film.codfilme); ///aki mudar nome variavel
   		
   		promoted = insert(root, (con1*100+con2), &promo_rrn, &promo_key,pos,&con3);
       
        if(con3!=1)
		  {
		  	if (promoted)
          		root = create_root(promo_key, root, promo_rrn,pos);
		  	fwrite(reg, sizeof(char), strlen(reg), out);
		  }
          	
         else
         {
         	sprintf(reg,"#%c",temp);
		  	fwrite(reg, sizeof(char), strlen(reg), out);
		 }
	}

	fclose(out);  fclose(insere);btclose(); 
}
//////////////
int insert (int rrn, int key, int *promo_r_child, int *promo_key, int poss, int *encontrou){
       BTPAGE page, // current page
       newpage; // new page created if split occurs
       int found, promoted; // boolean values
       int pos,
       p_b_rrn; // rrn promoted from below
       int p_b_key; // key promoted from below
       int promo_pos;
       if (rrn == NIL){
          *promo_key = key;
          *promo_r_child = NIL;
          promo_pos = NIL;
          return(YES);
       }
      
       btread(rrn, &page);
       found = search_node ( key, &page, &pos);
       
       if (found){
          printf ("\nChave %d duplicada\n", key);  *encontrou=1; return(0);
       }
       
       promoted = insert(page.child[pos], key, &p_b_rrn, &p_b_key,poss,encontrou);
 
 
 		/*if (promoted==-1){
		  	return(-1);
       } */
       if (!promoted){
		  	return(NO);
       }
       if(page.keycount < MAXKEYS){
       		printf("\nChave %d inserida com sucesso\n",p_b_key);//mudar de lugar para n repetir
             ins_in_page(p_b_key, p_b_rrn, &page,poss);
             btwrite(rrn, &page);
             return(NO);
       }else{
             split(p_b_key, p_b_rrn, &page, promo_key, promo_r_child, &newpage,poss, &promo_pos);
             printf("\nChave %d inserida com sucesso\n",p_b_key);//mudar de lugar para n repetir
			 btwrite(rrn, &page);
             btwrite(*promo_r_child, &newpage);
             return(YES);
       }
}
int btopen(){
         btfd = fopen("btree.bin", "r+b");
         return (btfd > 0);
}
void btclose(){
          fclose(btfd);
}
int getroot(){
      int root;

      fseek(btfd, 0, 0);
      
      if (fread(&root, sizeof(int), 1, btfd) == 0){
         printf("Error: Unable to get root. \007\n");
         exit(1);
      }      
      return (root); 
}
void putroot(int root){
              fseek(btfd, 0, 0);
              fwrite(&root, sizeof(int), 1, btfd);
}
int create_tree(){
      
      btfd = fopen("btree.bin","w+b");
      fclose (btfd);
      btopen();
      
      return 0;
}
int getpage(){
      long addr;
      
      fseek(btfd, 0, 2);
      addr = ftell(btfd);
      
      if (addr == 0)
        return 0;
      else { 
             addr = addr - sizeof(int);
             return ((int) addr / PAGESIZE);
           }
}
void btread (int rrn, BTPAGE *page_ptr){
       long addr;
       
       addr = ((long)rrn * (long)PAGESIZE) + sizeof(int);
       fseek(btfd, addr, 0);
       fread(page_ptr, PAGESIZE, 1, btfd);
}
void btwrite(int rrn, BTPAGE *page_ptr){
              long addr;
              
              addr = ((long)rrn * (long)PAGESIZE) + sizeof(int);
              fseek(btfd, addr, 0);
              fwrite(page_ptr, PAGESIZE, 1, btfd);
}
int create_root(int key, int left, int right, int pos){
                 BTPAGE page;  int rrn;
                 
                 rrn = getpage();
                 pageinit (&page);
                 page.key[0] = key;
                 page.child[0] = left;
                 page.child[1] = right;
                 page.keycount = 1;
                 page.pos[0]=pos;
                 btwrite(rrn, &page);
                 putroot(rrn);
                 printf("Pos:%d",page.pos[0]);
                 return(rrn);
}
void pageinit(BTPAGE *p_page){
 int j;
 
     for (j = 0; j < MAXKEYS; j++){
         p_page->key[j] = NOKEY;
         p_page->child[j] = NIL;
         p_page->pos[j]=NIL;
     }
     p_page->child[MAXKEYS] = NIL;
     p_page->pos[MAXKEYS] = NIL;
}
int search_node(int key, BTPAGE *p_page, int *pos){
 int i;

     for (i = 0; i < p_page->keycount && key > p_page->key[i]; i++);
     *pos = i;
     if (*pos < p_page->keycount && key == p_page->key[*pos]){
        return(YES);
     }else{
        return(NO);
     }
}
void ins_in_page(int key,int r_child, BTPAGE *p_page, int pos){
 int j;
 
     for(j = p_page-> keycount; key < p_page->key[j-1] && j > 0; j--){
           p_page->key[j] = p_page->key[j-1];
           p_page->child[j+1] = p_page->child[j];
     }
     
     p_page->keycount++;
     p_page->key[j] = key;
     p_page->child[j+1] = r_child;
     p_page->pos[j]=pos;
}
void split(int key, int r_child, BTPAGE *p_oldpage, int *promo_key, int *promo_r_child, BTPAGE *p_newpage,int poss, int *promo_pos){
 int j;
 int mid;
 int workkeys[MAXKEYS+1];  int workchil[MAXKEYS+2];int workpos[MAXKEYS+2];
	printf("\nDivisao de noh\n");
      for (j = 0; j < MAXKEYS; j++){
          workkeys[j] = p_oldpage->key[j];
          workchil[j] = p_oldpage->child[j];
          workpos[j] = p_oldpage->pos[j];
          //printf("%d %d A",p_oldpage->key[j],p_oldpage->child[j]);
      }
      //printf("\n");
      //workkeys[0], workkeys[1] *p_newpage
      workchil[j] = p_oldpage->child[j];
      workpos[j] = p_oldpage->pos[j];
      for (j = MAXKEYS; key < workkeys[j-1] && j > 0; j--){
          workkeys[j] = workkeys[j-1];
          workchil[j+1] = workchil[j];
          workpos[j+1] = workpos[j];
      }
      workkeys[j] = key;
      workchil[j+1] = r_child;
      workpos[j+1]=poss;

      *promo_r_child = getpage();
      *promo_pos = getpage();
      pageinit(p_newpage);
      //Alterar aki para funcionar para ordem par (chave=3)
      /*for (j = 0; j <= MINKEYS; j++){ //att mudei isso e qse foi
          p_oldpage->key[j] = workkeys[j];
          //printf("B%d %d %d\n",workkeys[j],workkeys[j+1+MINKEYS],NOKEY);
          p_oldpage->child[j] = workchil[j];
          p_newpage->key[j] = workkeys[j+1+MINKEYS];
          p_newpage->child[j] = workchil[j+1+MINKEYS];
          p_oldpage->key[j+MINKEYS] = NOKEY;
          p_oldpage->child[j+1+MINKEYS] = NIL;
          //printf("B%d %d %d\n",j,j+MINKEYS,j+1+MINKEYS);
      }*/
       p_oldpage->key[0] = workkeys[0];
        p_oldpage->child[0] = workchil[0];
        p_oldpage->pos[0] = workpos[0];
        p_newpage->key[0] = workkeys[2];
        p_newpage->child[0] = workchil[2];
        p_newpage->pos[0] = workpos[2];
        
        p_oldpage->key[1] = NOKEY;
        p_oldpage->key[2] = NOKEY;//adicioei isso att
        p_oldpage->child[2] = NIL;
        p_oldpage->child[1] = workchil[1];
        p_oldpage->pos[2] = NIL;
        p_oldpage->pos[1] = workpos[1];
        //att qse la
        //talvez fazer ideia (ver anots)
        p_newpage->key[1] = workkeys[3];
        p_newpage->child[1] = workchil[3];
        p_newpage->child[2] = workchil[4]; //ultima mudança e aparentemente certyo
        p_newpage->pos[1] = workpos[3];
        p_newpage->pos[2] = workpos[4]; //ultima mudança e aparentemente certyo
      
      p_newpage->keycount = MAXKEYS - MINKEYS;
      p_oldpage->keycount = MINKEYS;
      *promo_key = workkeys[MINKEYS];//att *promo_key = workkeys[MINKEYS-1];

      printf("Chave %d promovida",*promo_key);
} 
/////////
void Listar_todos_aux(int rrn){
    BTPAGE page;
    int i;
    if(rrn != NIL){
        btread(rrn, &page);
        for(i = 0; i<page.keycount; i++)
        {
            Listar_todos_aux(page.child[i]);
            printf(" %d",  page.key[i]);
        }
        Listar_todos_aux(page.child[i]);
    }
}	

void Listar_todos(int rrn){
	int root;
	BTPAGE page;
		
	root = getroot();
	btread(root, &page);
	
	//printf("%d %d ", *page.pos, page.key[1]);	
	Listar_todos_aux(page.child[0]);
	if(page.key[0]!=-1)
	printf(" %d ", page.key[0]);
	
	Listar_todos_aux(page.child[1]);
	if(page.key[1]!=-1)
	printf(" %d ", page.key[1]);
	
	Listar_todos_aux(page.child[2]);
	if(page.key[2]!=-1)
	printf(" %d ", page.key[2]);
	
	Listar_todos_aux(page.child[3]);
	
				
}


///////////////
int main()
{
	int op=1;
	FILE *insere,*out,*arvore;
	

	while(op!=4)
	{
		printf("\n1- Insercao\n2-Procurar por chave primaria\n3-Procurar por chave secundaria\n4-Sair\nEscolha uma opcao:");
		scanf("%d",&op);
		printf("\n");
			switch(op)
				{
				case 1:
						if ((insere = fopen("insere.bin","r+b")) == NULL)
						 {
							printf("Nao foi possivel abrir o arquivo");
							getche();
							return 0;
						 }
						  printf("\ninsere.bin carregado");
						if ((out = fopen("principal.bin","a+b")) == NULL)
						 {
							printf("Nao foi possivel abrir o arquivo");
							getche();
							return 0;
						 }
						printf("\nprincipal.bin carregado");
					
						insercao(out,insere);	
						break;
				case 2:
						btopen();
						Listar_todos(0);
						btclose();
				case 4:
						printf("\nSaindo...\n");break;
				default: printf("\nErro\n");
				}
	}
	fclose(out); fclose(insere); 
}
