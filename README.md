# Project 3B 2014                         
>	Readme for final COMP90045 submission.
>	Developed by: #undef TEAMNAME

The purpose of this readme is to note a few assumptions/modifications
we have made to the language at the bequest of Harald who has given
permission to make these changes. Then we note the extra details and usage
instructions for our wiz compiler.

###	Language assumptions

1. We are assuming interval terminals in array declaration are non-negative

2. We allow both binary and unary minus operators to work regardless
   of whitespace

3. Floating point precision does not matter in the pretty printer. The 
   alternatives are to store number of digits of precision alongside the 
   float (messy), or store as a string (inconsistent with boolean and 
   integer values), so this is the least of three evils.

##	Wiz usage
--------------------------------------------------------
We have added several options to the compiler to accomodate for the various
new functions we have added. These additonal functions are as follows.

    -c : Optimise and reduce expressions, printing the before
         and after results, along with any errors.
         Output is written to stdout.
    -f : Compile with optimisations enabled.
         Output is written to file WIZ_SOURCE_PREFIX.oz (where
         WIZ_SOURCE_PREFIX is the prefix of wiz_source_file
         - i.e. with '.wiz' suffix removed, if present).
    NO_ARGS : 
         Compile with optimisations enabled.
         Output is written to stdout.

These options are also available through the usage prompt printed when a user
enters "wiz" or "./wiz" without appropriate input options. 

##	Optimisation Strategies
--------------------------------------------------------
### Algebraic reduction of expressions

We implemented an extensive procedure that is able to perform a variety
of reductions of expressions based on algebraic rules and constant folding.
This is an optimization that takes place on the ast.
For example, it is able to perform reductions such as

`1 + 3 => 4`                    	     (constant folding)

`(1 + (x + (y + 3))) => (4 + (x + y))`       (taking advantage of commutitivity)

`-a + b - c + d - e => b + d - (a + c + e)`  (factorizing negation)
`---a => -a  `                    (removal of double negative)
`not (not ((not a) and (not b))) and not (not (not c)) and not false  => not (a or b or c)`  (reduction of boolean expressions)
etc.


### Optimized array access

As much of the calculation of addresses for array access is done at compile
time as possible. Our code program calculates the memory address offsets
for array dimensions whose index in a given array-expression is constant (i.e.
statically determined), and aggregates them at compile time, and generates
expressions (which are reduced using previous optimization) for calculating
the dynamically determind offsets (again, doing as much work as possible at
compile time).

For example, for the array access given by expression:

`x[1,a,3,b]`

where the array declaration is:

`int x[0..2, 7..9, 2..4, 9..10]`

- It works out statically that the offset `(1-0) * (2*3*3) + (3-2) * (2) = 20`
  needs to be added to the overall offset due to the consent first
  and third indices.

- It calculates expressions for the dynamic offsets that need to be calculated,
  which are `(a-7) * (2*3)` and `(b-9)`. These dynamic offset expressions are
  reduced using the above reduction procedure to simplify them as much as
  possible.

- The run-time address can then be calculated very efficiently by evaluating
  the two dynamic expressions, and adding them both to static offset of 20.

In addition, array accesses which are entirely statically determined (e.g.
`x[1,8,3,9]` in above example) are compiled even more efficiently, and access
the required slot directly without using an indirect load.


### Minimising register use

We designed a procedure to minimise the register use for temporaries when
compiling expressions. For the compilation of binary expressions
(e.g. e1 BINOP e2):
- we first calculate whether the calculation of e1 or e2
  will require more new live registers recursively (assuming recursively that
  we are making this optimization of course).
- Then we evaluate the sub-expression with greater register use first,
  storing it in the first available register
- Finally the other subexpression is compiled second and stored in the
  second available register.

In many cases (in particular left-heavy unbalanced expression trees) this
results in MUCH lower register usage than the naive process of evaluating
e1 first to the first available register, and e2 second to the second.

For example, for binary expression trees such that every node has at least
one child of depth 0, the above procedure will only need a single register
for all temporaries.


##  Other clever things
------------------------------------------------------------------------------------
### Dynamic array bounds checking

Full array bounds checking is performed at run-time, and the program throws
error message and halts if it encounters such a violation (these checks are
performed in an optimized way in the spirit of the above array optimization).


## Important Note
Our compiler will optimise by default, because of this (and the variable
precision implied by reading floats etc) there may be small rounding errors
due to the decrease in operations (or ordering) that we apply to constant
expressions. This may cause testing scripts to fail, but you will note that 
these issues generally only occur at 8 or more decimal places.

Further,our error printing uses ascii colours that will appear as gibberish
characters when printed to file. To view this correctly, simply open the file
in any editor that supports these ascii escaped characters (i.e. a terminal).


Please contact any of the group members should you have any additional
concerns. 

Cheerio,

#undef TEAMNAME
