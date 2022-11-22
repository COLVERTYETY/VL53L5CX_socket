import socket
import sys
import time
import matplotlib.pyplot as plt
import numpy as np
import signal
import struct
import time

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

def signal_handler(sig, frame):
    global sock
    sock.detach()
    sock.close()
    print("ctrl C caught and handled")
    exit(0)


def main():
    plt.ion()
    last = time.time()
    matrix = np.zeros((8,8))
    # Create a TCP/IP socket
    # sock = socket.socket(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1) # (socket.AF_INET, socket.SOCK_STREAM,
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1)
    # Bind the socket to the port
    server_address = ('0.0.0.0', 10000)
    print('starting up on {} port {}'.format(*server_address))
    sock.bind(server_address)

    # Listen for incoming connections
    sock.listen(0)

    buffer = ""

    while True:
        # Wait for a connection
        print('waiting for a connection')
        connection, client_address = sock.accept()
        try:
            print('connection from', client_address)

            # Receive the data in small chunks and retransmit it
            while True:
                data = connection.recv(1)
                if data:
                    # print('sending data back to the client')
                    # connection.sendall(data)
                    buffer+=data.decode("utf-8")
                    # print("r")
                    # print('received {!r}'.format(buffer))
                    if '\n' in buffer:
                        # print("n")
                        # print("data received ", buffer)
                        arr = str.split(buffer,':')
                        if len(arr)==2:
                            index = int(arr[0])
                            val = float(arr[1])
                            matrix[index%matrix.shape[0]][index//matrix.shape[1]] = 255.0*val/2500.0
                            if index == matrix.shape[0]*matrix.shape[1]-1:
                                # plt.imshow(matrix, vmax = 255, vmin = 0)
                                # plt.pause(0.01)
                                print("FPS: ", 1.0/(time.time()-last))
                                last = time.time()
                        else:
                            print("ERRRRRRRRRRRRRRRRRRRRRRRRRROR")

                        buffer=""

                else:
                    print('no data from', client_address)
                    break

        finally:
            # Clean up the connection
            connection.close()

if __name__ == '__main__':
    signal.signal(signal.SIGINT, signal_handler)
    main()