import time
import serial
import matplotlib.pyplot as plt


def read_value(ser):
    while True:
        try:
            line = ser.readline().decode('ascii')
            value = float(line)
            return value
        except ValueError:
            continue
        
           
def main():
    ser = serial.Serial(port='COM6', baudrate=115200, timeout=0.0)
    if ser.is_open:
        print(f"Port {ser.name} opened")
    else:
        print(f"Port {ser.name} closed")
   
    measure_temperature_C = []
    measure_pressure_kPa = []
    measure_humidity_perc = []
    measure_ts = []    
    start_ts = time.time()
    
    try:
        while True:
            ts = time.time() - start_ts
            ser.write("temp\n".encode('ascii'))
            temp = read_value(ser)
            ser.write("press\n".encode('ascii'))
            press = read_value(ser)
            ser.write("hum\n".encode('ascii'))
            hum = read_value(ser)            
            measure_ts.append(ts)            
            measure_temperature_C.append(temp)
            measure_pressure_kPa.append(press)
            measure_humidity_perc.append(hum)
            print(f'{temp:.2f} C - {press:.3f} kPa - {hum:.2f} %- {ts:.2f}s')
            time.sleep(0.3)
    finally:
        ser.close()
        print("Port closed")
        
        plt.subplot(3, 1, 1)
        plt.plot(measure_ts, measure_temperature_C)
        plt.title('График зависимости температуры от времени')
        plt.xlabel('Время, с')
        plt.ylabel('Температура, C')
        
        plt.subplot(3, 1, 2)
        plt.plot(measure_ts, measure_pressure_kPa)
        plt.title('График зависимости давления от времени')
        plt.xlabel('Время, с')
        plt.ylabel('Давление, кПа')

        plt.subplot(3, 1, 3)
        plt.plot(measure_ts, measure_humidity_perc)
        plt.title('График зависимости влажности от времени')
        plt.xlabel('Время, с')
        plt.ylabel('Влажность, %')
        plt.tight_layout()
        plt.show()


if __name__ == "__main__":
	main()        