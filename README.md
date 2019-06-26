# grip
Grep files for IP addresses contained in given subnets


Problem:

You want to search files (logfiles, config files) for occurrences of IPv4 addresses that are inside a bunch of subnets.
Standard grep works well when those subnets are aligned at octett boundaries - /8, /16, /24. You then just use a regexp of e.g. "192\.168\.[0-9]+\.[0-9]+". However, for other network sizes, the regexp needded quickly become ... interesting.

Solution:

This is where grip comes in. grip behaves in a small part like grep, but uses subnets instead of regular expressions.
So, you can use
    
    $ grip 172.18.16.0/20  logfile
    
to print all lines from "logfile" containing addresses from this subnet.

Synopsis:

    grip [OPTIONS] subnet1 [subnet2 ...] [file1 file2 ... ]
    
Description:

    Print lines from files containing IP addresses from subnets. If files are omitted, STDIN will be used.
    
Options:
 
  -h:  Short help 
  
  -v:  Invert logic, i.e. print only lines that would otherwise not be printed
  

Author:

  Written by Bernhard Griener
