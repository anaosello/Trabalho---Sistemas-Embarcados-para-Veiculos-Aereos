# Trabalho---Sistemas-Embarcados-para-Veiculos-Aereos
Este repositório contém os códigos base para o trabalho da disciplina de Sistemas Embarcados para veículos aéreos.

Para cloná-lo e fazer seu trabalho, crie um fork dele e faça as alterações que julgar necessárias para demonstrar seu entendimento do assunto.

Serão avaliados as suas mudanças e seu comentários sobre elas.

O seu trabalho deve ser compilado e relatado no README.md de seu repositório.

Qualquer dúvida estamos à disposição! 
e-mail: arthurdsd@usp.br

Bom trabalho!

--------------------------------------

## Melhoria 1 — Validação dos canais do receptor

**Responsável:** Esther Vieira Parise Bianchini - 15636362

**Branch:** `fix/receiver-validation`

**Arquivo modificado:** `YMFC-AL_setup.ino`

### 1. Problema identificado

O código original apresenta limitações na etapa de
validação dos quatro canais PWM do receptor.

A função `wait_for_receiver()` utiliza uma máscara
de bits para identificar os canais que possuem
pulsos com duração entre 900 e 2100 microssegundos.

Entretanto, a condição de erro original verifica
apenas se a máscara possui valor igual a zero.
Dessa forma, o sistema pode considerar a validação
bem-sucedida mesmo quando apenas parte dos canais
está funcionando.

Além disso, as variáveis responsáveis por armazenar
os sinais PWM podem manter seus últimos valores
mesmo após a interrupção do recebimento dos pulsos.

### 2. Solução proposta

A implementação foi dividida em duas etapas.

#### 2.1. Validação dos quatro canais

Na primeira etapa, foram realizadas as seguintes
alterações:

- Validação obrigatória dos quatro canais do receptor;
- Reinicialização da máscara de bits a cada verificação;
- Exigência de cinco verificações consecutivas válidas;
- Utilização de operações atômicas para leitura das
  variáveis compartilhadas com a interrupção;
- Identificação individual dos canais inválidos;
- Correção da comparação temporal para reduzir
  problemas relacionados ao overflow do temporizador.

A utilização da biblioteca `util/atomic.h` garante
que os valores compartilhados com a rotina de
interrupção sejam copiados sem que uma interrupção
ocorra durante a operação de leitura.

Essa alteração é relevante porque o ATmega328P
possui arquitetura de 8 bits, enquanto os valores
dos canais são armazenados em variáveis de 16 bits.

#### 2.2. Verificação temporal dos pulsos PWM

Na segunda etapa, a rotina de interrupção
`ISR(PCINT0_vect)` foi modificada para armazenar,
além da duração do pulso, o instante de recebimento
de cada sinal.

Foram acrescentadas as seguintes variáveis:

- `last_pulse_time`: armazena o instante do último
  pulso recebido em cada canal;
- `received_channels`: identifica os canais que
  já receberam pelo menos um pulso;
- `RECEIVER_TIMEOUT_US`: estabelece o tempo máximo
  de ausência de novos pulsos.

O limite temporal adotado inicialmente foi de
100 ms (100000 microssegundos).

Considerando um receptor operando a 50 Hz, esse
intervalo corresponde a cinco períodos nominais
do sinal PWM.

Assim, cada canal deve satisfazer simultaneamente
três condições:

1. Ter recebido pelo menos um pulso;
2. Apresentar duração superior a 900 e inferior
   a 2100 microssegundos;
3. Ter recebido seu último pulso há menos de 100 ms.

Também foi acrescentada uma proteção contra a
conversão de durações excessivamente grandes
para variáveis do tipo `int`.

### 3. Comparação entre as implementações

| Característica | Original | Modificado |
|---|---|---|
| Validação obrigatória dos quatro canais | Não | Sim |
| Leitura atômica | Não | Sim |
| Verificações consecutivas | Não | Sim |
| Verificação temporal dos pulsos | Não | Sim |
| Diagnóstico individual de falhas | Não | Sim |
| Tempo máximo de configuração do receptor | 10 s | 10 s |

### 4. Benefícios esperados

As alterações propostas têm como objetivo aumentar
a confiabilidade da etapa de configuração do drone,
evitando a aceitação de canais ausentes, inválidos
ou com informações desatualizadas no momento
da validação.

Também permitem identificar possíveis problemas
de conexão entre o receptor e o microcontrolador,
facilitando o diagnóstico durante a configuração.


### 5. Compilação e validação

#### 5.1. Ambiente de compilação

As duas etapas da implementação foram compiladas
utilizando o Arduino IDE, com a placa Arduino Uno
selecionada, baseada no microcontrolador ATmega328P.

As compilações foram realizadas com sucesso, sem
erros, permitindo analisar o consumo de memória
associado às modificações implementadas.

#### 5.2. Resultados das compilações

| Parâmetro | Commit 1 | Commit 2 |
|---|---:|---:|
| Memória Flash utilizada | 19990 bytes | 20572 bytes |
| Ocupação da memória Flash | 61% | 63% |
| Memória SRAM global utilizada | 515 bytes | 532 bytes |
| Ocupação da memória SRAM | 25% | 25% |
| Memória SRAM restante | 1533 bytes | 1516 bytes |
| Resultado da compilação | Sucesso | Sucesso |

O Arduino Uno possui 32256 bytes de memória Flash
disponíveis para o programa e 2048 bytes de memória
SRAM.

#### 5.3. Análise do custo computacional

A implementação da verificação temporal dos pulsos
PWM resultou em um aumento de 582 bytes na memória
Flash e de 17 bytes na memória SRAM global, em
relação à primeira etapa da implementação.

O aumento observado na memória SRAM é consistente
com a declaração das novas variáveis globais:

| Variável | Quantidade | Memória ocupada |
|---|---:|---:|
| `last_pulse_time` | 4 unsigned long | 16 bytes |
| `received_channels` | 1 byte | 1 byte |
| **Total** | | **17 bytes** |

Apesar do aumento no consumo de memória, o programa
permaneceu dentro dos limites de armazenamento do
microcontrolador.

A segunda implementação utiliza 63% da memória
Flash disponível e 25% da memória SRAM, conforme
os percentuais apresentados pelo Arduino IDE.

Dessa forma, a melhoria acrescenta mecanismos
de validação e diagnóstico do receptor com um
baixo custo adicional de memória global.

#### 5.4. Validação da implementação

A compilação bem-sucedida confirma a compatibilidade
do código com o ambiente de desenvolvimento e com
a arquitetura do microcontrolador selecionado.

Entretanto, a compilação não comprova isoladamente
o funcionamento do sistema em condições reais.

Para a validação experimental, propõem-se os
seguintes testes de bancada:

| Teste | Resultado esperado |
|---|---|
| Quatro canais válidos e ativos | Configuração autorizada |
| Apenas três canais ativos | Configuração rejeitada |
| Ausência de pulsos em todos os canais | Configuração rejeitada |
| Pulso com duração fora dos limites | Canal identificado como inválido |
| Interrupção do sinal durante a validação | Detecção de informação desatualizada |
| Restabelecimento dos sinais dentro do timeout | Validação após cinco verificações consecutivas |

Os testes experimentais em hardware permanecem
pendentes e deverão ser realizados em condições
controladas de bancada, sem as hélices instaladas.


### 6. Limitações

A melhoria implementada atua na validação inicial
do receptor durante a etapa de configuração.

Ela não realiza o monitoramento contínuo dos canais
durante o voo, que será abordado posteriormente
na implementação do failsafe.

Além disso, receptores que continuam gerando pulsos
PWM após a perda de comunicação com o transmissor
podem não ter essa falha identificada apenas pela
verificação temporal.

O limite de 100 ms deverá ser confirmado de acordo
com as características do receptor utilizado.

A validação experimental em hardware permanece
pendente.

### 7. Histórico das alterações

- `052b656`: correção da validação dos quatro canais;
- `94adff4`: verificação da atualização dos sinais PWM.


---

## Melhoria 2 — Monitoramento da bateria com histerese e aprimoramento da filtragem digital

**Responsável:** Esther Vieira Parise Bianchini - 15636362

**Branch:** `feat/battery-monitoring`

**Arquivo modificado:** `YMFC-AL_Flight_controller.ino`

### 1. Problema identificado

O monitoramento da tensão da bateria é importante
para a segurança operacional de veículos aéreos
não tripulados, pois permite identificar condições
de alimentação inadequadas durante a operação.

No código original, foram identificadas as seguintes
limitações:

- Ausência de histerese no indicador de bateria;
- Ausência de uma condição explícita para desativar
  o alerta após a recuperação da tensão;
- Compartilhamento do LED entre os indicadores de
  bateria baixa e falhas de temporização;
- Utilização de limites de tensão definidos
  diretamente nas condições do código;
- Perda de precisão numérica durante a filtragem
  da tensão, devido ao armazenamento do resultado
  em uma variável do tipo `int`.

Essas características podem comprometer a precisão
do monitoramento e dificultar a interpretação dos
alertas apresentados pelo sistema.

### 2. Solução proposta

A implementação foi dividida em duas etapas:

1. Monitoramento da tensão utilizando histerese;
2. Aprimoramento da precisão numérica do filtro
   digital utilizado no monitoramento da bateria.

#### 2.1. Monitoramento da bateria com histerese

Na primeira etapa, foram definidos dois limites
distintos para a ativação e desativação do alerta
de bateria baixa.

Considerando inicialmente uma bateria LiPo 3S,
foram adotados os seguintes parâmetros:

| Parâmetro | Valor |
|---|---:|
| Limite de ativação do alerta | 10,5 V |
| Limite de desativação do alerta | 10,8 V |
| Faixa de histerese | 0,3 V |

Os limites foram armazenados nas constantes
`BATTERY_LOW_THRESHOLD` e
`BATTERY_RECOVERY_THRESHOLD`.

Como o código representa a tensão em centésimos
de volt, os valores utilizados são 1050 e 1080,
respectivamente.

A histerese permite que pequenas oscilações de
tensão próximas ao limite não provoquem mudanças
sucessivas no estado do indicador.

O estado do alerta é atualizado pela função
`update_battery_warning()`.

O comportamento implementado é apresentado
na tabela a seguir:

| Condição | Estado do alerta |
|---|---|
| Tensão menor ou igual a 10,5 V | Ativado |
| Tensão entre 10,5 V e 10,8 V | Estado anterior mantido |
| Tensão maior ou igual a 10,8 V | Desativado |

Os limites adotados são parâmetros iniciais de
projeto e deverão ser confirmados experimentalmente
com a bateria utilizada.

#### 2.2. Separação dos indicadores de falha

O código original utiliza o LED conectado ao pino
digital 12 para indicar tanto condições de bateria
baixa quanto falhas de temporização.

Para evitar que a recuperação da tensão apague um
alerta de temporização, foram introduzidas duas
variáveis booleanas independentes:

- `low_battery`: representa o estado do alerta
  de bateria baixa;
- `timing_fault`: representa a ocorrência de
  uma falha de temporização.

O estado do LED é determinado pela operação
lógica OU entre as duas variáveis:

```cpp
digitalWrite(12, low_battery || timing_fault);
```

| Bateria baixa | Falha temporal | LED |
|---|---|---|
| Não | Não | Desligado |
| Sim | Não | Ligado |
| Não | Sim | Ligado |
| Sim | Sim | Ligado |

A falha de temporização permanece registrada até
a reinicialização do sistema, preservando o
comportamento original desse indicador.

#### 2.3. Aprimoramento do filtro digital

O código original utiliza um filtro passa-baixas
digital de primeira ordem para reduzir a influência
de ruídos sobre a medição da tensão da bateria.

A equação do filtro pode ser expressa por:

$$
V_f[k] = (1-\alpha)V_f[k-1] + \alpha V_m[k]
$$

Em que:

- $V_f[k]$ representa a tensão filtrada atual;
- $V_f[k-1]$ representa a tensão filtrada anterior;
- $V_m[k]$ representa a nova tensão medida;
- $\alpha$ representa o peso da nova medição.

O coeficiente utilizado no código original é
$\alpha = 0,08$.

