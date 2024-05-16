import socket
import threading 
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np
from queue import Queue
# Khoi tao gia tri nhan thoi gian
# receivedata = []
# time = []
# Create a queue to handle client connections
connection_queue = Queue()

# x_data = []
# y_data = []
# pwm_values = []

# # Khởi tạo đồ thị
# fig, ax = plt.subplots()
# line, = ax.plot([], [], lw=2)
# ax.set_ylim(0, 1.1)
# ax.set_xlim(0, 50)
# ax.set_ylabel('PWM Value')
# ax.set_xlabel('Time')
# ax.set_title('PWM Pulse')
# Tao socket
TCP = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
TCP.bind(('172.20.10.3',3333))
TCP.listen(5)
print("Waiting for connections...")
# second_client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
# second_client.connect(('192.168.1.8', 3333)) 
def handle_client(client_socket):
    # global receivedata, time 
    try:
            while True:
                # Nhập giá trị từ người dùng
                user_input = input("Nhập giá trị duty cycle (0 - 8191) hoặc 'exit' để thoát: ")
                if user_input.lower() == 'exit':
                    print("Đóng kết nối...")
                    break

                # Kiểm tra giá trị nhập vào có hợp lệ không
                try:
                    duty_cycle = int(user_input)
                    if 0 <= duty_cycle <= 8191:
                        message = f"{duty_cycle}"
                        client_socket.sendall(message.encode())
                        print(f"Đã gửi: {message}")

                    else:
                        print("Giá trị phải nằm trong khoảng 0 - 8191.")
                except ValueError:
                    print("Vui lòng nhập một số hợp lệ.")
                

    except (ConnectionResetError, BrokenPipeError):
        print("Kết nối bị ngắt.")
    finally:

        client_socket.close()
            
    #client_socket.close()
# def init():
#     line.set_data([], [])
#     return line,
# def receive_data():
#     global pwm_values
#     while True:
#         data = client_socket.recv(1024).decode('utf-8').strip()
#         if data:
#             pwm_value = float(data) / 8191  # Chuyển đổi giá trị PWM về khoảng [0, 1]
#             pwm_values.append(pwm_value)
#             if len(pwm_values) > 50:
#                 pwm_values.pop(0)
# def update(frame):
#     xdata = np.arange(len(pwm_values))
#     ydata = pwm_values
#     line.set_data(xdata, ydata)
#     return line,

def accept_connections():
    while True:
        client_socket, client_addr = TCP.accept()
        connection_queue.put((client_socket, client_addr))



# Start the thread to accept connections
accept_thread = threading.Thread(target=accept_connections)
accept_thread.start()
# ani = animation.FuncAnimation(fig, update, init_func=init, blit=True, interval=100)

# Hiển thị đồ thị
# plt.show()
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
    
