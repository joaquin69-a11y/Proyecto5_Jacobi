import matplotlib.pyplot as plt
import numpy as np

# Cargar datos ignorando la cabecera de texto
data = np.loadtxt("final_temp.txt", skiprows=1)

# Configurar el gráfico
plt.figure(figsize=(10, 8))
plt.imshow(data, cmap='inferno', interpolation='nearest')
plt.colorbar(label='Temperatura (°C)')
plt.title('Distribución de Temperatura 2D')
plt.xlabel('Posición X')
plt.ylabel('Posición Y')

# Guardar imagen
plt.savefig("resultado_final.png")