Entretanto, o armazenamento da estimativa em uma
variável do tipo `int` provoca o descarte da parte
fracionária do resultado a cada iteração.

Por exemplo, considerando uma estimativa inicial
de 1200 e uma nova medição igual a 1210, temos:

$$
V_f[k] = 0,92(1200) + 0,08(1210) = 1200,8
$$

Ao armazenar esse resultado em uma variável inteira,
o valor obtido será 1200.

Caso a entrada permaneça constante, a mesma situação
poderá ocorrer nas iterações seguintes, impedindo
que o filtro acompanhe pequenas variações de tensão.

Para reduzir essa perda de precisão, foi criada
uma variável independente do tipo `float`:

```cpp
float battery_voltage_filtered = 0.0f;
```

A equação do filtro foi reorganizada como:

$$
V_f[k] = V_f[k-1] +
\alpha(V_m[k]-V_f[k-1])
$$

A implementação correspondente é:

```cpp
battery_voltage_filtered +=
    BATTERY_FILTER_ALPHA *
    (measured_voltage - battery_voltage_filtered);
```

A utilização de uma variável de ponto flutuante
permite preservar as pequenas correções realizadas
entre as iterações.

Essa alteração melhora a precisão numérica da
estimativa, sem aumentar a resolução física
do conversor analógico-digital.

#### 2.4. Inicialização e modularização da filtragem

A variável `battery_voltage_filtered` é inicializada
com o valor da primeira medição analógica durante
a execução da função `setup()`.

Essa estratégia evita um transiente artificial
associado à inicialização do filtro com tensão
igual a zero.

Também foi criada a função `update_battery_voltage()`,
responsável pela aquisição e filtragem da tensão.

Os seguintes parâmetros foram definidos como
constantes:

| Constante | Valor | Finalidade |
|---|---:|---|
| `BATTERY_ADC_GAIN` | 1.2317 | Fator de conversão |
| `BATTERY_ADC_OFFSET` | 65 | Compensação original |
| `BATTERY_FILTER_ALPHA` | 0.08 | Coeficiente do filtro |

Os fatores de conversão e o coeficiente do filtro
foram preservados em relação ao projeto original.

A implementação utiliza uma única leitura do ADC
por iteração, compartilhada pelos dois filtros.

O filtro original foi mantido para a compensação
da tensão dos motores, enquanto o novo filtro é
utilizado exclusivamente no monitoramento da
bateria.

Dessa forma, a alteração não modifica diretamente
os ganhos PID nem a equação original de compensação
dos motores.

#### 2.5. Análise da resposta temporal do filtro

O controlador de voo possui um período nominal
de amostragem de 4 ms, correspondente a uma
frequência de 250 Hz.

A constante de tempo equivalente do filtro pode
ser calculada por:

$$
\tau = \frac{-T_s}{\ln(1-\alpha)}
$$

Substituindo os parâmetros adotados:

$$
\tau = \frac{-0,004}{\ln(0,92)}
$$

$$
\tau \approx 0,048 \text{ s}
$$

Portanto, a constante de tempo equivalente é de
aproximadamente 48 ms.

O tempo teórico necessário para atingir 95% de
uma variação em degrau é de aproximadamente 144 ms,
considerando a resposta exponencial equivalente.

O coeficiente do filtro original foi preservado
porque ainda não existem medições experimentais
que permitam justificar uma nova frequência de
filtragem.

### 3. Comparação entre as implementações

| Característica | Original | Commit 1 | Commit 2 |
|---|---|---|---|
| Monitoramento com histerese | Não | Sim | Sim |
| Limites configuráveis | Não | Sim | Sim |
| Estados independentes de alerta | Não | Sim | Sim |
| Tipo do filtro de monitoramento | int | int | float |
| Preservação da parte fracionária | Não | Não | Sim |
| Função específica de filtragem | Não | Não | Sim |
| Leituras ADC por iteração | 1 | 1 | 1 |
| Coeficiente do filtro | 0,08 | 0,08 | 0,08 |
| Compensação original dos motores | Preservada | Preservada | Preservada |

### 4. Benefícios esperados

As alterações propostas apresentam os seguintes
benefícios:

- Maior confiabilidade do indicador de bateria;
- Redução de mudanças sucessivas no estado do alerta;
- Preservação dos avisos de falhas de temporização;
- Redução dos erros associados ao truncamento
  durante a filtragem digital;
- Melhor organização e modularização do código;
- Facilidade de configuração dos parâmetros;
- Preservação da lógica original de compensação
  dos motores.

### 5. Compilação e validação

#### 5.1. Ambiente de compilação

O código original e as duas versões modificadas
foram compilados utilizando o Arduino IDE, com
a placa Arduino Uno selecionada, baseada no
microcontrolador ATmega328P.

As três compilações foram realizadas com sucesso.

#### 5.2. Resultados das compilações

| Parâmetro | Original | Commit 1 | Commit 2 |
|---|---:|---:|---:|
| Memória Flash utilizada | 12374 bytes | 12418 bytes | 12548 bytes |
| Ocupação da memória Flash | 38% | 38% | 38% |
| Memória SRAM global | 495 bytes | 497 bytes | 501 bytes |
| Ocupação da memória SRAM | 24% | 24% | 24% |
| Memória SRAM restante | 1553 bytes | 1551 bytes | 1547 bytes |
| Resultado da compilação | Sucesso | Sucesso | Sucesso |

A capacidade disponível para armazenamento do
programa é de 32256 bytes de memória Flash,
enquanto a capacidade total de SRAM é de 2048 bytes.

#### 5.3. Análise do consumo adicional de memória

| Implementação | Flash adicional | SRAM global adicional |
|---|---:|---:|
| Monitoramento com histerese | 44 bytes | 2 bytes |
| Aprimoramento do filtro | 130 bytes | 4 bytes |
| **Total** | **174 bytes** | **6 bytes** |

O aumento total corresponde a aproximadamente
1,41% da memória Flash utilizada originalmente
e 1,21% da memória SRAM global utilizada
originalmente.

O aumento de SRAM é consistente com as novas
variáveis globais declaradas:

| Variável | Tipo | Memória ocupada |
|---|---|---:|
| `low_battery` | bool | 1 byte |
| `timing_fault` | bool | 1 byte |
| `battery_voltage_filtered` | float | 4 bytes |
| **Total** | | **6 bytes** |

Apesar das funcionalidades acrescentadas, o código
permanece dentro dos limites de memória disponíveis
no microcontrolador.

#### 5.4. Testes funcionais propostos

Para avaliar o comportamento das implementações,
propõem-se os seguintes testes de bancada:

| Teste | Resultado esperado |
|---|---|
| Inicialização com tensão de 12,6 V | Alerta desativado |
| Redução da tensão filtrada para 10,5 V | Alerta ativado |
| Recuperação da tensão filtrada para 10,6 V após o alerta | Alerta permanece ativado |
| Recuperação da tensão filtrada para 10,8 V ou mais | Alerta desativado, na ausência de falha temporal |
| Simulação de falha de temporização | LED ativado |
| Recuperação da bateria após falha temporal | LED permanece ativado |
| Aplicação de pequena variação positiva na tensão medida | Novo filtro acompanha a variação |
| Verificação do período de execução | Manutenção do período nominal de 4 ms |

Os resultados apresentados nessa tabela representam
o comportamento esperado, e não medições
experimentais já realizadas.

### 6. Limitações

A compilação bem-sucedida confirma que o código
é aceito pelo compilador e permanece dentro dos
limites de memória informados.

Entretanto, a compilação não garante o cumprimento
dos requisitos temporais nem o funcionamento
adequado em condições reais.

Os limites de tensão e os fatores de conversão
do ADC deverão ser verificados de acordo com
a bateria e o circuito de condicionamento utilizados.

O monitoramento da tensão total da bateria não
permite identificar individualmente a tensão
de cada célula e não substitui um sistema de
proteção contra descarga excessiva.

A melhoria da precisão numérica foi aplicada
exclusivamente ao filtro de monitoramento.
O filtro original utilizado na compensação
dos motores foi preservado.

O impacto das operações adicionais de ponto
flutuante sobre o período de execução do
controlador deverá ser avaliado experimentalmente.

Os testes físicos permanecem pendentes e deverão
ser realizados em bancada, sem hélices e com
os motores desconectados nos testes de tensão.

### 7. Histórico das alterações

- `c7b0677`: implementação do monitoramento de
  bateria com histerese;
- `ef596c9`: aprimoramento da precisão do filtro
  digital da bateria.

---

## Contribuição AO — Recalibração do giroscópio

**Responsável:** Ana Carolina Saggioratto Osello - 13683104

**Branch:** `fix/gyro-calibration`

**Arquivos modificados:** `YMFC-AL_esc_calibrate.ino` e `README.md`.

### 1. Problema identificado

No programa de calibração dos ESCs, o comando `a` do monitor serial
inicia a calibração do giroscópio e a exibição dos ângulos. O programa
soma 2000 amostras de cada eixo e divide os totais por 2000 para
estimar os offsets, isto é, a leitura do giroscópio em repouso.

Ao receber outro comando, `cal_int` volta a zero. Entretanto, os
valores de `gyro_axis_cal[1]`, `[2]` e `[3]` mantêm os offsets calculados
anteriormente. Assim, um novo comando `a` acumula as novas amostras
sobre os offsets anteriores, contaminando a nova média.

Para cada eixo, o cálculo incorreto equivale a:

$$
b_{novo} = \frac{b_{anterior} + \sum_{i=1}^{2000} x_i}{2000}
$$

O resultado deve depender apenas das amostras da calibração atual:

$$
b_{novo} = \frac{\sum_{i=1}^{2000} x_i}{2000}
$$

### 2. Alteração implementada

Antes do laço de aquisição de uma nova calibração:

- Os três acumuladores `gyro_axis_cal[1..3]` são zerados;
- As estimativas `angle_pitch` e `angle_roll` são zeradas;
- A flag `first_angle` recebe `false`, para que a primeira estimativa
  após a calibração seja inicializada a partir do acelerômetro.

O laço começa com `cal_int = 0`, permitindo adquirir as amostras brutas
sem descontar o offset anterior. A compensação continua sendo aplicada
por `gyro_signalen()` somente quando `cal_int == 2000`, após o cálculo
da média. Foram mantidas as 2000 amostras e os pulsos de 1000 µs
enviados aos ESCs durante a calibração.

### 3. Compilação e conferência numérica

Em 26/11/2026, o sketch foi compilado separadamente para Arduino Uno
(ATmega328P, FQBN `arduino:avr:uno`), com Arduino CLI 1.6.0 e
Arduino AVR Boards 1.8.8. Tanto a versão de referência da `develop`
(`e7f1245`) quanto a versão corrigida compilaram com sucesso.

| Recurso | Referência da develop | Versão corrigida |
|---|---:|---:|
| Flash utilizada | 13042 bytes | 13126 bytes |
| SRAM global utilizada | 1106 bytes | 1106 bytes |
| SRAM restante para variáveis locais | 942 bytes | 942 bytes |

A diferença foi de 84 bytes de Flash, sem aumento de SRAM global.
Para reproduzir a compilação no Arduino IDE, copiar somente
`YMFC-AL_esc_calibrate.ino` para uma pasta chamada
`YMFC-AL_esc_calibrate`, selecionar Arduino Uno e usar Verificar.
Os três arquivos `.ino` do repositório são programas independentes
e devem ser compilados em pastas separadas.

Uma simulação numérica em Python, com operações em ponto flutuante
de 32 bits, conferiu primeira calibração, repetição, mudança de offsets
e entrada zero. Por exemplo, para amostras constantes de 100, -200 e
50 unidades nos três eixos, repetir a calibração sem zerar os
acumuladores resulta em aproximadamente 100,05, -200,10 e 50,025.
Com a reinicialização, as médias voltam a ser 100, -200 e 50.
Essa simulação verifica o cálculo da média; não executa o sketch nem
simula o sensor, as interrupções ou os pulsos dos ESCs.

### 4. Validação de bancada proposta

Os testes em hardware permanecem pendentes. Com as hélices removidas,
o setup previamente gravado na EEPROM e o conjunto imóvel:

