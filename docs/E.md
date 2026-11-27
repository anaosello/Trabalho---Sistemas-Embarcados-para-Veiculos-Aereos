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

`tests/receiver_diagnostics/receiver_diagnostics.ino` é um sketch novo, independente do controlador de voo, e não recupera código anterior. Com os critérios da E.1, ele classifica cada canal como válido, ausente, desatualizado ou com pulso inválido e indica quando o canal é restabelecido. Os 12 cenários, com amostras simuladas em uma `struct`, incluem limites de largura e de tempo e a volta a zero de `micros()`.

O sketch trabalha só com dados simulados e mostra o resultado no monitor serial, sem acionar motores.
