The following program was implemented in C.

If "zmsHuffEnc.exe" does not have a .exe extension, please add the extension before continuing.


To run the program from a command line, go to the program's directory and type the following:

zmsHuffEnc.exe "Name of file to compress" "Name of file for code lengths" "Name of compressed file" Name of decompressed file"


Files have been included for convenience

zmsHuffEnc.exe sherlock.txt lengths.lng compressed.cmp decompressed.txt

zmsHuffEnc.exe harvest.jpg lengths2.lng compressed2.cmp decompressed2.jpg

zmsHuffEnc.exe "STM3210xxx Reference Manual.pdf" lengths3.lng compressed3.cmp decompressed3.pdf


If errors persist, make sure that any of the files called are not open in other applications.

Should the executable not run on a certain machine use a C compiler for the following

 gcc main.c moffatKatajainen.c huffman_length_encoder.c huffman_length_decoder.c -o zmsHuffEnc.exe