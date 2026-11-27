# Grupo E — Receptor e monitoramento da bateria

[← Voltar ao README](../README.md)

Melhorias na validação do receptor e no alerta de bateria. Os seis commits da Esther (`aae68f6`, `69149ab` e `d703e22`, de 25/11/2026; `1461008`, `ed81b51` e `70ebdb6`, de 26/11/2026) mantêm a autoria e as datas originais.

## E.1 — Validação dos canais do receptor

Branch `fix/receiver-validation` · Responsável: Esther Vieira Parise Bianchini

Em `wait_for_receiver()`, no setup, bastava um canal com pulso entre 900 e 2100 µs para o receptor ser aceito, mesmo com os pulsos parados. Agora os quatro canais precisam ter recebido pulso, ter largura nessa faixa e ter sido atualizados há menos de 100 ms, em cinco verificações seguidas. A rotina de interrupção registra o instante de cada pulso, os dados são copiados em uma seção crítica e a mensagem de erro indica a situação de cada canal.

A validação vale só para a configuração, e não há failsafe. Um receptor que continue enviando pulsos válidos após perder o transmissor não é detectado. Foi verificada por compilação e inspeção do código.

## E.2 — Monitoramento da bateria

Branch `feat/battery-monitoring` · Responsável: Esther Vieira Parise Bianchini

O alerta original acendia o LED do pino 12 abaixo de 10,0 V e só apagava com o reset. O limite passou a 10,5 V, com histerese: o alerta só desliga acima de 10,8 V, o que evita oscilações perto do limite. A tensão do alerta passou a ser filtrada em `float`, preservando a parte fracionária que o filtro inteiro descartava; o filtro original continua na compensação dos motores.

O LED continua compartilhado com a falha de temporização, sem distinguir a causa. Como a condição `> 600` foi removida, alimentar a placa só pela USB mantém o alerta aceso. Limites e calibração do ADC não foram confirmados em bancada.

## E.3 — Diagnóstico de perda do sinal do receptor

Branch `feat/receiver-failsafe` · Responsável: identificação pendente

`tests/receiver_diagnostics/receiver_diagnostics.ino` é um sketch novo, independente do controlador de voo, e não recupera código anterior. Com os critérios da E.1, ele classifica cada canal como válido, ausente, desatualizado ou com pulso inválido e indica quando o canal é restabelecido. Os 12 cenários, com amostras simuladas em uma `struct`, incluem limites de largura e de tempo e a volta a zero de `micros()`.

O sketch não lê o receptor real, não aciona motores e não é um failsafe. Foi compilado, mas ainda não executado em placa.
