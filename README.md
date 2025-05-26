GLL Recogniser Implementation
=========================

# Introduction
A C program that implements a toy version of the GLL Parsing algorithm from the paper [GLL Parsing](https://dotat.at/tmp/gll.pdf) by Elizabeth Scott and Adrian Johnstone.

# Usage
Assumptions:
- The compiler used, should use the ASCII character set as signed chars (0-127).

## Compilation
The makefile uses the gcc compiler and gives (3 + 1) options and compiles to 'builds/':

    make build
Builds gll_parser without any additional debug information

    make debug
Also builds gll_parser but with the -g flag (see man page of gcc) and additinal log statments. Who print information about the data structure state during execution.

    make test
Build both gll_parser_test and gll_parser_inf_test who's usage is explained in the [File Testing](#file-testing) and [Generative Testing](#generative-testing) section respectifly.

    make generator
Builds input_gen who's usage is explained in the [Input Generator](#input-generator) section.

## Execution
### Default Parser
Executes the algorithm one with the provided grammer and input.

    gll_parser 'file_path' [-l\L | -f\F] 'input'
- `'file_path'` is a path to a file containing a grammer of format:

        X -> X1 | X2 | ...
        X -> Z
        Y -> Y1 | Y2 | ...
        ...
        [#]
- `'-l\L'` flag to indicate 'input' to be a string
- `'-f\F'` flag to indicate 'input' to be a file containing a string
- `'input'` is a string or file path containing a string of terminals that will be parsed


### File Testing
For each grammer file in the folder at `'folder_path'` executes the algorithm `'repetition'` times on each `'inputi'` in the same `grammer_file` denoting an execution passed if the parser returns the same value as provided next to the input.

    gll_parser_test 'folder_path' | 'grammer_file" 'repetition'
- '`folder_path'` is a path to a folder containing files with grammer of format `'grammer_file'` can optionaly be just one specific file:

        X -> X1 | X2 | ...
        X -> Z
        Y -> Y1 | Y2 | ...
        ...
        #
        'input0' '0/1'
        'input1' '0/1'
        ...
- `'repetition'` is a natural number denoting the amount of times each test is executed. (Execution time is then averaged)

### Generative Testing
Executes the algorithm 'repetition' times on an input concatinated of all substrings provided where second (with an odd index) substring is repeated. The amount of repetition is changed by 'op' every iteration. And the algorithm terminates after 'count' tests or will not terminate if 'count' == -1

    gll_parser_test 'grammer_file' 'repetitions' 'count' 'start_repetition' 'op' 'substr0' 'substr1' ...
 - 'grammer_file' should be a path to a file containing grammers of format:

        X -> X_1 | X_2 | ...
        Y -> Y_1 | Y_2 | ...
        ...
   
 - `'repetition'` is a natural number denoting the amount of times each test is executed. (Execution time is then averaged)
 - `'count'` will be the amounts of tests generated (-1 to never terminate)
 - `'start_repetition'` Is the amount each odd substring will be repeated at the beginning
 - `'op'` is of form \"+N\" or \"*N\" (N being a natural umber) denoting the growth of the input per new test
 - `'substr0...n'` the input given to the parse is the concatination of these substring where all substr's with odd indicies are repeated

### Input Generation
Generates for a given grammer valid inputs.

    input_gen 'grammer_file' 'count'
 - `'grammer_file'` should be a path to a file containing grammers of format:

        X -> X_1 | X_2 | ...
        Y -> Y_1 | Y_2 | ...
        ...

 - `'count'` will be the amounts of tests generated (-1 to never terminate)
> [!CAUTION]  
> The implementation of input generation is rather naive hence its memory usage is absurd so its highly recommented to not execute it for a prolonged time!!!
# Implementation details
The following section will discuss how the algorithm of the paper was implemented as a C program. It is assumed the reader has some understanding of the working pseudo code from the paper.
## Topics
1. [Labels](#labels)
2. [GSS](#gss)
3. [Sets](#sets)

## Labels
The pseudo code from the paper uses some form of dynamic label onwhich they call goto. Clearly this is not something directly supported by C. Hence we will need a different approach.
We differentiate between 4 different label types:
- `base_loop` ~ $L_0$
- `init_rule` ~ $code(A, j)$
- `start_new_alternative` ~ $code(A::= \alpha_k, j)$
- `continue_alternative` ~ $code(\alpha, j, X)$

We can then implement each of these types as its seperate function and just store a enums (and state information) instead of a label in all data structures and index into the correct functions.
This however leaves us with a new problem. Since we are jumping from function to function we will at some point fill our call stack.
We can of course solve this quite quickly we could just change our `base_loop` from a function that is called everytime to a while loop that checks if $R$ is empty and all other functions just return back when done.
For this implementation we have however choosen a slightly different approach (not because of it having much of an advantage but mostly cause funni). We instead of following the normal call/return structure, use `longjump` and `setjmp`. Hence we can call `setjmp` in the `base_loop` function and all the other function when they reach the end of a derivation will call `longjump` to return to the `base_loop` conviniently ereasing the call stack used by the derivation.
The only caviat in this approach is that we must store all changes we want to keep accross jumps in a pointer whose address cannot be changed

## GSS
The GSS graph was implemented with the improvments described in [Faster, Practical GLL Parsing](https://ir.cwi.nl/pub/24026/24026B.pdf). Since we must search both edges and nodes for existance and these graphs can get quite large its (as you might see for yourself in the [Performance](#performance) section) important to have these checks be fast.
Hence what this implementation does is allocate space for the maximal amount of gss nodes possible as pointers. This uses alot of memory which is its main disadvandage. We know this amount at the start of our program since gss nodes are of from `(uint16_t non-terminal, uint32_t input_position)` and we know the amount of nonterminals and input_size as soon as we know the grammer and the input.
We can then just allocate a new node with additional space for an edge array / U_set ring buffer and two uint32_t for the P set.

## Sets
This is arguable the most interesting part of any implementation as there is quite a bit of room here for how these sets are implemented. In a higher level language it is probably a good idea to just use some form of a set implementation if that language provieds a good one. C does however to my knowledge not provide such a data structure in its standart library. Hence since I'm quite attached to my sanity we'll need to find a different approach.
#### Naive Approach
The naive approach of course is to just implement all 3 sets as dynamically growing array. Where we treat $R$ as a stack. Clearly this brings heavy memory problems as the $U$ and $P$ stuctures may grow large quickly.

#### Paper's Approach
The paper provides an idea of how this could be implemented a bit more efficiently. They propose to make $R$ and $U$ into a 2 dimensional data structures $R_i, U_i$. Where $i$ is the input position. This has one main benefit since if $R_i$ is empty (hence we have processed the last descriptor at input pos i) we can free $U_i$.
This implementation would be quite possible in C. Where this is a 2D dynamic array with a pool that instead of freeing unused memory holds on to them for reuse as higher indicies.

#### Ring buffer Approach
This is the approach used in this implementation. Mainly cause of its simplicity and the fact that it still performes well with how much memory this approach uses.
The idea is as follows if we can slightly modify the algorithm s.t. it is garanteed that if it picks a descriptor $A$ from $R$ in the `base_loop` with input pos $i$. By the time the algorithm writes new descriptors $B_1$ ... $B_n$ to $R$ derived from computation on $A$ the input position of $B_i$ is either $i$ or $i + 1$.
Then can implement $R$, $U$ and $P$ as a sorted ring buffer where descriptors of size $i$ are always added to R on one side (lets call this side left) of the ring and $i+1$ to the other (right) which ensure it remains sorted. New descriptors are always picked from the left and if our descriptor has a higher input position then the last one we can move the left pointer of U and P past all descriptors with the old input size.

##### Modifing the algorithm
If we analyse the original pseudo code given in the paper we can see that the only way we will ever parse more then one char before returning back to $L_0$ (baseloop) is in the definition for $code(A::=x_1...x_f, j)$ for when $x_1$ is a terminal. Since we then enter $code(x_2...x_f, j, A)$ which them may parse another character if $x_2$ is a terminal before it then continues to $code(x_3...x_f, j, A)$ and so on. We can change this quite easily by just changing the algorithm inside $code(A::=x_1..x_f, j)$. So it then adds a descriptor corresponding to a later call to $code(x_2...x_f, j, A)$ instead of moving on to it directly. We can do the same inside of $code(x_2...x_f, j, A)$ to also add descriptors instead of just moving on.

This modification and the behavior (mainly the sorting) of our ring buffer ensures that we will always try every possibility at a given input position before moving to the next position.

##### Making it GSS local
Since the algorithm requires lookup both in the $U$ set and the $P$ set and we liniar search for that lookup its in our interest to make these as small as possible. As such we can make them local to the GSS node. It was already mentioned earlier that our GSS gives us the ability to lookup nodes in O(1) we can use that and make a gss local ringbuffer for each node for the $U$ set. This leads to much smaller $U$ sets. The problem that now arises is cleaning up the $U$ set when our input rises it is clearly not optimal to run through all nodes and clean each on up individually. What we do instead is do it lazily since each time we wish to modify (add) something to some $U$ set we need to search through it anyway we can also clean up in the same step by checking the current `lower input position`. We can do the same with the $P$ set but even simpler. Since the $P$ set is just a node and an input position but we know we are at any point at most at two different input position we can simply allocate an static array of size 2 per node and clean in up in the same way when nessessary.

# Performance
Here are 3 Grammers and how they perform

g1:
S -> aSB | B
A -> ab
B -> A | B
TODO: Image

g6:
S -> aBc | abC | Abc
A -> a' | a'' | D
B -> b' | b'' | E
C -> c' | c'' | D
D -> d | dD | ddD | dddD | ddddD | dddddD | ddddddD | dddddddD | ddddddddD
E -> e | Ee | Eeee | Eeeeeeee | Eeeeeeeeeeeeeeee
TODO: Image

g13:
S -> \[ E \]
E -> I | E , E
I -> M K | N
K -> K K | K
N -> 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 0
M -> 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9
TODO: Image

g14:
S -> C a | d
B -> _ | a
C -> b | B C b | b b
TODO: Image

g15:
S -> b | S S | S S S
TODO: Image
