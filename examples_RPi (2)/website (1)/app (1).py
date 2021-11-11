from flask import Flask, render_template, jsonify, request
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

        global currentLevelsDynamic
        global currentLevelsDynamicTemp

        currentLevels = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
        #triggeredNodeID = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]

        currentLevelsDynamic = []

        currentLevelsDynamic.clear()

        for node in nodes:
            split_node = (node.strip()).split(' ')
            #print(split_node, file=sys.stdout)
            
            if len(split_node) >= 4:
                nodeID = split_node[0]
                nodeCurrent = split_node[1]
                nodeRelay = split_node[2]
                nodeSmoke = split_node[3]
                #print('nodeid', file=sys.stdout)
                #print(nodeID, file=sys.stdout)

                # Create and add tuple to the list
                nodeTuple = (nodeID, nodeCurrent, nodeRelay, nodeSmoke)
                currentLevelsDynamic.append(nodeTuple) 

                if nodeID.isnumeric():
                    currentLevels[int(nodeID)-1] = nodeCurrent
                else:
                    break
                #triggeredNodeID[int(nodeID)] = 1

            currentLevelsDynamic.sort(key=lambda tup: tup[0])


def toggleFIFO(node):
    print("Writing to FIFO")
    charNode = bytes((node+"\0"),'utf-8')
    #blankMessage = bytes("", 'utf-8')
    wf = os.open("../toggleFIFO", os.O_WRONLY)
    print("writing now")
    os.write(wf, charNode)
    print("finished write")
    os.close(wf)
    #fifo_write = open('../toggleFIFO', 'w')
    #fifo_write.write("Hello World\n")
    #@fifo_write.close()
    

t = threading.Thread(target=currentLevelFIFO, args=("CurrentLevelFIFOThread",))
t.start()

@app.route('/update_current', methods=['POST'])
def update_current():
    updatedCurrentLevels = currentLevels
    updatedNodeList = currentLevelsDynamic
    return jsonify('', render_template('updated_current.html', currLevels=updatedCurrentLevels, allNodes = updatedNodeList))

@app.route('/toggle_node/<node>') 
def toggle_node(node):
    print ("Node toggled")
    print (node)
    print("Toggle thread started")
    toggleThread = threading.Thread(target=toggleFIFO, args=(node,))
    toggleThread.start()

    return (''), 204


@app.route("/")
@app.route("/home")
def home_page():
    #print('This is error output', file=sys.stderr)
    #print('The variable is:', file=sys.stdout)
    #print(testValue, file=sys.stdout)
    print(currentLevels, file=sys.stdout)

    #print(triggeredNodeID, file=sys.stdout)

    return render_template('home.html', currLevels=currentLevels, allNodes = currentLevelsDynamic )

if __name__ == "__main__":
    app.run(host='0.0.0.0', port=8000, debug=True)
