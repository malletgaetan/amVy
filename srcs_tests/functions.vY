let five = 5;
let three = 3;
let ten = 10;

fn add (x, y) {
	fn bang() {
		let x = 44;
		return x;
	}
	return (x + y);
}

let result = add(five, ten);
let bambam = add(five, three);
