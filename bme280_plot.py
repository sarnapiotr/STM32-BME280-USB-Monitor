import serial
import matplotlib
matplotlib.use('TkAgg')
import matplotlib.pyplot as plt


def main():
    port = 'COM' + input("Enter COM port number: ")

    try:
        com_port = serial.Serial(port, baudrate=115200, timeout=2)
    except serial.SerialException as e:
        print("Error caught: " + str(e))
        return

    temp, press, hum = [], [], []

    fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(10, 10))
    fig.tight_layout(pad=3.0)

    while True:
        line = com_port.readline().decode('utf-8', errors='ignore').strip()

        if line:
            print(line)
            parts = line.split()

            if len(parts) >= 8:
                temp.append(float(parts[1]))
                press.append(float(parts[4]))
                hum.append(float(parts[7]))

                if len(temp) > 100:
                    temp.pop(0)
                    press.pop(0)
                    hum.pop(0)

                ax1.clear()
                ax2.clear()
                ax3.clear()

                ax1.plot(temp, 'r')
                ax2.plot(press, 'g')
                ax3.plot(hum, 'b')

                ax1.set_title("BME280 Sensor Data")
                ax1.set_xlabel("Samples")
                ax1.set_ylabel("Temperature [C]")
                ax1.grid(True)

                ax2.set_xlabel("Samples")
                ax2.set_ylabel("Pressure [hPa]")
                ax2.grid(True)

                ax3.set_xlabel("Samples")
                ax3.set_ylabel("Humidity [%]")
                ax3.grid(True)
        else:
            break

        plt.pause(0.01)

if __name__ == '__main__':
    main()
