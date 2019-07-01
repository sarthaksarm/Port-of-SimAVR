//From https://github.com/beatgammit/base64-js/blob/master/lib/b64.js
'use strict';

var lookup = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/';

function b64ToByteArray(b64) {
	var i, l, tmp, hasPadding, arr = [];

	if (i % 4 > 0) {
		throw 'Invalid string. Length must be a multiple of 4';
	}

	hasPadding = /=$/.test(b64);

	l = hasPadding ? b64.length - 4: b64.length;

	for (i = 0; i < l; i += 4) {
		tmp = (lookup.indexOf(b64[i]) << 18) | (lookup.indexOf(b64[i + 1]) << 12) | (lookup.indexOf(b64[i + 2]) << 6) | lookup.indexOf(b64[i + 3]);
		arr.push((tmp & 0xFF0000) >> 16);
		arr.push((tmp & 0xFF00) >> 8);
		arr.push(tmp & 0xFF);
	}

	if (hasPadding) {
		b64 = b64.substring(i, b64.indexOf('='));

		if (b64.length === 2) {
			tmp = (lookup.indexOf(b64[0]) << 2) | (lookup.indexOf(b64[1]) >> 4);
			arr.push(tmp & 0xFF);
		} else {
			tmp = (lookup.indexOf(b64[0]) << 10) | (lookup.indexOf(b64[1]) << 4) | (lookup.indexOf(b64[2]) >> 2);
			arr.push((tmp >> 8) & 0xFF);
			arr.push(tmp & 0xFF);
		}
	}

	return arr;
}

function uint8ToBase64(uint8) {
	var i,
		extraBytes = uint8.length % 3, // if we have 1 byte left, pad 2 bytes
		output = "",
		temp, length;

	function tripletToBase64 (num) {
		return lookup[num >> 18 & 0x3F] + lookup[num >> 12 & 0x3F] + lookup[num >> 6 & 0x3F] + lookup[num & 0x3F];
	};

	// go through the array every three bytes, we'll deal with trailing stuff later
	for (i = 0, length = uint8.length - extraBytes; i < length; i += 3) {
		temp = (uint8[i] << 16) + (uint8[i + 1] << 8) + (uint8[i + 2]);
		output += tripletToBase64(temp);
	}

	// pad the end with zeros, but make sure to not forget the extra bytes
	switch (extraBytes) {
		case 1:
			temp = uint8[uint8.length - 1];
			output += lookup[temp >> 2];
			output += lookup[(temp << 4) & 0x3F];
			output += '==';
			break;
		case 2:
			temp = (uint8[uint8.length - 2] << 8) + (uint8[uint8.length - 1]);
			output += lookup[temp >> 10];
			output += lookup[(temp >> 4) & 0x3F];
			output += lookup[(temp << 2) & 0x3F];
			output += '=';
			break;
	}

	return output;
}