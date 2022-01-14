#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>

struct estrutura
  	{
    	char cliente[3],codfilme[3],nome[50], filme[50], genero[50];
  	}film; 
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
	
	fseek(insere,0,SEEK_END); //coloca o arquivo no fim
	tam_arq=ftell(insere);   //pega tamnho do arquivo
	fseek(insere,0,0);	     //coloca o arquivo no início
	
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
   		fwrite(reg, sizeof(char), strlen(reg), out); //registro é escrito no arquivo
   	
	}

	fclose(out);  fclose(insere);
}

int main()
{
	int op=1;
	FILE *insere,*out;

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
				case 4:
						printf("\nSaindo...\n");break;
				default: printf("\nErro\n");
				}
	}
	fclose(out); fclose(insere); 
}
