# Data-Compression -- Huffman-codes

To test the code:

	gcc encode.c -o encode

	gcc decode.c -o decode

	./encode Holmes.txt

	./decode


There will be three .txt files generated:

	'code.txt' contains the total number of characters in the original file and the codes.
	
	'encoded.txt' is the encoded file.
	
	'decoded.txt' is the decoded file and can be used to check the correctness with the original file.
	


The encoded files turn out to be about 40% smaller than the orginal ones. The data compression is successful!