1. Carregar o sketch de calibração dos ESCs e abrir o monitor serial
   em 57600 baud, mantendo o acelerador na posição mínima.
2. Enviar `a`, aguardar a calibração e observar os ângulos e a
   velocidade de yaw em repouso.
3. Enviar novamente `a`, sem reiniciar a placa. A nova calibração
   deve usar apenas as novas amostras e reinicializar os ângulos.
4. Enviar `r` e depois `a`, verificando também a troca de modo.
5. Repetir o procedimento algumas vezes e comparar as leituras em
   repouso, considerando o ruído e a variação térmica do sensor.

A correção elimina a reutilização dos offsets como soma inicial.
Ela não detecta movimento durante a calibração nem falhas de comunicação
I²C; o conjunto deve permanecer imóvel durante a aquisição.

---

## Melhoria AP.1 — Modularização do reset do PID

**Responsável:** AP (identificação completa a confirmar)

**Branch:** `refactor/pid-reset`

**Arquivos modificados:** `YMFC-AL_Flight_controller.ino` e `README.md`.

### 1. Problema identificado

O firmware original já reinicializa o PID ao armar os motores, mas as seis atribuições ficavam soltas no `if` de partida do `loop()`, junto da inicialização dos ângulos.
Como `calculate_pid()` é chamada em todo ciclo, inclusive com os motores parados, a memória integral acumula antes da partida; o reset evita levar esse valor aos motores na arrancada (comentário original "bumpless start").

### 2. Conteúdo das aulas relacionado

- Aula 0x0002, p. 28: função é uma rotina com identificador próprio, chamada no corpo principal, que "permite repetição de operações sem reescrita de código".
- Aula 0x0002, p. 29: o exemplo `Proximo` é uma função `void`, sem retorno.
- Aula 0x0002, p. 31: declaração prévia de uma função auxiliar que é definida depois de `main`.
- Aula 0x0001, p. 22: linguagem estruturada, com recursos de funções, condicionais e repetições.

Conhecimento externo às aulas (processo de compilação do Arduino): no `.ino`, a declaração prévia não é necessária, pois o pré-processamento gera os protótipos; o próprio sketch já chama `calculate_pid()`, definida após `loop()`.

### 3. Alteração realizada

Foi criada a função `void reset_pid()`, sem parâmetros e sem retorno, logo após `calculate_pid()`. As atribuições `pid_i_mem_roll = 0;`, `pid_last_roll_d_error = 0;`, `pid_i_mem_pitch = 0;`, `pid_last_pitch_d_error = 0;`, `pid_i_mem_yaw = 0;` e `pid_last_yaw_d_error = 0;` foram transferidas sem mudança e na mesma ordem.
No `loop()`, o bloco foi substituído por `reset_pid();` no mesmo ponto, após `gyro_angles_set = true;`, mantendo o comentário original.
Nenhuma variável foi criada; o código de ganhos, limites, equações, condições, ESCs e temporização não foi alterado.

### 4. Justificativa

O `loop()` passa a mostrar a etapa pelo nome e o reset fica agrupado junto à rotina do PID; se necessário, ele pode ser chamado de novo sem reescrever as linhas (Aula 0x0002, p. 28).
O mecanismo já existia: trata-se de uma refatoração, sem novo mecanismo de controle e sem ganho de desempenho ou de estabilidade.

### 5. Resultados de compilação

Em 26/11/2026, com Arduino CLI 1.6.0, Arduino AVR Boards 1.8.8 e FQBN `arduino:avr:uno`, os sketches foram compilados a partir de cópias em pastas separadas.

| Recurso | Referência da develop | Versão modificada |
|---|---:|---:|
| Flash utilizada | 12548 bytes | 12548 bytes |
| SRAM global utilizada | 501 bytes | 501 bytes |
| SRAM restante para variáveis locais | 1547 bytes | 1547 bytes |

`YMFC-AL_setup.ino` e `YMFC-AL_esc_calibrate.ino` não mudaram e compilaram com os mesmos valores da referência (20572/532 e 13126/1106 bytes de Flash/SRAM global).

### 6. Verificações realizadas

A comparação do diff mostra que as seis linhas removidas do `loop()` são iguais às seis do corpo de `reset_pid()`, na mesma ordem.
A função é chamada uma única vez, sob a mesma condição e no mesmo ponto da mesma iteração; portanto, as mesmas variáveis são zeradas no mesmo ponto do fluxo de execução.
A verificação foi feita por inspeção do código e compilação.

### 7. Limitações

Refatoração sem mudança de comportamento esperada; não foi testada em hardware.
O comportamento original no desarme, sem reset do PID, foi mantido de propósito.
Não houve medição experimental do tempo de execução nem comparação dos binários gerados.

---

## Melhoria H.1 — Redução de escritas desnecessárias na EEPROM

**Responsável:** H (identificação completa a confirmar)

**Branch:** `fix/eeprom-wear`

**Arquivos modificados:** `YMFC-AL_setup.ino` e `README.md`.

### 1. Problema identificado

Ao terminar sem erros, o `YMFC-AL_setup.ino` grava os 36 bytes de configuração (endereços 0 a 35) com `EEPROM.write()`, que grava o byte sempre, mesmo quando o valor guardado já é igual ao novo. Assim, repetir o setup com a mesma configuração regrava todos os 36 bytes.

### 2. Conteúdo das aulas relacionado

- Sistemas Embarcados (EESC, Ago/2026), p. 8/16: na visão geral de um sistema embarcado, o bloco "Memórias: ROM, RAM" aparece ligado ao microcontrolador; na p. 17, o MCU reúne em um único chip a CPU, a memória ("RAM & Flash") e o I/O básico; na p. 21, o Arduino UNO aparece com RAM de 2 KB e ROM de 32 KB.
- Sistemas Embarcados (EESC, Ago/2026), p. 30: no mapa de falhas de energia, a corrupção de dados aparece como "Erros em EEPROM, Flash ou RAM".
- Aula 0x0001, pp. 10–13: no processo típico de build, arquivos de biblioteca (`.a`, com `.h`) são unidos ao código compilado pelo linker para gerar o executável; a `EEPROM.h` já era incluída pelo sketch original (linha 19), ou seja, nenhuma biblioteca foi acrescentada.

Conhecimento externo às aulas: na biblioteca EEPROM do core `arduino:avr` 1.8.8 (`EEPROM.h`), `EEPROM.write(endereço, valor)` chama `eeprom_write_byte` da avr-libc em toda chamada; `EEPROM.update(endereço, valor)`, com a mesma assinatura, primeiro lê o byte com `eeprom_read_byte` e só chama `eeprom_write_byte` quando o valor novo é diferente. Segundo o "Atmel ATmega328/P Datasheet Complete" (documento Atmel-42735B-ATmega328/P_Datasheet_Complete-11/2016), seção 12.4 "EEPROM Data Memory", a EEPROM tem durabilidade de pelo menos 100.000 ciclos de escrita/apagamento.

### 3. Alteração realizada

As 36 chamadas `EEPROM.write()` do bloco de gravação foram trocadas por `EEPROM.update()`, com os mesmos argumentos e na mesma ordem:

| Endereços | Conteúdo |
|---|---|
| 0–23 | `center_channel_1..4`, `high_channel_1..4` e `low_channel_1..4` (byte baixo e byte alto) |
| 24–32 | `channel_1_assign` a `channel_4_assign` (24–27), `roll_axis`, `pitch_axis`, `yaw_axis` (28–30), `type` (31) e `gyro_address` (32) |
| 33–35 | assinatura `'J'`, `'M'`, `'B'` |

Foi acrescentado um comentário antes das chamadas explicando o comportamento de `EEPROM.update()`. A verificação que relê os 36 bytes, as mensagens seriais, o formato dos dados e as leituras feitas pelo `YMFC-AL_Flight_controller.ino` e pelo `YMFC-AL_esc_calibrate.ino` não foram alterados.

### 4. Justificativa

`EEPROM.write()` grava sempre; `EEPROM.update()` lê o byte atual, compara com o novo e só grava se for diferente, com o mesmo conteúdo final; cada gravação desnecessária gasta um ciclo de um recurso limitado, e evitá-la reduz o desgaste da EEPROM, sem ganho quantitativo estimado.
Comportamento esperado (não observado): os endereços 0–23 guardam medições do receptor (centro, máximo e mínimo, em µs), que podem variar alguns microssegundos entre execuções do setup; por isso esses bytes, principalmente os bytes baixos, tendem a ser regravados, e a economia é mais provável nos bytes que se repetem (24–35: atribuição de canais, eixos, tipo, endereço do giroscópio e assinatura `'J'`, `'M'`, `'B'`).

### 5. Resultados de compilação

Em 26/11/2026, com Arduino CLI 1.6.0, Arduino AVR Boards 1.8.8 e FQBN `arduino:avr:uno`, os sketches foram compilados a partir de cópias em pastas separadas.

| `YMFC-AL_setup.ino` | Referência da develop (`45b3d5e`) | Versão modificada |
|---|---:|---:|
| Flash utilizada | 20572 bytes | 20608 bytes |
| SRAM global utilizada | 532 bytes | 532 bytes |

O setup passou a ocupar 36 bytes a mais de Flash, diferença decorrente da troca por `EEPROM.update()`. `YMFC-AL_Flight_controller.ino` e `YMFC-AL_esc_calibrate.ino` não mudaram e compilaram com os mesmos valores da referência (12548/501 e 13126/1106 bytes de Flash/SRAM global).

### 6. Verificações realizadas

Verificações conceituais, descrevendo o comportamento esperado pela implementação da biblioteca (não observado em hardware), feitas por inspeção do código e compilação:

- Endereço com valor diferente: `update` lê, compara e chama a mesma `eeprom_write_byte` usada por `write`; o byte final é igual ao gravado por `write`.
- Endereço com o mesmo valor: não há gravação; o conteúdo final já é o valor pedido e a verificação que relê os 36 bytes continua passando.
- Configuração idêntica repetida: se os 36 bytes forem iguais, nenhuma escrita é feita; se só parte mudou, apenas os bytes diferentes são gravados. Na prática, como explicado na seção 4, os endereços 0–23 tendem a mudar entre execuções, e os 24–35 tendem a se repetir.
- Formato preservado: mesmos endereços 0–35, mesmos bytes, mesma ordem e mesma assinatura; as leituras do controlador de voo e do `esc_calibrate` continuam compatíveis.

### 7. Limitações

Não houve gravação nem leitura em hardware, nem contagem de ciclos de escrita. `EEPROM.update()` acrescenta uma leitura por byte durante a gravação da configuração, apenas no setup. A alteração não valida a coerência dos parâmetros gravados nem protege contra corrupção da EEPROM por queda de alimentação.

---

## Melhoria AO.2 — Tratamento de erros I²C

**Responsável:** AO (identificação completa a confirmar)

**Branch:** `feat/i2c-error-handling`

**Arquivos modificados:** `YMFC-AL_setup.ino` e `README.md`.

### 1. Problema identificado

No `YMFC-AL_setup.ino`, nenhum valor de retorno da comunicação I²C com o giroscópio era usado: `Wire.endTransmission()` e `Wire.requestFrom()` informam se a transação deu certo, mas o resultado era ignorado.
Depois de cada leitura havia uma espera `while(Wire.available() < N);` (seis no total, em `start_gyro()` e `gyro_signalen()`). Como `available()` não muda depois que `requestFrom()` retorna, essa espera termina na hora (dados completos) ou nunca (faltou byte).
Além disso, o timeout interno da biblioteca Wire vem desligado por padrão; se o barramento travar, a própria biblioteca pode ficar presa. Nos dois casos, o setup parava sem nenhuma mensagem no monitor serial.
Em `search_gyro()` a espera já tinha limite de 100 ms (`timer`) e um sensor ausente já terminava em ERROR 3; ali a mudança é de consistência e evita essas esperas, não corrige um travamento.

### 2. Conteúdo das aulas relacionado

