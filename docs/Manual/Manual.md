<div style="text-align: center;">

# The Rebekah Programming Language
### Version - 0.5a02

##### Documentation & User's Manual

</div>

## Contents

### I. An Introduction To Rebekah
#### 1. The core language
##### 1.1 Basics
##### 1.2 Data Types
##### 1.3 Variable Declarations
##### 1.4 Variable Scopes
##### 1.5 Operators
##### 1.6 Control Flow
##### 1.7 Functions
###### 1.7.1 Method
###### 1.7.2 Procedure
###### 1.7.3 Function

---

## Part I. An Introduction To Rebekah
### Chapter 1. The core language

This part of the manual is an introduction to the Rebekah language. It is of recommendation to have knowledge in other imperative programming languages (I.E C, Lua, etc.), but no prior experience in the industry is fine. This chapter introduces the core systems & parts of Rebekah.

#### 1.1 Basics

Rebekah is a high-level, imperative programming language. In this tutorial example, we will create a basic program. The Rebekah programming language uses the `.rbk` file extension, to signify a Rebekah file.

The Rebekah programming language is statically typed, meaning you have to state the value type you want your identifiers to be. If I wanted a character variable in Rebekah, it'd be like so: `VAR : Character = 'h'`. Variable initialization in Rebekah starts by first initializing the variable itself with a name, `VAR_NAME`. Next, you use the `:` symbol, followed up to be the type you want `TYPE`. Finally, you use `=`, the initialization operator, with the value of choice that corresponds to the type, `VALUE`.

During assignment, it is similar, yet different to the initialization of variables; assignment does not need the use of a type to be defined, but still needs the value to be of the same to the type. Assignment requires the use of a variable, and cannot use raw data/values to assign. You first start by calling the identifier, `VAR_NAME`, then using the assignment operator, which unlike the standalone `=` for initialization, uses `:=`. Finally, you state the value of the same type. During assignment (And initialization), you can assign another identifier to it, even functions. Functions will be talked about later, for simplisticy sakes. So if I have `VAR1`, equal to `15`, and initialize `VAR2`, being an `Integer`, I am able to use `VAR1` as the value, rather then a hardcoded value.

Comments in Rebekah are single-lined, meaning you cannot use and/or create multi-lined comments; comments are created via `--`, followed up by text, anything to the right of the `--` is a comment, anything to the left is fine & wont be affected by it.

Rebekah uses the `Main` and/or `main` function to start a program, if both are declared, Rebekah defaults to `Main`. Functions will be talked about and gotten into later, for the sake of learning basics first, so a simple example program would be so:

```ada
Hello : String = "Hello"
CopyValue : String = Hello
```

The program shown above wouldn't run like an ordinary program, due to the fact it doesn't start of a `main` function, but showcases the use of variables. Rebekah's case type, case type is a preference in which you word your variables, uses `PascalCase`, which capitalizes the start of every word, and doesn't space them. This is primarily for data types & variable naming, but is occasionally used in function names. Variable naming is purely of preference, meaning you can name your variables however you desire, meaning `HElLo` is just as acceptable as `Hello`, `hello`, etc. The only case of when variable naming ISN'T allowed as so is in the case of using: Numbers, Special Characters, and/or a previously used variable name. So cases like: `123Hello`, `!Value`, etc., aren't allowed, & can and would error. The case of previously used variable names prohibits the use of initializing a variable with the same name, to prevent redeclarations & UB (Undefined Behaviour).

#### 1.2 Data Types

The Rebekah programming language has multiple different data types, `Float`, `Integer`, `String`, etc. The full list with examples would be so:

`Boolean`:
```ada
-- True/False
BoolType : Boolean = true
```

`Integer`:
```ada
-- Integers require WHOLE numbers, meaning they cannot contain decimals
IntType : Integer = 5
```

`Float`:
```ada
-- Unlike integers, floats can contain decimals
-- During initialization & assignment, making it contain the decimal isn't required, but recommended
FloatType : Float = 2.0
-- That works the same as:
FloatType2 : Float = 2
```

`Character`:
```ada
-- Characters are single byte data types that use '' to hold data
CharType : Character = 'H'
```

`String`:
```ada
-- Unlike characters, strings can hold an infinite amount of data in them, & use ""
StringType : String = "Hello from Rebekah"
```

