#include <stdint.h>
#include <math.h>
#include <generated/csr.h> // Acesso ao Hardware

#undef min
#undef max

#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "models/hello_world_float_model_data.h"

// ==========================================
// --- DRIVER DE UART E PRINTF CUSTOMIZADO ---
// ==========================================

void uart_write_char(char c) {
    if (c == '\n') uart_write_char('\r'); 
    while (uart_txfull_read());           
    uart_rxtx_write(c);                   
}

void uart_print_str(const char* s) {
    while (*s) uart_write_char(*s++);
}

void uart_print_int(int n) {
    char buf[16];
    int i = 0;
    int sign = n < 0;
    if (sign) n = -n;
    if (n == 0) { uart_write_char('0'); return; }
    while (n > 0) { buf[i++] = (n % 10) + '0'; n /= 10; }
    if (sign) uart_write_char('-');
    while (i > 0) uart_write_char(buf[--i]);
}

// Função auxiliar para imprimir binário (8 bits)
void uart_print_binary(int n) {
    uart_print_str("0b"); // Prefixo opcional para indicar binário
    for (int i = 7; i >= 0; i--) {
        // Verifica se o bit na posição 'i' é 1
        if ((n >> i) & 1) {
            uart_write_char('1');
        } else {
            uart_write_char('0');
        }
    }
}

// xprintf tunado: suporta %d, %s, %c e %b (binário)
void xprintf(const char* format, ...) {
    __builtin_va_list args;
    __builtin_va_start(args, format);
    
    while (*format) {
        if (*format == '%') {
            format++;
            if (*format == 'd') {
                int val = __builtin_va_arg(args, int);
                uart_print_int(val);
            } else if (*format == 's') {
                char* val = __builtin_va_arg(args, char*);
                uart_print_str(val);
            } else if (*format == 'c') {
                int val = __builtin_va_arg(args, int);
                uart_write_char((char)val);
            } else if (*format == 'b') { 
                int val = __builtin_va_arg(args, int);
                uart_print_binary(val);
            }
        } else {
            uart_write_char(*format);
        }
        format++;
    }
    __builtin_va_end(args);
}
// ==========================================


// Memoria TFLM
const int kTensorArenaSize = 4000;
uint8_t tensor_arena[kTensorArenaSize];

void delay_loop(int count) {
    volatile int i = count * 5000; 
    while (i--) __asm__("nop");
}

uint8_t sine_to_led_bar(float y_val) {
    float normalized = (y_val + 1.0f) / 2.0f; 
    int num_leds = (int)(normalized * 8.0f + 0.5f);
    if (num_leds < 0) num_leds = 0;
    if (num_leds > 8) num_leds = 8;
    if (num_leds == 0) return 0;
    return (1 << num_leds) - 1;
}

int main(void) {
    xprintf("Iniciando IA...\n");

    const tflite::Model* model = tflite::GetModel(g_hello_world_float_model_data);
    static tflite::MicroMutableOpResolver<1> resolver;
    resolver.AddFullyConnected();
    
    static tflite::MicroInterpreter interpreter(model, resolver, tensor_arena, kTensorArenaSize, nullptr);
    interpreter.AllocateTensors();
    
    TfLiteTensor* input = interpreter.input(0);
    TfLiteTensor* output = interpreter.output(0);

    int i = 0;
    while (1) {
        float x_val = (i * 3.14159f * 2.0f) / 100.0f; 
        input->data.f[0] = x_val;

        interpreter.Invoke();
        float y_val = output->data.f[0];

        // Calcula LEDs
        uint8_t leds = sine_to_led_bar(y_val);
        leds_8bit_out_write(leds); // Atualiza hardware

        // --- DEBUG COMPLETO ---
        int d1 = (int)y_val;
        int d2 = (int)((fabs(y_val) - abs(d1)) * 100);
        
        // Imprime Valor Float e LEDs em Binário na mesma linha
        xprintf("Y: %d.", d1);
        if (abs(d2) < 10) xprintf("0");
        xprintf("%d | LEDs: %b\n", abs(d2), leds);

        i++;
        if (i >= 100) i = 0;
        delay_loop(15);
    }
    return 0;
}