import socket
import threading 
import matplotlib.pyplot as plt
from queue import Queue
# Khoi tao gia tri nhan thoi gian
# receivedata = []
# time = []
# Create a queue to handle client connections
connection_queue = Queue()
x_data = []
y_data = []

# Tao socket
TCP = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
TCP.bind(('172.20.10.3',3333))
TCP.listen(5)
print("Waiting for connections...")

def handle_client(client_socket):
    # global receivedata, time
    global x_data, y_data
    try:
        while True:
            data = client_socket.recv(1024).decode()
            #/////
            
            # x, y = map(float, data.strip().split(','))
            # x_data.append(x)
            # y_data.append(y)  
            # plt.plot(x_data, y_data)
            # plt.xlabel('Góc (độ)')
            # plt.ylabel('sin(x)')
            # plt.title('Biểu đồ hình sin')
            # plt.draw()  # Update the plot
            # plt.pause(0.001) 
            # plt.show()
            data = input("Nhập phím điều khiển:")
            if not data:
                continue
            if data ==b'2':
                print('vibrate.....')
                client_socket.sendall(b'2')
            elif data ==b'stop':
                client_socket.sendall(b'stop')
                print('stopping motor...')
            #value = float(data)
            # Gửi phản hồi lại cho client    
            client_socket.sendall(b'OK')
            print("Data Send")
           
            # Append numerical data to receivedata
            # receivedata.append(value)
            
            # # Append time
            # time.append(len(receivedata)) 
            
            # # Plot the data
            # plt.plot(time, receivedata, marker='o', linestyle='-')
            # plt.xlabel('Thoi gian')
            # plt.ylabel("Du lieu nhan duoc")
            # plt.show()  # Show plot
    except Exception as e:
        print("Error handling client:", e)     
            
    #client_socket.close()

def accept_connections():
    while True:
        client_socket, client_addr = TCP.accept()
        connection_queue.put((client_socket, client_addr))

# Start the thread to accept connections
accept_thread = threading.Thread(target=accept_connections)
accept_thread.start()

try:
    while True:
        # Get client connection from the queue
        client_socket, client_addr = connection_queue.get()
        client_thread = threading.Thread(target=handle_client, args=(client_socket,))
        client_thread.start()
except KeyboardInterrupt:
    print("Server shutting down...")
finally:
    TCP.close()
    
