#!/bin/bash

# sleep 12

# Output the HTTP headers
echo "Content-Type: text/html"
echo ""

# Generate a random string
rand=$(tr -dc 'A-Za-z0-9!@#$%^&*()' < /dev/urandom | head -c 10)

# Output the HTML content
echo "<html>"
echo "<head><title>Simple CGI Script</title></head>"
echo "<body>"
echo "<h1>Hello, World!</h1>"
echo "<p>This is a simple CGI script written in a shell script.</p>"
echo "<p>$rand</p>"
echo "</body>"
echo "</html>"