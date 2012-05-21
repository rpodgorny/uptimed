Contents
--------

	1. Using uprecords as CGI program
	2. Configuring the output of uprecords as CGI program
	3. Inserting the output of uprecords into existing pages

1. Using uprecords as CGI program
---------------------------------

Copy the uprecords binary to your CGI directory and rename it:

	cp uprecords /www/cgi-bin/uprecords.cgi

Uprecords will automatically recognize it is being run as CGI program by
it's .cgi filename and add the necessary Content-type header and some HTML
tags.

2. Configuring the output of uprecords as CGI program
-----------------------------------------------------

When run as CGI, uprecords.cgi checks for a file called uprecords.conf in
the current (= cgi-bin) directory. This configuration file allows you to
control the way uprecords.cgi will look.

LAYOUT can be any of "pre", "list", or "table". "pre" uses the <pre> tag to
create output close to that of running uprecords from the console and is the
default option. "list" creates a list using the <ul> and <li> tags and
"table" creates a table using the <table>, <tr> and <td> tags.

SHOW_MAX defines how many entries should be shown at maximum.

If the file uprecords.header is present, it will be printed before the
output. You can use this to create the most smashing looks, from a simple
<body> tag to change to background colour to a comprehensive page including
all the latest CSS style-sheets. If the file uprecords.footer is present, it
will be printed after the output.

There are some sample files in the sample-cgi/ directory of this package.

3. Inserting the output of uprecords into existing pages
--------------------------------------------------------

You can include the output within existing HTML pages to get the same
lay-out as the rest of your site using server-side includes, PHP, etc.

Example tags to put within your pages:

SSI:	<!--#include virtual="/cgi-bin/uprecords.cgi"-->
PHP:	<? virtual("/cgi-bin/uprecords.cgi"); ?>

For more information on SSI, read the Apache manuals (if Apache is your
webserver) or contact your Internet Service/Presence Provider. For more
information on PHP, visit http://www.php.net/
