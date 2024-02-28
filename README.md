# A small and simple interpreter for amVy

amVy is a simple and concise context oriented programming language

> [!NOTE]
> amVy is just a toy, don't use it


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