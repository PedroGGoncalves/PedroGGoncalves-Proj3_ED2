#define MAXKEYS 4
#define MINKEYS MAXKEYS/2
#define NIL (-1)
#define NOKEY -1
#define NO 0
#define YES 1
#include <stdio.h>
#include <stdlib.h>
#define PAGESIZE sizeof(BTPAGE)

typedef struct{
        int keycount; // number of keys in page
        int key[MAXKEYS]; // the actual keys
        int child[MAXKEYS+1]; // ptrs to rrns of descendants
}BTPAGE;

int root; // rrn of root page
FILE *btfd; // global file descriptor for "btree.dat"

/* prototypes */
int insert (int rrn, int key, int *promo_r_child, int *promo_key);
int btopen();
void btclose();
int getroot();
void putroot(int root);
int create_tree();
int getpage();
void btread (int rrn, BTPAGE *page_ptr);
void btwrite(int rrn, BTPAGE *page_ptr);
int create_root(int key, int left, int right);
void pageinit(BTPAGE *p_page);
int search_node(int key, BTPAGE *p_page, int *pos);
void ins_in_page(int key,int r_child, BTPAGE *p_page);
void split(int key, int r_child, BTPAGE *p_oldpage, int *promo_key, int *promo_r_child, BTPAGE *p_newpage);

int main(){
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
    scanf("%d",&key);
    while (key != -1){
          promoted = insert(root, key, &promo_rrn, &promo_key);
          if (promoted)
          root = create_root(promo_key, root, promo_rrn);
          scanf("%d",&key);
    }
    btclose(); 
}
int insert (int rrn, int key, int *promo_r_child, int *promo_key){
       BTPAGE page, // current page
       newpage; // new page created if split occurs
       int found, promoted; // boolean values
       int pos,
       p_b_rrn; // rrn promoted from below
       int p_b_key; // key promoted from below
       
       if (rrn == NIL){
          *promo_key = key;
          *promo_r_child = NIL;
          return(YES);
       }
       
       btread(rrn, &page);
       found = search_node ( key, &page, &pos);
       
       if (found){
          printf ("Chave %d duplicada\n", key);  return(0);
       }
       
       promoted = insert(page.child[pos], key, &p_b_rrn, &p_b_key);
 
       if (!promoted){
          return(NO);
       }
 
       if(page.keycount < MAXKEYS){
       		printf("Chave %d inserida com sucesso\n",p_b_key);//mudar de lugar para n repetir
             ins_in_page(p_b_key, p_b_rrn, &page);
             btwrite(rrn, &page);
             return(NO);
       }else{
             split(p_b_key, p_b_rrn, &page, promo_key, promo_r_child, &newpage);
             printf("Chave %d inserida com sucesso\n",p_b_key);//mudar de lugar para n repetir
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
      int key;
      
      btfd = fopen("btree.bin","w+b");
      fclose (btfd);
      btopen();
      scanf("%d",&key);
      return (create_root(key, NIL, NIL));
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
int create_root(int key, int left, int right){
                 BTPAGE page;  int rrn;
                 
                 rrn = getpage();
                 pageinit (&page);
                 page.key[0] = key;
                 page.child[0] = left;
                 page.child[1] = right;
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
void ins_in_page(int key,int r_child, BTPAGE *p_page){
 int j;
 
     for(j = p_page-> keycount; key < p_page->key[j-1] && j > 0; j--){
           p_page->key[j] = p_page->key[j-1];
           p_page->child[j+1] = p_page->child[j];
     }
     p_page->keycount++;
     p_page->key[j] = key;
     p_page->child[j+1] = r_child;
}
void split(int key, int r_child, BTPAGE *p_oldpage, int *promo_key, int *promo_r_child, BTPAGE *p_newpage){
 int j;
 int mid;
 int workkeys[MAXKEYS+1];  int workchil[MAXKEYS+2];
	printf("Divisao de noh\n");
      for (j = 0; j < MAXKEYS; j++){
          workkeys[j] = p_oldpage->key[j];
          workchil[j] = p_oldpage->child[j];
      }
      
      workchil[j] = p_oldpage->child[j];
      
      for (j = MAXKEYS; key < workkeys[j-1] && j > 0; j--){
          workkeys[j] = workkeys[j-1];
          workchil[j+1] = workchil[j];
      }
 
      workkeys[j] = key;
      workchil[j+1] = r_child;

      *promo_r_child = getpage();
      pageinit(p_newpage);
      //Alterar aki para funcionar para ordem par (chave=3)
      for (j = 0; j < MINKEYS; j++){
          p_oldpage->key[j] = workkeys[j];
          p_oldpage->child[j] = workchil[j];
          p_newpage->key[j] = workkeys[j+1+MINKEYS];
          p_newpage->child[j] = workchil[j+1+MINKEYS];
          p_oldpage->key[j+MINKEYS] = NOKEY;
          p_oldpage->child[j+1+MINKEYS] = NIL;
      }
      p_oldpage->child[MINKEYS] = workchil[MINKEYS];
      p_newpage->child[MINKEYS] = workchil[j+1+MINKEYS];
      p_newpage->keycount = MAXKEYS - MINKEYS;
      p_oldpage->keycount = MINKEYS;
      *promo_key = workkeys[MINKEYS];
      //
      printf("Chave %d promovida\n",*promo_key);
} 
