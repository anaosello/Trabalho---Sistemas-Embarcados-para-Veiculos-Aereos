/*
  receiver_diagnostics.ino - Melhoria E.3: diagnostico de perda do sinal do receptor

  Finalidade: classificar cada canal do receptor como valido, ausente,
  desatualizado ou com pulso invalido, com os mesmos criterios da E.1
  (wait_for_receiver() do YMFC-AL_setup.ino), e mostrar quando um canal
  volta a ser valido (restabelecido).

  - Sketch independente do controlador de voo: e gravado sozinho na placa.
  - Usa so dados simulados (amostras fixas); nao le o receptor real.
  - Nao e um failsafe: nao muda nada no voo.
  - Nao aciona motores: nao configura pinos nem gera pulsos de saida.

  Como rodar: gravar no Arduino Uno, abrir o Monitor Serial a 57600 bps
  e pressionar reset para repetir os cenarios.
*/

// Uma struct por AMOSTRA (uma leitura dos quatro canais), com os campos
// iguais as variaveis locais da E.1. Nao se usa uma struct por canal
// porque assim ficam preservados a mascara de bits (received_mask) e os
// operadores binarios da E.1, como (received_mask & (1 << i)).
struct ReceiverSample {
  int channels[4];               // = channels[4] da E.1
  unsigned long pulse_times[4];  // = pulse_times[4] da E.1
  byte received_mask;            // = received_mask da E.1, bit i = canal i+1
  unsigned long now;             // = now da E.1, aqui um valor fixo simulado
};

// Mesmo nome, tipo e valor de RECEIVER_TIMEOUT_US no YMFC-AL_setup.ino (E.1): 100 ms
const unsigned long RECEIVER_TIMEOUT_US = 100000UL;

// Estados possiveis de um canal
const byte CHANNEL_VALID = 0, CHANNEL_ABSENT = 1, CHANNEL_STALE = 2, CHANNEL_INVALID = 3;

// Amostras simuladas. Os instantes nunca sao futuros em relacao a now
// (perto do rollover, o instante e anterior a volta a zero do contador).
const ReceiverSample all_valid = {{1500, 1500, 1000, 1500},
  {990000UL, 985000UL, 995000UL, 980000UL}, B00001111, 1000000UL};

const ReceiverSample one_absent = {{1500, 0, 1000, 1500},
  {990000UL, 0UL, 995000UL, 980000UL}, B00001101, 1000000UL};

const ReceiverSample two_absent = {{1500, 1500, 0, 0},
  {990000UL, 985000UL, 0UL, 0UL}, B00000011, 1000000UL};

const ReceiverSample low_pulse = {{850, 1500, 1000, 1500},
  {990000UL, 985000UL, 995000UL, 980000UL}, B00001111, 1000000UL};

const ReceiverSample restored = {{1500, 1500, 1000, 1500},
  {1010000UL, 1005000UL, 1015000UL, 1000000UL}, B00001111, 1020000UL};

const ReceiverSample high_pulse = {{1500, 1500, 1000, 2200},
  {990000UL, 985000UL, 995000UL, 980000UL}, B00001111, 1000000UL};

const ReceiverSample old_data = {{1500, 1500, 1000, 1500},
  {1000000UL, 995000UL, 1005000UL, 990000UL}, B00001111, 1200000UL};

const ReceiverSample pulse_limits = {{900, 901, 2099, 2100},
  {990000UL, 985000UL, 995000UL, 980000UL}, B00001111, 1000000UL};

const ReceiverSample timeout_limit = {{1500, 1500, 1000, 1500},
  {900001UL, 900000UL, 899999UL, 1000000UL}, B00001111, 1000000UL};

const ReceiverSample rollover = {{1500, 1500, 1000, 1500},
  {0xFFFF3CB1UL, 0xFFFF3CB0UL, 0xFFFFFFFFUL, 40000UL}, B00001111, 50000UL};

const ReceiverSample multiple_faults = {{0, 2500, 500, 1500},
  {0UL, 800000UL, 995000UL, 850000UL}, B00001110, 1000000UL};

// Classifica um canal com as tres condicoes da E.1 (mesmas expressoes)
byte classify_channel(int pulse, unsigned long pulse_time, unsigned long now, byte received_mask, byte i){
  bool valid_width =
      pulse > 900 &&
      pulse < 2100;
  bool pulse_received =
      (received_mask & (1 << i)) != 0;
  // Subtracao sem sinal: continua correta depois do rollover (H.2)
  bool recent_pulse =
      (now - pulse_time) < RECEIVER_TIMEOUT_US;

  // Prioridade igual a do diagnostico final de wait_for_receiver() (E.1):
  // sem presenca, o pulso e o instante sao so valores iniciais;
  if(!pulse_received){
    return CHANNEL_ABSENT;
  }
  // um pulso antigo nao representa a medida atual;
  else if(!recent_pulse){
    return CHANNEL_STALE;
  }
  // so um pulso presente e recente tem a largura julgada.
  else if(!valid_width){
    return CHANNEL_INVALID;
  }
  return CHANNEL_VALID;
}

