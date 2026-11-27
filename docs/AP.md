# Grupo AP — Organização do código de controle

[← Voltar ao README](../README.md)

Melhorias no `YMFC-AL_Flight_controller.ino` que não alteram a lógica de controle, os ganhos nem o período de 4 ms.

## AP.1 — Modularização da reinicialização do PID

Branch `refactor/pid-reset` · Responsável: identificação pendente

Ao armar os motores, o `loop()` zerava diretamente as memórias integrais e os erros anteriores dos três eixos, para que valores acumulados antes da partida não chegassem aos motores. Transferimos essas seis atribuições, sem alteração e na mesma ordem, para a função `reset_pid()`, chamada no mesmo ponto e sob a mesma condição.

É apenas uma refatoração: a lógica original do PID foi preservada, e o tamanho do programa não mudou. Como no original, não há reinicialização ao desarmar.

## AP.2 — Reutilização do processamento dos comandos

Branch `refactor/receiver-command` · Responsável: identificação pendente

O setpoint de roll, pitch e yaw vem do pulso do receptor, com zona morta entre 1492 e 1508 µs, e o mesmo par de condicionais se repetia nos três eixos. Concentramos esse cálculo em `apply_dead_band(int pulse)`, que devolve `pulse - 1508`, `pulse - 1492` ou 0.

Os limites passaram a ficar em um único lugar, e as particularidades de cada eixo (nivelamento, divisão por 3 e condição do throttle no yaw) foram mantidas. Conferimos à mão que pulsos de 1000, 1492, 1508 e 1509 µs produzem os mesmos setpoints de antes. A redução de 144 bytes de Flash não foi analisada e não é apresentada como ganho.

## AP.3 — Proteção de variáveis compartilhadas

Branch `fix/atomic-receiver-read` · Responsável: identificação pendente

A rotina de interrupção grava a largura dos pulsos em `receiver_input[]`, e o programa principal lê esses valores em `convert_receiver_channel()`. Como o ATmega328P lê um `int` em duas instruções, uma interrupção entre elas pode misturar bytes de valores diferentes: uma transição de 1276 para 1280 µs poderia ser lida como 1532.

Declaramos o array como `volatile` e passamos a copiar cada canal dentro de uma seção crítica:

```cpp
ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
  actual = receiver_input[channel];
}
```

Isso garante a atomicidade da leitura de cada canal, mas não que os quatro sejam do mesmo instante. As interrupções ficam desabilitadas por cerca de 0,75 µs por leitura, valor estimado e não medido. O esc_calibrate mantém a leitura sem proteção.
