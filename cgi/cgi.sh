#!/bin/bash

# sleep 12
rand=$(tr -dc 'A-Za-z0-9!@#$%^&*()' < /dev/urandom | head -c 10)
method=$(printenv REQUEST_METHOD)

# Output the HTTP headers
echo "Status: 200 OK"
echo "Content-Type: text/html"
echo ""

# Output the HTML content
echo "<html>"
echo "<head><title>Simple CGI Script</title></head>"
echo "<body>"
echo "<h1>Hello, World!</h1>"
echo "<p>This is a simple CGI script written in a shell script.</p>"
echo "<p>Request method: $method</p>"
echo "<p>$rand</p>"
echo "</body>"
echo "</html>"