- Aula 0x0002 (Revisão de Linguagem C), pp. 8–9: condicionais e `if/else` para variar o fluxo conforme uma condição; p. 18: o slide apresenta o `while` com a "condição para permanência no laço". Daí se conclui que uma condição que nunca muda gera um laço sem fim.
- Aula 0x0002, p. 15: laço `for` e o comentário "para sair da repetição é necessário break", usado para interromper a calibração; p. 19: operadores lógicos `==` e `!=`, usados nas conferências.
- Aula 0x0002, pp. 28–29: funções podem "retornar resultados para interface com o restante do programa"; no exemplo `void Proximo`, `return;` encerra uma função sem retorno. P. 30: a função principal retorna "0 para ausência de erro", a mesma convenção de `endTransmission()` e da variável `error`.
- Sistemas Embarcados (EESC, Ago/2026), p. 25: I2C aparece apenas listado entre as comunicações com fio; p. 18: I2C aparece listado entre as portas de uma placa com MCU. P. 37: no exemplo de aviônica sobre a escolha do sistema operacional, "crashes are not acceptable in flight" (o setup roda em solo; a citação é só motivação).
- Analogia (CAN e I²C são protocolos diferentes): em Protocolos CAN, p. 26, no campo ACK o receptor que recebeu certo sobrescreve o bit com dominante; p. 29, o "ACK error" é detectado pelo transmissor quando ninguém confirma. O ACK/NACK do I²C não é apresentado nas aulas: que ele tenha papel parecido e apareça nos códigos 2 e 3 de `endTransmission()` vem da documentação e do código da Wire (conhecimento externo, abaixo).

Conhecimento externo às aulas: biblioteca Wire do core `arduino:avr` 1.8.8. Em `utility/twi.c`, `twi_writeTo()` (base de `endTransmission()`) retorna 0 (sucesso), 1 (dados longos demais), 2 (NACK no endereço), 3 (NACK no dado), 4 (outro erro) ou 5 (timeout); `twi_timeout_us = 0ul` desliga o timeout por padrão. Em `Wire.cpp`, `requestFrom()` retorna quantos bytes foram recebidos (0 em NACK ou timeout) e `available()` só calcula os bytes que restam no buffer. Em `Wire.h`, `setWireTimeout(uint32_t timeout = 25000, bool reset_with_timeout = false)`; o comentário de `Wire.cpp` indica 25 ms como valor típico.

### 3. Alteração realizada

- `setup()`: `Wire.setWireTimeout(25000, false);` logo após `Wire.begin();`.
- `search_gyro()`: se `endTransmission()` não retornar 0 ou `requestFrom()` não trouxer 1 byte, a função retorna 0, e o `loop()` tenta o próximo endereço. Não é erro: se nenhum endereço responder, o ERROR 3 existente interrompe o setup. A espera com `timer` ficou redundante após a conferência do `requestFrom()`, mas foi mantida para não sobrepor a melhoria H.2.
- Nova função `gyro_i2c_error()`: imprime `I2C communication with the gyro failed!!! (ERROR 9)` e faz `error = 1`. ERROR 9 é um código novo (1 a 8 já existiam).
- `start_gyro()`: as 8 chamadas de `endTransmission()` e as 4 de `requestFrom(address, 1)` passaram a ser conferidas; em falha, chama `gyro_i2c_error()` e retorna. As quatro esperas `while(Wire.available() < 1);` foram removidas; um único comentário explica que `requestFrom()` é bloqueante e devolve quantos bytes chegaram.
- `gyro_signalen()`: mesma conferência nas 2 transmissões e nas 2 leituras de 6 bytes; as duas esperas `while(Wire.available() < 6);` foram removidas.
- Calibração: `if(error == 1)break;` logo após `gyro_signalen()`, antes de somar a leitura. O bloco `if(error == 0)` foi dividido em dois blocos seguidos, no mesmo nível, como as demais etapas do `loop()`: o primeiro faz as 2000 leituras e o segundo calcula e mostra os offsets e configura os eixos.
- `check_gyro_axes()`: `if(error == 1)return;` após as duas chamadas de `gyro_signalen()`.

Registradores, valores escritos, ordem das operações, mensagens existentes e a gravação na EEPROM não foram alterados.

### 4. Justificativa

Uma espera sem limite deixa o setup parado sem explicar o motivo. Conferir o retorno de cada operação e a quantidade de bytes recebidos mostra a falha na hora e impede o uso de dados incompletos.
Só conferir retornos não basta: se o barramento travar, a Wire nem chega a retornar. O timeout limita cada espera interna da biblioteca e faz a função retornar com erro, que então é conferido.
Na busca, um endereço sem resposta é esperado (o sensor pode estar em outro endereço); depois que o giroscópio foi encontrado, qualquer falha é impeditiva. Nesse caso, `error = 1` faz o `loop()` pular as etapas seguintes (todas sob `if(error == 0)`), inclusive a gravação na EEPROM, e mostrar a mensagem de aborto.
25 ms é o valor sugerido pela própria biblioteca e vale para cada espera interna da Wire, separadamente, e não para a transação inteira. Em operação normal, uma transação completa a 400 kHz dura bem menos que isso (uma leitura de 6 bytes, com o endereço, ocupa cerca de 63 bits de 2,5 µs, perto de 0,16 ms), e nenhuma espera se aproxima do limite. Em uma falha, a operação pode passar por mais de uma espera; por isso a demora total até o erro pode passar de 25 ms (algumas dezenas de milissegundos), mas continua limitada. `false` mantém o comportamento simples: sem reinicializar o hardware TWI, pois não há recuperação automática.

### 5. Resultados de compilação

Em 26/11/2026, com Arduino CLI 1.6.0, Arduino AVR Boards 1.8.8 e FQBN `arduino:avr:uno`, os sketches foram compilados a partir de cópias em pastas separadas.

| `YMFC-AL_setup.ino` | Referência da develop (`dea7bd1`) | Versão modificada |
|---|---:|---:|
| Flash utilizada | 20608 bytes | 20686 bytes |
| SRAM global utilizada | 532 bytes | 532 bytes |

O setup passou a ocupar 78 bytes a mais de Flash, sem aumento de SRAM global (a nova mensagem usa `F()`). `YMFC-AL_Flight_controller.ino` e `YMFC-AL_esc_calibrate.ino` não mudaram e compilaram com os mesmos valores da referência (12548/501 e 13126/1106 bytes de Flash/SRAM global).
Com `--warnings all`, o setup apresentou os mesmos 5 avisos antes e depois: 4 de `new.cpp` do core e 1 já existente em `wait_for_receiver()` (`'now' may be used uninitialized`). Nenhum aviso novo.

### 6. Verificações realizadas

Análise conceitual, por inspeção do código e compilação, sem observação em hardware:

1. Sensor presente e funcionamento normal: as conferências passam, a busca identifica o sensor como antes e as mesmas operações são feitas na mesma ordem; as esperas removidas já terminavam imediatamente nesse caso.
2. Sensor ausente: todos os endereços retornam NACK, `search_gyro()` retorna 0 e o ERROR 3 existente interrompe o setup, como antes, mas sem as esperas de 100 ms por endereço.
3. Endereço sem resposta na busca (ex.: MPU-6050 em 0x69): a tentativa em 0x68 retorna 0 e a busca segue para 0x69, sem ERROR 9.
4. Falha de transmissão após a busca (`endTransmission()` de 1 a 5): ERROR 9.
5. Menos bytes que o pedido, ou nenhum: `requestFrom()` retorna menos que N (0, se nada chegou) e gera ERROR 9 sem ler dados incompletos; antes, o `while` nunca terminaria.
6. Bloqueio interno do barramento: cada espera interna da Wire é interrompida após cerca de 25 ms; depois de uma ou mais dessas esperas, a operação retorna com erro e a falha vira ERROR 9; sem o timeout, nenhuma conferência resolveria. Se o bloqueio ocorrer durante a busca, o resultado é o ERROR 3.
7. Interrupção após o erro: em `start_gyro()`, na calibração e em `check_gyro_axes()`, a função para na primeira falha; a mensagem não se repete e nenhuma leitura antiga é somada ou integrada.
8. Nenhuma gravação inválida: a EEPROM só é gravada dentro de `if(error == 0)`, e `error` nunca volta a 0.

### 7. Limitações

- Não houve teste em hardware nem simulação de falhas no barramento.
- Um bloqueio do barramento durante a busca aparece como ERROR 3 ("No gyro device found"), e não como ERROR 9.
- O ERROR 9 não informa qual operação falhou, e os valores lidos dos registradores não são comparados com os esperados.
- Não há recuperação nem nova tentativa: o setup é interrompido.
- O limite de 25 ms vale para cada espera interna da Wire, não para a transação inteira; em caso de falha, a demora até a mensagem de erro pode somar algumas dezenas de milissegundos. O timeout só foi configurado no setup.
- `YMFC-AL_Flight_controller.ino` e `YMFC-AL_esc_calibrate.ino` continuam com esperas I²C sem timeout; as comparações temporais com `millis()` ficam para a melhoria H.2.

---

## Melhoria H.2 — Correção das comparações temporais

**Responsável:** H (identificação completa a confirmar)

**Branch:** `fix/loop-timing`

**Arquivos modificados:** `YMFC-AL_setup.ino` e `README.md`.

### 1. Problema identificado

`millis()` conta os milissegundos desde o reset em um número de 32 bits sem sinal e volta a zero depois de cerca de 49,7 dias (`micros()`, depois de cerca de 71,6 minutos).
No `YMFC-AL_setup.ino`, três esperas calculavam um instante futuro (`timer = millis() + X;`) e o comparavam com o atual (`while(timer > millis() ...)`): `search_gyro()` (100 ms), `check_receiver_inputs()` (30 s) e `check_gyro_axes()` (10 s).
Se a espera começar perto da volta a zero, a soma também dá a volta e vira um número pequeno; a condição já começa falsa e a espera termina na hora. Em `check_receiver_inputs()` e `check_gyro_axes()`, isso mostraria ERROR 2 ou ERROR 4 sem esperar o movimento do usuário.

### 2. Conteúdo das aulas relacionado

- Aula 0x0002 (Revisão de Linguagem C), p. 3: `int` com 32 bits, "[16 bits (2 bytes) em plataformas mais antigas]", com os limites dos tipos em `limits.h`. Os limites 100, 10000 e 30000 são constantes `int` que cabem nesses 16 bits e são comparadas com `millis() - timer` (`unsigned long`).
- Aula 0x0002, p. 5: `unsigned` faz o tipo representar "apenas números não negativos"; entre os modificadores aparece `unsigned long int (uint32_t)`, o tipo de `timer` e do valor de `millis()`.
- Aula 0x0002, p. 14 (estruturas de repetição) e p. 18 (`while`, com a "condição para permanência no laço"): as três esperas são laços `while` cuja condição define quanto tempo o programa fica neles.
- Aula 0x0002, p. 19: operadores matemáticos (`+`, `-`) e lógicos (`<`, `>`, `&&`) usados nas condições.

O estouro e a volta a zero (wrap-around) de inteiros não são ensinados nas aulas.

Conhecimento externo às aulas: no core `arduino:avr` 1.8.8, `millis()` e `micros()` são declaradas em `Arduino.h`, implementadas em `wiring.c` e retornam `unsigned long`. No avr-gcc usado pelo core, `int` tem 16 bits e `long` tem 32 bits (`uint32_t` é `unsigned long`); por isso os contadores voltam a zero após 2^32 ms ou 2^32 µs. Soma e subtração de inteiros sem sinal são feitas módulo 2^32: o resultado dá a volta, em vez de ficar negativo. Na comparação com `millis() - timer`, as constantes `int` (16 bits no avr-gcc) são convertidas para `unsigned long`; por isso o sufixo `UL` não é necessário.

### 3. Alteração realizada

Nos três pontos, `timer` passou a guardar o instante inicial e a condição passou a medir o tempo decorrido, com um comentário curto em cada ponto:

| Função | Antes | Depois |
|---|---|---|
| `search_gyro()` | `timer = millis() + 100;` e `timer > millis()` | `timer = millis();` e `millis() - timer < 100` |
| `check_receiver_inputs()` | `timer = millis() + 30000;` e `timer > millis()` | `timer = millis();` e `millis() - timer < 30000` |
| `check_gyro_axes()` | `timer = millis() + 10000;` e `timer > millis()` | `timer = millis();` e `millis() - timer < 10000` |

