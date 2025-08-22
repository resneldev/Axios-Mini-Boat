"""
Axios PC-Programm - Version Améliorée

Ce programme contrôle un ESP8266 pour mesurer la conductivité de l'eau avec :
- Une interface graphique Tkinter
- Un système asynchrone pour les communications
- Une fermeture propre des ressources
"""

import asyncio
import aiohttp
import numpy as np
import matplotlib.pyplot as plt
from tkinter import *
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import webbrowser
from openpyxl import Workbook, load_workbook
import os
import datetime
from typing import Optional, List

class ESP8266Controller:
    def __init__(self, ip_address: str):
        self.base_url = f"http://{ip_address}/control"
        self.auto_mode_active = False
        self.address = ip_address
        self.session: Optional[aiohttp.ClientSession] = None
        print(f"IP-Adresse: {ip_address}")

    async def connect(self):
        """Crée une session HTTP partagée"""
        self.session = aiohttp.ClientSession()

    async def close(self):
        """Ferme proprement la session HTTP"""
        if self.session and not self.session.closed:
            await self.session.close()

    async def send_command(self, action: str) -> str:
        try:
            if not self.session:
                await self.connect()
                
            async with self.session.get(
                f"{self.base_url}?action={action}", 
                timeout=aiohttp.ClientTimeout(total=2)
            ) as response:
                return await response.text()
        except Exception as e:
            return f"Fehler: {str(e)}"

    async def toggle_auto_mode(self, start: bool) -> str:
        self.auto_mode_active = start
        cmd = "auto_modus" if start else "manuell_modus"
        return await self.send_command(cmd)

