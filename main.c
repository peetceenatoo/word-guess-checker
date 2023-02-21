// TOGLIERE SUM


#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define ALPHABET_SIZE 64

unsigned int k, k_max;                                  // lunghezza delle parole
char *word, *ref, *res,                                 // parola corrente, ref e res globali così li alloco una volta
*stack,                                                 // lo stack per lo stesso motivo
*tempString;                                            // e un appoggio

struct trie_node{
    struct trie_node *leftMostChild;
    struct trie_node *nextSibling;
    char *string;
};
struct trie_node head, **vStack;

typedef struct {                                        // memorizza per ogni simbolo un'informazione sul numero di occorrenze
    unsigned char info;                                 // 1: conosco le occorrenze esatte; 2: conosco il numero minimo; 0: nulla
    unsigned int occurrences;                           // completa l'informazione precedente
} f_o_struct;

typedef struct {                                        // memorizza per ogni posizione un'informazione sui simboli ammissibili
    unsigned char info;                                 // 1: conosco il simbolo; 2: escludo la lista di simboli; 0: nulla
    char fixed;                                         // contiene il simbolo nel caso 1
    bool symbols[ALPHABET_SIZE];                        // i simboli incompatibili sono true nel caso 2
} f_s_struct;

typedef struct {
    f_o_struct fixed_occurrences[ALPHABET_SIZE];        // array statico con ALPHABET_SIZE (una per simbolo)
    f_s_struct *fixed_symbols;                          // allocherò un array dinamico con k celle (una per posizione)
} constrictions;

constrictions rules;

typedef struct {                                        // questa serve a contare + e | per ogni simbolo
    unsigned int plus;                                  // conta i +
    unsigned int bar;                                   // conta i |
    unsigned int sum;                                   // conta il numero di + più |
} s_c_struct;

/******************************************************************************************************************************************/

void updateDictionary( const bool start );
void insertWord( const bool start );

void newGame();

bool isAllowed();

void wordChecker( const unsigned int refOccurrencesCounter[ALPHABET_SIZE] );
unsigned char getPos( const char c );
char getChar( const unsigned char p );

void updateRules( const unsigned int wordOccurrencesCounter[ALPHABET_SIZE] );
void initRules();
unsigned int filter();
void printFiltered();
void clearTrie();

/******************************************************************************************************************************************/

/******************************************************************************************************************************************/

int main(){

    bool flag;                                                              // per vedere quando arrivo alla fine del file
    unsigned char j;                                                        // indice per contare fino ad ALPHABET_SIZE
    char k_string[11];                                                      // per leggere l'intero da input

    j=0;                                                                    // leggo k
    do{
        k_string[j] = fgetc(stdin);
        j++;
    } while( k_string[j-1] != '\n' && !feof(stdin) );
    k = atoi( k_string );

    k_max = ( k < 17 ) ? 17 : k;                                            // necessariamente +inserisci_inizio deve entrare in word
    word = calloc( k_max+2, 1 );                                            // alloco il massimo tra k e 17 (lunghezza di +inserisci_inizio) + 1 per il \n + 1 per il terminatore
    ref = calloc( k+2, 1 );                                                 // alloco k+1 caratteri per ref + 1 per il \n + 1 per il terminatore
    res = calloc( k+1, 1 );                                                 // alloco k+1 caratteri per res
    tempString = calloc( k+1, 1 );                                          // alloco k+1 caratteri per tempString
    stack = calloc( k+1, 1 );                                               // alloco k+1 caratteri per stack
    vStack = malloc( (k+1)*sizeof(struct trie_node*) );                     // alloco k puntatori a nodi

    head.leftMostChild = NULL;
    head.nextSibling = NULL;
    head.string = NULL;

    rules.fixed_symbols = malloc( k*sizeof(f_s_struct) );                   // alloco l' array dinamico di k fixedSymbolStruct

    flag = true;
    while( flag ){                                                          // continuo a leggere fino alla fine del file (nota: dal primo comando qui leggo solo altri comandi)
        if( fgets( word, k_max+2, stdin ) == NULL ) flag = false;           // leggo la parola (ricorda che se io passassi k_max+1 alla fgets, questa per costruzione quando leggo +inserisci_inizio non leggerebbe il \n lasciandolomelo lì
        else{                                                               // ...altrimenti analizzo la parola
            if( word[0] != '+' ){
                word[strlen(word)-1] = '\0';
                insertWord( true );
            }
            else if( word[1] == 'i' ) updateDictionary( true );             // aggiungo nuove parole
            else{
                initRules();                                                // (inizializzando le regole)
                newGame();
                clearTrie();                                                // pulendo le flag nell'albero
            }
        }
    }

}

/******************************************************************************************************************************************/

/******************************************************************************************************************************************/

void updateDictionary( const bool start ){

    static bool flag;

    flag = true;
    while( flag ){
        if( fgets( word, k_max+2, stdin ) == NULL ) flag = false;
        else if( word[0] == '+' ) flag = false;
        else{
            word[strlen(word)-1] = '\0';
            insertWord( start );
        }
    }

}

/******************************************************************************************************************************************/

