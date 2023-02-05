# bearcalc

Bearcalc is a simple command-line expression evaluator, with full
trig/log/exponential support. Expressions can be passed on the command line,
e.g.,

    bearcalc sin(pi/4)

but, if no expression is given, Bearcalc enters interactive mode. 

Bearcalc is a bit like GNU bc, but with more user-friendly ways of
entering and displaying numbers. Unlike bc, however, Bearcal uses
ordinary, double-precision math.

## Number input 

Numbers are decimal, unless they begin with '#', in which case they are
hexadecimal. Decimal numbers may be entered in scientific notiation, but hex
numbers must be integers. Hex numbers may, however, be negative, should the
need arise.  So "#-C0" is -192.

The symbol '@' expands to the result of the last calculation.

## Number output

Bearcal can display results in decimal, hexadecimal, or approximate
fractions. Use the commands `dec`, `hex`, or `rat` (ratio) to select
these modes.

In ratio mode, Bearcal displays the fraction with the smallest number
of digits that approximates the decimal result within three significant
figures. This display mode is useful for seeing the results of fractional
computations.  

    > rat
    > 3/4 + 5/16
    1 1/16

## Operators

Bearcal supports the usual: +, -, \*, /, and % (modulus). 

## Commands

In interactive mode, certain entries are treated as commands, rather than
expressions. 

- `dec` - display numbers as decimal (this is the default)
- `deg` - do trig calculations in degrees
- `eng` - display numbers in engineering notation (see below)
- `hex` - display numbers as hexadecimal
- `quit	- quit the program. Can be used in interactive mode, 
  or in a script file.
- `rad` - do trig calculations in radians (this is the default)
- `rat` - display numbers as ratios (fractions)
- `scale N` - set the number of significant figures displayed
- `stat` - show status information

## Constants

The usual `pi` and `e` are defined.

## Functions

Bearcalc supports the usual math functions: `abs`, `acos`, `asin`, `atan`,
`atan2`, `ciel`, `cos`, `cosh`, `exp`, `floor`, `ln`, `log`, `pow`, `sinh`,
`sinh`, `sqrt`, `tan`, and `tanh`.  The meanings should be self-explanatory.
There are a few other functions with names that are not as universal.

- `degtorad` -- convert a number in degrees to radians
- `radtodeg` -- convert a number in radians to degrees
- `ncr (n,p) -- number of combinations of p items from n
- `npr (n,p) -- number of distinct permutations of p items from n
- `fac` -- factorial

Trig functions take results in radians, and return result in radians, unless
'deg' angle mode is enabled.

Note that all function and variable names are case-sensitive. 

## Defining functions and variables 

In general, Bearcalc uses the following syntax to define a function, or
assign a name to a number.

    name: n_args: text...

`n\_args` is the number of arguments the function expects, which can be
0-3. 

For example, to define the funtion `sec(x)` to be `1/cos(x)`

    sec: 1: 1/cos(x)

The spaces can be ommited, but make the expressions more readable.

Up to three arguments are supported, and they must be referred to in
expressions as `x`, `y`, and `z`. A function with zero arguments is just
a variable. In this case, the number of arguments can be ommitted. So
these definitions of the constant `g` are equivalent:

    g: 9.80665
    g: 0: 9.80665

In subsequent expressions, the number 'q' can be referred to just as `g`,
or as `g()`. 

Function definitions can be made in terms of one another. So, for example,
to define the functions `root0` and `root1`, to obtain the two roots of
a quadratic equation with coefficients `x`, `y`, and `z`:

    disc: 3: sqrt(y^2 - 4*x*z) 
    root0: 3: (-y + disc (x,y,z)) / (2*x)
    root1: 3: (-y - disc (x,y,z)) / (2*x)

then:
    root1 (1, 5, 6)
    -2

## Script files

Bearcal can read expressions and definitions from a text file. Each
definition is expected to fit onto one line, although lines can be of 
any length. To process a file, use the `-f` switch; multiple switches
can be used to read multiple files. 

Once these files have been read, then Bearcal will process the expression
on the command line or go into interactive mode. To prevent this behaviour,
a script file can end with the command `quit`, which will cause Bearcal
to exit.

In a script file -- and, in fact, in interactive mode -- any text after
a semicolon is taken to be a comment, and the rest of the line is
ignored. 

## Notes

- `log` returns the base-10, common logarithm. For the natural, base-e
  logarith, use `ln`.
- Exponentiation is left-associative. This is, "-a^b" is "(-a)^b". This is a
  rather informal usage, but one that people now tend to expect.
- The modulus operator provides the _integer_ modulus. Any factional part of
  either number is ignored.
- If one function is defined in terms of another, and the earlier definition is
  changed, strange things will happen. Bearcal parses expressions as far as
  possible when functions are defined, and the enclosed function will not be
  parsed again -- in fact, its text will be unknown at that point. 
- Everything about Bearcalc is case-sensitive, except for the 'e' in a number
  in scientific notation, which can be 'e' or 'E'.
- The `scale` command sets the number of significant figures displayed in `dec`
  mode. Although this can be set to an arbitrarily large value, most platforms
  don't give more than about 15 digits of precision. `scale` also sets
  the order of the number above which scientific notation is used.
- "Engineering" notation amounts to displaying a number such that its exponent
  is multiple of 3. So, instead of '1234', we write '1.234E3'. For numbers
  whose order is less than 15, standard suffices are used instead of
  exponents. So 1E7 = 10E6 = 10M. There are standard suffices for larger
  orders than 15, but they aren't widely used.