Those are all basic data types in Rebekah. Rebekah has 1 special data type called the `Array`, which allows for holding a list of data. The Array data type uses `<>` after calling the type to request for the data type it holds, & does allow for nested arrays. An example of such is like so:

```ada
SimpleArrayType : Array<Integer> = [5, 2, 8, 3]
NestedArrayType : Array<Array<Character>> = [['H', 'e', 'l', 'l', 'o'], ['W', 'o', 'r', 'l', 'd', '!']]
```

#### 1.3 Variable Declarations

In Rebekah, variables are by default, immutable. Immutable, meaning that they cannot be changed, they are a constant value. To make a variable mutable, something that can be changed, you use the `MUTABLE` keyword. The `MUTABLE` keyword, `MUT` for short, is used before initializing the variable. For reference, if I have `i`, & I want to change it to `7`, then we would need `i` to be mutable, like so: `MUTABLE i : Integer = 3`.

Rebekah also has the `CONST` keyword, short for `CONSTANT`. The `CONST` keyword allows for the variable to be thoroughly checked during compilation, to make sure it stays of constant value. The `CONST` keyword is mainly for use of clarity, for example, if I have a variable for the digits of Pi, `Pi : Float = 3.14159`, if I want to make sure people know it's not to be touched, we can say `CONST Pi : Float = 3.14159`.

As of the current version this manual is written for (`0.5a02`), `CONST` is mainly for the extended use of clarity when writting programs. With the ongoing switch to a registered virtual machine for Rebekah, `CONST` based variables will be getting extended support for such instances, to make it a non-redundant keyword (I.E Inlining, Compiler/VM optimizations, etc.)

#### 1.4 Variable Scopes

Variables in Rebekah have scopes. Scopes are the general range in which something can be seen; Rebekah has scopes for all variables, defining where they are accessible. There are two main scope types, local & global scopes. Local scopes are variables called inside of a subprogram, a subprogram is a separate part of the main program that works on its on, functions, if statements, for loops, are all examples of such. If I have a variable called `x`, inside a function, then it is a local variable, meaning it cannot be called by variables or functions outside the scope, or declared before the variable.

Rebekah runs top-down for programs, meaning if I have two variables, `VarA` & `VarB`, if `VarB` is initialized second, like so:

```rust
MUTABLE VarA : Integer = VarB
VarB : Integer = 2
```

That program, as shown above, would result in an error. Regardless of if both variables are global or not, `VarB` is initialized after `VarA`, which is calling it.

The second main scope type, global variables, are variables at the top-level of a program, outside of any scopes, anything on an inner-scope, and below, can call the variable.

#### 1.5 Operators

Rebekah uses a simple yet rich palette of operations to use. The first operation types we'll get into are the arithmetic operators. Rebekah has the following for arithmetic operations: `+` | `-` | `*` | `/`. They allow for simple mathematical expressions, and open up capabilities for more, advanced, operations in the mathematical field.

During the control flow, our next section/sub-chapter, we use a multitude of operations. Operations for control flow are usually called comparison operators, the following include: `>` | `<` | `>=` | `<=` | `==` | `!=`. The `>` operator is the greater then operator, used to check if the value on the left, is greater then the one on the right, & vice-versa for `<`. `>=` is similar to `>`, but checks if it's greater then OR equal to, vice-versa for `<=`. The `==` comparison operator is used to check if two values are exactly equal, with its polar-opposite being `!=`, to check if the two values aren't equal.

The control flow operators also have another type out of the two, the first one, as previously shown are the comparison operators, the second type is logical operators, as such: `and` | `or` | `not`. The `and` keyword, although used as an operator & referred to as such, is to check for 2 or more conditions, and if both or more are correct. Example of such is if `y > 2`, and I also want to check if `y < 10`, then I use `y > 2 and y < 10`. The `or` keyword is used for checking if one or the other condition(s) are true, so if `y = 2`, then it wouldn't pass the first test of `y > 2`, due to it being equal to, not greater. This is where our `or` keyword comes in, replacing `and` with `or`, we'd have `y > 2 or y < 10`. With that check, the check is true, because although the first condition failed, the second is true, as `y` is less then 10. The last keyword is the `not` operator, used to check if a condition is NOT true. If we have the same principle as last time, `y = 2`, and use the `and` keyword, we can check if `y` is NOT less then 10. Using `and` with `not`, `and not`, we get something like this, `y > 2 and not y < 10`, that means the check is false, as `y` is not greater then 2, and `y` is NOT less then 10.