// Imprime o nome do estado
void print_state(byte state){
  switch(state){
    case CHANNEL_VALID:
      Serial.print(F("VALIDO"));
      break;
    case CHANNEL_ABSENT:
      Serial.print(F("AUSENTE"));
      break;
    case CHANNEL_STALE:
      Serial.print(F("DESATUALIZADO"));
      break;
    case CHANNEL_INVALID:
      Serial.print(F("PULSO INVALIDO"));
      break;
  }
}

// Imprime os bits 3..0 da mascara (canal 4 a esquerda, canal 1 a direita)
void print_mask(byte mask){
  // Contador int: com byte, bit-- em 0 viraria 255 e o laco nunca terminaria
  for(int bit = 3; bit >= 0; bit--){
    Serial.print((mask >> bit) & 1);
  }
}

// Diagnostica uma amostra e devolve a mascara de canais validos.
// "Restabelecido" depende do historico (previous_valid_mask), e nao so da
// amostra atual; por isso nao e um codigo de classify_channel().
byte diagnose_sample(ReceiverSample sample, byte previous_valid_mask){
  byte valid_mask = 0;

  for(byte i = 0; i < 4; i++){
    byte state = classify_channel(sample.channels[i], sample.pulse_times[i],
                                  sample.now, sample.received_mask, i);

    Serial.print(F("  Canal "));
    Serial.print(i + 1);
    Serial.print(F(": pulso "));
    Serial.print(sample.channels[i]);
    Serial.print(F(" us, recebido "));
    if((sample.received_mask & (1 << i)) != 0){
      Serial.print(F("sim"));
    }
    else{
      Serial.print(F("nao"));
    }
    Serial.print(F(", decorrido "));
    Serial.print(sample.now - sample.pulse_times[i]);
    Serial.print(F(" us -> "));
    print_state(state);

    if(state == CHANNEL_VALID){
      valid_mask |= (1 << i);
      // Valido agora e nao valido antes (ausente, desatualizado ou invalido)
      if((previous_valid_mask & (1 << i)) == 0){
        Serial.print(F(" (RESTABELECIDO)"));
      }
    }
    Serial.println();
  }

  Serial.print(F("  Mascara de validos (bits 3..0 = canais 4..1): "));
  print_mask(valid_mask);
  Serial.println();
  Serial.print(F("  Quatro canais validos: "));
  if(valid_mask == B00001111){
    Serial.println(F("SIM"));
  }
  else{
    Serial.println(F("NAO"));
  }
  Serial.println();
  return valid_mask;
}

void setup(){
  byte valid_mask;  // encadeia o historico entre dois cenarios

  Serial.begin(57600);  // mesma taxa do Serial.begin() do YMFC-AL_setup.ino
  Serial.println(F("Diagnostico do receptor - dados simulados"));
  Serial.println(F("Criterios da E.1: 900 < pulso < 2100 us e ultimo pulso ha menos de 100000 us"));
  Serial.println(F("Prioridade: ausente > desatualizado > pulso invalido"));
  Serial.println();

  // Nos cenarios independentes, B00001111 = historico simulado:
  // os quatro validos no diagnostico anterior
  Serial.println(F("Cenario 1: quatro canais validos"));
  diagnose_sample(all_valid, B00001111);
  Serial.println(F("Cenario 2: um canal ausente (canal 2)"));
  diagnose_sample(one_absent, B00001111);
  Serial.println(F("Cenario 3: mais de um canal ausente (canais 3 e 4)"));
  diagnose_sample(two_absent, B00001111);

  // Cenarios 4 e 5 encadeados: o historico do 5 e a mascara do 4
  Serial.println(F("Cenario 4: pulso abaixo do limite (canal 1 = 850 us)"));
  valid_mask = diagnose_sample(low_pulse, B00001111);
  Serial.println(F("Cenario 5: restabelecimento do canal 1 (historico do cenario 4)"));
  diagnose_sample(restored, valid_mask);

  Serial.println(F("Cenario 6: pulso acima do limite (canal 4 = 2200 us)"));
  diagnose_sample(high_pulse, B00001111);
  Serial.println(F("Cenario 7: dados antigos (perda do sinal nos quatro canais)"));
  diagnose_sample(old_data, B00001111);
  Serial.println(F("Cenario 8: pulsos nos limites da E.1 (900, 901, 2099, 2100 us)"));
  diagnose_sample(pulse_limits, B00001111);
  Serial.println(F("Cenario 9: limite do timeout (99999, 100000, 100001 e 0 us)"));
  diagnose_sample(timeout_limit, B00001111);
  Serial.println(F("Cenario 10: comparacao perto do rollover de micros()"));
  diagnose_sample(rollover, B00001111);

  // Cenarios 11 e 12 encadeados: a mesma amostra restored do cenario 5,
  // com outro historico, mostra outros restabelecimentos
  Serial.println(F("Cenario 11: canais com mais de um problema (prioridade)"));
  valid_mask = diagnose_sample(multiple_faults, B00001111);
  Serial.println(F("Cenario 12: restabelecimento dos quatro canais (historico do cenario 11)"));
  diagnose_sample(restored, valid_mask);

  Serial.println(F("Fim dos cenarios (nenhum sinal real lido, nenhum motor acionado)."));
}

void loop(){
  // os cenarios rodam uma vez no setup(); para repetir, pressione reset
}
