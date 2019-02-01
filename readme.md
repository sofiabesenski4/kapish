readme.md for kapish.c


This tutorial was used heavily for the input processing
https://brennan.io/2015/01/16/write-a-shell-in-c/
Stack overflow was also used to solve various bugs.
There was no collaboration with classmates during the development of this program.


Known Limitations:
-After using a control character/signal, the input buffer contains the special character.
 To continue use, simply press return to send the input through the shell program, where it will
 return an error. At this point the program will work normally.
-The .kapishrc file cannot contain errors or there will be memory leaks. I'm not entirely sure why.
-For some reason, the commands specified in .kapishrc are run twice. This could be an issue for some commands, if they
	end up modifying and changing parts of the filesystem 
 
