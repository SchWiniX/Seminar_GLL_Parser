GLL Recogniser Implementation
=========================

# Introduction
A C program that implements a toy version of the GLL Parsing algorithm from the paper [GLL Parsing](https://dotat.at/tmp/gll.pdf) by Elizabeth Scott and Adrian Johnstone. The implementation is my attempt at making it fairly memory efficient/cache local in hopes that cache locality will provide enought of a speed boost to compete with higher level implementation that have data structures such as sets.

# Usage
Assumptions:
- The compiler used, should use the ASCII character set as signed chars (0-127).
- Inputs strings may not contain '\0', '|', '_', ' ', or capitilized letters as they are used inside the algorithm
- '_' on the right hand side of a grammer rule is used to represent the empty string
- Capitilized letters are used as non terminals, the set of terminals is then the remainder of characters not mentioned above

## Compilation
The makefile uses the gcc compiler and gives (3 + 1) options and compiles to 'builds/':

    make build
Builds gll_parser without any additional debug information

    make debug
Also builds gll_parser but with the -g flag (see man page of gcc) and additinal log statments. Who print information about the data structure state during execution.

    make test
Build both gll_parser_test and gll_parser_inf_test who's usage is explained in the [#File Testing](#file_testing) and [#Generative Testing](#generative_testing) section respectifly.

    make generator
Builds input_gen who's usage is explained in the [Input Generator](#input_generator) section.

## Execution
### Default Parser
Executes the algorithm one with the provided grammer and input.

    gll_parser 'file_path' 'input'
- `'file_path'` is a path to a file containing a grammer of format:

        X -> X1 | X2 | ...
        X -> Z
        Y -> Y1 | Y2 | ...
        ...
        [#]
- `'input'` is a string of terminals that will be parsed


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

    gll_parser_test 'grammer_file' 'repetitions' 'count' 'op' 'substr0' 'substr1' ...
 - 'grammer_file' should be a path to a file containing grammers of format:

        X -> X_1 | X_2 | ...
        Y -> Y_1 | Y_2 | ...
        ...
   
 - `'repetition'` is a natural number denoting the amount of times each test is executed. (Execution time is then averaged)
 - `'count'` will be the amounts of tests generated (-1 to never terminate)
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
1. [Labels](#label)
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
For this implementation we have however choosen a slightly different approach (not because of it having much of an advantage but mostly just cause we wanted to). We instead of following the normal call/return structure most are familiar with instead use `longjump` and `setjmp`. Hence we call `setjmp` in the `base_loop` function and all the other function when they reach the end of a derivation call `longjump` to return to the `base_loop` conviniently ereasing the call stack used by the derivation.
The only caviat in this approach is that we must store all changes we want to keep accross jumps in either heap memory or stack memory assigned before the call to `set_jump`

## GSS
The GSS graph was implemented with the improvments described in [Faster, Practical GLL Parsing](https://ir.cwi.nl/pub/24026/24026B.pdf). The actuall datastructure are just two dynamic array one containing nodes and edges. A comparison to a more traditional GSS can be found in the [performance](#performace) section.

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
If we analyse the original pseudo code given in the paper we can see that the only way we will ever parse more then one char before returning back to $L_0$ (baseloop) is in the definition for $code(A::=x_1...x_f, j)$ for when $x_1$ is a terminal. Since we then enter $code(x_2...x_f, j, A)$ which them may parse another character before it then calls $goto$ $L_0$. We can change this quite easily by just having the algorithm instead of entering $code(x_2...x_f, j, A)$, add a descriptor that corresponts to this call.

This modification and the behavior (mainly the sorting) of our ring buffer ensures that we will always try every possibility at a given input position before moving to the next position.

# Performance
ToDo