Os limites, as demais condições dos laços, as mensagens e a sequência do setup não mudaram, e nenhuma variável foi criada. A espera de `search_gyro()` continua redundante desde a AO.2 (o retorno de `requestFrom()` já é conferido), mas foi corrigida e mantida, e não removida. `wait_for_receiver()` (E.1) e a rotina de interrupção do receptor já usavam diferenças de tempo e não foram alteradas.

### 4. Justificativa

`millis() - timer` é o tempo decorrido desde o início da espera. Como a subtração sem sinal é feita módulo 2^32, o resultado continua correto mesmo se `millis()` voltar a zero no meio da espera, desde que o tempo decorrido seja menor que cerca de 49,7 dias. No original, `timer = millis() + X;` com `timer > millis()` falhava de dois modos: se a soma dava a volta, `timer` virava um número pequeno e a espera terminava na hora; se a soma não dava a volta, mas o fim previsto ficava muito perto do máximo, `millis()` podia passar desse fim e voltar a zero entre duas verificações do laço (possível com o `delay(250)` de `check_receiver_inputs()`), e então `timer > millis()` voltava a ser verdadeiro e a espera só terminaria com movimento do stick ou após cerca de 49,7 dias. O novo cálculo evita os dois casos.
Em funcionamento normal, a condição de saída mantém o mesmo limite temporal, com limite estrito e o caso "igual" tratado como antes: o original sai quando `millis()` chega a `timer` (início + X), e o novo, quando o decorrido chega a X. O encerramento efetivo depende de quando a condição é avaliada, ou seja, da frequência das verificações do laço (por exemplo, o `delay(250)` de `check_receiver_inputs()`); não houve medição experimental. É uma correção de comportamento, sem ganho de desempenho ou de precisão.

### 5. Resultados de compilação

Em 26/11/2026, com Arduino CLI 1.6.0, Arduino AVR Boards 1.8.8 e FQBN `arduino:avr:uno`, os sketches foram compilados a partir de cópias em pastas separadas.

| `YMFC-AL_setup.ino` | Referência da develop (`9e58c60`) | Versão modificada |
|---|---:|---:|
| Flash utilizada | 20686 bytes | 20680 bytes |
| SRAM global utilizada | 532 bytes | 532 bytes |

O setup passou a ocupar 6 bytes a menos de Flash, sem mudança de SRAM global. `YMFC-AL_Flight_controller.ino` e `YMFC-AL_esc_calibrate.ino` não mudaram e compilaram com os mesmos valores da referência (12548/501 e 13126/1106 bytes de Flash/SRAM global).
Com `--warnings all`, o setup apresentou os mesmos 5 avisos antes e depois (4 de `new.cpp` do core e 1 já existente em `wait_for_receiver()`); nenhum aviso novo.

### 6. Verificações realizadas

Contas feitas à mão sobre o código, sem observação em hardware:

1. Funcionamento normal (`check_receiver_inputs()`, início em 5000 ms, limite de 30000 ms): com `millis()` = 20000, o decorrido é 15000 e a espera continua; em 34999, 29999, continua; em 35000, 30000, sai. No original, `timer` = 35000: `35000 > 34999` continua e `35000 > 35000` é falso e sai, ou seja, o limite de saída é o mesmo. Nos dois casos, o laço só detecta esse limite na verificação seguinte da condição, feita depois de cada `delay(250)`.
2. Perto da volta a zero (início em 4 294 962 296 = 2^32 − 5000, limite de 30000 ms): no original, `timer` = (início + 30000) mod 2^32 = 25000 e `25000 > 4 294 962 296` é falso, então a espera termina na hora (ERROR 2 sem esperar). No novo, o decorrido vale 0 no início, 4999 em `millis()` = 4 294 967 295, 5000 em `millis()` = 0, 29999 em 24999 (continua) e 30000 em 25000 (condição de saída atingida 30 s após o início). Em `check_gyro_axes()` (10000 ms), com o mesmo início, o original teria `timer` = 5000 e terminaria na hora (ERROR 4); no novo, a condição de saída é atingida em `millis()` = 5000, 10 s após o início.

As demais condições de saída (movimento do stick, ângulo fora de ±30°, byte disponível na Wire) continuam encerrando os laços como antes.

### 7. Limitações

- No setup, a volta a zero quase nunca acontece, porque o sketch roda logo após o reset.
- Não houve teste em hardware nem simulação.
- O mesmo padrão de instante futuro continua em `YMFC-AL_esc_calibrate.ino` (linha 93, com `micros()`) e na geração dos pulsos dos ESCs (`YMFC-AL_Flight_controller.ino`, linhas 350–365, e `YMFC-AL_esc_calibrate.ino`, linhas 442–454). Esses trechos ficaram fora do escopo, sem alterar o período nominal do laço nem a temporização dos pulsos.
- Uma espera maior que cerca de 49,7 dias não seria medida corretamente, o que é irrelevante para limites de no máximo 30 s.

---

## Melhoria AP.2 — Reutilização do processamento dos comandos

**Responsável:** AP (identificação completa a confirmar)

**Branch:** `refactor/receiver-command`

**Arquivos modificados:** `YMFC-AL_Flight_controller.ino` e `README.md`.

### 1. Problema identificado

No `loop()`, o setpoint de cada eixo é calculado a partir do pulso padronizado do receptor (1000 a 2000 µs). Em torno do centro do stick existe uma zona morta, de 1492 a 1508 µs (inclusive), em que a contribuição do receptor vale zero, para que pequenas variações do stick ou do receptor em repouso não gerem comando; em roll e pitch, o setpoint final ainda inclui a correção de nivelamento, e no yaw, que não tem nivelamento, o setpoint fica zero. Acima dela, o comando é `pulso - 1508`; abaixo, `pulso - 1492`.
O mesmo par `if`/`else if` aparecia três vezes, para roll (canal 1), pitch (canal 2) e yaw (canal 4); qualquer mudança nos limites teria de ser repetida nos três lugares.

### 2. Conteúdo das aulas relacionado

- Aula 0x0002, p. 28: função é uma rotina com identificador próprio, que "permite repetição de operações sem reescrita de código", pode receber parâmetros e retornar resultados, com passagem de argumentos por valor.
- Aula 0x0002, p. 29: o exemplo `int Soma( int a, int b )` recebe dois `int` por valor e devolve um `int` com `return`, o mesmo formato da nova função.
- Aula 0x0002, p. 9: condicional `if`/`else`, usada para separar os três casos (acima, abaixo e dentro da zona morta).
- Aula 0x0002, p. 19: operadores matemáticos (`-`) e lógicos (`>`, `<`) usados no cálculo.
- Aula 0x0002, p. 3: tipos `int` e `float`; `int` com "[16 bits (2 bytes) em plataformas mais antigas]".

Conhecimento externo às aulas: no avr-gcc usado pelo core `arduino:avr`, `int` tem 16 bits e `double` é tratado como `float` (32 bits), por isso as divisões por `3.0` são feitas em 32 bits, antes e depois da mudança. No `.ino`, o pré-processamento do Arduino gera o protótipo da função automaticamente, como já ocorre com `reset_pid()`.

### 3. Alteração realizada

Foi criada a função `int apply_dead_band(int pulse)`, logo após `reset_pid()`. Ela recebe o pulso por parâmetro e devolve a diferença até a zona morta: `pulse - 1508` se `pulse > 1508`, `pulse - 1492` se `pulse < 1492` e `0` nos demais casos. As condições, os limites e as subtrações são os mesmos do original, na mesma ordem.
No `loop()`, foram feitas três substituições, com um comentário curto em cada uma:

| Eixo | Antes | Depois |
|---|---|---|
| Roll | `pid_roll_setpoint = 0;` e o par `if`/`else if` com `receiver_input_channel_1` | `pid_roll_setpoint = apply_dead_band(receiver_input_channel_1);` |
| Pitch | `pid_pitch_setpoint = 0;` e o par `if`/`else if` com `receiver_input_channel_2` | `pid_pitch_setpoint = apply_dead_band(receiver_input_channel_2);` |
| Yaw | o par `if`/`else if` com `(receiver_input_channel_4 - 1508)/3.0` e `(receiver_input_channel_4 - 1492)/3.0` | `pid_yaw_setpoint = apply_dead_band(receiver_input_channel_4)/3.0;` |

Em roll e pitch, as linhas seguintes (`-= roll_level_adjust` / `-= pitch_level_adjust` e `/= 3.0`) não mudaram e continuam depois da chamada. No yaw, continuam `pid_yaw_setpoint = 0;`, a condição `if(receiver_input_channel_3 > 1050)` (não aplicar yaw ao desligar os motores), a divisão por `3.0` na mesma posição e a ausência de nivelamento. Os comentários originais foram mantidos; nenhuma variável global foi criada.

### 4. Justificativa

Os limites 1492 e 1508 passam a ficar em um único ponto do código, e o cálculo repetido nos três eixos passa a ser escrito uma vez (Aula 0x0002, p. 28). Trata-se de uma refatoração: a zona morta já existia e nenhum mecanismo novo de controle foi criado.

### 5. Resultados de compilação

Em 27/11/2026, com Arduino CLI 1.6.0, Arduino AVR Boards 1.8.8 e FQBN `arduino:avr:uno`, os sketches foram compilados a partir de cópias em pastas separadas.

| `YMFC-AL_Flight_controller.ino` | Referência da develop (`e878bc7`) | Versão modificada |
|---|---:|---:|
| Flash utilizada | 12548 bytes | 12404 bytes |
| SRAM global utilizada | 501 bytes | 501 bytes |
| SRAM restante para variáveis locais | 1547 bytes | 1547 bytes |

A diferença de Flash depende de como o compilador organiza o código gerado; ela não foi analisada e não é apresentada como ganho. `YMFC-AL_setup.ino` e `YMFC-AL_esc_calibrate.ino` não mudaram e compilaram com os mesmos valores da referência (20680/532 e 13126/1106 bytes de Flash/SRAM global).
Com `--warnings all`, os três sketches apresentaram os mesmos avisos antes e depois (4 de `new.cpp` do core em cada um e, no setup, 1 já existente em `wait_for_receiver()`); nenhum aviso novo.

### 6. Verificações realizadas

Pelo diff, as condições, os limites e as subtrações removidos do `loop()` são os mesmos do corpo de `apply_dead_band()`. Quanto aos tipos: no original, `receiver_input_channel_x - 1508` era uma subtração `int - int`, convertida para `float` na atribuição; agora a função devolve o mesmo `int`, atribuído à mesma variável `float`. No yaw, o `int` continua sendo dividido por `3.0` como antes. Na zona morta, a contribuição do receptor (retorno convertido para `float`) é `0.0` nos dois casos; o setpoint final de roll e pitch depende ainda do ajuste de nível, como mostra a tabela. Com o pulso entre 1000 e 2000 µs, o retorno fica entre −492 e 492, sem risco de estouro do `int` de 16 bits.

Contas feitas à mão (roll/pitch = (retorno − ajuste de nível) / 3,0; yaw = retorno / 3,0 com canal 3 > 1050), iguais para o código original e o modificado:

| Pulso (µs) | Caso | Retorno | Roll/pitch, ajuste 0 | Ajuste +30,0 | Ajuste −45,0 | Yaw |
|---:|---|---:|---:|---:|---:|---:|
| 1000 | abaixo da zona morta | −492 | −164 | −174 | −149 | −164 |
| 1491 | logo abaixo do limite inferior | −1 | −0,3333 | −10,3333 | 14,6667 | −0,3333 |
| 1492 | igual ao limite inferior | 0 | 0 | −10 | 15 | 0 |
| 1493, 1500, 1507 | dentro (1500 = centro) | 0 | 0 | −10 | 15 | 0 |
| 1508 | igual ao limite superior | 0 | 0 | −10 | 15 | 0 |
| 1509 | logo acima do limite superior | 1 | 0,3333 | −9,6667 | 15,3333 | 0,3333 |
| 2000 | acima da zona morta | 492 | 164 | 154 | 179 | 164 |

