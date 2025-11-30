# Tarefa 06: TensorFlow Lite Micro no LiteX SoC

Este repositório contém a implementação e os códigos fonte referentes à Tarefa 06, cujo objetivo é executar o **TensorFlow Lite for Microcontrollers (TFLM)** em um System-on-Chip (SoC) gerado através do framework **LiteX**.

O projeto executa o modelo *Hello World* (Senoide), onde a inferência da rede neural controla uma barra de LEDs na FPGA, variando a intensidade/quantidade conforme a onda senoidal predita.

## 🎯 Objetivos
* Gerar um SoC baseado em arquitetura RISC-V (VexRiscv) utilizando o LiteX.
* Compilar a biblioteca TensorFlow Lite Micro do zero utilizando toolchain atualizada (GCC 15).
* Integrar e executar o modelo de Machine Learning *Hello World* (Sine Wave).
* Validar a inferência visualmente (LEDs) e via terminal serial (UART).

## 🛠️ Hardware e Ferramentas Utilizadas

### Hardware
* **Placa FPGA:** Colorlight i5 (Lattice ECP5)
* **Microcontrolador (Soft-core):** VexRiscv (RISC-V 32-bit `rv32im`)
* **Interface de Comunicação:** UART (Serial via USB)
* **Periféricos:** LEDs onboard (controlados via CSR)


## 🧷 Mapa de Pinos – LEDs (GPIO 8 bits)

Cada bit no registrador `leds_8bit_out_write` acende exatamente um LED.
A tabela abaixo mostra o valor binário correspondente a cada LED.

| LED (barra) | Bit em `leds_8bit_out_write` | Valor binário do bit | Pino da FPGA (ECP5) | Observação |
|-------------|------------------------------|-----------------------|----------------------|------------|
| LED0        | bit 0 (LSB)                  | 0b00000001            | **P17**              | LED mais à direita |
| LED1        | bit 1                        | 0b00000010            | **P18**              |            |
| LED2        | bit 2                        | 0b00000100            | **N18**              |            |
| LED3        | bit 3                        | 0b00001000            | **L20**              |            |
| LED4        | bit 4                        | 0b00010000            | **L18**              |            |
| LED5        | bit 5                        | 0b00100000            | **G20**              |            |
| LED6        | bit 6                        | 0b01000000            | **M18**              |            |
| LED7        | bit 7 (MSB)                  | 0b10000000            | **N17**              | LED mais à esquerda |

---