void insertWord( const bool start ){

    static unsigned int i, l, m;                                                                                    // indici per contare fino a k
    static struct trie_node *currentDaddy, *curr, *temp;                                                            // temp lo uso per splittare e per creare la desinenza
    static bool flag,                                                                                               // flag che rappresenta la possibile presenza del carattere i-esimo nell'albero
    valid;                                                                                                          // flag della nuova parola da mettere
    static unsigned char tempBool, j;
    static unsigned int wordCounter[ALPHABET_SIZE];                                                                 // conto occorrenze della parola che sto inserendo

    currentDaddy = &head;
    curr = currentDaddy->leftMostChild;
    i=0;
    flag = true;

    if( start ){
        while( flag ){                                                                                              // finché credo di poter trovare il carattere i-esimo nell'albero...
            if( ( curr == NULL ) || ( curr->string[1] > word[i] ) ){                                                // caso in cui inserisco in testa (no figli o primo figlio più grande, nel nuovo livello)
                temp = malloc( sizeof(struct trie_node) );                                                          // creo desinenza
                temp->nextSibling = curr;
                temp->leftMostChild = NULL;
                temp->string = calloc( k-i+2, 1 );
                temp->string[0] = 49;                                                                               // imposto il bit di validità
                m=0;                                                                                                // scrivo la stringa nel nodo
                do temp->string[m+1] = word[i+m];
                while( ++m <= k-i );
                currentDaddy->leftMostChild = temp;                                                                 // collego
                flag = false;                                                                                       // termino
            }
            else{                                                                                                   // altrimenti...
                if( curr->string[1] != word[i] ){                                                                   // cerco il nodo, e mi fermo al precedente se non lo trovo
                    while( ( curr->nextSibling != NULL ) && ( curr->nextSibling->string[1] <= word[i] ) )
                        curr = curr->nextSibling;
                }
                if( curr->string[1] == word[i] ){                                                                   // quando trovo la lettera [1] in comune...
                    l=1;                                                                                            // conto quante lettere c'erano in comune (almeno 1)
                    while( ( curr->string[l+1] != '\0' ) && ( curr->string[l+1] == word[i+l] ) )
                        l++;
                    i+=l;                                                                                           // e faccio il conto delle lettere raggiunte finora
                    if( curr->string[l+1] == '\0' ){                                                                // e qui o devo andare avanti nell'albero
                        currentDaddy = curr;
                        curr = currentDaddy->leftMostChild;
                    }
                    else{                                                                                           // oppure devo splittare
                        m=0;                                                                                        // mi salvo la stringa del nodo non splittato
                        do tempString[m] = curr->string[m+1];
                        while( curr->string[++m] != '\0' );
                        free( curr->string );                                                                       // libero lo spazio occupato dalla stringa del nodo non splittato
                        temp = malloc( sizeof(struct trie_node) );                                                  // alloco la seconda metà dello split
                        temp->string = calloc( strlen(tempString)-l+2, 1 );
                        temp->string[0] = 49;
                        temp->leftMostChild = curr->leftMostChild;
                        temp->nextSibling = NULL;
                        curr->string = calloc( l+2, 1 );                                                            // ri-alloco la stringa per la prima metà dello split
                        curr->string[0] = 49;
                        curr->leftMostChild = temp;                                                                 // collego
                        m=0;                                                                                        // copio le lettere in comune sulla prima metà
                        do curr->string[m+1] = tempString[m];
                        while( ++m < l );
                        curr->string[l+1] = '\0';
                        do temp->string[m-l+1] = tempString[m];
                        while( ++m <= strlen(tempString) );                                                         // copio le lettere non in comune sulla seconda metà
                        currentDaddy = curr;                                                                        // mi sposto per mettere la desinenza
                        curr = currentDaddy->leftMostChild;
                        temp = malloc( sizeof(struct trie_node) );                                                  // creo desinenza
                        temp->leftMostChild = NULL;
                        temp->string = calloc( k-i+2, 1 );
                        temp->string[0] = 49;
                        m=0;                                                                                        // copio le lettere rimaste (non in comune) nel nodo-desinenza
                        do temp->string[m+1] = word[i+m];
                        while( ++m <= k-i );
                        if( strcmp( curr->string+1, word+i ) > 0 ){                                                   // se lo devo mettere a sinistra
                            temp->nextSibling = curr;
                            currentDaddy->leftMostChild = temp;
                        }
                        else{                                                                                       // se lo devo mettere a destra
                            temp->nextSibling = curr->nextSibling;
                            curr->nextSibling = temp;
                        }
                        flag = false;                                                                               // termino
                    }
                }
                else{                                                                                               // altrimenti aggiungo alla fine
                    temp = malloc( sizeof(struct trie_node) );
                    temp->nextSibling = curr->nextSibling;
                    temp->leftMostChild = NULL;
                    temp->string = calloc( k-i+2, 1 );
                    temp->string[0] = 49;
                    m=0;
                    do temp->string[m+1] = word[i+m];
                    while( ++m <= k-i );
                    curr->nextSibling = temp;
                    flag = false;
                }
            }
        }
    }
    else{
        valid = true;
        for( j=0; j<ALPHABET_SIZE; j++ ) wordCounter[j] = 0;
        while( flag ){                                                                                              // finchè credo di poter trovare il carattere i-esimo nell'albero...
            if( ( curr == NULL ) || ( curr->string[1] > word[i] ) ){                                                // caso in cui inserisco in testa (no figli o primo figlio più grande, nel nuovo livello)
                temp = malloc( sizeof(struct trie_node) );                                                          // creo desinenza
                temp->nextSibling = curr;
                temp->leftMostChild = NULL;
                temp->string = calloc( k-i+2, 1 );
                m=0;                                                                                                // scrivo la stringa nel nodo e nel frattempo controllo che bit di validità mettere
                do{
                    temp->string[m+1] = word[i+m];                                                                  // occhio che m deve partire da zero ma di fatto nella parola sono già al carattere i-esimo
                    wordCounter[getPos(word[i+m])]++;
                    if( valid ){
                        if( rules.fixed_symbols[i+m].info == 1 ){
                            if( rules.fixed_symbols[i+m].fixed != word[i+m] ) valid = false;
                        }
                        else if( rules.fixed_symbols[i+m].info == 2 ){
                            if( rules.fixed_symbols[i+m].symbols[getPos(word[i+m])] ) valid = false;
                        }
                        if( rules.fixed_occurrences[getPos(word[i+m])].info == 1 ){
                            if( rules.fixed_occurrences[getPos(word[i+m])].occurrences < wordCounter[getPos(word[i+m])] )
                                valid = false;
                        }
                    }
                } while( ++m < k-i );
                temp->string[m+1] = '\0';
                for( m=0; m<k; m++ ){ // OCCCHIOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO
                    if(rules.fixed_occurrences[getPos(ref[m])].info == 1 ){
                        if( rules.fixed_occurrences[getPos(ref[m])].occurrences != wordCounter[getPos(ref[m])] ) valid = false;
                    }
                    else if( rules.fixed_occurrences[getPos(ref[m])].info == 2 ){
                        if( rules.fixed_occurrences[getPos(ref[m])].occurrences > wordCounter[getPos(ref[m])] ) valid = false;
                    }
                }
                //for( j=0; j<ALPHABET_SIZE && valid; j++ ) {                                                                             // ed i controlli finali sulle occorrenze
                //    if (rules.fixed_occurrences[j].info == 1) {
                //        if (rules.fixed_occurrences[j].occurrences != wordCounter[j]) valid = false;
                //    } else if (rules.fixed_occurrences[j].info == 2) {
                //        if (rules.fixed_occurrences[j].occurrences > wordCounter[j]) valid = false;
                //    }
                //}
                temp->string[0] = ( valid ) ? 49 : 48;
                currentDaddy->leftMostChild = temp;                                                                                     // collego
                flag = false;                                                                                                           // termino
            }
            else{                                                                                                                       // altrimenti...
                if( curr->string[1] != word[i] ){                                                                                       // cerco il nodo, e mi fermo al precedente se non lo trovo
                    while( ( curr->nextSibling != NULL ) && ( curr->nextSibling->string[1] <= word[i] ) )
                        curr = curr->nextSibling;
                }
                if( curr->string[1] == word[i] ){                                                                                       // quando trovo la lettera [1] in comune...
                    l=0;                                                                                                                // conto quante lettere c'erano in comune (parto da zero perchè devo fare i controlli
                    while( ( curr->string[l+1] != '\0' ) && ( curr->string[l+1] == word[i+l] ) ){
                        wordCounter[getPos(curr->string[l+1])]++;
                        if( ( curr->string[0] == 48 ) && valid ){
                            if( rules.fixed_symbols[i+l].info == 1 ){
                                if( rules.fixed_symbols[i+l].fixed != word[i+l] ) valid = false;
                            }
                            else if( rules.fixed_symbols[i+l].info == 2 ){
                                if( rules.fixed_symbols[i+l].symbols[getPos(word[i+l])] ) valid = false;
                            }
                            if( rules.fixed_occurrences[getPos(word[i+l])].info == 1 ){
                                if( rules.fixed_occurrences[getPos(word[i+l])].occurrences < wordCounter[getPos(word[i+l])] )
                                    valid = false;
                            }
                        }
                        l++;
                    }
                    i+=l;                                                                                                               // e faccio il conto delle lettere raggiunte finora
                    if( curr->string[l+1] == '\0' ){                                                                                    // e qui o devo andare avanti nell'albero
                        currentDaddy = curr;
                        curr = currentDaddy->leftMostChild;
                    }
                    else{                                                                                                               // oppure devo splittare
                        m=0;                                                                                                            // mi salvo la stringa del nodo non splittato
                        do tempString[m] = curr->string[m+1];
                        while( curr->string[++m] != '\0' );
                        tempBool = curr->string[0];
                        free( curr->string );                                                                                           // libero lo spazio occupato dalla stringa del nodo non splittato
                        temp = malloc( sizeof(struct trie_node) );                                                                      // alloco la seconda metà dello split
                        temp->string = calloc( strlen(tempString)-l+2, 1 );
                        temp->leftMostChild = curr->leftMostChild;
                        temp->nextSibling = NULL;
                        curr->string = calloc( l+2, 1 );                                                                                // ri-alloco la stringa per la prima metà dello split
                        curr->leftMostChild = temp;                                                                                     // collego
                        m=0;                                                                                                            // copio le lettere in comune sulla prima metà e nel frattempo controllo che bit di validità mettere
                        do curr->string[m+1] = tempString[m];
                        while( ++m < l );
                        curr->string[l+1] = '\0';
                        curr->string[0] = ( valid ) ? 49 : 48;
                        do temp->string[m-l+1] = tempString[m];                                                                         // copio le lettere non in comune sulla seconda metà
                        while( ++m <= strlen(tempString) );
                        temp->string[0] = tempBool;
                        currentDaddy = curr;                                                                                            // mi sposto per mettere la desinenza
                        curr = currentDaddy->leftMostChild;

                        temp = malloc( sizeof(struct trie_node) );                                                                      // creo la desinenza
                        temp->leftMostChild = NULL;
                        temp->string = calloc( k-i+2, 1 );
                        m=0;                                                                                                            // copio le lettere rimaste (non in comune) nel nodo-desinenza e nel frattempo controllo che bit di validità mettere
                        do{
                            temp->string[m+1] = word[i+m];
                            wordCounter[getPos(word[i+m])]++;
                            if( valid ){
                                if( rules.fixed_symbols[i+m].info == 1 ){
                                    if( rules.fixed_symbols[i+m].fixed != word[i+m] ) valid = false;
                                }
                                else if( rules.fixed_symbols[i+m].info == 2 ){
                                    if( rules.fixed_symbols[i+m].symbols[getPos(word[i+m])] ) valid = false;
                                }
                                if( rules.fixed_occurrences[getPos(word[i+m])].info == 1 ){
                                    if( rules.fixed_occurrences[getPos(word[i+m])].occurrences < wordCounter[getPos(word[i+m])] )
                                        valid = false;
                                }
                            }
                        } while( ++m < k-i );
                        temp->string[m+1] = '\0';
                        for( m=0; m<k; m++ ){ // OCCCHIOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO
                            if(rules.fixed_occurrences[getPos(ref[m])].info == 1 ){
                                if( rules.fixed_occurrences[getPos(ref[m])].occurrences != wordCounter[getPos(ref[m])] ) valid = false;
                            }
                            else if( rules.fixed_occurrences[getPos(ref[m])].info == 2 ){
                                if( rules.fixed_occurrences[getPos(ref[m])].occurrences > wordCounter[getPos(ref[m])] ) valid = false;
                            }
                        }
                        //for( j=0; j<ALPHABET_SIZE && valid; j++ ) {                                                                             // ed i controlli finali sulle occorrenze
                        //    if (rules.fixed_occurrences[j].info == 1) {
                        //        if (rules.fixed_occurrences[j].occurrences != wordCounter[j]) valid = false;
                        //    } else if (rules.fixed_occurrences[j].info == 2) {
                        //        if (rules.fixed_occurrences[j].occurrences > wordCounter[j]) valid = false;
                        //    }
                        //}
                        temp->string[0] = ( valid ) ? 49 : 48;
                        if( strcmp( curr->string+1, word+i ) > 0 ){                                                                     // se lo devo mettere a sinistra
                            temp->nextSibling = curr;
                            currentDaddy->leftMostChild = temp;
                        }
                        else{                                                                                                           // se lo devo mettere a destra
                            temp->nextSibling = curr->nextSibling;
                            curr->nextSibling = temp;
                        }
                        flag = false;                                                                                                   // termino
                    }
                }
                else{                                                                                                                   // altrimenti aggiungo alla fine
                    temp = malloc( sizeof(struct trie_node) );
                    temp->nextSibling = curr->nextSibling;
                    temp->leftMostChild = NULL;
                    temp->string = calloc( k-i+2, 1 );
                    m=0;
                    do{                                                                                                                 // facendo tutti i controlli live
                        temp->string[m+1] = word[i+m];
                        wordCounter[getPos(word[i+m])]++;
                        if( valid ){
                            if( rules.fixed_symbols[i+m].info == 1 ){
                                if( rules.fixed_symbols[i+m].fixed != word[i+m] ) valid = false;
                            }
                            else if( rules.fixed_symbols[i+m].info == 2 ){
                                if( rules.fixed_symbols[i+m].symbols[getPos(word[i+m])] ) valid = false;
                            }
                            if( rules.fixed_occurrences[getPos(word[i+m])].info == 1 ){
                                if( rules.fixed_occurrences[getPos(word[i+m])].occurrences < wordCounter[getPos(word[i+m])] )
                                    valid = false;
                            }
                        }
                    } while( ++m < k-i );
                    temp->string[m+1] = '\0';
                    for( m=0; m<k; m++ ){ // OCCCHIOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO
                        if(rules.fixed_occurrences[getPos(ref[m])].info == 1 ){
                            if( rules.fixed_occurrences[getPos(ref[m])].occurrences != wordCounter[getPos(ref[m])] ) valid = false;
                        }
                        else if( rules.fixed_occurrences[getPos(ref[m])].info == 2 ){
                            if( rules.fixed_occurrences[getPos(ref[m])].occurrences > wordCounter[getPos(ref[m])] ) valid = false;
                        }
                    }
                    //for( j=0; j<ALPHABET_SIZE && valid; j++ ) {                                                                             // ed i controlli finali sulle occorrenze
                    //    if (rules.fixed_occurrences[j].info == 1) {
                    //        if (rules.fixed_occurrences[j].occurrences != wordCounter[j]) valid = false;
                    //    } else if (rules.fixed_occurrences[j].info == 2) {
                    //        if (rules.fixed_occurrences[j].occurrences > wordCounter[j]) valid = false;
                    //    }
                    //}
                    temp->string[0] = ( valid ) ? 49 : 48;
                    curr->nextSibling = temp;
                    flag = false;
                }
            }
        }
    }
}

