fn f(n) {
	return n + 1;
}
let a = 1;
let b = 2;
let c = 3;
a = (1 % (4 + 2)) + (f(a) * (f(b) + f(c)));
print(a);
