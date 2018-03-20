/*
	a < b => -1 ; a == b => 0 ; a > b => 1
*/
int compare_strings (char * a, char * b) {

	int re = strcmp(a, b);
	if (re < 0)
		return -1;
	else if (re > 0)
		return 1;
	else
		return 0;
}


/*
	check if char is english alphabet
*/
bool is_alphabet (char ch) {

	if (ch >= 65 && ch <=90)
		return true;
	if (ch >= 97 && ch <= 122)
		return true;
	return false;
}

/*
	check if char is numbner
*/
bool is_number (char ch) {

	if (ch >= 48 && ch <=57)
		return true;
	return false;
}