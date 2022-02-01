#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAXKEYS 3
#define MINKEYS (MAXKEYS-1)/2
#define NIL (-1)
#define NOKEY -1
#define NO 0
#define YES 1
#define PAGESIZE sizeof(BTPAGE)
/*
Grupo Gustavo Rosseto e Pedro Gonçalves
*/

struct estrutura
  	{
    	char cliente[3],codfilme[3],nome[50], filme[50], genero[50];
  	}film; 
 //////////
 typedef struct{
        int keycount; // number of keys in page
        int key[MAXKEYS]; // the actual keys
        int child[MAXKEYS+1]; // ptrs to rrns of descendants
        int pos[MAXKEYS];
}BTPAGE;

int root; // rrn of root page
FILE *btfd; // global file descriptor for "btree.dat"

/* prototypes */
int insert (int rrn, int key, int *promo_r_child, int *promo_key, int poss,int *encontrou, int *promo_pos);
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
	return 0;
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
    int temp,temp_2=0,tam_reg,tam_arq,temp1,temp4=0;
	char reg[160],temp2[100];
	//////////
	int promoted; // boolean: tells if a promotion from below
    int root, // rrn of root page
    promo_rrn; // rrn promoted from below
    int promo_key,promo_pos; // key promoted from below
    //key; // next key to insert in tree

    if (btopen()){
       root = getroot();
    }else{
       root = create_tree();
    }
    //////////////
	
	fseek(insere,0,SEEK_END); //coloca o arquivo no fim
	tam_arq=ftell(insere);   //pega tamnho do arquivo
	fseek(insere,0,0);	     //coloca o arquivo no início
	
	fseek(out,0,SEEK_END);	//mesma coisa mas do arquivo de saida
	int pos=ftell(out);
	fseek(out,0,0);
	
	tam_reg=pega_registro(out,reg); // tam_reg == 0, arquivo vazio
	if (tam_reg==0) //nada no arquivo
	{
		
   		fread(&film,sizeof(film),1,insere); 
   		temp=(ftell(insere))/156; 	//marcar posicao							
   		sprintf(reg,"##%s#%s#%s#%s#%s#%c",film.cliente,film.codfilme,film.nome,film.filme,film.genero,temp);
   		temp_2=strlen(reg); //temp2 armazenará o tamanho do registro reg que será usado para se locomover no arquivo saida
   		
		sprintf(reg,"%c#%s#%s#%s#%s#%s#%c",temp_2,film.cliente,film.codfilme,film.nome,film.filme,film.genero,temp);
   		fwrite(reg, sizeof(char), strlen(reg), out); //registro é escrito no arquivo
   		
   		strcat(strcpy(temp2,film.cliente),film.codfilme); //copia o codigo cliente e concatena com o condifo do fime
   		temp1=atoi(temp2);
        root = create_root(temp1, NIL, NIL,pos);
   		printf("\nChave %d inserida com sucesso\n",temp1);
   		}
	else //já tem conteúdo
	{	
		fseek(out,0,SEEK_END); 
	//	temp3=ftell(out);
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
		
   		strcat(strcpy(temp2,film.cliente),film.codfilme);
   		temp1=atoi(temp2);
   		
   		promoted = insert(root, temp1, &promo_rrn, &promo_key,pos,&temp4,&promo_pos);
       
        if(temp4!=1)	//se temp4 ==1, significa que ja existe esse registro
		  {
		  	if (promoted)
          		root = create_root(promo_key, root, promo_rrn,promo_pos);
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
int insert (int rrn, int key, int *promo_r_child, int *promo_key, int poss, int *encontrou, int *promo_pos){
       BTPAGE page, // current page
       newpage; // new page created if split occurs
       int found, promoted; // boolean values
       int pos,
       p_b_rrn; // rrn promoted from below
       int p_b_key; // key promoted from below
       int p_b_pos; // pos promoted from below
       if (rrn == NIL){
          *promo_key = key;
          *promo_r_child = NIL;
          *promo_pos = poss;
          return(YES);
       }
      
       btread(rrn, &page);
       found = search_node ( key, &page, &pos);
       
       if (found){
          printf ("\nChave %d duplicada\n", key);  *encontrou=1; return(0);
       }
       
       promoted = insert(page.child[pos], key, &p_b_rrn, &p_b_key,poss,encontrou,&p_b_pos);
 
       if (!promoted){
		  	return(NO);
       }
       if(page.keycount < MAXKEYS){
       		printf("\nChave %d inserida com sucesso\n",p_b_key);//mudar de lugar para n repetir
             ins_in_page(p_b_key, p_b_rrn, &page,p_b_pos);
             btwrite(rrn, &page);
             return(NO);
       }else{
             split(p_b_key, p_b_rrn, &page, promo_key, promo_r_child, &newpage,p_b_pos, promo_pos);
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
                 page.pos[0]=pos;
                 page.keycount = 1;
                 btwrite(rrn, &page);
                 putroot(rrn);
                 return(rrn);
}
void pageinit(BTPAGE *p_page){
 int j;
 
     for (j = 0; j < MAXKEYS; j++){
         p_page->key[j] = NOKEY;
         p_page->child[j] = NIL;
         p_page->pos[j]=NOKEY;
     }
     p_page->child[MAXKEYS] = NIL;
    
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
           p_page->pos[j]=p_page->pos[j-1];
     }
     
     p_page->keycount++;
     p_page->key[j] = key;
     p_page->child[j+1] = r_child;
     p_page->pos[j]=pos;
}
void split(int key, int r_child, BTPAGE *p_oldpage, int *promo_key, int *promo_r_child, BTPAGE *p_newpage,int poss, int *promo_pos){
 int j;
 int workkeys[MAXKEYS+1];  int workchil[MAXKEYS+2];int workpos[MAXKEYS+2];
	printf("\nDivisao de noh\n");
      for (j = 0; j < MAXKEYS; j++){
          workkeys[j] = p_oldpage->key[j];
          workchil[j] = p_oldpage->child[j];
          workpos[j] = p_oldpage->pos[j]; 
      }
      workchil[j] = p_oldpage->child[j];
      for (j = MAXKEYS; key < workkeys[j-1] && j > 0; j--){
          workkeys[j] = workkeys[j-1];
          workchil[j+1] = workchil[j];
          workpos[j] = workpos[j-1];
      }
      workkeys[j] = key;
      workchil[j+1] = r_child;
      workpos[j]=poss;

      *promo_r_child = getpage();
      
      pageinit(p_newpage);
   
       p_oldpage->key[0] = workkeys[0];
        p_oldpage->child[0] = workchil[0];
        p_oldpage->pos[0] = workpos[0];
        p_newpage->key[0] = workkeys[2];
        p_newpage->child[0] = workchil[2];
        p_newpage->pos[0] = workpos[2];
        
        p_oldpage->key[1] = NOKEY;
        p_oldpage->key[2] = NOKEY;
        p_oldpage->child[2] = NIL;
        p_oldpage->child[1] = workchil[1];
        p_oldpage->pos[1] = NOKEY;
        p_oldpage->pos[2] = NOKEY;
      
        p_newpage->key[1] = workkeys[3];
        p_newpage->child[1] = workchil[3];
        p_newpage->child[2] = workchil[4]; 
        p_newpage->pos[1] = workpos[3];
      
      p_newpage->keycount = MAXKEYS - MINKEYS;
      p_oldpage->keycount = MINKEYS;
      *promo_key = workkeys[MINKEYS];
	  *promo_pos = workpos[MINKEYS]; 
      printf("Chave %d promovida",*promo_key);
} 
/////////
void exibirTela (char *filme)			
{
	char *pch;
	pch = identifica_campo(filme,1);
	printf("Cod Cliente:%s\n",pch);
	pch = identifica_campo(NULL,1);
	printf("Cod Filme:%s\n",pch);
	pch = identifica_campo(NULL,1);
	printf("Nome Cliente:%s\n",pch);
	pch = identifica_campo(NULL,1);
	printf("Nome Filme:%s\n",pch);
	pch = identifica_campo(NULL,1);
	printf("Genero:%s\n",pch);
	pch = identifica_campo(NULL,1);
}
void Listar_todos_aux(int rrn,FILE *out){		//
    BTPAGE page;
    int i;
    char num5;
    fseek(out,0,0);
    if(rrn != NIL){
        btread(rrn, &page);
        for(i = 0; i<page.keycount; i++)
        {
            Listar_todos_aux(page.child[i],out);
            printf("%d\n",  page.key[i]);
			fseek(out,page.pos[i],0);
            fread(&num5,sizeof(char),1,out);//posição para se locomover
			char filme[(int)num5];
			fread(&filme,sizeof(filme),1,out);
			exibirTela(filme);
        }
        Listar_todos_aux(page.child[i],out);
    }
}	

void Listar_todos(int rrn,FILE *out){
	int root;
	BTPAGE page;
	char num5;
    fseek(out,0,0);	
	root = getroot();
	btread(root, &page);
	
	Listar_todos_aux(page.child[0],out);
	if(page.key[0]!=-1)
	{
		printf("%d\n",  page.key[0]);
		fseek(out,page.pos[0],0);
	    fread(&num5,sizeof(char),1,out);//posição para se locomover
		char filme[(int)num5];
		fread(&filme,sizeof(filme),1,out);
		exibirTela(filme);
	}
	
	Listar_todos_aux(page.child[1],out);
	if(page.key[1]!=-1)
	{
		printf("%d\n",  page.key[1]);
		fseek(out,page.pos[1],0);
	    fread(&num5,sizeof(char),1,out);//posição para se locomover
		char filme[(int)num5];
		fread(&filme,sizeof(filme),1,out);
		exibirTela(filme);	
	}
	Listar_todos_aux(page.child[2],out);
	if(page.key[2]!=-1)
	{
		printf("%d\n",  page.key[2]);
		fseek(out,page.pos[2],0);
	    fread(&num5,sizeof(char),1,out);//posição para se locomover
		char filme[(int)num5];
		fread(&filme,sizeof(filme),1,out);
		exibirTela(filme);
	}
	Listar_todos_aux(page.child[3],out);	
	
	fclose(out);			
}
///////////////
void Listar_especifico_aux(FILE *out, int rrn, int valor, int *cond){
	BTPAGE page;
    int i;
    char num5;
    fseek(out,0,0);
    if(rrn != NIL && *cond==0){
        btread(rrn, &page);
        for(i = 0; i<page.keycount; i++)
        {
            if(valor==page.key[i])
            {
            printf("Chave %d encontrada, pagina %d, posicao %d\n",  page.key[i], rrn  , i);//pagina ta errado
			fseek(out,page.pos[i],0);
            fread(&num5,sizeof(char),1,out);//posição para se locomover
			char filme[(int)num5];
			fread(&filme,sizeof(filme),1,out);
			exibirTela(filme);
			*cond=1;
			return;
			}
			Listar_especifico_aux(out,page.child[i],valor,cond);   
        }
       	Listar_especifico_aux(out,page.child[i],valor,cond);
    }
}
void Listar_especifico(FILE* busca,FILE *out, int rrn)
{
	int root,cont=0,cond=0,temp1,temp3;
	BTPAGE page;
	root = getroot();
	btread(root, &page);
	char num5,reg[160],temp2[100];
	int temp4;
	
    fseek(out,0,0);
	fseek(busca,0,0);
	FILE *aux;
	if ((aux = fopen("auxbusca.bin","a+b")) == NULL)
	{
		printf("Nao foi possivel abrir o arquivo");
		return ;
	}
	int tam_reg=pega_registro(aux,reg);
	if(tam_reg==0)
	{
		sprintf(reg,"%c",1);
		fwrite(reg, sizeof(char), strlen(reg), aux);
	}
	else
	{
		fseek(aux,0,SEEK_END);	
		
		fseek(aux,ftell(aux)-1,0);	
		fread(&temp4,sizeof(int),1,aux);
		
		fseek(aux,ftell(aux)+1,0);
		
		cont= temp4;
		cont=cont+1;
		sprintf(reg,"%c",cont);
		fwrite(reg, sizeof(char), strlen(reg), aux);
		fseek(busca,0,SEEK_END);
		if(ftell(busca)<=(cont-1)*6)
		{
			printf("Fim de arquivo\n");
			return;	
		}
		else
		{
			fseek(busca,(cont-1)*6,0);
		}
		
	}
	fread(&film.cliente,sizeof(film.cliente),1,busca); 
	fread(&film.codfilme,sizeof(film.codfilme),1,busca); 
   	
	strcat(strcpy(temp2,film.cliente),film.codfilme);
	temp1=atoi(temp2);
    temp3=page.key[0];
	if(page.key[0]!=-1&&temp1==temp3&&cond==0)
	{
		printf("Chave %d encontrada, pagina %d, posicao %d\n",  page.key[0], getroot(), 0);//conferir isso de pag
		fseek(out,page.pos[0],0);
	    fread(&num5,sizeof(char),1,out);//posição para se locomover
		char filme[(int)num5];
		fread(&filme,sizeof(filme),1,out);
		exibirTela(filme);
		cond=1;
	}
	Listar_especifico_aux(out,page.child[0],temp1,&cond);

	temp3=page.key[1];
	if(page.key[1]!=-1&&temp1==temp3&&cond==0)
	{
		printf("Chave %d encontrada, pagina %d, posicao %d\n",  page.key[1], getroot(), 1);
		fseek(out,page.pos[1],0);
	    fread(&num5,sizeof(char),1,out);//posição para se locomover
		char filme[(int)num5];
		fread(&filme,sizeof(filme),1,out);
		exibirTela(filme);
		cond=1;
	}
	
	Listar_especifico_aux(out,page.child[1],temp1,&cond);
	
	temp3=page.key[2];
	
	if(page.key[2]!=-1&&temp1==temp3&&cond==0)
	{
		printf("Chave %d encontrada, pagina %d, posicao %d\n",  page.key[2], getroot(), 2);
		fseek(out,page.pos[2],0);
	    fread(&num5,sizeof(char),1,out);//posição para se locomover
		char filme[(int)num5];
		fread(&filme,sizeof(filme),1,out);
		exibirTela(filme);
		cond=1;
	}
	Listar_especifico_aux(out,page.child[2],(temp1),&cond);
	
	Listar_especifico_aux(out,page.child[3],(temp1),&cond);	
	if(cond==0)
	{
		printf("Chave %d nao encontrada\n",temp1);
	}  				
	fclose(out);fclose(busca);fclose(aux);

}
////////////
int main()
{
	int op=1;
	FILE *insere,*out,*busca;

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
						
							return 0;
						 }
						  printf("\ninsere.bin carregado");
						if ((out = fopen("principal.bin","a+b")) == NULL)
						 {
							printf("Nao foi possivel abrir o arquivo");
							
							return 0;
						 }
						printf("\nprincipal.bin carregado");
					
						insercao(out,insere);	
						break;
				case 2:
						if ((out = fopen("principal.bin","a+b")) == NULL)
						 {
							printf("Nao foi possivel abrir o arquivo");
						
							return 0;
						 }
						printf("\nprincipal.bin carregado\n");
						btopen();
						Listar_todos(0,out);
						btclose();
						break;
				case 3:
						if ((busca = fopen("busca.bin","r+b")) == NULL)
						 {
							printf("Nao foi possivel abrir o arquivo");
						
							return 0;
						 }
						 
						printf("\nbusca.bin carregado");
						  if ((out = fopen("principal.bin","a+b")) == NULL)
						 {
							printf("Nao foi possivel abrir o arquivo");
						
							return 0;
						 }
						printf("\nprincipal.bin carregado\n");
						btopen();
						Listar_especifico(busca,out,0);
						btclose();
						break;
				case 4:
						printf("\nSaindo...\n");break;
				default: printf("\nErro\n");
				}
	}
	fclose(out); fclose(insere); 
}
