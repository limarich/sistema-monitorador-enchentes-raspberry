# Sistema Monitorador de Enchentes - BitDogLab

## **📋 Descrição do Projeto**

O **Sistema Monitorador de Enchentes BitDogLab** é um projeto que utiliza a placa **BitdogLab** para simular sensores de nível de água e chuva com o auxílio de um joystick, aproveitando as capacidades de multitarefas e filas do **FreeRTOS**. O sistema integra diversos periféricos para fornecer feedback visual e sonoro, incluindo uma matriz de LEDs, LED RGB, display OLED SSD1306 e buzzers.

---

## **🎯 Objetivo Geral**

Desenvolver um sistema inteligente para monitorar níveis de água e chuva, consolidando os conhecimentos adquiridos sobre sistemas embarcados e gerenciamento de filas no FreeRTOS.

---

## **⚙️ Funcionalidades**

O sistema opera em dois modos principais:

### **🔄 Modo Normal**  
- Exibe continuamente os valores dos níveis de água e chuva.  
- LED RGB pisca lentamente em verde, indicando condições normais.  
- A matriz de LEDs mostra gráficos dos níveis em cinco etapas, com:  
  - **Azul** para o nível de água.  
  - **Amarelo** para o volume de chuva.  

### **🚨 Modo Alerta**  
Ativado automaticamente quando:  
- **Nível de água** ≥ 70%  
- **Volume de chuva** ≥ 80%  

Neste modo:  
- A matriz de LEDs muda o fundo para **vermelho**.  
- O LED RGB pisca para indicar a situação:  
  - **Azul** - Água acima do limite.  
  - **Amarelo** - Chuva acima do limite.  
  - **Vermelho** - Ambos os parâmetros acima do limite.  
- O buzzer emite alertas sonoros específicos para cada situação.  

---

## **📂 Tarefas (Tasks)**

O sistema é organizado em cinco tarefas paralelas, que se comunicam via filas:

- **vTaskLed**  
  Controla a cor e a frequência de piscagem do LED RGB com base nos dados dos sensores.  

- **vBuzzerTask**  
  Verifica os dados dos sensores para ativar os alertas sonoros adequados.  

- **vDisplayTask**  
  Lê os dados dos sensores e exibe as informações no display OLED, incluindo alertas visuais.  

- **vMatrixLedsTask**  
  Exibe gráficos na matriz de LEDs, mudando a cor de fundo para indicar alertas.  

- **vJoystick**  
  Lê os eixos X e Y do joystick:  
  - **X** - Sensor de chuva  
  - **Y** - Sensor de nível de água  
  Envia esses dados para as filas que alimentam as outras tasks.  

---

## **🔌 Periféricos Utilizados**

- **Joystick**  
  Simula a leitura de sensores de nível de água (Y) e chuva (X).  

- **Matriz de LEDs**  
  Fornece feedback visual, representando os níveis lidos em gráficos.  

- **LED RGB**  
  Indica o status do sistema com diferentes padrões de pisca.  

- **Buzzers**  
  Emite alertas sonoros para condições fora do padrão.  

- **Display SSD1306**  
  Mostra as leituras dos sensores e mensagens de alerta.  

---

## **📂 Vídeo de Demonstração**

- **Vídeo de Demonstração:** [YouTube](https://youtu.be/itfCwftKND4?si=MNK8MZUAp4a0Pmme)  

---
