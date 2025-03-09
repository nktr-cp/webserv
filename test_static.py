import requests
import subprocess
import time

def test_static():
	process = subprocess.Popen(['./webserv', 'config/test.conf'], stderr=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
	time.sleep(1)

	try:
		print("--- test correct static requeset with [GET POST DELETE] ---")
		data = "testdata"
		print("[1]sending POST request to http://localhost:8080/test/test, expecting 201")
		response = requests.post("http://localhost:8080/test/test", data=data)
		assert response.status_code == 201, f"[1] Failed to get 201 response. {response.status_code}"
		print("[2]sending GET request to http://localhost:8080/test/test, expecting 200")
		response = requests.get("http://localhost:8080/test/test")
		assert response.status_code == 200, f"[2] Failed to get response from server. {response.status_code}"
		print("[3]checking content of response")
		assert response.text == data, f"[3] Failed to get expected content. {data}"
		print("[4]sending DELETE request to http://localhost:8080/test/test, expecting 200")
		response = requests.delete("http://localhost:8080/test/test")
		assert response.status_code == 200, f"[4] Failed to get 200 response. {response.status_code}"
		print("[5]sending GET request to http://localhost:8080/test/test, expecting 404")
		response = requests.get("http://localhost:8080/test/test")
		assert response.status_code == 404, f"[5] Failed to get 404 response from server. {response.status_code}"

		print("--- test incorrect static requeset with [GET POST DELETE] ---")
		print("[6]sending too large POST request to http://localhost:8080/test/test, expecting 413")
		data = "a" * 50000
		response = requests.post("http://localhost:8080/test/test", data=data)
		assert response.status_code == 413, f"[6] Failed to get 413 response. {response.status_code}"
		print("[7]sending GET request with too long uri to http://localhost:8080/test, expecting 414")
		response = requests.get("http://localhost:8080/test" + "/a" * 5000)
		assert response.status_code == 414, f"[7] Failed to get 414 response. {response.status_code}"

		print("All tests passed")
	
	except AssertionError as e:
		print("Assertion failed: {e}")

	finally:
		process.terminate()
		stderr = process.communicate()[1]
		print("Standard Error Output:\n", stderr)
		process.wait()

if __name__ == "__main__":
	test_static()