#### 1.6 Control Flow

The Rebekah control flow, a control flow being a subprogram that runs if/while something is true, is simple yet deep in control. The most basic of control flows is the `if` statement. The if statement is a way to check if a condition is true, you learnt the basics of the if statement indirectly in section `1.5` with operators. To start off an if statement in Rebekah, you first say `if`, after that, you create the condition to check if it's true. In this case, we'll use `y` again, and make it 17. In this case we'll also want to check if `y` is GREATER then 5, to assembly it, we'd say `if y > 5`. Finishing an if statement, and going into the actual code itself if the check is true, we'd finish the if statement off with the `then` keyword. Checking if `y` is GREATER then 5, when it's 17, would look around so: `if y > 5 then`. To finish your if statement, we use the `end` keyword, meaning to end off our control flow. `end` is used in other control flows, I.E `for` loops. Relatively simple, to extend our check, we can add multiple conditions, if we also want to check if `y` is exactly 17, or it's NOT less then 10, we can can use all our logical keywords, `and`, `or`, & `not`. The first check, `y > 5`, followed up by `and` to check if `y` is also 17, `y > 5 and y == 17`. The last check uses `or` & `not`, `or` allows it so we can have two or more separate checks, so even if the first check is false, we can check if the second one isn't. So our last check will use or, to build it like so: `y > 5 and y == 17 or y < 10`. The last step to finishing is using the `not` operator, to check is `y` is NOT 10, so adding `not` after `or`, we finish it with `y > 5 and y == 17 or not y < 10`. To use it in a real example, let's say we want to add 5 to `y` after the check, we can make a program like so:

```lua
MUTABLE y : Integer = 17

if y > 5 and y == 17 or not y < 10 then
    y := y + 5
end
```

If we want to do something if the check is NOT true, we can use the `else` keyword. The `else` keyword is used inside the if statement itself, and is a separate subprogram from the original one in the case that the check was true. So that means if we make a variable inside of the true subprogram, it cannot be called in the false one, even if they're both from the same control flow. An example of `else` would be like so, using the same scenario:

```lua
MUTABLE y : Integer = 17

if y > 5 and y == 17 or not y < 10 then
    y := y + 5
else
    y := y - 9
end
```

If we want to run loops in Rebekah, we can use 1 of the 3 types given, a `for`, `while`, or `repeat` loop. The `for` loop is a loop type used as a check 'for' each iteration. For example, if we have an integer called `i`, also the variable named standardly used for for loops, as `i` usually stands for iteration, we can do a for loop which loops each iteration until it reaches the condition. Rebekah's for loop is different from standard for loops (I.E C, Rust, etc.), and uses the `in` operator, to set the starting value, followed up by `..`, meaning the value to go to. To finish the condition, and go into the subprogram of the loop itself, we use the `do` operator, similar to `then` for the if statement, as explained above. To end off a for loop, as stated in the if statement section, we use the `end` keyword, to end the subprogram. An example if we want to go from 0 to 3 would be:

```lua
MUTABLE y : Integer = 0

for i in 0 .. 3 do
    y := y + 1
end
```

A pseudocode example of it, if you don't understand would be like so:

```lua
y = 0

for i = 3, i < 0 do
    y = y + 1
end
```

The second loop type in Rebekah is the `while` loop, a less restrictive version of `for`, allowing for continuous loops while a value is true. If we have a variable called `y`, with the boolean value `true`, and we want a loop to run while `y` is `true`, we can use the `while` loop to do such. The while loop uses `do` to start the subprogram, similarly to `for` & `if`. The `while` loop also ends similarly with `end`. An example of such would e like so:

```lua
CONST y : Boolean = true

MUTABLE Iteration : Integer = 0

while y == true do
    if Iteration == 10 then
        y = false
    end
    
    Iteration = Iteration + 1
end
```

