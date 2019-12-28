
const BG_IMGS = [
	"/imgs/arcade.jpg",
	"/imgs/canyon.jpg",
	"/imgs/galactic.jpg",
	"/imgs/html.jpg",
	"/imgs/jelly.jpg",
	"/imgs/parade.jpg"
];



function randInt(min, max) {
	return Math.floor(Math.random() * (max - min + 1)) + min;
}

function windowLoaded() {
	let bg;
	if(!BG_IMGS || BG_IMGS.length < 1) {
		return;
	}
	bg = randInt(0, BG_IMGS.length - 1);
	document.body.style.backgroundImage = "url('" + BG_IMGS[bg] + "')";
}

window.onload = windowLoaded;