O ajuste de nível é `roll_level_adjust` ou `pitch_level_adjust`, calculado como ângulo × 15 (+30,0 corresponde a 2° e −45,0 a −3°; com `auto_level = false` vale 0). No yaw, com o canal 3 em 1049 ou 1050 (condição estrita), o setpoint fica 0 qualquer que seja o canal 4; com o canal 3 em 1051, o canal 4 em 2000, 1000 e 1500 dá 164, −164 e 0.

A verificação foi feita por inspeção do código, contas à mão e compilação; não comprova equivalência temporal nem o funcionamento físico.

### 7. Limitações

- Não houve teste em hardware nem medição do tempo de execução, e os binários gerados não foram comparados.
- O original lia cada canal `volatile` duas ou três vezes (comparações e subtração); o novo lê uma vez e passa a cópia por valor. A diferença não altera o resultado, porque esses canais só são escritos no contexto principal (`gyro_signalen()` e `setup()`), e não pela rotina de interrupção.
- Na zona morta, roll e pitch passam a sempre converter o retorno `int` para `float`, e o yaw (com canal 3 > 1050) passa a sempre fazer a divisão por `3.0`, operações que antes eram puladas. Há ainda uma chamada de função por eixo no código-fonte (o compilador pode ou não expandi-la no local); o efeito no tempo do laço não foi medido.
- A atomicidade da leitura dos dados do receptor gravados pela rotina de interrupção não foi tratada aqui e fica para a AP.3.

---

## Melhoria AO.3 — Validação dos cálculos do acelerômetro

**Responsável:** AO (identificação completa a confirmar)

**Branch:** `fix/accelerometer-validation`

**Arquivos modificados:** `YMFC-AL_esc_calibrate.ino` e `README.md`.

### 1. Problema identificado

No comando `'a'` do `YMFC-AL_esc_calibrate.ino`, os ângulos de pitch e roll do acelerômetro são calculados com `asin(acc / acc_total_vector[0])`, em que `acc_total_vector[0]` é o módulo do vetor de aceleração (a raiz da soma dos quadrados dos três eixos). O código não conferia o denominador nem o argumento do `asin`, e a soma dos quadrados era feita em um tipo que podia estourar. Isso deixava três problemas possíveis:

- **Divisão por zero:** se os três eixos valem zero, o módulo é 0 e a conta vira 0.0/0.0, cujo resultado é NaN ("not a number", um valor que não representa número nenhum). Isso pode acontecer nestes casos:
  - gyro tipo 2 ou 3 gravado na EEPROM: `gyro_signalen()` só lê o acelerômetro quando `eeprom_data[31] == 1`; nos outros casos, `acc_axis` continua valendo 0;
  - mapeamento de eixo em `eeprom_data[28..30]` com os dois bits mais baixos valendo 0 (EEPROM corrompida): `acc_axis[0]` nunca é escrito e vale 0;
  - sensor devolvendo zeros.
- **Argumento fora do domínio:** o `asin` só aceita valores entre −1 e +1 e, fora desse intervalo, também devolve NaN.
- **Estouro na soma dos quadrados:** a soma `(acc_x*acc_x)+(acc_y*acc_y)+(acc_z*acc_z)` era feita em `long` com sinal. Cada quadrado cabe (no máximo 32768² = 1 073 741 824), mas a soma dos três pode chegar a 3 221 225 472, acima de 2 147 483 647 (LONG_MAX), quando o módulo combinado passa de cerca de 46 341 LSB (≈ 11,31 g). O resultado é comportamento indefinido em C++ e, na prática, um denominador sem sentido: normalmente o valor dá a volta e fica negativo, e o `sqrt` dá NaN. Isso só ocorre com o sensor saturado; é improvável na bancada, mas representável pelos dados.

O `YMFC-AL_Flight_controller.ino` já tinha essa proteção (linhas 208 e 211, `if(abs(acc_y) < acc_total_vector)`, com o comentário "Prevent the asin function to produce a NaN"), mas o esc_calibrate não tinha.

Em linguagem simples: depois que um NaN gerado no cálculo do acelerômetro (por qualquer um dos três problemas acima) entra na estimativa do ângulo, ele não sai mais, porque qualquer conta com NaN dá NaN (NaN · 0.9996 + ... continua NaN). Além disso, a transferência de yaw (linhas 284–285) leva o NaN de um eixo para o outro. A estimativa só volta ao normal com um novo comando serial, que zera `cal_int` e `first_angle`, e a recalibração (AO.1) zera os ângulos. Até lá, o monitor serial mostra "nan" em Pitch e Roll.

### 2. Conteúdo das aulas relacionado

- Aula 0x0002 (Revisão de Linguagem C), p. 3: tipos `int` e `float` e seus limites; o slide indica `int` com 16 bits "em plataformas mais antigas" e remete aos limites descritos em `limits.h` (de onde vêm `LONG_MAX` e `ULONG_MAX`).
- Aula 0x0002, p. 4: tipo `bool`, com os valores `true` e `false`, usado nas duas variáveis de validação (`boolean` no Arduino).
- Aula 0x0002, p. 5: modificadores de `int`; `long` "garante um tamanho de 32 bits", `unsigned` faz o tipo representar "apenas números não negativos" e, entre as demais possibilidades, aparece `unsigned long int (uint32_t)`, o tipo usado na soma dos quadrados.
- Aula 0x0002, pp. 8–9: condicionais `if`/`else`, usadas para calcular e aplicar cada eixo somente quando ele é válido.
- Aula 0x0002, p. 19: operadores matemáticos e lógicos (`<`, `&&`) usados na condição.
- Sistemas Embarcados (EESC, Ago/2026), p. 23: sensores (apenas fotos, sem conteúdo específico sobre acelerômetro).

Conhecimento externo às aulas:

- `math.h` do avr-libc: `asin` tem erro de domínio para argumentos fora de [−1, +1] e `sqrt` calcula a raiz quadrada.
- MPU-6050 configurado em ±8 g, ou seja, 4096 LSB/g (registrador ACCEL_CONFIG com `0x10`, linhas 481–482 do esc_calibrate).
- avr-gcc: `double` é igual a `float` de 32 bits, `int` tem 16 bits e `long` tem 32 bits; `unsigned long` também tem 32 bits, de 0 a 4 294 967 295.
- A macro `abs` do `Arduino.h`.
- Propagação de NaN em ponto flutuante: uma operação com NaN dá NaN.
- Estouro de inteiro com sinal: em C++, é comportamento indefinido (o padrão não garante o resultado).
- Formato `float` de 32 bits (IEEE 754): inteiros até 2^24 = 16 777 216 são representados exatamente.

### 3. Alteração realizada

Antes (soma dos quadrados em `long`; cálculo e filtro aplicados sempre):

```cpp
acc_total_vector[0] = sqrt((acc_x*acc_x)+(acc_y*acc_y)+(acc_z*acc_z));

angle_pitch_acc = asin((float)acc_y/acc_total_vector[0])* 57.296;
angle_roll_acc = asin((float)acc_x/acc_total_vector[0])* -57.296;

if(!first_angle){
  angle_pitch = angle_pitch_acc;
  angle_roll = angle_roll_acc;
  first_angle = true;
}
else{
  angle_pitch = angle_pitch * 0.9996 + angle_pitch_acc * 0.0004;
  angle_roll = angle_roll * 0.9996 + angle_roll_acc * 0.0004;
}
```

Depois (soma dos quadrados em `unsigned long`; validação por eixo, com a mesma condição do Flight controller):

```cpp
acc_total_vector[0] = sqrt((unsigned long)(acc_x*acc_x)+(unsigned long)(acc_y*acc_y)+(unsigned long)(acc_z*acc_z));

boolean pitch_acc_valid = abs(acc_y) < acc_total_vector[0];
boolean roll_acc_valid = abs(acc_x) < acc_total_vector[0];

if(pitch_acc_valid)angle_pitch_acc = asin((float)acc_y/acc_total_vector[0])* 57.296;
if(roll_acc_valid)angle_roll_acc = asin((float)acc_x/acc_total_vector[0])* -57.296;

if(!first_angle){
  if(pitch_acc_valid && roll_acc_valid){
    angle_pitch = angle_pitch_acc;
    angle_roll = angle_roll_acc;
    first_angle = true;
  }
}
else{
  if(pitch_acc_valid)angle_pitch = angle_pitch * 0.9996 + angle_pitch_acc * 0.0004;
  if(roll_acc_valid)angle_roll = angle_roll * 0.9996 + angle_roll_acc * 0.0004;
}
```

As duas variáveis `boolean` são locais, dentro do ramo `else` do comando `'a'`. Nenhuma variável global, função, constante ou biblioteca foi criada, e nenhum tipo de 64 bits foi usado. A equação do módulo é a mesma; só o tipo da soma passou para `unsigned long`. As equações dos ângulos, os coeficientes (`57.296`, `0.9996`, `0.0004`), a integração do giroscópio e a transferência de yaw (linhas 280–285), a recalibração da AO.1 (linhas 236–244), as impressões e os pulsos dos ESCs não mudaram, e os comentários originais foram mantidos. `YMFC-AL_Flight_controller.ino` e `YMFC-AL_setup.ino` não foram alterados.

### 4. Justificativa

- Uma única condição por eixo cobre os dois primeiros problemas: se `|acc| < total`, o total é necessariamente maior que zero (não há divisão por zero) e o argumento do `asin` fica entre −1 e +1, sem incluir os extremos.
- O domínio matemático do `asin` é [−1, +1] e inclui os extremos (asin(±1) = ±90°). A condição adotada, `|acc| < total`, é mais restritiva (exclui ±1) e é a mesma usada pelo autor no Flight controller (linhas 208 e 211), sem ampliar o limite.
- Com o truncamento do módulo, o argumento ±1 também surge em vetores cujo ângulo real é menor que 90°. Por exemplo, (1, 1, 0) dá total 1, ou seja ±90°, com ângulo real de 45°. A condição estrita descarta esses casos e também medições legítimas perto de ±90°; não se afirma ganho de precisão.
- A validação é feita por eixo: um eixo inválido não impede a correção do outro. A inicialização (`first_angle`) só acontece quando os dois ângulos do acelerômetro são válidos, para que a estimativa não comece com um ângulo do acelerômetro inválido.
- O terceiro problema é tratado no tipo da soma: `unsigned long` (32 bits, até 4 294 967 295) é suficiente para a soma máxima de 3 221 225 472, sem tipos de 64 bits. Os quadrados nunca são negativos, e o cast em cada termo deixa a conversão explícita: cada multiplicação continua em `long` (onde cada quadrado cabe) e só depois é convertida, antes da soma. Uma validação feita depois do `sqrt` não bastaria, porque o estouro acontece antes dele.
- Para S ≤ 2 147 483 647, o resultado é idêntico ao original: um valor não negativo é o mesmo número em `long` e em `unsigned long` e, portanto, é convertido para o mesmo `float`. No código gerado (avr-objdump), `__floatunsisf` só zera o indicador de sinal e salta para o mesmo trecho que `__floatsisf` usa quando o valor não é negativo.
- As equações e os coeficientes do filtro foram preservados; no eixo inválido, a correção daquele ciclo é pulada. A validação apenas evita usar um ângulo do acelerômetro que não pode ser calculado.

### 5. Resultados de compilação

Em 27/11/2026, com Arduino CLI 1.6.0, Arduino AVR Boards 1.8.8 e FQBN `arduino:avr:uno`, os três sketches foram compilados com `--clean --warnings all` a partir de cópias em pastas separadas: a referência ("antes") foi exportada da develop (`786df67`) e a versão final ("depois", já com a soma em `unsigned long`) foi copiada da cópia de trabalho.

| Sketch | Flash antes | Flash depois | SRAM global antes | SRAM global depois |
|---|---:|---:|---:|---:|
| `YMFC-AL_Flight_controller.ino` | 12404 bytes | 12404 bytes | 501 bytes | 501 bytes |
| `YMFC-AL_esc_calibrate.ino` | 13126 bytes | 13444 bytes | 1106 bytes | 1114 bytes |
| `YMFC-AL_setup.ino` | 20680 bytes | 20680 bytes | 532 bytes | 532 bytes |

