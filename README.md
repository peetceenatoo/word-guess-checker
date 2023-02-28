# Word-checker

## Warning

This code is not meant to be a final product. Through this code, it is possible to easily implement a game such as Wordle, but:<br/>
    1. there is no input check;<br/>
    2. the user is supposed to enter the word to guess and then guess the word.<br/>
    3. the 4 instructions to be entered are coded in the Italian language.<br/>
    4. there is no GUI.<br/>
Although, a few adjustments are enough to make it a playable game.

## Game Rules

A word is a string of $k$ characters in $a$ to $z$, $A$ to $Z$, $0$ to $9$, $-$ or $\\_$.
The system will read from stdin information and instructions and print strings, namely:<br/>
    1. the value $k$, which is the length for the current game;<br/> 
    2. a sequence of words, which compose the dictionary of allowed words (HP: no duplicates);<br/>
    3. the instruction "+nuova_partita".<br/>
When that happens, the game starts. The system will now read:<br/>
    1. a reference word (HP: it must be an allowed word);<br/>
    2. the value $n$, which is the maximum number of attempts;<br/>
    3. a sequence of words to compare to the reference or instructions to execute.<br/>
For each read word, the system prints a string of $k$ characters as it follows:<br/>
    1. if the $i$-th character in the read word equals the $i$-th character in the reference word, the $i$-th printed character will be "+", as they are in the right position;<br/>
    2. if the $i$-th character in the read word doesn't match any character in the reference word, the $i$-th character will be "/";<br/>
    3. if the $i$-th character in the read word doesn't match the $i$-th character in the reference word but it matches a character in the reference word, and, be $a$ the number of characters in the read word which equal the $i$-th character and which are in the right position, and $b$ be the number of characters in the reference word which equal $i$-th, if there are less than $a-b$ characters in lower positions than $i$ in the read word then the $i$-th output character will be "|", otherwise it will be "/" (it basically prints "|" for each "right character in the wrong position" until there are no more right characters to assign for such character).<br/>
If the system reads the instruction "+stampa_filtrate", the system prints by lexicographic order the set of allowed words in the dictionary which are still suitable.<br/>
If the system reads the instruction "+inserisci_inizio", it will start reading a new sequence of words to add to the allowed words in the dictionary, until it will read "+inserisci_fine" (HP: no duplicates).<br/>
In the sequence of words to compare, if the system reads an attempted word which is not allowed, it prints "not_exists". If the system reads the reference word, it prints "ok". If the system reads $n$ allowed words which were not the reference word, it prints "ko".<br/>
When the game ends, it can only read "+nuova_partita" to start a new game or "+inserisci_inizio" to insert more words into the dictionary.<br/>

## Data structure and time complexity

I used a Patricia Trie to implement the dictionary of allowed words. Due to this choice:<br/>
    1. the time required to find a word in the dictionary (to check if it's allowed) is $O(k)$;<br/>
    2. the time required to insert a word in the dictionary is $O(k)$;<br/>
    3. if a wrong character is detected in the current word, or if, while visiting the trie, the number of occurrences of a character is higher than the number of occurrences of such character in the reference word, it is no more needed to visit the nodes below such word, so a whole sub-trie gets cut off, which makes the filtering function much faster.
    

