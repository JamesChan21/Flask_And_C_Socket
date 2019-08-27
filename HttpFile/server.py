import sys
reload(sys)
sys.setdefaultencoding( "utf-8" )

from flask import Flask, request
from flask_restful import Api, Resource, reqparse

app = Flask(__name__)
api = Api(app)

@app.route('/')
def hello_world():
	return 'Hello World!!'

@app.route('/upload', methods = ['POST'])
def upload_file():
	print(request.headers)
	print("request.method: ", request.method)
	if request.method == 'POST':
		f = open('haha.txt', 'w+')
		f.write(request.form['myFile'])
		f.close()
		print(request.form['file_name'])
		return 'success!'

if __name__ == '__main__':
	app.run(threaded=True, debug=True)