Our last, and most technical control flow, alongside loop type, is the `repeat` loop. The `repeat` loop is different from the previous control flows, and repeats until the condition is true. The `repeat` loop is usually written as a one liner, contrary to the other control flows, but can be written extended. The subprogram of a `repeat` loop is said directly after calling the word, `repeat`. If we want to increment `y` by 1, unlike having to say the condition first, or say `then` or `do` to start the subprogram, it is directly called like so: `repeat y = y+ 1`. To stop repeating a loop, we use the `until` keyword, where we say our condition after the `until` keyword. The `repeat .. until` loop also doesn't require an ending keyword to finish it off, and stops instantly after writing the condition. If we want `y`, which is is incremented by 1 every loop, to stop after it's greater or equal to 5, it'd look like so: `repeat y = y + 1 until y >= 5`.

#### 1.7 Functions

One of Rebekah's strongsuits is its extensibility of function call types. The Rebekah language has 3 call types: Methods, Procedures, & Functions.

Functions in all programming languages, are ways to call a subprogram, which doesn't run until it's called, hence why functions are called call types. The way to start a program in Rebekah, one that actually runs, is by creating a function with the name `main` or `Main`. The `Main`/`main`, which will be referred to & used as `Main` for the rest of the manual, is called via the `method` call type, because it should not intake or return data, as it is the entry point of any runnable program.

To create any function, regardless of call type, requires the start of the call type, `method`, `procedure`, or `function`, followed up by the call name, then (skipping function/procedure exclusive information) establishes the function with the `is` keyword, initializing what the function is. To begin the function itself, you want to use the `begin` keyword, to begin the function. After finishing that, you have the top section, the header, of the function. To end a function, you finish it with the `end` keyword, optionally finished with the name of the function for clarity of functions or control flows, `end FUNC_NAME`. Example with placeholder names would be so, `CALL_TYPE FUNC_NAME is begin ... end FUNC_NAME`. That is the root of any call type, regardless of the type itself.

When calling a function, regardless of type, we use the name, followed by parenthesis, to signify it's a call.

##### 1.7.1 Method

The `method` call type is used as a way to call a sequence. The `method` keyword cannot intake arguments, nor return values. The `method` call type does not require parenthesis for the call type, as it does not intake arguments. An example of `method`, using our `Main`, which is the entry of a program, would be like so:

```ada
method Main is
begin
    y : Integer = 2
end Main
```

To expand our knowledge of the `method` call type, and how exactly it is a 'sequence', we can make a method that increments a counter by one, like so:

```rust
method Main is
begin
    MUTABLE Counter : Integer = 0

    method NextCount is
    begin
        Counter := Counter + 1
    end NextCount

    NextCount()
end Main
```

##### 1.7.2 Procedure

The `procedure` call type is used to right functions without a return. The `procedure` call type can & requires arguments. To create arguments in Rebekah, you say the function name, followed by parenthesis, then data inside. An example of such, would be so: `procedure IncrementCount(CONST Amount : Integer) is`. To create multiple arguments, you would use a comma, followed by another argument; arguments cannot end in commas, as it'd be an unfinished list, resulting in an error. An example of a `procedure`, building onto our previous call example, would be so:

```rust
method Main is
begin
    MUTABLE Counter : Integer = 0

    procedure IncrementCount(CONST Amount : Integer) is
    begin
        Counter := Counter + Amount
    end IncrementCount

    IncrementCount(3)
end Main
```

##### 1.7.3 Function

The `function` call type is the most rich call type, allowing, and requiring arguments & returning. To return in Rebekah, you say the function name & arguments, followed up by `->`, signifying the data type to return, followed up by the data type itself. An example of such would be so: `function CreateCounter(CONST StartingAmount : Integer) -> Integer is`. Although a `procedure` can do the same, functions open up the possibility to global functions, not limited to needing variables hardcoded into them. An example of such, building onto the previous 2 examples, would be so:

```rust
function CreateCounter(CONST StartingAmount : Integer) -> Integer is
begin
    return StartingAmount
end CreateCounter

procedure IncrementCount(MUTABLE Counter : Integer, CONST Amount : Integer) is
begin
    Counter := Counter + Amount
end IncrementCount

method Main is
begin
    MUTABLE Counter : Integer = CreateCounter(5)

    method NextCount is
    begin
        Counter := Counter + 1
    end NextCount

    IncrementCount(Counter, 3)
    NextCount()
end Main
```
