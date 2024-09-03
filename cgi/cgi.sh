#!/bin/bash

# Output the HTTP headers
echo "Content-Type: text/html"
echo ""

# Output the HTML content
echo "<html>"
echo "<head><title>Simple CGI Script</title></head>"
echo "<body>"
echo "<h1>Hello, World!</h1>"
echo "<p>This is a simple CGI script written in a shell script.</p>"
echo "</body>"
echo "</html>"