/******************************************************************************************************************************************/

void newGame(){

    static unsigned int refOccurrencesCounter[ALPHABET_SIZE], chances;                          // refOccurrencesCounter serve a contare gli / da mettere, mentre chances mi permette di terminare la partita se l'utente perde
    static bool win;                                                                            // per vedere quando finire il ciclo in caso di vittoria
    static unsigned char j;                                                                     // indice per contare fino ad ALPHABET_SIZE
    static unsigned int i;                                                                      // indice per contare fino a k
    static char chances_string[11];                                                             // per leggere l'intero da ingresso
    static unsigned int left;

    for ( j=0; j<ALPHABET_SIZE; j++ ) refOccurrencesCounter[j] = 0;                             // inizializzo tutto il necessario
    win = false;

    if( fgets( ref, k+2, stdin ) != NULL ){
        ref[strlen(ref)-1] = '\0';
        j=0;                                                                                    // leggo chances
        do chances_string[j++] = fgetc(stdin);
        while( chances_string[j-1] != '\n' && !feof(stdin) );
        chances = atoi( chances_string );

        for( i=0; i<k; i++ ) refOccurrencesCounter[getPos(ref[i])]++;                           // conto le occorrenze dei simboli in ref

        while( ( chances > 0 ) && ( !win ) ){                                                   // partita
            if( fgets( word, k_max+2, stdin ) != NULL ){
                if( word[0] == '+' ){                                                           // se il primo carattere è un più, decido cosa fare
                    if( word[1] == 'i' ) updateDictionary( false );                             // aggiungo parole
                    else printFiltered();                                                       // stampo l'albero filtrato
                }
                else{                                                                           // altrimenti...
                    if( word[strlen(word)-1] == '\n' )                                          // (mattia occhio alla fine del file però...)
                        word[strlen(word)-1] = '\0';
                    if( strcmp( word, ref ) == 0 ) win = true;                                  // se è la parola giusta win
                    else if( isAllowed() ){                                                     // se era ammissibile, chance sprecata
                        chances--;
                        wordChecker( refOccurrencesCounter );
                        left = filter();                                                        // con print=false conta soltanto e mette in last
                        printf( "%s\n%u\n", res, left );                               // stampo res e il numero di filtrate rimaste
                    }
                    else printf( "not_exists\n" );                                     // se non è ammessa not_exists
                }
            }
        }
        if( win ) printf( "ok\n" );                                                    // se ho vinto: ok
        else printf( "ko\n" );                                                         // altrimenti: ko

    }

}

