# Math Engine

A lightweight command-line mathematical expression engine written in C. It provides an interactive environment for evaluating mathematical expressions, managing variables, and defining custom functions.

The project implements a complete processing pipeline consisting of a **Lexer, Parser, Semantic Analyzer, Abstract Syntax Tree (AST), and Interpreter**.

## Features

- **Arithmetic Operations**
  - Addition (`+`)
  - Subtraction (`-`)
  - Multiplication (`*`)
  - Division (`/`)
  - Modulo (`%`)
  - Unary operators (`+`, `-`)
  - Parenthesized expressions

- **Variables**
  - Declare variables and assign values.
  - Use previously declared variables in expressions.
  - Display all stored variables.
  - Delete existing variables.

- **Custom Functions**
  - Define functions with multiple parameters.
  - Evaluate function calls.
  - Display all defined functions.
  - Delete existing functions.

- **Built-in Constants**
  - `PI` — represents the constant value `3.14`.

- **Interactive Commands**
  - Clear the terminal.
  - Display available commands.
  - Exit the engine.

- **Error Handling**
  - Syntax validation.
  - Semantic analysis.
  - Undefined variable detection.
  - Undefined function detection.
  - Function argument validation.
  - Division-by-zero detection.

## Requirements

To build the project, you need:

- GCC (or another compatible C compiler).
- GNU Make.
- A Unix-like environment (Linux/macOS).

## Installation & Build

Clone the repository:

```bash
git clone <repository-url>
cd <project-directory>
```

Build the project using Make:

```bash
make
```

## Running

After building, start the Math Engine with:

```bash
make run
```

You will be greeted by an interactive prompt:

```text
[math-engine]>
```

You can now start entering mathematical expressions and commands.

## Usage

### Arithmetic Expressions

Evaluate mathematical expressions directly:

```text
[math-engine]> 10 + 5
➜ 15.00

[math-engine]> (10 + 5) * 2
➜ 30.00

[math-engine]> 20 / 4
➜ 5.00

[math-engine]> 10 % 3
➜ 1.00
```

### Variables

Declare variables using the assignment operator (`=`):

```text
[math-engine]> x = 10
✔ variable 'x' has been added successfully

[math-engine]> y = 20
✔ variable 'y' has been added successfully
```

Use variables in expressions:

```text
[math-engine]> x + y
➜ 30.00
```

Display all stored variables:

```text
[math-engine]> showvars
```

Delete a variable:

```text
[math-engine]> del x
```

### Custom Functions

Define custom functions using the following syntax:

```text
function_name(parameter1, parameter2) = expression
```

For example:

```text
[math-engine]> square(x) = x * x
✔ function 'square' has been added successfully
```

Call the function:

```text
[math-engine]> square(5)
➜ 25.00
```

Functions can also accept multiple parameters:

```text
[math-engine]> add(a, b) = a + b
[math-engine]> add(10, 20)
➜ 30.00
```

Display all defined functions:

```text
[math-engine]> showfuncs
```

Delete a function:

```text
[math-engine]> fdel square
```

### Built-in Constants

The engine provides the built-in constant `PI`:

```text
[math-engine]> PI
➜ 3.14

[math-engine]> PI * 2
➜ 6.28
```

`PI` is reserved and cannot be reassigned or deleted.

### Available Commands

| Command | Description |
|---|---|
| `help` | Displays all available commands. |
| `showvars` | Displays all stored variables. |
| `showfuncs` | Displays all defined functions. |
| `del VAR` | Deletes a variable. |
| `fdel FUNC` | Deletes a function. |
| `clear` | Clears the terminal. |
| `exit` | Exits the Math Engine. |

---

## this README is build by AI's as well as the outputs of the program with `colors` and `emojis` **BUT** the actual source code is writen by ME
# math-engine
