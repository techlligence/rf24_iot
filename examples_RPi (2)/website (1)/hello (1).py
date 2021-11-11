from flask import Flask, render_template, jsonify
import sys
import os
import time
import threading

app = Flask(__name__)

def currentLevelFIFO(name):
    print('Starting the current level fifo thread', file=sys.stdout)
    print('Opening the fifo:', file=sys.stdout)
    rf = os.open("../currentLevelFIFO", os.O_RDONLY)

    while True:
        # Read the data
        s = os.read(rf, 2048)

        # Format the string nicely
        strippedString = s.strip()
        decodedString = strippedString.decode('utf-8')
        correctString = decodedString.split('@')[0]
        formatedString = (correctString[:-2]).strip()
        #print("Received:")
        #print(formatedString)

        nodes = (formatedString.strip()).split( '#' )
        i = 0

        global currentLevels

        currentLevels = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]

        for node in nodes:
            split_node = (node.strip()).split(' ')
            #print(split_node, file=sys.stdout)
            if len(split_node) > 1:
                nodeID = split_node[0]
                nodeCurrent = split_node[1]
                #print('nodeid', file=sys.stdout)
                #print(nodeID, file=sys.stdout)
                currentLevels[int(nodeID)] = nodeCurrent
                i+=1


def thread_function(name):
    global testValue
    print('Starting thread', file=sys.stdout)
    testValue = False
    while True:
        if testValue == False:
            testValue = True
        else:
            testValue = False

        time.sleep(1)
        print('Running function now', file=sys.stdout)

#x = threading.Thread(target=thread_function, args=("Thread-1",))
#x.start()

t = threading.Thread(target=currentLevelFIFO, args=("CurrentLevelFIFOThread",))
t.start()

@app.route('/update_current', methods=['POST'])
def update_current():
    updatedCurrentLevels = currentLevels
    return jsonify('', render_template('updated_current.html', currLevels=updatedCurrentLevels))

@app.route("/")
@app.route("/home")
def home_page():
    #print('This is error output', file=sys.stderr)
    #print('The variable is:', file=sys.stdout)
    #print(testValue, file=sys.stdout)
    print(currentLevels, file=sys.stdout)

    return render_template('home.html', currLevels=currentLevels)
