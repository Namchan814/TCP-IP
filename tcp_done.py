import tkinter as tk
from tkinter import scrolledtext
import socket
import threading

# Dictionary to store client sockets
clients = {}

def handle_client(client_socket, client_id):
    while True:
        try:
            # Receive message from the client
            message = client_socket.recv(1024)
            if not message:
                break
            
            # Print the data received from Client 1
            if client_id == 1:
                print(f"Received from Client 1: {message.decode('utf-8')}")
            
            # Send the message to the other client
            if client_id == 1 and 2 in clients:
                print(f"Sending to Client 2: {message.decode('utf-8')}")
                clients[2].sendall(message)
            elif client_id == 2 and 1 in clients:
                print(f"Sending to Client 1: {message.decode('utf-8')}")
                clients[1].sendall(message)
        except ConnectionResetError:
            break

    print(f"Client {client_id} disconnected")
    del clients[client_id]
    client_socket.close()

def start_server():
    # Create a socket for the server
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.bind(('172.20.10.3', 3333))  # Change to your server IP
    server.listen(2) 
    print("Server listening on port 3333")
    
    client_id = 1  # ID for clients
    
    while client_id <= 2: 
        # Accept a new connection
        client_socket, client_address = server.accept()
        print(f"Accepted connection from {client_address} with client ID {client_id}")
        
        # Store the client socket in the dictionary
        clients[client_id] = client_socket
        
        # Create a new thread to handle the client
        client_handler = threading.Thread(
            target=handle_client, 
            args=(client_socket, client_id)
        )
        client_handler.start() 
        client_id += 1

if __name__ == "__main__":
    start_server()