/******************************************************************************************************************************************/

bool isAllowed(){

    static unsigned int i, l;                                                                               // indici per contare fino a k
    static struct trie_node *currentDaddy, *curr;                                                           // per scorrere l'albero

    currentDaddy = &head;                                                                                   // parto dalla radice
    curr = currentDaddy->leftMostChild;                                                                     // e mi metto sul primissimo nodo
    i=0;
    while( i < k ){
        if( ( curr == NULL ) || ( curr->string[1] > word[i] ) ) return false;
        if( curr->string[1] != word[i] ){
            while( ( curr->nextSibling != NULL ) && ( curr->nextSibling->string[1] <= word[i] ) )
                curr = curr->nextSibling;
        }
        if( curr->string[1] != word[i] ) return false;
        l=1;
        while( ( curr->string[l+1] != '\0' ) && ( curr->string[l+1] == word[i+l] ) )
            l++;
        i+=l;
        if( curr->string[l+1] != '\0' ) return false;
        if( i < k ){
            currentDaddy = curr;
            curr = currentDaddy->leftMostChild;
        }
    }

    return true;

}

/******************************************************************************************************************************************/

void wordChecker( const unsigned int refOccurrencesCounter[ALPHABET_SIZE] ){

    static unsigned int wordRightOccurrencesCounter[ALPHABET_SIZE],                                                         // wordRightOccurrencesCounter contiene le occorrenze del simbolo in ref
    resNotPlusCounter[ALPHABET_SIZE],                                                                                       // wordOccurrencesCounter conta iterazione per iterazione le volte in cui il simbolo è "occorso" in posizione sbagliata
    wordOccurrencesCounter[ALPHABET_SIZE];                                                                                  // parole ammissibili rimaste (devo stampare il numero)
    static unsigned int i;                                                                                                  // indice per contare fino a k
    static unsigned char j;                                                                                                 // indice per contare fino ad ALPHABET_SIZE

    for ( j=0; j<ALPHABET_SIZE; j++ ){                                                                                      // inizializzo
        wordRightOccurrencesCounter[j] = 0;
        resNotPlusCounter[j] = 0;
        wordOccurrencesCounter[j] = 0;
    }

    for( i=0; i<k; i++ ){                                                                                                   // cerco i + e metto - altrove in res
        if( word[i] == ref[i] ){
            wordRightOccurrencesCounter[getPos(ref[i])]++;                                                                  // nel frattempo conto le occorrenze del simbolo
            res[i] = '+';
        }
        else res[i] = '-';
    }
    res[i] = '\0';

    for( i=0; i<k; i++ ){                                                                                                                                       // ovunque non ci sia un + decido se mettere / o |
        if( res[i] != '+' ){
            if( resNotPlusCounter[getPos(word[i])] >= ( refOccurrencesCounter[getPos(word[i])] - wordRightOccurrencesCounter[getPos(word[i])] ) )               // nota che wordRightOccurrencesCounter <= refOccurrencesCounter, quindi se sono uguali è impossibile che io metta una | per quel carattere
                res[i] = '/';                                                                                                                                   // sbagliata e finite
            else
                res[i] = '|';                                                                                                                                   // sbagliata, ma c'è altrove
            resNotPlusCounter[getPos(word[i])]++;                                                                                                               // (mi serve contare "live" le occorrenze del simbolo nella parola)
        }
        wordOccurrencesCounter[getPos(word[i])]++;
    }

    updateRules( wordOccurrencesCounter );                                                                                                                      // aggiorno le regole
}