### Software
* **Sistema Operacional:** Linux (Ubuntu)
* **Framework SoC:** [LiteX](https://github.com/enjoy-digital/litex)
* **Toolchain:** [xPack GNU RISC-V Embedded GCC 15.2.0](https://xpack.github.io/dev-tools/riscv-none-elf-gcc/) (Essencial para suporte a C++17 atualizado)
* **Biblioteca ML:** [TensorFlow Lite for Microcontrollers](https://www.tensorflow.org/lite/microcontrollers)

## 📂 Estrutura do Projeto

```text
.
├── tools/                # Toolchain utilizado na copilação
├── firmware/
│   ├── main.cc           # Lógica principal (Carrega modelo, Invoke, Controle LEDs)
│   ├── Makefile          # Makefile unificado (Compila TFLM + Firmware)
│   ├── linker.ld         # Script de linkagem para VexRiscv
│   ├── tflm/             # Código fonte do TensorFlow Lite Micro (Submódulo/Cópia)
│   └── models/           # Dados do modelo treinado (Arrays C)
├── build/                # Artefatos gerados pelo LiteX (CSRs, bibliotecas base)
└── README.md             # Documentação do projeto
```


## 🚀 Como Reproduzir
### 1. Clonar o Repositório

``` Bash

git clone [https://github.com/Alan-Huss/tarefa06-tflm-litex-soc.git](https://github.com/Alan-Huss/tarefa06-tflm-litex-soc.git)
cd tarefa06-tflm-litex-soc
```


### 2. Configurar o Toolchain (Dentro do OSS CAD Suite)

Antes de começar, certifique-se de exportar o caminho do compilador RISC-V atualizado (GCC 15+). Ajuste o `caminho_do_projeto` conforme a localização no seu computador.

```bash
# Adiciona o xPack GCC 15 ao PATH do sistema
export PATH=/caminho_do_projeto/tools/xpack-riscv-none-elf-gcc-15.2.0-1/bin:$PATH
### 2. Gerar o SoC (Bitstream)
```

### 3. Gerar o SoC e Carregar na FPGA

Gere o bitstream do SoC VexRiscv e carregue-o na placa.

```Bash
# Gera o hardware e carrega via openFPGALoader
python3 litex/colorlight_i5.py --board i9 --revision 7.2 --cpu-type=vexriscv --build --load --ecppack-compress
```

### 4. Compilar a Biblioteca TensorFlow Lite Micro

Navegue até o diretório do TFLM e compile a biblioteca estática (`libtflm.a`).
```Bash
cd firmware/tflm

# Limpa builds antigos e compila utilizando múltiplos núcleos (-j12) para rapidez
make clean && make -j12
```

### 5. Compilar o Firmware da Aplicação

Retorne ao diretório do firmware e compile o binário final (main.bin), que une seu código C++ com a biblioteca gerada no passo anterior.

```Bash
cd ../
make clean && make
```

### 6. Executar na FPGA

Carregue o firmware compilado para a memória RAM do SoC através da interface serial.

```Bash
# Substitua /dev/ttyACMxx pela porta serial correta (ex: /dev/ttyACM0)
litex_term --kernel build/main.bin /dev/ttyACMxx
```

### 7. Inicialização

Assim que o comando acima estiver aguardando (exibindo "Booting..."), force a reinicialização do processador para iniciar a execução do firmware:

```bash
reboot
```

## 📊 Resultados

Após carregar o firmware e reiniciar o processador (reboot), o sistema entrará no loop de inferência contínua. O funcionamento pode ser validado de duas formas:

#### 1. Feedback Visual (LEDs)

A barra de LEDs da placa FPGA atuará como um display gráfico, oscilando suavemente conforme a onda senoidal gerada pelo modelo de Machine Learning.

* Intensidade Baixa: Poucos ou nenhum LED aceso (valores próximos de -1.0 ou 0).

* Intensidade Alta: Todos os LEDs acesos (valores próximos de 1.0).

#### 2. Monitoramento Serial (UART)

O terminal serial exibirá os dados da inferência em tempo real. Devido ao driver xprintf customizado incluído no main.cc, a saída apresenta o valor de ponto flutuante calculado e a representação binária direta do registro de LEDs.

#### Exemplo de saída no console:

```Plaintext
Iniciando IA...
Y: 0.00 | LEDs: 0b00001000
Y: 0.18 | LEDs: 0b00011000
Y: 0.36 | LEDs: 0b00011100
Y: 0.53 | LEDs: 0b00111100
...
Y: 0.99 | LEDs: 0b11111111
...
Y: -0.98 | LEDs: 0b00000000
```
## 🧠 Detalhes de Implementação

Para viabilizar a execução do TensorFlow Lite em um soft-core RISC-V com recursos limitados, foram aplicadas otimizações específicas no código fonte:

* **Arena de Tensores Otimizada**: A kTensorArenaSize foi ajustada para 4000 bytes, tamanho suficiente para alocar os tensores do modelo Hello World sem exceder a RAM do SoC.

*  **Resolução de Operadores (OpResolver)**: Utilizou-se a classe MicroMutableOpResolver adicionando apenas a operação AddFullyConnected. Isso evita o carregamento de todos os operadores da biblioteca, reduzindo drasticamente o tamanho do binário final.

*  **Printf Leve (xprintf)**: A biblioteca padrão stdio foi substituída por uma implementação minimalista que escreve diretamente no buffer da UART, economizando espaço de armazenamento e ciclos de clock.

## 🔗 Referências

* **[Colorlight i9 Examples](https://github.com/dvcirilo/colorlight-i9-examples):** Exemplos de projetos para FPGA Colorlight i9, utilizado como base para configuração do hardware.
* **[TensorFlow Lite Micro](https://github.com/tensorflow/tflite-micro):** Repositório oficial da biblioteca TensorFlow Lite otimizada para microcontroladores.
* **[Pico TFLMicro](https://github.com/raspberrypi/pico-tflmicro):** Implementação do TFLM para Raspberry Pi Pico, utilizada como referência de estrutura de código.