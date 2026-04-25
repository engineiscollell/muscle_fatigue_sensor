import asyncio
import struct
import threading
import time
import sys
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from bleak import BleakClient, BleakScanner

# --- CONFIGURACIÓ ---
DEVICE_MAC = "88:57:21:6A:1B:EA"  
CHARACTERISTIC_UUID = "191bb640-16f1-4dc1-b7f5-f1114cb80a77"

y_data = [0] * 100
x_data = list(range(100))
is_connected = False  # Variable de control!

def notification_handler(sender, data):
    raw_value = struct.unpack('<H', data)[0]
    smo2 = raw_value / 100.0
    y_data.append(smo2)
    y_data.pop(0)
    print(f"SmO2: {smo2:.2f}%")

async def run_ble():
    global is_connected
    print(f"Buscant l'ESP32 a la MAC {DEVICE_MAC}...")
    try:
        device = await BleakScanner.find_device_by_address(DEVICE_MAC, timeout=5.0)
        
        if not device:
            print("No s'ha trobat el dispositiu a l'aire.")
            return

        print(f"Trobat! Intentant connectar...")
        async with BleakClient(device) as client:
            print("¡CONNECTAT AMB ÈXIT AL FIRMWARE!")
            is_connected = True # Avisem a la gràfica que ja pot obrir-se!
            
            await client.start_notify(CHARACTERISTIC_UUID, notification_handler)
            while True:
                await asyncio.sleep(1)
                
    except Exception as e:
        print(f"\n[ERROR CRÍTIC BLE]: {e}")
        print("Pista 1: El mòbil encara està connectat al sensor? APAGA el Bluetooth del telèfon.")
        print("Pista 2: Treu el cable USB de l'ESP32 i torna'l a posar (Hard Reset).")

def start_ble_loop():
    loop = asyncio.new_event_loop()
    asyncio.set_event_loop(loop)
    loop.run_until_complete(run_ble())

# ==========================================
# 1. INICIEM EL BLUETOOTH EN SEGON PLA
# ==========================================
ble_thread = threading.Thread(target=start_ble_loop, daemon=True)
ble_thread.start()

# ==========================================
# 2. LA SALA D'ESPERA
# ==========================================
print("Esperant a establir la connexió BLE abans d'obrir Matplotlib...")
while not is_connected:
    time.sleep(0.1)
    if not ble_thread.is_alive():
        print("\nS'ha cancel·lat l'obertura de la gràfica per un error de connexió.")
        sys.exit()

# ==========================================
# 3. OBRIM LA GRÀFICA (Només si s'ha connectat)
# ==========================================
print("Obrint la finestra de Matplotlib...")
fig, ax = plt.subplots()
line, = ax.plot(x_data, y_data, color='blue', lw=2)
ax.set_ylim(0, 100)
ax.set_ylabel("SmO2 (%)")
ax.set_title("Monitorització ESP32_NIRS")
ax.grid(True)

def update_plot(frame):
    line.set_ydata(y_data)
    return line,

ani = FuncAnimation(fig, update_plot, interval=100, cache_frame_data=False)
plt.show()