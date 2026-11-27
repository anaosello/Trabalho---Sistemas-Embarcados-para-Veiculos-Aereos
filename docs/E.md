# Grupo E — Receptor e monitoramento da bateria

[← Voltar ao README](../README.md)

Responsável: Esther Vieira Parise Bianchini (Nº USP 15636362). Melhorias na validação do receptor e no alerta de bateria. Os seis commits da Esther (`aae68f6`, `69149ab` e `d703e22`, de 25/11/2026; `1461008`, `ed81b51` e `70ebdb6`, de 26/11/2026) mantêm a autoria e as datas originais.

## E.1 — Validação dos canais do receptor

Branch `fix/receiver-validation`

Em `wait_for_receiver()`, no setup, bastava um canal com pulso entre 900 e 2100 µs para o receptor ser aceito, mesmo com os pulsos parados. Agora os quatro canais precisam ter recebido pulso, ter largura nessa faixa e ter sido atualizados há menos de 100 ms, em cinco verificações seguidas. A rotina de interrupção registra o instante de cada pulso, os dados são copiados em uma seção crítica e a mensagem de erro indica a situação de cada canal.

## E.2 — Monitoramento da bateria

Branch `feat/battery-monitoring`

O alerta original acendia o LED do pino 12 abaixo de 10,0 V e só apagava com o reset. O limite passou a 10,5 V, com histerese: o alerta só desliga a partir de 10,8 V, o que evita oscilações perto do limite. A tensão do alerta passou a ser filtrada em `float`, preservando a parte fracionária que o filtro inteiro descartava; o filtro original continua na compensação dos motores.

O alerta de bateria e a falha de temporização passaram a ter estados separados (`low_battery` e `timing_fault`), indicados no mesmo LED.

## E.3 — Diagnóstico de perda do sinal do receptor

Branch `feat/receiver-failsafe`

Foi desenvolvido um sketch independente (`tests/receiver_diagnostics/receiver_diagnostics.ino`) que utiliza os critérios da E.1 para classificar os canais do receptor como válidos, ausentes, desatualizados ou com pulsos inválidos, além de identificar o restabelecimento do sinal.

### Testes e resultados

Os testes foram realizados no Wokwi, utilizando um Arduino Uno virtual e 12 cenários com dados simulados. Os resultados apresentados no monitor serial foram comparados com os valores esperados.

| Cenário | Máscara esperada | Máscara obtida |
|---|---|---|
| Quatro canais válidos | `1111` | `1111` |
| Um canal ausente | `1101` | `1101` |
| Dois canais ausentes | `0011` | `0011` |
| Pulso abaixo do limite | `1110` | `1110` |
| Restabelecimento de um canal | `1111` | `1111` |
| Pulso acima do limite | `0111` | `0111` |
| Dados desatualizados | `0000` | `0000` |
| Limites de largura dos pulsos | `0110` | `0110` |
| Limite do timeout | `1001` | `1001` |
| Overflow do contador temporal | `1101` | `1101` |
| Falhas simultâneas | `0000` | `0000` |
| Restabelecimento dos quatro canais | `1111` | `1111` |

Todos os 12 cenários apresentaram os resultados esperados, sem divergências nas 48 classificações individuais. A simulação confirmou o funcionamento da lógica de diagnóstico, incluindo a identificação de falhas, a prioridade entre elas e o restabelecimento dos canais.