/******************************************************************************************************************************************/

unsigned char getPos( const char c ){                                                           // funzione biunivoca che assegna la posizione su ALPHABET_SIZE ai simboli ammessi

    if( ( c >= 65 ) && ( c <= 90 ) ) return c-54;
    if( ( c >= 97 ) && ( c <= 122 ) ) return c-59;
    if( ( c >= 48 ) && ( c <= 57 ) ) return c-47;
    if( ( c == 95 ) ) return 37;

    return 0;
}

/******************************************************************************************************************************************/

char getChar( const unsigned char p ){                                                          // inversa della funzione precedente

    if( ( p >= 11 ) && ( p <= 36 ) ) return p+54;
    if( ( p >= 38 ) && ( p <= 63 ) ) return p+59;
    if( ( p >= 1 ) && ( p <= 10 ) ) return p+47;
    if( ( p == 37 ) ) return 95;

    return 45;
}

/******************************************************************************************************************************************/

void updateRules( const unsigned int wordOccurrencesCounter[ALPHABET_SIZE] ){

    static unsigned char j;                                                                     // indice per contare fino ad ALPHABET_SIZE
    static unsigned int i;                                                                      // indice per contare fino a k
    static s_c_struct resSignsCounter[ALPHABET_SIZE];                                           // array di strutture per contare i + e le |

    for( j=0; j<ALPHABET_SIZE; j++ ){                                                           // inizializzo l'array di strutture
        resSignsCounter[j].plus = 0;
        resSignsCounter[j].bar = 0;
        resSignsCounter[j].sum = 0;
    }

    for( i=0; i<k; i++ ){                                                                       // cerco informazioni sui simboli nelle posizioni i, contando intanto per ogni simbolo i + e le |
        if( res[i] == '+' ){                                                                    // cerco simboli fissati
            resSignsCounter[getPos(word[i])].plus++;
            resSignsCounter[getPos(word[i])].sum++;
            if( rules.fixed_symbols[i].info != 1 ){                                             // se avevo un'informazione meno precisa, aggiorno e fisso il carattere in posizione i
                rules.fixed_symbols[i].info = 1;
                rules.fixed_symbols[i].fixed = word[i];
            }
        }
        else{                                                                                   // cerco simboli da escludere
            if( res[i] == '|' ){
                resSignsCounter[getPos(word[i])].bar++;
                resSignsCounter[getPos(word[i])].sum++;
            }

            if( rules.fixed_symbols[i].info == 0 )                                              // se non sapevo nulla, aggiorno...
                rules.fixed_symbols[i].info = 2;
            if( rules.fixed_symbols[i].info == 2 )                                              // e a meno di sapere il simbolo esatto, escludo il corrente
                rules.fixed_symbols[i].symbols[getPos(word[i])] = true;
        }
    }

    for( j=0; j<ALPHABET_SIZE; j++ ){                                                                       // cerco informazioni sulle occorrenze per ogni simbolo
        if( ( rules.fixed_occurrences[j].info != 1 ) && ( wordOccurrencesCounter[j] != 0 ) ){               // (solo se non so già il numero esatto e il simbolo effettivamente occorre)
            if( resSignsCounter[j].sum == wordOccurrencesCounter[j] ) {                                     // se "non sono arrivato allo /" ho un numero minimo
                if( rules.fixed_occurrences[j].info == 0 ){                                                 // se non avevo informazione, aggiorno
                    rules.fixed_occurrences[j].occurrences = resSignsCounter[j].sum;
                }
                else {                                                                                      // altrimenti ( info == 2 ) controllo che la mia nuova informazione sia significativa
                    if ( rules.fixed_occurrences[j].occurrences < resSignsCounter[j].sum )
                        rules.fixed_occurrences[j].occurrences = resSignsCounter[j].sum;
                }
                rules.fixed_occurrences[j].info = 2;                                                        // e specifico l'informazione
            }
            else {                                                                                          // altrimenti so esattamente quante sono (anche zero)
                rules.fixed_occurrences[j].info = 1;
                rules.fixed_occurrences[j].occurrences = resSignsCounter[j].sum;
            }
        }
    }

}

