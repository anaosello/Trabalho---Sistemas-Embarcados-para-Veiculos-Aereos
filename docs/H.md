# Grupo H — Arquitetura embarcada

[← Voltar ao README](../README.md)

Responsável: Hugo Ayello Paura (Nº USP 15575883). Melhorias no uso da EEPROM e na medição de tempo.

## H.1 — Redução de escritas desnecessárias na EEPROM

Branch `fix/eeprom-wear`

Ao final do setup, os 36 bytes de configuração eram gravados com `EEPROM.write()`, que regrava o byte mesmo quando o valor não mudou. Substituímos as 36 chamadas por `EEPROM.update()`, que lê o byte e só grava se ele for diferente, mantendo endereços, ordem e formato.

Como a EEPROM suporta um número limitado de ciclos de escrita, a mudança reduz o desgaste quando o setup é repetido.

## H.2 — Correção das comparações temporais

Branch `fix/loop-timing`

Em `search_gyro()`, `check_receiver_inputs()` e `check_gyro_axes()`, o setup calculava um instante futuro (`millis() + X`) e esperava enquanto ele fosse maior que `millis()`. Perto da volta a zero do contador, a soma também volta, e a espera termina na hora.

Passamos a guardar o instante inicial e a comparar o tempo decorrido, `millis() - timer < X`, que continua correto com a aritmética sem sinal de 32 bits. Começando em 2^32 − 5000 ms, por exemplo, o original sairia na hora com ERROR 2, e a nova versão espera os 30 s.

## H.3 — Validação das configurações armazenadas na EEPROM

Branch `feat/eeprom-validation`

O controlador de voo só conferia a assinatura "JMB", que não garante a coerência dos outros bytes: uma gravação interrompida, por exemplo, mantém a assinatura antiga. Esses parâmetros são usados como índice de array e divisor em `convert_receiver_channel()`, o que permitia leituras fora do array, divisão por zero e estouro do `int`.

Criamos `receiver_settings_valid()`, que exige identificações de canal entre 1 e 4, sem repetição, e `0 < mínimo < centro < máximo` em cada canal. Se algo falhar, o `setup()` para antes de armar os ESCs, com o LED aceso. O formato da EEPROM não mudou, e a validação não usa CRC: ela confere a coerência dos parâmetros do receptor.
