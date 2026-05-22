import asyncio
import struct
import csv
import time
import matplotlib.pyplot as plt
from bleak import BleakScanner, BleakClient

# CONFIGURACIÓ  
DEVICE_NAME = "ESP32_NIRS"
CHARACTERISTIC_UUID = "8a3e2b1f-4c5d-4a9b-8c7d-3e2f1a5b6c4d"
LOG_FILE = "nirs_data_log.csv"

# Variables globals
smo2_history = []
time_history = []
max_points = 50 
start_time = 0

def notification_handler(sender, data):
    global start_time
    
    # Desempaquetat uint16_t
    smo2_x100 = struct.unpack("<H", data)[0]
    smo2 = smo2_x100 / 100.0
    
    current_relative_time = time.time() - start_time
    
    # Escriptura en CSV
    with open(LOG_FILE, mode='a', newline='') as f:
        writer = csv.writer(f)
        writer.writerow([round(current_relative_time, 2), smo2])

    print(f"[{current_relative_time:.2f}s] SmO2: {smo2:.2f}%")
    
    smo2_history.append(smo2)
    time_history.append(current_relative_time)
    
    if len(smo2_history) > max_points:
        smo2_history.pop(0)
        time_history.pop(0)

async def main():
    global start_time
    print(f"Buscant el dispositiu '{DEVICE_NAME}' (Timeout: 20s)...")
    
    device = await BleakScanner.find_device_by_filter(
        lambda d, ad: d.name and d.name.lower() == DEVICE_NAME.lower(),
        timeout=20.0
    )
    
    if not device:
        print(f"Error: No s'ha trobat el dispositiu '{DEVICE_NAME}'.")
        return

    # Preparació del fitxer de registre
    with open(LOG_FILE, mode='w', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(["Temps_Relatiu_s", "SmO2_Percentatge"])

    async with BleakClient(device) as client:
        print(f"Connectat a {device.address}")
        
        # Configuració de la interfície gràfica
        plt.ion() 
        plt.style.use('seaborn-v0_8-darkgrid')
        fig, ax = plt.subplots()
        line, = ax.plot([], [], 'r-', linewidth=2)
        
        ax.set_ylim(0, 100)
        ax.set_title("Monitorització $SmO_2$ (Beer-Lambert)")
        ax.set_xlabel("Temps (segons)")
        ax.set_ylabel("Saturació d'Oxigen (%)")
        
        # Forcem l'aparició de la finestra
        plt.show(block=False)
        plt.pause(0.1)
        
        start_time = time.time()
        await client.start_notify(CHARACTERISTIC_UUID, notification_handler)

        try:
            while plt.fignum_exists(fig.number):
                if smo2_history:
                    line.set_xdata(time_history)
                    line.set_ydata(smo2_history)
                    
                    # Ajust d'eixos
                    ax.set_xlim(time_history[0], time_history[-1] + 1)
                    
                    # Refresc de la interfície
                    fig.canvas.draw()
                    fig.canvas.flush_events()
                
                # plt.pause és necessari per processar els esdeveniments de la GUI
                plt.pause(0.01)
                await asyncio.sleep(0.05)
                    
        except Exception as e:
            print(f"\nError en el bucle principal: {e}")
        finally:
            print("Finalitzant sessió...")
            await client.stop_notify(CHARACTERISTIC_UUID)

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\nProcés interromput per l'usuari.")