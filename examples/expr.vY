fn f(n) {
	return n + 1;
}
let n = 1;
let b = 2;
let c = 3;
n = (1 % (4 + 2)) + (f(n) * (f(b) + f(c)));
print(n);
