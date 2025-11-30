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

## 🧩 Porte do TensorFlow Lite Micro para o SoC LiteX / VexRiscv

O firmware desta tarefa foi desenvolvido em ambiente **bare-metal C**, rodando sobre o
SoC LiteX com CPU **VexRiscv**.  
Para isso, foi realizado o **porte do TensorFlow Lite Micro (TFLM)** para a nova
plataforma, incluindo:

- Geração de uma árvore mínima do TFLM;
- Ajustes para compilação cruzada com GCC RISC-V;
- Criação de Makefiles dedicados;
- Redução do conjunto de operadores;
- Configuração da `tensor_arena` e do runtime TFLM.

---

### 📌 1. Clonando o TensorFlow Lite Micro

Inicialmente foi feito o clone recursivo do repositório oficial:

```bash
git clone --recursive https://github.com/tensorflow/tflite-micro.git
```

### 📌 2. Geração do projeto mínimo usando o criador de árvores TFLM

O TensorFlow Lite Micro possui uma ferramenta oficial de geração de projetos para novas
plataformas.
Ela organiza os diretórios, copia somente os arquivos necessários e prepara uma base
limpa para portar o runtime.

O comando utilizado foi:

```bash
python3 tensorflow/lite/micro/tools/project_generation/create_tflm_tree.py \
    -e hello_world \
    ~/Documentos/tflm/firmware/tflm/
```

Onde:

* `e hello_world`
<br>→ Indica que o exemplo hello_world será incluído na árvore do projeto.

* `~/Documentos/tflm/firmware/tflm/`
<br>→ Diretório destino contendo a cópia mínima da biblioteca TFLM
    (este diretório está presente no repositório deste projeto).

O resultado é uma pasta contendo:

```bash
tflm/
 ├── tensorflow/
 ├── third_party/
 ├── signal/
 ├── make/
 ├── examples/
 └── Makefile (base, modificado para RISC-V)
```
Essa estrutura serve como “SDK" do TFLM para nossa plataforma.

### 📌 3. Ajustes para compilação no ambiente LiteX / VexRiscv

Após gerar a árvore TFLM, foram feitos os seguintes ajustes:

* Compilação cruzada com o toolchain
riscv-none-elf-gcc (xPack GCC 15);

* Criação de um Makefile próprio dentro de firmware/tflm/
para gerar a biblioteca libtflm.a;

* Remoção de operadores desnecessários, mantendo apenas o requerido pelo modelo
(sine) via MicroMutableOpResolver;

* Definição de uma tensor_arena de 4000 bytes, suficiente para o modelo
hello_world quantizado.

A compilação da biblioteca é feita com:

```bash
cd firmware/tflm
make -j12
```

Gerando:

```text
libtflm.a
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

### 4. Compilar o Firmware da Aplicação

Retorne ao diretório do firmware e compile o binário final (main.bin), que une seu código C++ com a biblioteca gerada no passo anterior.

```Bash
cd ../
make clean && make
```

### 5. Executar na FPGA

Carregue o firmware compilado para a memória RAM do SoC através da interface serial.

```Bash
# Substitua /dev/ttyACMxx pela porta serial correta (ex: /dev/ttyACM0)
litex_term --kernel build/main.bin /dev/ttyACMxx
```

### 6. Inicialização

Assim que o comando acima estiver aguardando (exibindo "Booting..."), force a reinicialização do processador para iniciar a execução do firmware:

```bash
reboot
```

## 📊 Resultados
<div align="center">
  <a href="https://youtu.be/_qyGwfRWc7w">
    <img src="https://img.youtube.com/vi/_qyGwfRWc7w/0.jpg" alt="Demonstração em Vídeo" style="max-width: 100%; height: auto;">
  </a>
</div>

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