"""
    Description.
"""


import serial
import time
import sys
import matplotlib.pyplot as plt
import matplotlib.animation as animation
plt.style.use('seaborn-whitegrid')


# ------------
def set_pid_values():
    serial_port = '/dev/cu.DSDTECHHC-05-DevB';
    baud_rate = 9600; 
    ser = serial.Serial(serial_port, baud_rate)
    kp = 0
    kd = 0
    ki = 0
    while True:
        input_ = input("instruction,kp,kd,ki: ").split(',')
        instruction = int(input_[0])
        if len(input_) > 1:  # Update PID parameters
            kp = float(input_[1])
            kd = float(input_[2])
            ki = float(input_[3])
        msg = "<" + str(instruction) + "/" + str(kp) + "/" + str(kd) + \
                "/" + str(ki) + ">"
        print(msg)
        ser.write(msg.encode())


# ------------
def plot_measures():
    # serial_port = '/dev/cu.wchusbserial1d1120';
    seq_number = 0
    serial_port = '/dev/cu.wchusbserial14120';
    baud_rate = 9600; 
    ser = serial.Serial(serial_port, baud_rate, timeout=0.1)
    last_lower_interval = seq_number
    size_interval = 600
    plt.axis([last_lower_interval, last_lower_interval+size_interval, 5, 9])
    while True:
        time.sleep(0.05)
        try: 
            ser.flushInput()
            msg = ser.readline().decode("utf-8").rstrip();
            measures = msg.split('/')
            measures = list(map(lambda x: float(x), measures))
        except: 
            # print("Error")
            continue
        if len(measures) != 3: continue
        if seq_number >= measures[0]: continue
        # print(measures)
        seq_number = measures[0]
        plt.scatter(seq_number, measures[1], color='red')
        plt.scatter(seq_number, measures[2], color='blue')
        plt.pause(0.001)
        if seq_number > (last_lower_interval+size_interval-10): 
            last_lower_interval += size_interval
            x1,x2,y1,y2 = plt.axis()
            plt.axis([last_lower_interval, last_lower_interval+size_interval, y1, y2])
    plt.show()


# ------------
def sub_plot_measures():
    # serial_port = '/dev/cu.wchusbserial1d1120';
    seq_number = 0
    serial_port = '/dev/cu.wchusbserial14120';
    baud_rate = 9600; 
    ser = serial.Serial(serial_port, baud_rate, timeout=0.1)
    last_lower_interval = seq_number
    size_interval = 600

    fig,ax = plt.subplots(2,1)
    ax[0].axis([last_lower_interval, last_lower_interval+size_interval, 0, 10])
    ax[1].axis([last_lower_interval, last_lower_interval+size_interval, 0, 10])

    while True:
        time.sleep(0.05)
        try: 
            ser.flushInput()
            msg = ser.readline().decode("utf-8").rstrip();
            measures = msg.split('/')
            measures = list(map(lambda x: float(x), measures))
        except: 
            # print("Error")
            continue
        # print(measures)
        if len(measures) != 5: continue
        if seq_number >= measures[0]: continue
        seq_number = measures[0]
        ax[0].scatter(seq_number, measures[1], color='red', label='v_target')
        ax[0].scatter(seq_number, measures[2], color='blue', label='v_measured')
        ax[1].scatter(seq_number, measures[3], color='green', label='v_left')
        ax[1].scatter(seq_number, measures[4], color='orange', label='v_right')
        plt.pause(0.001)
        if seq_number > (last_lower_interval+size_interval-10): 
            last_lower_interval += size_interval
            x1,x2,y1,y2 = ax[0].axis()
            ax[0].axis([last_lower_interval, last_lower_interval+size_interval, y1, y2])
            ax[1].axis([last_lower_interval, last_lower_interval+size_interval, y1, y2])
    plt.show()


# ------------
if __name__ == "__main__":
    if len(sys.argv) <= 1: 
        print("Wrong parameters: python master.py [pid/plot]")
        exit(0)
    param = sys.argv[1]
    if param == "pid":
        set_pid_values()
    elif param == "plot":
        plot_measures()
    elif param == "biplot":
        sub_plot_measures()
    else:
        print("Wrong parameters: python master.py [pid/plot]")















