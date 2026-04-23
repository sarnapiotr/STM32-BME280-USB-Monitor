import serial
import matplotlib
matplotlib.use('TkAgg')
import matplotlib.pyplot as plt

def main():
    port = 'COM' + input("Enter COM port number: ")
    com_port = serial.Serial(port, 115200)

    temp, pres, humid = [], [], []

    plt.ion()
    fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(8, 8))
    fig.tight_layout(pad=3.0)

    line_t, = ax1.plot([], [], 'r')
    line_p, = ax2.plot([], [], 'g')
    line_h, = ax3.plot([], [], 'b')

    ax1.set_title("BME280 Sensor Data")
    ax3.set_xlabel("Samples")

    ax1.set_ylabel("Temperature [C]")
    ax1.grid(True)
    ax2.set_ylabel("Pressure [hPa]")
    ax2.grid(True)
    ax3.set_ylabel("Humidity [%]")
    ax3.grid(True)

    while True:
        line = com_port.readline().decode('utf-8', errors='ignore').strip()

        if line:
            print(line)

            parts = line.split()

            if len(parts) >= 8:
                val_t = float(parts[1])
                val_p = float(parts[4])
                val_h = float(parts[7])

                temp.append(val_t)
                pres.append(val_p)
                humid.append(val_h)

                if len(temp) > 100:
                    temp.pop(0)
                    pres.pop(0)
                    humid.pop(0)

                x_data = range(len(temp))
                line_t.set_data(x_data, temp)
                line_p.set_data(x_data, pres)
                line_h.set_data(x_data, humid)

                for ax in (ax1, ax2, ax3):
                    ax.relim()
                    ax.autoscale_view()

                plt.pause(0.01)

if __name__ == '__main__':
    main()