# Grupo AO — Instrumentação e sensores

[← Voltar ao README](../README.md)

Responsável: Ana Carolina Saggioratto Osello (Nº USP 13683104). Melhorias no tratamento dos dados do giroscópio e do acelerômetro (MPU-6050, via I²C).

## AO.1 — Recalibração do giroscópio

Branch `fix/gyro-calibration`

No `YMFC-AL_esc_calibrate.ino`, o comando `a` calcula o offset do giroscópio pela média de 2000 amostras. O vetor `gyro_axis_cal[]` guarda a soma e depois o offset; como não era zerado, uma nova calibração começava somando o offset anterior, o que acrescentava ao resultado o offset anterior dividido por 2000 a cada repetição.

Passamos a zerar os acumuladores antes do laço, além dos ângulos e de `first_angle`. Uma simulação da média mostrou 100 no lugar de 100,05. Uma tentativa anterior da Esther (`7a456cc`) foi revertida (`fa158b3`) e não faz parte da versão final.

## AO.2 — Tratamento de erros I²C

Branch `feat/i2c-error-handling`

No `YMFC-AL_setup.ino`, os retornos de `Wire.endTransmission()` e `Wire.requestFrom()` eram ignorados, e esperas como `while(Wire.available() < 6);` podiam travar a configuração sem nenhuma mensagem.

Ativamos o timeout da biblioteca Wire (25 ms), passamos a conferir cada retorno e o número de bytes recebidos e removemos essas esperas. Em caso de falha, a função `gyro_i2c_error()` mostra "ERROR 9" e interrompe as etapas seguintes, inclusive a gravação da EEPROM. O tratamento foi aplicado ao setup, onde o giroscópio é identificado e configurado.

## AO.3 — Validação dos cálculos do acelerômetro

Branch `fix/accelerometer-validation`

No esc_calibrate, os ângulos do acelerômetro são calculados com `asin(acc / total)`. Com os três eixos em zero, a divisão gerava NaN; com o sensor saturado, a soma dos quadrados estourava o `long`. Um NaN, depois de entrar na estimativa, permanecia até um novo comando.

Calculamos a soma em `unsigned long` e validamos cada eixo com a condição já usada no controlador de voo, `abs(acc) < total`. O ângulo só é atualizado com o eixo válido, e a inicialização exige os dois eixos válidos. Casos calculados, como (0, 0, 0) e (32768, 32768, 32768), deixaram de produzir NaN; leituras muito próximas de 90° também passam a ser descartadas.