/******************************************************************************************************************************************/

void initRules(){

    static unsigned char j;                                                                // indice per contare fino ad ALPHABET_SIZE
    static unsigned int i;                                                                 // indice per contare fino a k

    for( j=0; j<ALPHABET_SIZE; j++ ) rules.fixed_occurrences[j].info = 0;                  // inizializzo l'array con i vincoli per le occorrenze per simbolo (non serve inizializzare gli interi, quando cambio info scrivo sempre sull'intero associato)
    for( i=0; i<k; i++ ){                                                                  // inizializzo l'array dinamico con i vincoli per i simboli per posizione
        rules.fixed_symbols[i].info = 0;
        for( j=0; j<ALPHABET_SIZE; j++ ) rules.fixed_symbols[i].symbols[j] = false;        // qui mi serve sempre avere tutto a false per quando cambio il valore di info
    }
}

/******************************************************************************************************************************************/

unsigned int filter( ){

    static unsigned char cCont = 0, nCont = 0;                                                  // contatori per numero di caratteri e numero di nodi letti
    unsigned char cLastCont = cCont;                                                            // per ricordarmi quando sono stato chiamato quante lettere avevo già contato
    unsigned int i;                                                                             // indice per contare fino a k
    static bool firstTime = true,                                                               // per sapere se devo inizializzare lo stack una prima volta (purtroppo è globale e non static)
    flag, idk;                                                                                  // flag booleana che uso per uscire dai cicli o per sapere se eseguire alcune istruzioni
    static unsigned int stackCounter[ALPHABET_SIZE] = { 0 };                                    // contatore dei simboli nello stack usato per il controllo dei vincoli sulle occorrenze subito prima di stampare
    struct trie_node *temp, *temp2;
    static unsigned int left;

    if( nCont == 0 ){                                                                           // se sono ancora al nodo radice
        if( firstTime ){                                                                        // potrebbe essere la prima volta che chiamo la funzione ricorsiva in assoluto
            for( i=0; i<=k; i++ ){                                                              // allora gli stack devono essere inizializzato
                stack[i] = '\0';
                vStack[i] = NULL;
            }
            vStack[nCont] = &head;                                                              // e devo ancora mettere la radice in vStack[0]
            firstTime = false;
        }
        left = 0;                                                                               // inoltre potrei dover inizializzare left per usarlo come contatore
    }

    temp = vStack[nCont]->leftMostChild;                                                        // su questo livello parto dal figlio sinistro

    if( rules.fixed_symbols[cCont].info == 1 ){                                                 // se è noto il cCount-esimo carattere posso scartare tutti i nodi tranne uno
        flag = false;
        temp2 = temp;
        while( temp2 != NULL ){                                                                 // metto a non valid tutti i fratelli e mi tengo, se lo trovo, quello che cerco
            if( temp2->string[1] == rules.fixed_symbols[cCont].fixed ){
                flag = true;
                temp = temp2;
            }
            else temp2->string[0] = 48;
            temp2 = temp2->nextSibling;
        }
        if( ( temp->string[0] == 49 ) && flag ){                                                                                                                    // se l'ho trovato ed è valid
            while( temp->string[cCont-cLastCont+1] != '\0' && flag ){                                                                                               // controllo che nessun carattere del nodo scelto dia problemi
                if( rules.fixed_symbols[cCont].info == 1 ){                                                                                                         // controllando se conosco quello esatto
                    if( rules.fixed_symbols[cCont].fixed != temp->string[cCont-cLastCont+1] ) flag = false;
                }
                else if( rules.fixed_symbols[cCont].info == 2 ){                                                                                                    // controllando che non sia inaccettabile
                    if( rules.fixed_symbols[cCont].symbols[getPos(temp->string[cCont-cLastCont+1])] ) flag = false;
                }
                if( rules.fixed_occurrences[getPos(temp->string[cCont-cLastCont+1])].info == 1 ){                                                                   // controllando che io non abbia già troppi caratteri di un certo tipo
                    if( stackCounter[getPos(temp->string[cCont-cLastCont+1])]>= rules.fixed_occurrences[getPos(temp->string[cCont-cLastCont+1])].occurrences )
                        flag = false;
                }
                if( flag ){                                                                                                                                         // e se ho passato tutti i controlli vado al prossimo
                    stack[cCont] = temp->string[cCont-cLastCont+1];
                    stackCounter[getPos(stack[cCont])]++;
                    cCont++;
                }
            }
            if( flag ){                                                                                                                                                                 // se ho trovato il nodo e se tutti i suoi caratteri hanno passato tutti i controlli finora
                if( cCont == k ){                                                                                                                                                       // se ho finito controllo le occorrenze, esatte e minime
                    for( i=0; i<k && flag; i++ ){
                        if( ( rules.fixed_occurrences[getPos(ref[i])].info == 1 ) && ( stackCounter[getPos(ref[i])] != rules.fixed_occurrences[getPos(ref[i])].occurrences ) )
                            flag = false;
                        if( ( rules.fixed_occurrences[getPos(ref[i])].info == 2 ) && ( stackCounter[getPos(ref[i])] < rules.fixed_occurrences[getPos(ref[i])].occurrences ) )
                            flag = false;
                    }
                    //for( j=0; j<ALPHABET_SIZE && flag; j++ ){
                    //    if( ( rules.fixed_occurrences[j].info == 1 ) && ( stackCounter[j] != rules.fixed_occurrences[j].occurrences ) ) flag = false;
                    //    if( ( rules.fixed_occurrences[j].info == 2 ) && ( stackCounter[j] < rules.fixed_occurrences[j].occurrences ) ) flag = false;
                    //}
                    if( !flag ) temp->string[0] = 48;                                                                                                                                   // e tolgo il valid se non va bene
                    else left++;
                }
                else{                                                                                                                                                                   // se non ho finito posso scendere nell'albero
                    vStack[++nCont] = temp;
                    left = filter();
                    nCont--;
                }
            }
            else temp->string[0] = 48;                                                                                                                                                  // ovviamente se non ho passato i controlli tolgo il valid
            while( cCont - cLastCont > 0 ){                                                                                                                                             // ripristino i contatori e lo stack
                stackCounter[getPos(stack[--cCont])]--;
                stack[cCont] = '\0';
            }
        }
    }
    else{
        idk = ( rules.fixed_symbols[cCont].info == 0 ) ? true : false;                                                                                                          // se non ho informazione sul primo carattere dovrò controllare tutti i fratelli su questo livello
        while( temp != NULL ){
            if( idk || !rules.fixed_symbols[cCont].symbols[getPos(temp->string[1])] ){                                                                                          // (forse non ha senso questo controllo preliminare se l'info non è di tipo 1, ma amen, non perdo tempo, nel caso è solo insensato controllarlo differenziandolo dagli altri ahahhahahaha)
                if( temp->string[0] == 49 ){                                                                                                                                    // (se invece non è valid non ha proprio senso fare niente)
                    flag = true;
                    while( temp->string[cCont-cLastCont+1] != '\0' && flag ){                                                                                                   // controllo che nessun carattere del nodo scelto dia problemi
                        if( rules.fixed_symbols[cCont].info == 1 ){                                                                                                             // controllando se conosco quello esatto
                            if( rules.fixed_symbols[cCont].fixed != temp->string[cCont-cLastCont+1] ) flag = false;
                        }
                        else if( rules.fixed_symbols[cCont].info == 2 ){                                                                                                        // controllando che non sia inaccettabile
                            if( rules.fixed_symbols[cCont].symbols[getPos(temp->string[cCont-cLastCont+1])] ) flag = false;
                        }
                        if( rules.fixed_occurrences[getPos(temp->string[cCont-cLastCont+1])].info == 1 ){                                                                       // controllando che io non abbia già troppi caratteri di un certo tipo
                            if( stackCounter[getPos(temp->string[cCont-cLastCont+1])] >= rules.fixed_occurrences[getPos(temp->string[cCont-cLastCont+1])].occurrences )
                                flag = false;
                        }
                        if( flag ){                                                                                                                                             // e se ho passato tutti i controlli vado al prossimo
                            stack[cCont] = temp->string[cCont-cLastCont+1];
                            stackCounter[getPos(stack[cCont])]++;
                            cCont++;
                        }
                    }
                    if( flag ){                                                                                                                                                 // se ho trovato il nodo e se tutti i suoi caratteri hanno passato tutti i controlli finora
                        if( cCont == k ){                                                                                                                                       // se ho finito controllo le occorrenze, esatte e minime
                            for( i=0; i<k && flag; i++ ){
                                if( ( rules.fixed_occurrences[getPos(ref[i])].info == 1 ) && ( stackCounter[getPos(ref[i])] != rules.fixed_occurrences[getPos(ref[i])].occurrences ) )
                                    flag = false;
                                if( ( rules.fixed_occurrences[getPos(ref[i])].info == 2 ) && ( stackCounter[getPos(ref[i])] < rules.fixed_occurrences[getPos(ref[i])].occurrences ) )
                                    flag = false;
                            }
                            //for( j=0; j<ALPHABET_SIZE && flag; j++ ){
                            //    if( ( rules.fixed_occurrences[j].info == 1 ) && ( stackCounter[j] != rules.fixed_occurrences[j].occurrences ) ) flag = false;
                            //    if( ( rules.fixed_occurrences[j].info == 2 ) && ( stackCounter[j] < rules.fixed_occurrences[j].occurrences ) ) flag = false;
                            //}
                            if( !flag ) temp->string[0] = 48;                                                                                                                   // e tolgo il valid se non va bene
                            else left++;
                        }
                        else{                                                                                                                                                   // se non ho finito posso scendere nell'albero
                            vStack[++nCont] = temp;
                            left = filter();
                            nCont--;
                        }
                    }
                    else temp->string[0] = 48;                                                                                                                                  // ovviamente se non ho passato i controlli tolgo il valid
                    while( cCont - cLastCont > 0 ){                                                                                                                             // ripristino i contatori e lo stack
                        stackCounter[getPos(stack[--cCont])]--;
                        stack[cCont] = '\0';
                    }
                }
            }
            else temp->string[0] = 48;                                                                                                                                          // (anche se il primo carattere andava escluso)
            temp = temp->nextSibling;
        }
    }

    return left;

}

