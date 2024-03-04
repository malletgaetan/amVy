# amVy interpreter

amVy is a simple language, derived from the Monkey programming language, which was my first source of knowledge while designing amVy.
The final objective is to build a language with no heap, that still implement a sort of dynamic memory allocation.

## Example

```vY
fn fibonacci(n) {
	if (n < 2) {
		return (1);
	}
	return fibonacci(n - 2) + fibonacci(n - 1);
}

let res = fibonacci(10);
print(res);
```

## Usage

First build the interpreter
```
make
```

Execute code
```
./amVy mycode.vY
```

Print the generated AST
```
./amVy mycode.vY --ast
```

Compile amVy with an internal stack trace for debugging purposes
```
make debug
```

## Main resources

- https://www.amazon.fr/Writing-Interpreter-Go-Thorsten-Ball/dp/300055808X
- https://chidiwilliams.com/posts/on-recursive-descent-and-pratt-parsing
- https://www.gingerbill.org/series/memory-allocation-strategies/

## TODO

- support for arrays
- add callstack limit
- make the tracer stack depth aware, like --ast opt
- own hashmap implementation
