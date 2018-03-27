/************************************************
 * feed.h					*
 * Created by: Harry Bullen			*
 * Created on: 7-11-02				*
 * Current Version: 1.00			*
 *						*
 * Description:					*
 *	This a collection of little functions	*
 * used in manipulating streams.  First,	*
 * linefeed and flinefeed are very similar.	*
 * Linefeed is a specific version of flinefeed.	*
 * Flinefeed takes two arguments, an in file and*
 * an out file.  It then feeds one line,	*
 * including the '\n' off the in file and into	*
 * the out file. Linefeed doses the same	*
 * thing but it takes no arguments and uses	*
 * stdin and stdout.				*
 *	Eatline and featline are the same as	*
 * their counterparts linefeed and flinefeed	*
 * except that neither prints anything back out.*
 * Also, feed is used to feed the intire	*
 * contents of one file into another.		*
 *	Peek is a little function that is used	*
 * to get the next character off stdin, but not *
 * remove it from the stream.  It takes no	*
 * arguments and returns an integer representing*
 * the next character on stdin.			*
 ************************************************/

#ifndef FEED_H
#define FEED_H

#include <stdio.h>

/* Reads a file from in and prints it to out*/
void feed(in, out)
FILE *in;
FILE *out;
{
	int c;
	while( (c = getc(in)) != EOF)
		putc(c,out);
}

/* Reads a line from in and prints it to out*/
void flinefeed(in, out)
FILE *in;
FILE *out;
{
	char c;
	while( (c = getc(in)) != '\n')
		putc(c,out);
	putc('\n',out);
}

/* Reads a line from stdin and prints it to stdout*/
void linefeed()
{
	char c;
	while( (c = getchar()) != '\n')
		putchar(c);
	putchar('\n');
}

/* Reads a line from stdin and eats it*/
void eatline()
{
	while( getchar() != '\n');
}

/* Reads a line from in and eats it*/
void featline(in)
FILE *in;
{
	while( getc(in) != '\n');
}


/* Returns the char from the stdin but does not remove it from the stream*/
int peek()
{
	int c;
	c = getchar();
	ungetc(c,stdin);
	return c;
}

/* Returns the char from IN but does not remove it from the stream*/
int fpeek(IN)
FILE *IN;
{
	int c;
	c = getc(IN);
	ungetc(c,IN);
	return c;
}
#endif /*#ifndef FEED_H*/