/******************************************************************************************************************************************/

void printFiltered(){

    static unsigned char cCont = 0, nCont = 0;                                                  // contatori per numero di caratteri e numero di nodi letti
    unsigned char cLastCont = cCont;                                                            // per ricordarmi quando sono stato chiamato quante lettere avevo già contato
    unsigned int i;                                                                             // indice per contare fino a k
    static bool firstTime = true;                                                               // per sapere se devo inizializzare lo stack una prima volta (purtroppo è globale e non static)
    struct trie_node *temp;

    if( nCont == 0 ){                                                                           // se sono ancora al nodo radice
        if( firstTime ){                                                                        // potrebbe essere la prima volta che chiamo la funzione ricorsiva in assoluto
            for( i=0; i<=k; i++ ){                                                              // allora gli stack devono essere inizializzato
                stack[i] = '\0';
                vStack[i] = NULL;
            }
            vStack[nCont] = &head;                                                              // e devo ancora mettere la radice in vStack[0]
            firstTime = false;
        }
    }

    temp = vStack[nCont]->leftMostChild;

    while( temp != NULL ){
        if( temp->string[0] == 49 ){                                                            // se (e solo se) è valid
            while( temp->string[cCont-cLastCont+1] != '\0'){                                    // aggiungo tutti i caratteri allo stack
                stack[cCont] = temp->string[cCont-cLastCont+1];
                cCont++;
            }
            if( cCont == k ) printf( "%s\n", stack );                                           // se ho tutta la parola la stampa
            else{                                                                               // mentre se non ho finito chiamo i figli
                vStack[++nCont] = temp;
                printFiltered();
                nCont--;
            }
            while( cCont - cLastCont > 0 ) stack[--cCont] = '\0';                               // e quando sarò ritornato qui potrò dunque ripristinare
        }
        temp = temp->nextSibling;
    }

}

/******************************************************************************************************************************************/

void clearTrie(){

    static unsigned char nCont = 0;                                                             // contatori per numero di caratteri e numero di nodi letti
    unsigned int i;                                                                             // indice per contare fino a k
    static bool firstTime = true;                                                               // per sapere se devo inizializzare lo stack una prima volta (purtroppo è globale e non static)
    struct trie_node *temp;

    if( nCont == 0 ){                                                                           // se sono ancora al nodo radice
        if( firstTime ){                                                                        // potrebbe essere la prima volta che chiamo la funzione ricorsiva in assoluto
            for( i=0; i<=k; i++ ) vStack[i] = NULL;                                             // allora devo inizializzare lo stack
            vStack[nCont] = &head;                                                              // e devo ancora mettere la radice in vStack[0]
            firstTime = false;
        }
    }

    temp = vStack[nCont]->leftMostChild;

    while( temp != NULL ){
        temp->string[0] = 49;
        if( temp->leftMostChild != NULL ){
            vStack[++nCont] = temp;
            clearTrie();
            nCont--;
        }
        temp = temp->nextSibling;
    }

}

/******************************************************************************************************************************************/