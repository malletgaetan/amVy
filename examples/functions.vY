let five = 5;
let three = 3;
let ten = 10;

fn add (x, y) {
	x = x + 10;
	fn bang(a) {
		return a * 10;
	}
	return bang(x + y);
}

let result = add(five, ten);
print(result);
let bambam = add(five, three);
print(bambam);
