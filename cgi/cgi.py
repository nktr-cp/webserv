#!/usr/bin/env python
import time
import random
import string
import os

# time.sleep(12)
rand = ''.join(random.choice(string.ascii_letters + string.digits + string.punctuation) for _ in range(10))
method = os.getenv("REQUEST_METHOD", "N/A")

# CGI header
print("Status: 200 OK")
print("Content-type: text/html\n")

# Output the HTML content
print("<html>")
print("<head><title>Simple CGI Script</title></head>")
print("<body>")
print("<h1>Hello, World!</h1>")
print("<p>This is a simple CGI script written in Python.</p>")
print("<p>Request method: {}</p>".format(method))
print("<p>{}</p>".format(rand))
print("</body>")
print("</html>")