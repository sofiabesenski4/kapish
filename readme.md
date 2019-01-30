readme.md for kapish.c

Known Limitations:
-After using a control character/signal, the input buffer contains the special character.
 To continue use, simply press return to send the input through the shell program, where it will
 return an error. At this point the program will work normally.
-The .kapishrc file cannot contain errors or there will be memory leaks. I'm not entirely sure why.
-
 