class AxiosAPP:
    def __init__(self, controller: ESP8266Controller):
        self.controller = controller
        self.bg_control_color = "#035E01"
        
        # Configuration de la fenêtre principale
        self.window = Tk()
        self.window.title("Axios App - Verbessert")
        self.window.geometry("800x480")
        self.window.protocol("WM_DELETE_WINDOW", self.on_closing)
        
        # Variables d'état
        self.max_data_points = 100
        self.update_interval = 500
        self.auto_refresh = True
        self.ip_ready = False
        self.running = True
        
        # Données du graphique
        self.x_data = np.linspace(0, 1, self.max_data_points)
        self.y_data = np.zeros(self.max_data_points)
        
        # Gestion des tâches asynchrones
        self.tasks: List[asyncio.Task] = []
        
        # Initialisation des composants
        self.setup_gui()
        self.setup_plot()
        self.initialize_excel_file()
        
        # Démarrer la mise à jour automatique
        self.schedule_update()

    def setup_gui(self):
        """Configure l'interface graphique"""
        # Cadres principaux
        self.control_frame = Frame(self.window, bg="#44C942", borderwidth=1, relief='groove')
        self.graph_frame = Frame(self.window, bg="#A3E7A3")
        
        # Organisation des cadres
        self.control_frame.pack(side='right', expand=YES, fill='both')
        self.graph_frame.pack(side='left', expand=YES, fill='both')
        
        # Création des widgets
        self.create_widgets()

    def create_widgets(self):
        """Crée tous les widgets de l'interface"""
        # Widgets de statut
        self.status_var = StringVar(value="Bereit - Nicht verbunden")
        Label(self.control_frame, textvariable=self.status_var, 
              font=('Arial', 10), bg="#69CF67").pack(side="top")
        
        # Boutons de mode
        self.mode_frame = Frame(self.control_frame, bg="#44C942")
        self.mode_frame.pack()
        
        Button(self.mode_frame, text="Auto", font=("Helvetica", 20),
               bg='#18AF15', fg='white',
               command=lambda: self.run_async(self.activate_auto_modus()),
               width=6).pack(side='left', padx=5)
               
        Button(self.mode_frame, text="Manuel", font=("Helvetica", 20),
               bg='#18AF15', fg='white',
               command=lambda: self.run_async(self.activate_manuell_modus()),
               width=6).pack(side='right', padx=5)
        
        # Contrôles manuels
        self.manual_frame = Frame(self.control_frame, bg="#44C942")
        self.setup_manual_controls()
        
        # Contrôles automatiques
        self.auto_frame = Frame(self.control_frame, bg="#44C942")
        self.setup_auto_controls()
        
        # Graphique et contrôles associés
        self.setup_graph_controls()

    def setup_graph_controls(self):
        """Configure les contrôles du graphique"""
        under_graph_frame = Frame(self.graph_frame, bg="#44C942")
        under_graph_frame.pack(side='bottom', expand=YES, fill='both')
        
        # Affichage de la valeur
        self.value_var = StringVar(value="Leitwert: 0.000 µS")
        Label(under_graph_frame, textvariable=self.value_var,
              font=('Arial', 12), bg='#18AF15').pack(side='right')
        
        # Boutons de contrôle
        Button(under_graph_frame, text="Pause", font=("Arial", 15),
               bg='#18AF15', fg='white',
               command=self.toggle_pause, width=10).pack(side='left')
               
        Button(under_graph_frame, text="Speichern", font=("Arial", 15),
               bg='#18AF15', fg='white',
               command=self.save_graph, width=10).pack(side='left')

    def setup_plot(self):
        """Configure le graphique matplotlib"""
        self.fig, self.ax = plt.subplots(figsize=(7, 3), dpi=100)
        self.fig.patch.set_facecolor("#74bb80")
        self.ax.set_facecolor("#74bb80")
        self.ax.set_title('Echtzeit-Leitwertanzeige in µS')
        self.ax.set_xlabel('Zeit (s)')
        self.ax.set_ylabel('Leitwert (µS)')
        self.ax.set_ylim(0, 100)
        self.ax.grid(True, color='black', linestyle='--', linewidth=0.5)
        
        self.line, = self.ax.plot(self.x_data, self.y_data, 'b-')
        self.canvas = FigureCanvasTkAgg(self.fig, master=self.graph_frame)
        self.canvas.draw()
        self.canvas.get_tk_widget().pack(fill=BOTH, expand=True)

    async def update_graph_data(self):
        """Met à jour les données du graphique de manière asynchrone"""
        while self.running and self.auto_refresh:
            try:
                value_str = await self.controller.send_command("read_Leitwert")
                try:
                    value = float(value_str)
                except ValueError:
                    value = 0
                
                self.y_data = np.roll(self.y_data, -1)
                self.y_data[-1] = value
                
                self.line.set_ydata(self.y_data)
                self.ax.relim()
                self.ax.autoscale_view(scalex=False)
                self.canvas.draw()
                self.value_var.set(f"Leitwert: {value:.3f} µS")
                
                self.save_to_excel(value)
                
            except Exception as e:
                self.status_var.set(f"Fehler: {e}")
            
            await asyncio.sleep(self.update_interval / 1000)

    def schedule_update(self):
        """Planifie la mise à jour du graphique"""
        if not hasattr(self, 'update_task') or self.update_task.done():
            self.update_task = self.run_async(self.update_graph_data())

    def run_async(self, coro):
        """Exécute une coroutine et garde une référence à la tâche"""
        task = asyncio.create_task(coro)
        self.tasks.append(task)
        task.add_done_callback(lambda t: self.tasks.remove(t))
        return task

    async def cleanup(self):
        """Nettoyage avant fermeture"""
        self.running = False
        
        # Annuler toutes les tâches en cours
        for task in self.tasks:
            task.cancel()
            try:
                await task
            except (asyncio.CancelledError, Exception):
                pass
        
        # Fermer la session HTTP
        await self.controller.close()

    def on_closing(self):
        """Gère la fermeture de l'application"""
        self.run_async(self.cleanup())
        plt.close(self.fig)
        self.window.destroy()

    # ... (autres méthodes restantes comme avant mais utilisant run_async)

async def main():
    try:
        controller = ESP8266Controller("192.168.51.190")
        app = AxiosAPP(controller)
        
        # Boucle principale Tkinter adaptée pour asyncio
        while app.running:
            app.window.update()
            await asyncio.sleep(0.01)
            
    except Exception as e:
        print(f"Critical error: {e}")
    finally:
        if 'app' in locals():
            await app.cleanup()

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("Programme proprement arrêté")