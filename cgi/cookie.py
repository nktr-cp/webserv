#!/usr/bin/env python3
import os
import time
import random
import json
import tempfile

def get_cookie(cookie_string, key):
    cookies = dict(pair.split('=') for pair in cookie_string.split('; ') if '=' in pair)
    return cookies.get(key)

def set_cookie(key, value):
    return f"Set-Cookie: {key}={value}; Path=/; HttpOnly; SameSite=Strict"

def get_temp_file_path():
    return os.path.join(tempfile.gettempdir(), 'visit_counts.json')

def load_visit_counts():
    file_path = get_temp_file_path()
    if os.path.exists(file_path):
        with open(file_path, 'r') as f:
            return json.load(f)
    return {}

def save_visit_counts(visit_counts):
    file_path = get_temp_file_path()
    with open(file_path, 'w') as f:
        json.dump(visit_counts, f)

def main():
    cookie_string = os.environ.get("HTTP_COOKIE", "")
    user_id = get_cookie(cookie_string, "user_id")

    visit_counts = load_visit_counts()

    if not user_id:
        user_id = f"{int(time.time())}-{random.randint(1000, 9999)}"
        print(set_cookie("user_id", user_id))
        visit_counts[user_id] = 1
    else:
        visit_counts[user_id] = visit_counts.get(user_id, 0) + 1

    save_visit_counts(visit_counts)

    colors = ["#3498db", "#e74c3c", "#2ecc71", "#9b59b6", "#f39c12"]
    color = colors[visit_counts[user_id] % len(colors)]

    print("Content-Type: text/html")
    print()
    print(f"""
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>Welcome Back!</title>
        <style>
            body {{
                font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
                display: flex;
                justify-content: center;
                align-items: center;
                height: 100vh;
                margin: 0;
                background-color: #f4f4f9;
            }}
            .container {{
                text-align: center;
                padding: 3em;
                background-color: white;
                border-radius: 15px;
                box-shadow: 0 0 20px rgba(0, 0, 0, 0.1);
            }}
            h1 {{
                color: {color};
                font-size: 2.5em;
            }}
            .count {{
                font-size: 4em;
                font-weight: bold;
                color: {color};
            }}
            p {{
                margin: 0.5em 0;
                font-size: 1.2em;
            }}
        </style>
    </head>
    <body>
        <div class="container">
            <h1>Welcome Back!</h1>
            <p>You have visited this page</p>
            <p class="count">{visit_counts[user_id]}</p>
            <p>times.</p>
        </div>
    </body>
    </html>
    """)

if __name__ == "__main__":
    main()