O esc_calibrate passou a ocupar 318 bytes a mais de Flash e 8 bytes a mais de SRAM global (sobram 934 bytes para variáveis locais, contra 942 antes). Pela tabela de símbolos (`avr-nm`), todo o aumento de Flash está em `main` (onde o compilador expande o `loop()`: 4252 bytes antes e 4570 depois), e nenhuma função de biblioteca nova foi ligada; a distribuição desses bytes no código gerado não foi analisada. A soma em `unsigned long` não alterou nenhum tamanho: uma compilação intermediária, só com a validação e ainda com a soma em `long`, deu os mesmos 13444 bytes de Flash e 1114 bytes de SRAM, com a mesma tabela de símbolos. Comparando os dois códigos com o `avr-objdump`, só uma instrução mudou: a chamada de conversão para `float` antes do `sqrt` do comando `'a'`, que passou de `__floatsisf` para `__floatunsisf`. As duas rotinas já estavam ligadas na develop, no mesmo bloco de código. A conversão do modo de vibração (linha 207) continua chamando `__floatsisf`. Também pelo `avr-nm`, os 8 bytes de SRAM são as variáveis globais já existentes `angle_pitch_acc` e `angle_roll_acc` (4 bytes cada). Na versão original, o compilador as eliminava, porque eram sempre escritas antes de serem lidas; agora, com a escrita condicional, elas passam a ocupar memória. As duas variáveis `boolean` novas são locais e não entram na SRAM global.

Com `--warnings all`, os avisos foram os mesmos antes e depois: 4 de `new.cpp` do core em cada sketch e, no setup, 1 já existente em `wait_for_receiver()` (`'now' may be used uninitialized`). Nenhum aviso novo, nem nas linhas alteradas.

### 6. Verificações realizadas

Os casos abaixo foram simulados por cálculo (4096 LSB/g, `long` de 32 bits com volta no original, soma em `unsigned long` na versão final, `float` de 32 bits e truncamento do `sqrt` na atribuição a `long`), e não executados na placa. Os valores foram recalculados com um script Python que simula o `float` de 32 bits (numpy `float32`). O "total" é o valor guardado em `acc_total_vector[0]`.

Cenários físicos (sensor lido normalmente, cerca de 1 g):

| Entrada (x, y, z) | Original | Após | Ângulos |
|---|---|---|---|
| (0, 0, 4096), nivelado | total 4096; pitch 0°, roll 0° | igual | atualizados |
| (0, 2048, 3547) | total 4095; pitch 30,008° | igual | atualizados |
| (0, −2048, 3547) | total 4095; pitch −30,008° | igual | atualizados |
| (2048, 0, 3547) | total 4095; roll −30,008° (sinal negativo pelo `-57.296`) | igual | atualizados |
| (0, 0, −4096), invertido | total 4096; argumentos 0, válidos | igual | atualizados |
| (0, 4096, 0), argumento exatamente +1 | pitch 90° | pitch rejeitado (4096 < 4096 é falso); roll válido | pitch preservado, roll atualizado; a inicialização espera |
| (0, −4096, 0), argumento exatamente −1 | pitch −90° | pitch rejeitado | pitch preservado |
| (0, 4096, 50) | total 4096 por truncamento → 90° (o valor real seria 89,30°) | pitch rejeitado | pitch preservado |
| (0, 4096, 91) | total 4097 → 88,734° | válido | atualizado |

Extremos representáveis e falhas (os valores perto de ±32768 exigem o sensor saturado em ±8 g, o que só é possível sob choque):

| Entrada (x, y, z) | S | Original | Após (total; pitch; roll) |
|---|---:|---|---|
| (0, 0, 0), componentes nulos | 0 | total 0 → 0/0 → NaN nos dois eixos | 0; os dois eixos rejeitados (0 < 0 é falso), sem divisão e sem `asin`; ângulos preservados (só o giroscópio) |
| (1, 1, 0) e (1, 1, 1), vetor minúsculo | 2 e 3 | total 1 → pitch +90° e roll −90° espúrios | 1; os dois eixos rejeitados; ângulos preservados |
| (32768, 32768, 32768) | 3 221 225 472 | estouro (indefinido; normalmente −1 073 741 824 → `sqrt` NaN) | 56755; 35,265°; −35,265°; atualizados |
| (−32768, −32768, −32768), só em eixos não invertidos | 3 221 225 472 | estouro | 56755; −35,265°; +35,265° |
| (32767, 32767, 32767) | 3 221 028 867 | estouro | 56754; 35,265°; −35,265° |
| (26755, 26755, 26755), primeiro trio igual que estoura | 2 147 490 075 | estouro | 46341; 35,265°; −35,265° |
| (26754, 26754, 26754) | 2 147 329 548 | 46339; ±35,265° | idêntico |
| (32767, 32767, 363) | 2 147 484 347 | estouro | 46340; 45,000°; −45,000° |
| (32767, 32767, 362) | 2 147 483 622 | 46340; ±45,000° | idêntico |
| (32768, 32768, 0) | 2 147 483 648 = 2^31 | estouro | 46340; 45,001°; −45,001° |
| (−32768, 32768, 0) | 2^31 | estouro | 46340; pitch 45,001°; roll +45,001° |
| (32768, 0, 0) | 2^30 | total 32768; roll com argumento +1 (−90°, pelo `-57.296`) | igual; roll rejeitado, pitch 0° atualizado |
| (0, 32768, 0) | 2^30 | total 32768; pitch +90° | pitch rejeitado, roll 0° atualizado |
| (0, 32768, 256) | 1 073 807 360 | total 32769 (sqrt exato 32768,99998, arredondado em `float` para 32769,0); pitch 89,553° | idêntico; aceito (o limite depende do arredondamento) |
| Argumento < −1 ou > +1 | — | NaN | não ocorre para nenhum valor representável (\|acc\| ≤ 32768), porque floor(sqrt(float(S))) ≥ \|acc\|; se ocorresse, a condição estrita rejeitaria; ângulos preservados |

S é a soma dos três quadrados, antes do `sqrt`. Um eixo só vale +32768 quando é invertido (`*= -1` aplicado a −32768), e −32768 só aparece em eixo não invertido. A desigualdade floor(sqrt(float(S))) ≥ |acc| foi conferida por cálculo para todos os |acc| de 0 a 32768 com os outros dois eixos em zero, que é o pior caso: a conversão para `float` e o `sqrt` não diminuem quando S aumenta.

Sequências de ciclos:

1. **Inválido na primeira estimativa e válido depois.** Original: o NaN entra em `angle_pitch`/`angle_roll`, `first_angle` vira `true`, o NaN fica permanente e passa ao outro eixo. Após: `first_angle` continua `false` até um ciclo com os dois eixos válidos, e só então a estimativa é iniciada.
2. **Inválido depois de uma estimativa válida.** Original: o NaN fica permanente. Após: a correção daquele eixo é pulada no ciclo e a estimativa mantém a integração do giroscópio.
3. **Restabelecimento.** Original: só com um novo comando serial. Após: no próximo ciclo válido, o filtro volta a corrigir o eixo.
4. **AO.1.** As linhas 236–244 ficaram intactas. Depois do reset, `first_angle = false`, e a nova regra só controla em que ciclo a inicialização acontece.

A análise de NaN se limita ao trecho do acelerômetro alterado; outras partes do firmware não foram analisadas.

Tipos e estouro:

- A faixa de cada eixo vai de −32768 a +32768: `acc_axis` é `int` (−32768 a 32767), e o `*= -1` é feito em `long` (linhas 533–538), então −32768 invertido vira +32768.
- Cada quadrado é no máximo 32768² = 1 073 741 824 e cabe em `long`.
- A soma dos três é no máximo 3 221 225 472: acima de LONG_MAX (2 147 483 647), mas abaixo de ULONG_MAX (4 294 967 295).
- No original, o estouro começava quando o módulo combinado passava de cerca de 46 341 LSB (≈ 11,31 g). Com a soma em `unsigned long`, não há estouro em toda a faixa.
- O total é no máximo 56 755.
- Inteiros até 2^24 = 16 777 216 são exatos em `float`. Como o numerador (|acc| ≤ 32768) e o denominador (total ≤ 56 755) ficam abaixo disso, os dois chegam exatos à divisão. Um S acima de 2^24 é arredondado na conversão para `float`, antes do `sqrt`, como já acontecia no original.
- Em repouso, S ≈ 16,8 milhões (4096² = 16 777 216), cerca de 0,8 % de LONG_MAX.

### 7. Limitações

- Não houve teste em hardware nem medição de tempo. As duas comparações e os desvios acrescentados têm custo pequeno, que não foi medido.
- Uma medição é rejeitada quando o módulo truncado fica igual a |acc|. Sem considerar o arredondamento do `float`, isso ocorre quando a soma dos quadrados dos outros dois eixos é ≤ 2·|acc|. O ângulo correspondente não é fixo: ≈ 35,26° para |acc| = 1 (vetor (1, 1, 1)), ≈ 88,73° para |acc| = 4096 (1 g) e ≈ 89,55° para |acc| = 32768. O arredondamento do `float` pode deslocar esse limite; por exemplo, (0, 32768, 256) é aceito com 89,553°.
- Se um eixo ficar permanentemente perto de 90°, a inicialização espera.
- A análise supõe que o `sqrt` do avr-libc é corretamente arredondado, o que o header não documenta.
- Os gyros tipo 2 ou 3 continuam sem leitura no esc_calibrate: `gyro_signalen()` só lê o MPU-6050, então o acelerômetro e o giroscópio ficam em zero. Agora os dois eixos são rejeitados e o trecho do acelerômetro deixa de gerar NaN, mas os ângulos continuam sem medição real.
- O Flight controller continua reaproveitando o `angle_*_acc` antigo quando um eixo é inválido (fora do escopo desta melhoria).
- O mesmo cálculo com soma em `long` continua em `YMFC-AL_Flight_controller.ino` (linha 206) e no modo de vibração do esc_calibrate (linha 207, comandos `'1'` a `'5'`). Esses trechos ficaram fora do escopo da AO.3, que trata dos ângulos do comando `'a'`.
- Com o sensor saturado, o ângulo agora é calculado e usado pelo filtro. Sob choque, porém, o acelerômetro não mede só a gravidade, e esse ângulo não representa a orientação.

---

## Melhoria AP.3 — Proteção de variáveis compartilhadas

**Responsável:** AP (identificação completa a confirmar)

**Branch:** `fix/atomic-receiver-read`

**Arquivos modificados:** `YMFC-AL_Flight_controller.ino` e `README.md`.

Os números de linha citados são os do código original (develop `7f82cc8`), exceto onde se indica "na versão modificada".

### 1. Problema identificado

No `YMFC-AL_Flight_controller.ino`, a rotina de interrupção `ISR(PCINT0_vect)` mede a largura dos pulsos do receptor e grava o resultado em `receiver_input[1..4]`, que o programa principal lê em `convert_receiver_channel()`.

| Variável | Tipo e tamanho | Quem escreve | Quem lê | Compartilhada com a ISR? |
|---|---|---|---|---|
| `receiver_input[1..4]` | `int`, 2 bytes cada (vetor de 5, 10 bytes) | ISR (linhas 380, 391, 402 e 414) | `convert_receiver_channel()` (linha 547) | Sim |
| `receiver_input_channel_1..4` | `volatile int`, 2 bytes | programa principal (linhas 152–153 e 429–432) | `setup()` e `loop()` | Não |
| `timer_1..4`, `current_time` | `unsigned long`, 4 bytes | ISR | ISR | Não |
| `last_channel_1..4` | `byte`, 1 byte | ISR | ISR | Não |

A cópia `actual = receiver_input[channel];` (linha 547) lê 16 bits, mas o ATmega328P carrega só 8 bits por instrução. No avr-objdump do original, ela vira dois loads: `ld r0, X+` (byte baixo) e depois `ld r27, X` (byte alto). Se o pulso daquele canal terminar entre essas duas instruções, a ISR roda no meio da leitura, e o programa fica com o byte baixo do valor antigo e o byte alto do novo. As larguras medidas são múltiplas de 4 µs (resolução do `micros()` no Uno a 16 MHz). Exemplo, com o byte baixo lido primeiro, como no objdump:

