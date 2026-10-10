import serial
import time
import psutil
from datetime import datetime 
from py3nvml import py3nvml


# Configuração da porta serial
porta_serial = 'COM11'
velocidade = 115200


try:
    # Inicializa o NVML para acessar informações da GPU NVIDIA
    py3nvml.nvmlInit()

    # Pega a primeira GPU NVIDIA encontrada
    gpu = py3nvml.nvmlDeviceGetHandleByIndex(0)

    # Abre a conexão com o ESP32
    conexao = serial.Serial(
        porta_serial,
        velocidade,
        timeout=1
    )

    print(
        f"Conectado com sucesso na porta {porta_serial}! "
        "Enviando dados..."
    )

    time.sleep(2)

    while True:

        # CPU
        uso_cpu = int(psutil.cpu_percent(interval=None))

        # RAM
        uso_RAM = int(psutil.virtual_memory().percent)

        # GPU
        informacoes_gpu = py3nvml.nvmlDeviceGetUtilizationRates(gpu)
        uso_gpu = int(informacoes_gpu.gpu)

        # tempo
        relogio = datetime.now().strftime("%H:%M")

        # Formato:
        # CPU,RAM,GPU
        dados = f"{uso_cpu},{uso_RAM},{uso_gpu},{relogio}\n"

        # Mostra no terminal o que está sendo enviado
        print("Enviando:", dados.strip())

        # Envia para o ESP32
        conexao.write(dados.encode('utf-8'))

        time.sleep(1)


except KeyboardInterrupt:

    print("\nMonitoramento encerrado pelo usuário.")


except Exception as e:

    print(f"\nOcorreu um erro: {e}")


finally:

    if 'conexao' in locals() and conexao.is_open:
        conexao.close()
        print("Conexão serial fechada de forma segura.")

    try:
        py3nvml.nvmlShutdown()
    except:
        pass