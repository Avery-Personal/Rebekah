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

Hello := "Hi"
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