| Transição | Byte baixo lido (antigo) | Byte alto lido (novo) | Valor copiado |
|---|---|---|---|
| 1276 (0x04FC) → 1280 (0x0500) | 0xFC | 0x05 | 0x05FC = 1532 |

Na transição inversa (1280 → 1276), o valor copiado seria 0x0400 = 1024. Nos dois casos, o valor copiado não é o antigo nem o novo. O erro só aparece quando o byte alto muda, ou seja, quando a largura cruza 1024, 1280, 1536 ou 1792 µs, e a ISR cai exatamente naquela janela; é raro, mas possível. O valor errado seria convertido e viraria o comando do canal por um ciclo de 4 ms; em tese, um throttle ou yaw errado poderia satisfazer uma das condições de armamento ou desarmamento (linhas 232–245). Isso não foi observado; é uma possibilidade da análise.

Os `receiver_input_channel_1..4` não são compartilhados, porque a ISR não mexe neles: eles recebem o retorno de `convert_receiver_channel()` em `gyro_signalen()` ou no laço do `setup()` e são lidos no `loop()`, tudo no programa principal, que não interrompe a si mesmo. Por isso a AP.2 pôde passá-los por valor sem proteção e deixou a atomicidade para esta melhoria. (No `YMFC-AL_setup.ino`, ao contrário, a ISR grava diretamente os `receiver_input_channel_*`, e a E.1 os protegeu lá.)

### 2. Conteúdo das aulas relacionado

- Semáforos, Parte 1, pp. 2–4: comunicação interprocessos; vantagens de múltiplos processos ("Concorrência", "Características assíncronas", "Orientado a Dados / Eventos") e formas de comunicar, entre elas "Memória compartilhada (superposição de contextos)" e "Instruções de sincronização (suporte de hardware)". Aqui, os dois contextos são o `loop()` e a ISR, e a memória compartilhada é `receiver_input[]`.
- Semáforos, Parte 2, pp. 5–6 ("Simultaneous Transactions" e "But, what if…"): duas execuções intercaladas no nível das instruções (`LD`, `SUB`, `ST`) corrompem um dado compartilhado; os trechos chamados "CRITICAL SECTIONS" não podem se sobrepor, restrição chamada "MUTUAL EXCLUSION", e a solução é garantir sua "atomicity", isto é, fazê-los parecer "single, instantaneous operations". É o mesmo problema dos dois `ld`.
- Semáforos, Parte 2, pp. 7–8: "Semaphores for Mutual Exclusion" e "Producer/Consumer Atomicity Problems". A "Granularity of lock" (p. 7) corresponde aqui à escolha de proteger um canal por vez, e o semáforo com `wait`/`signal` (Parte 1, pp. 8–9) fica só como contraste.
- Internet e Sockets, pp. 45–47 (analogia: ali o contexto são threads e sistema operacional): threads compartilham "Variáveis em memória", com "problemas de concorrência" (p. 45); exemplos são "variáveis e arrays globais", e a solução é "Proteger o acesso concorrente" (p. 46); o mutex isola "regiões críticas (trechos do código que utilizam recursos compartilhados)" (p. 47).
- Aula 0x0002 (Revisão de Linguagem C), p. 3: `int` com "16 bits (2 bytes) em plataformas mais antigas", caso do AVR.

Conhecimento externo às aulas:

- `volatile`: obriga o compilador a ler e escrever a variável na memória a cada acesso.
- `ISR(PCINT0_vect)`: na entrada da interrupção, o hardware zera o bit I do SREG, e a ISR não é interrompida pelo programa principal.
- SREG, `cli` e `util/atomic.h` (avr-libc): o bit I do registrador de estado (SREG, I/O 0x3F) habilita as interrupções e `cli` o zera; `ATOMIC_BLOCK(ATOMIC_RESTORESTATE)` salva o SREG, executa `cli` e restaura o SREG na saída do bloco.
- AVR de 8 bits: `ld` carrega um byte por instrução, e a CPU atende interrupções ao fim de qualquer instrução.
- `micros()` com resolução de 4 µs no Uno a 16 MHz (Timer0 com prescaler 64).

### 3. Alteração realizada

Três alterações em `YMFC-AL_Flight_controller.ino`. As duas primeiras são um novo include depois de `EEPROM.h` (linha 20 na versão modificada) e o `volatile` na declaração da linha 71, que antes era `int receiver_input[5];` (linha 72 na versão modificada):

```cpp
#include <util/atomic.h>                   //Inclui a util/atomic.h para usar ATOMIC_BLOCK (secao critica na leitura do receptor)
volatile int receiver_input[5]; // Escrito pela ISR(PCINT0_vect) e lido em convert_receiver_channel()
```

A terceira, em `convert_receiver_channel()`, coloca a cópia da linha 547 (a linha do meio é a linha 547 original, com dois espaços a mais de recuo) dentro de uma seção crítica (linhas 548–550 na versão modificada):

```cpp
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE){                                           //Secao critica: a ISR(PCINT0_vect) nao interrompe a leitura dos 2 bytes
    actual = receiver_input[channel];                                          //Read the actual receiver value for the corresponding function
  }                                                                            //Fim da secao critica: o SREG (bit I) volta ao estado anterior
```

Dentro do bloco fica só a cópia, com o comentário original. Não mudaram: a ISR, a escolha de `channel` e `reverse`, a leitura da EEPROM e a conversão para 1000–2000 µs, as condições de armamento, a zona morta da AP.2 (`apply_dead_band()`), os pulsos dos ESCs, os ganhos, o loop de 4 ms e os outros dois sketches.

### 4. Justificativa

- `volatile` não garante atomicidade: só obriga o compilador a ir à memória a cada acesso. Um `volatile int` continua sendo lido em duas instruções; no objdump, os dois `ld` são iguais no original (sem `volatile`) e no modificado. O `volatile` foi acrescentado porque a variável é escrita por uma ISR; a proteção vem da seção crítica.
- Seção crítica: como a CPU atende interrupções ao fim de qualquer instrução, a cópia dos dois bytes só é indivisível com as interrupções desabilitadas, e o `ATOMIC_BLOCK` executa `cli` antes dela. Um PCINT0 que chegue nesse intervalo não se perde: fica pendente na flag PCIF0, e a ISR roda logo depois que o SREG é restaurado. O valor novo entra na próxima conversão.
- `ATOMIC_RESTORESTATE` salva e restaura o SREG, e o bit I volta ao estado anterior. `interrupts()` ou `ATOMIC_FORCEON` religariam as interrupções sempre, mesmo se a função fosse chamada com elas desligadas.
- A seção crítica deve ser curta, porque com I = 0 nenhuma ISR é atendida (PCINT0, o estouro do Timer0 de `millis()`/`micros()`, o TWI do `Wire`). Por isso o bloco não tem I²C, serial, `micros()`, contas nem chamadas de função.
- Consistência não é simultaneidade: cada valor copiado é um valor que a ISR realmente gravou. Isso não garante que os canais foram recebidos no mesmo instante nem que pertencem ao mesmo quadro do receptor.
- A seção crítica protege a leitura individual de cada canal: ela fica dentro de `convert_receiver_channel()`, e os quatro canais não são copiados em uma única seção crítica, porque uma única alteração cobre os dois pontos de chamada (`gyro_signalen()` e o laço do `setup()`), sem mudar a assinatura nem a frequência das chamadas.
- Reutiliza o padrão da E.1 (Melhoria 1 deste README), que usa `ATOMIC_BLOCK(ATOMIC_RESTORESTATE)` no `YMFC-AL_setup.ino` (linha 742).
- Em bare metal, sem sistema operacional, aplica-se o conceito de seção crítica e de exclusão mútua entre dois contextos, o `loop()` e a ISR. Não se implementa um semáforo: não há `wait`/`signal`, fila nem bloqueio.

### 5. Resultados de compilação

Em 27/11/2026, com Arduino CLI 1.6.0, Arduino AVR Boards 1.8.8 e FQBN `arduino:avr:uno`, os três sketches foram compilados com `--clean --warnings all` a partir de cópias em pastas separadas: a referência ("antes") foi exportada da develop (`7f82cc8`) e a versão "depois" foi copiada da cópia de trabalho.

| Sketch | Flash antes | Flash depois | SRAM global antes | SRAM global depois |
|---|---:|---:|---:|---:|
| `YMFC-AL_Flight_controller.ino` | 12404 bytes | 12410 bytes | 501 bytes | 501 bytes |
| `YMFC-AL_esc_calibrate.ino` | 13444 bytes | 13444 bytes | 1114 bytes | 1114 bytes |
| `YMFC-AL_setup.ino` | 20680 bytes | 20680 bytes | 532 bytes | 532 bytes |

Os 6 bytes a mais do Flight são as três instruções do bloco (`in`, `cli`, `out`, 2 bytes cada). Os avisos já existentes (do core e um do setup) foram os mesmos antes e depois; nenhum aviso novo.

Trecho do avr-objdump do bloco, na versão modificada (as instruções omitidas calculam o endereço do elemento; não há `call` no bloco):

```
8c0:  in   r24, 0x3f   ; salva o SREG
8c2:  cli              ; desabilita as interrupcoes
8d0:  ld   r0, X+      ; byte baixo
8d2:  ld   r27, X      ; byte alto
8d6:  out  0x3f, r24   ; restaura o SREG (bit I)
```

### 6. Verificações realizadas

Por inspeção do código, análise do objdump e compilação; sem hardware.

| # | Verificação | Resultado |
|---|---|---|
| 1 | Leitura normal | sem interrupção durante a cópia, `actual` recebe o mesmo valor do original |
| 2 | Interrupção entre os bytes, no original | possível: nada impedia a ISR entre `ld r0, X+` e `ld r27, X` (exemplo da seção 1) |
| 3 | Cópia dentro da seção crítica | os dois `ld` ficam entre `cli` e `out 0x3f`, sem `call`; a ISR não roda entre eles |
| 4 | Atualização depois da cópia | a borda que chega com I = 0 fica pendente (PCIF0); a ISR roda após o `out`, e o valor novo entra na próxima chamada |
| 5 | Interrupções e temporização | estimada, não medida (abaixo) |

Tempos estimados, não medidos, a partir da contagem das instruções no objdump e dos ciclos do AVR Instruction Set Manual, a 16 MHz: o bit I fica em 0 por cerca de 0,75 µs por chamada, ou seja, cerca de 3 µs a cada ciclo de 4 ms (4 chamadas em `gyro_signalen()`). Nesse intervalo o atendimento de uma ISR pode atrasar, o que pode deslocar a medida de um pulso em um passo de 4 µs; por isso não se afirma que a temporização é idêntica à do original. Além disso, as mudanças dos pinos do receptor ficam registradas numa única flag (PCIF0): se mais de uma mudança ocorrer enquanto as interrupções estão desabilitadas, a ISR roda uma vez só e verifica, pelo `PINB`, o estado atual de cada pino (8 a 11): mudanças em pinos diferentes são todas tratadas, mas com o mesmo instante de `micros()`, sem registro individual de cada mudança; duas mudanças do mesmo pino nesse intervalo não seriam vistas, o que exigiria um pulso muito mais curto que os do receptor (o mesmo já pode ocorrer no original enquanto outras ISRs executam).

### 7. Limitações

- Não houve teste em hardware nem medição de tempo; a seção 6 traz só uma estimativa.
- O `YMFC-AL_esc_calibrate.ino` tem o mesmo padrão sem proteção (declaração na linha 44, leituras nas linhas 382–385 em `wait_for_receiver()` e na linha 401 em `convert_receiver_channel()`); ficou fora do escopo.
- Um índice de canal 0 ou de 5 a 7 vindo da EEPROM continua sem tratamento (o 0 lê `receiver_input[0]`, que nunca é escrito, e 5 a 7 leem fora do vetor); é um problema anterior.
- A AP.3 não implementa um failsafe; o diagnóstico da perda de sinal do receptor fica para a E.3.
- O `volatile` dos `receiver_input_channel_*` (linha 47) foi mantido, embora não seja necessário.
