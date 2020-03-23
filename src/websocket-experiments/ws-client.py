import websocket
 
ws = websocket.WebSocket()
ws.connect("ws://192.168.0.142/ws")
 
result = ws.recv()
print(result)
 
ws.close()
