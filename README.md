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
